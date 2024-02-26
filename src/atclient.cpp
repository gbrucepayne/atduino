#include <atclient.h>

namespace at {

static const char rx_debug_tag[] = "[DEBUG][RX] <<<";
static const char tx_debug_tag[] = "[DEBUG][TX] >>>";

/**
 * @brief Add a preamble to raw character debug or newline for other debug
 * @param raw Adds a preamble for raw character logging
*/
void AtClient::toggleRaw(bool raw) {
  #ifdef ARDUINO
  if (LOG_GET_LEVEL() > DebugLogLevel::LVL_INFO) {
    if (raw && !debug_raw) {
      PRINT(rx_debug_tag, "");
      debug_raw = true;
    } else if (!raw && debug_raw) {
      PRINTLN();
      debug_raw = false;
    }
  }
  #endif
}

bool AtClient::isRxBufferFull() {
  #if defined(__AVR__) || defined(ESP8266)
  return strlen(res_buffer_P) == rx_buffer_size;
  #else
  return strlen(res_buffer) >= rx_buffer_size - 1;
  #endif
}

void AtClient::clearRxBuffer() {
  #if defined(__AVR__) || defined(ESP8266)
  memset(res_buffer_P, 0, rx_buffer_size);
  #else
  memset(res_buffer, 0, rx_buffer_size);
  #endif
}

void AtClient::clearPendingCommand() {
  memset(pending_command, 0, AT_CLIENT_TX_BUFFERSIZE);
}

bool AtClient::setPendingCommand(const char *at_command) {
  if (strlen(at_command) > AT_CLIENT_TX_BUFFERSIZE)
    return false;   // invalid_argument("command too large");
  strcpy(pending_command, at_command);
  return true;
}

String AtClient::sgetResponse(const char* prefix) {
  String temp;
  getResponse(temp, prefix);
  return temp;
}

void AtClient::getResponse(String& response, const char* prefix) {
  cleanResponse(prefix);
  #if defined(__AVR__)
  response = String(res_buffer_P);   // extract to SRAM first?
  #else  
  response = String(res_buffer);
  #endif
  response_ready = false;
}

void AtClient::getResponse(char* response, const char* prefix,
                                  size_t buffersize) {
  cleanResponse(prefix);
  #if defined(__AVR__)
  strncpy_P(response, res_buffer_P, strlen(res_buffer_P));
  #else
  strncpy(response, res_buffer, strlen(res_buffer));
  #endif
  response_ready = false;
}

char* AtClient::responsePtr() {
  #if defined(__AVR__)
  return &res_buffer_P[0];   // may need to extract to SRAM first?
  #else  
  return &res_buffer[0];
  #endif
}

char* AtClient::commandPtr() {
  return &pending_command[0];
}

char AtClient::lastCharRead(size_t n) {
  if (n <= 0)
    return -1;
  #if defined(__AVR__) || defined(ESP8266)
  return res_buffer_P[strlen(res_buffer) - 1];
  #else
  return res_buffer[strlen(res_buffer) - n];
  #endif
}

// If any data is on the serial port read until a match of read_until
bool AtClient::checkUrc(const char* read_until, time_t timeout_ms) {
  if (strlen(pending_command) > 0 || busy || serial.available() == 0) {
    if (strlen(pending_command) > 0) LOG_TRACE("AT command pending");
    if (busy) LOG_TRACE("busy");
    // if (serial.available() == 0) LOG_TRACE("No data");
    return false;
  }
  busy = true;
  if (read_until == nullptr)
    read_until = terminator;
  LOG_DEBUG("Processing URC until", debugString(read_until));
  toggleRaw(true);
  clearRxBuffer();
  response_ready = false;
  for (time_t start = millis(); millis() - start < timeout_ms;) {
    readSerialChar();
    if (strlen(responsePtr()) > strlen(read_until) &&
        endsWith(responsePtr(), read_until)) {
      toggleRaw(false);
      LOG_TRACE("Found terminator");
      response_ready = true;
      break;
    }
  }
  toggleRaw(false);
  if (!response_ready)
    LOG_WARN("Timed out waiting for terminator");
  busy = false;
  return true;
}

bool AtClient::sendAtCommand(const char *at_command, uint16_t timeout_ms) {
  if (strlen(pending_command) > 0 || busy || serial.available() > 0)
    return false;
  busy = true;
  response_ready = false;
  clearPendingCommand();
  clearRxBuffer();
  serial.flush();   // Wait for any prior outgoing data to complete
  setPendingCommand(at_command);
  if (crc) {
    applyCrc(pending_command, AT_CLIENT_TX_BUFFERSIZE);
  }
  pending_command[strlen(pending_command)] = '\r';
  #ifdef ARDUINO
  if (LOG_GET_LEVEL() > DebugLogLevel::LVL_DEBUG)
    PRINTLN(tx_debug_tag, debugString(pending_command));
  #endif
  serial.print(pending_command);
  serial.flush();
  busy = false;
  return readAtResponse(timeout_ms);
}

bool AtClient::sendAtCommand(const String &at_command, uint16_t timeout_ms) {
  return sendAtCommand(at_command.c_str(), timeout_ms);
}

bool AtClient::readAtResponse(uint16_t timeout_ms) {
  if (busy)
    return false;
  busy = true;
  cmd_parsing = echo ? PARSE_ECHO : PARSE_RESPONSE;
  uint16_t countdown = (uint16_t)(timeout_ms / 1000);
  time_t tick = LOG_GET_LEVEL() > DebugLogLevel::LVL_DEBUG ? 1 : 0;
  LOG_TRACE("[", millis(), "] Timeout:", timeout_ms, "ms; Countdown:", countdown, "s");
  for (time_t start = millis(); millis() - start < timeout_ms;) {
    while (serial.available() > 0 && cmd_parsing < PARSE_OK) {
      toggleRaw(true);
      readSerialChar();
      char last = lastCharRead();
      if (last == AT_LF) {
        // unsolicited, V0 info-suffix/multiline sep, V1 prefix/multiline/suffix
        char* res = responsePtr();
        if (cmd_parsing == PARSE_ECHO || !startsWith(res, terminator)) {
          // check if V0 info suffix or multiline separator
          if (lastCharRead(2) != AT_CR) {
            toggleRaw(false);
            LOG_WARN("Unexpected response data removed: %s", res);
            clearRxBuffer();
          }
        }
        if (endsWith(res, vres_ok)) {
          toggleRaw(false);
          cmd_parsing = parsingOk();
          verbose = true;
        } else if (endsWith(res, vres_err)) {
          toggleRaw(false);
          cmd_parsing = parsingError();
          verbose = true;
        } else if (cmd_parsing == PARSE_CRC) {
          toggleRaw(false);
          LOG_DEBUG("CRC parsing complete");
          if (!cmd_result_ok) {
            cmd_parsing = PARSE_ERROR;
          } else {
            if (validateCrc(res)) {
              cmd_parsing = PARSE_OK;
            } else {
              LOG_ERROR("Invalid CRC");
              cmd_parsing = PARSE_ERROR;
              cmd_error = AT_ERR_CRC;
              cmd_result_ok = false;
            }
          }
        }   // else intermediate line formatter - keep parsing
      } else if (last == AT_CR) {
        char* res = responsePtr();
        if (endsWith(res, pending_command)) {
          toggleRaw(false);
          if (!startsWith(res, pending_command))
            LOG_WARN("Unexpected pre-echo data removed: %s", res);
          LOG_DEBUG("Echo received - clearing RX buffer");
          clearRxBuffer();   // remove echo from response
          cmd_parsing = PARSE_RESPONSE;
        } else {
          int old_parsing = cmd_parsing;
          char p = serial.peek();
          if (p == -1 || p == CRC_SEP) {
            toggleRaw(false);
            cmd_parsing = parsingShort(cmd_parsing);
          }
        }
      } else {
        if (cmd_parsing == PARSE_CRC && last == CRC_SEP) {
          cmd_crc_found = true;
        }
      }
    }
    if (cmd_parsing >= PARSE_OK) {
      toggleRaw(false);
      LOG_TRACE("Parsing complete");
      break;   // don't wait for timeout
    }
    if (tick > 0 && strlen(responsePtr()) == 0) {
      if ((millis() - start) / 1000 >= tick) {
        tick++;
        countdown--;
        toggleRaw(false);
        LOG_TRACE("[", millis(), "] Countdown:", countdown);
      }
    }
  }
  toggleRaw(false);
  if (cmd_parsing < PARSE_OK) {
    if (cmd_result_ok) {
      if (verbose && endsWith(responsePtr(), "\r")) {
        LOG_INFO("Detected non-verbose - setting flag");
        verbose = false;
      } else if (crc && !cmd_crc_found) {
        LOG_INFO("CRC expected but not found - clearing flag");
        crc = false;
        cmd_error = AT_ERR_CRC_CONFIG;
      }
    } else {
      LOG_WARN("AT command timeout during parsing");
      cmd_error = AT_ERR_TIMEOUT;
    }
  } else if (cmd_parsing == PARSE_ERROR) {
    cmd_error = AT_ERR_GENERIC;
    if (!crc && cmd_crc_found) {
      LOG_WARN("CRC detected but not expected");
      crc = true;
      cmd_error = AT_ERR_CRC_CONFIG;
    }
  } else {
    response_ready = true;
    cmd_error = AT_OK;
  }
  clearPendingCommand();
  busy = false;
  return response_ready;
}

parse_state_t AtClient::parsingOk() {
  parse_state_t next_state = PARSE_OK;
  cmd_result_ok = true;
  LOG_DEBUG("Result OK");
  LOG_TRACE("Assessing pending command for CRC toggle: ", pending_command);
  if (!this->crc) {
    if (includes(pending_command, (const char*)"CRC=1\r") ||
        includes(pending_command, (const char*)"crc=1\r")) {
      LOG_INFO("CRC enabled by pending command - set flag");
      this->crc = true;
      next_state = PARSE_CRC;
    }
  } else {
    if ((includes(pending_command, (const char*)"CRC=0\r") ||
        includes(pending_command, (const char*)"crc=0\r")) ||
        includes(pending_command, 'Z') && serial.available() == 0) {
      LOG_INFO("CRC disabled by pending command - clear flag");
      this->crc = false;
    } else {
      next_state = PARSE_CRC;
    }
  }
  if (next_state == PARSE_CRC) {
    LOG_DEBUG("Parsing CRC...");
  }
  return next_state;
}

parse_state_t AtClient::parsingError() {
  parse_state_t next_state = PARSE_ERROR;
  LOG_WARN("Result ERROR");
  delay(CHAR_DELAY);
  if (this->crc || serial.available() > 0) {
    next_state = PARSE_CRC;
    LOG_DEBUG("Parsing CRC...");
  }
  return next_state;
}

parse_state_t AtClient::parsingShort(uint8_t current) {
  parse_state_t next_state = current;
  LOG_DEBUG("Checking candidate short response code");
  char* res = responsePtr();
  if (!startsWith(res, terminator)) {
    if (this->verbose) {
      LOG_WARN("Short response code found - clearing verbose flag");
      this->verbose = false;
    }
    if (endsWith(res, res_ok)) {
      next_state = parsingOk();
    } else {
      next_state = parsingError();
    }
  }
  return next_state;
}

void AtClient::cleanResponse(const char *prefix) {
  if (crc) {
    LOG_TRACE("Removing CRC");
    unsigned short int crc_length = 1 + CRC_LEN + strlen(terminator);
    size_t crc_offset = strlen(responsePtr()) - crc_length;
    remove(responsePtr(), crc_offset, crc_length);
  }
  const char* to_remove = this->verbose ? vres_ok : res_ok;
  LOG_TRACE("Removing result code:", debugString(to_remove));
  replace(responsePtr(), to_remove, "", rx_buffer_size);
  if (prefix != nullptr) {
    LOG_TRACE("Removing prefix:", prefix);
    replace(responsePtr(), prefix, "", rx_buffer_size);
  }
  trim(responsePtr(), rx_buffer_size);
  replace(responsePtr(), "\r\n", "\n", rx_buffer_size);
  replace(responsePtr(), "\n\n", "\n", rx_buffer_size);
  LOG_TRACE("Trimmed and consolidated line feeds:", debugString(responsePtr()));
}

bool AtClient::readSerialChar(bool ignore_unprintable) {
  bool success = false;
  if (serial.available() > 0) {
    if (!isRxBufferFull()) {
      char c = serial.read();
      if (!printableChar(c, LOG_GET_LEVEL() > DebugLogLevel::LVL_DEBUG)) {
        if (ignore_unprintable) {
          success = true;
        }
      } else {
        #if defined(__AVR__) || defined(ESP8266)
        res_buffer_P[strlen(res_buffer_P)] = c;
        #else
        res_buffer[(strlen(res_buffer))] = c;
        #endif
        success = true;
      }
    }
  }
  return success;
}

}   // namespace at
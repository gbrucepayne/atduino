#include <atclient.h>

namespace at {

static const char rx_trace_tag[] = "[TRACE][RX] <<<";
static const char tx_trace_tag[] = "[TRACE][TX] >>>";

/**
 * @brief Add a preamble to raw character debug or newline for other debug
 * @param raw Adds a preamble for raw character logging
*/
void AtClient::toggleRaw(bool raw) {
  #ifdef ARDUINO
  if (LOG_GET_LEVEL() > DebugLogLevel::LVL_DEBUG) {
    if (raw) {
      if (!debug_raw)
        PRINT(rx_trace_tag, "");
      debug_raw = true;
    } else {
      if (debug_raw)
        PRINTLN();
      debug_raw = false;
    }
  }
  #endif
}

char* AtClient::responsePtr() {
  #if defined(__AVR__)
  return &res_buffer_P[0];   // may need to extract to SRAM first?
  #else  
  return &res_buffer[0];
  #endif
}

char* AtClient::commandPtr() {
  #if defined(__AVR__)
  return &pending_command_P[0];
  #else
  return &pending_command[0];
  #endif
}

bool AtClient::isRxBufferFull() {
  return strlen(responsePtr()) >= rx_buffer_size - 1;
}

void AtClient::clearRxBuffer() {
  memset(responsePtr(), 0, rx_buffer_size);
}

void AtClient::getResponse(char* response, const char* prefix, size_t buffersize) {
  cleanResponse(prefix);
  strncpy(response, responsePtr(), buffersize);
  response_ready = false;
}

void AtClient::getResponse(String& response, const char* prefix) {
  response = sgetResponse(prefix);
}

String AtClient::sgetResponse(const char* prefix) {
  cleanResponse(prefix);
  response_ready = false;
  return String(responsePtr());
}

void AtClient::clearPendingCommand() {
  memset(commandPtr(), 0, tx_buffer_size);
}

bool AtClient::setPendingCommand(const char *at_command) {
  if (strlen(at_command) > tx_buffer_size)
    return false;   // invalid_argument("command too large");
  strncpy(commandPtr(), at_command, tx_buffer_size);
  return true;
}

char AtClient::lastCharRead(size_t n) {
  char* buffer = responsePtr();
  if (n <= 0 || strlen(buffer) < n)
    return -1;
  return buffer[strlen(buffer) - n];
}

// If any data is on the serial port read until a match of read_until
bool AtClient::checkUrc(const char* read_until, uint32_t timeout_ms,
                        const char prefix, uint16_t wait_ms) {
  if (strlen(commandPtr()) > 0 ||
      (wait_ms == 0 && serial.available() == 0)  ||
      busy) {
    if (strlen(commandPtr()) > 0) LOG_WARN("AT command pending");
    if (serial.available() == 0) LOG_DEBUG("No data");
    if (busy) LOG_WARN("Busy with prior operation");
    return false;
  }
  LOG_TRACE("Busy parsing URC");
  busy = true;
  if (read_until == nullptr)
    read_until = terminator;
  LOG_DEBUG("Processing URC until", debugString(read_until));
  toggleRaw(true);
  clearRxBuffer();
  response_ready = false;
  bool urc_found = false;
  timeout_ms += wait_ms;
  for (uint32_t start = millis(); (millis() - start) < timeout_ms;) {
    if (!readSerialChar() && urc_found) {
      LOG_WARN("Bad serial byte while parsing URC");
      break;
    }
    if (!urc_found) {
      if (lastCharRead() == prefix) {
        urc_found = true;
        if (!startsWith(responsePtr(), terminator) &&
            !startsWith(responsePtr(), prefix)) {
          LOG_WARN("Dumping pre-URC data:", debugString(responsePtr()));
          clearRxBuffer();
          responsePtr()[0] = prefix;
        }
      }
    } else if (strlen(responsePtr()) > (strlen(read_until) + 1) &&
               endsWith(responsePtr(), read_until)) {
      response_ready = true;
      break;
    }
  }
  toggleRaw(false);
  if (!response_ready) {
    LOG_WARN("Timed out waiting for prefix and/or terminator");
    clearRxBuffer();
  }
  busy = false;
  LOG_TRACE("Finished parsing URC");
  return response_ready;
}

at_error_t AtClient::sendAtCommand(const char *at_command, uint16_t timeout_ms) {
  // if (strlen(commandPtr()) > 0 || serial.available() > 0 || busy) {
  if (busy) {
    if (strlen(commandPtr()) > 0) {
      LOG_DEBUG("Prior AT command pending");
    } else {
      LOG_DEBUG("Parsing URC data");
    }
    cmd_error = AT_ERR_BUSY;
    return cmd_error;
  }
  LOG_DEBUG("Sending command:", at_command);
  LOG_TRACE("Busy processing AT command/response");
  busy = true;
  response_ready = false;
  if (serial.available() > 0) {
    while (serial.available() > 0) {
      readSerialChar();
    }
    LOG_WARN("Dumping Rx data:", responsePtr());
  }
  clearRxBuffer();
  serial.flush();   // Wait for any prior outgoing data to complete
  setPendingCommand(at_command);
  if (crc) applyCrc(commandPtr(), tx_buffer_size);
  commandPtr()[strlen(commandPtr())] = '\r';
  #ifdef LOG_GET_LEVEL
  if (LOG_GET_LEVEL() > DebugLogLevel::LVL_DEBUG)
    PRINTLN(tx_trace_tag, debugString(commandPtr()));
  #endif
  serial.print(commandPtr());
  serial.flush();
  return readAtResponse(timeout_ms);
}

at_error_t AtClient::sendAtCommand(const String &at_command, uint16_t timeout_ms) {
  return sendAtCommand(at_command.c_str(), timeout_ms);
}

at_error_t AtClient::readAtResponse(uint16_t timeout_ms) {
  // busy = true;   // should be redundant
  cmd_parsing = echo ? PARSE_ECHO : PARSE_RESPONSE;
  uint16_t countdown = (uint16_t)(timeout_ms / 1000);
  uint32_t tick = LOG_GET_LEVEL() > DebugLogLevel::LVL_DEBUG ? 1 : 0;
  LOG_TRACE("Timeout:", timeout_ms, "ms; Countdown:", countdown, "s");
  for (uint32_t start = millis(); millis() - start < timeout_ms;) {
    while (serial.available() > 0 && cmd_parsing < PARSE_OK) {
      toggleRaw(true);
      if (!readSerialChar()) {
        cmd_error = AT_ERR_BAD_BYTE;
        cmd_parsing = PARSE_ERROR;
        break;
      }
      char last = lastCharRead();
      if (last == AT_LF) {
        // unsolicited, V0 info-suffix/multiline sep, V1 prefix/multiline/suffix
        char* res = responsePtr();
        if (cmd_parsing == PARSE_ECHO || !startsWith(res, terminator)) {
          // check if V0 info suffix or multiline separator
          if (lastCharRead(2) != AT_CR) {
            toggleRaw(false);
            LOG_WARN("Unexpected response data removed:", debugString(res));
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
        if (endsWith(res, commandPtr())) {
          toggleRaw(false);
          if (!startsWith(res, commandPtr()))
            LOG_WARN("Unexpected pre-echo data removed:",
                     debugString(res, 0, strlen(res) - strlen(commandPtr())));
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
      } else if (last == CRC_SEP && cmd_parsing == PARSE_CRC) {
        cmd_crc_found = true;
      }
    }   // parsed available char
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
  }   // parsing timeout loop
  toggleRaw(false);
  if (cmd_parsing < PARSE_OK) {
    if (cmd_result_ok) {
      if (verbose && endsWith(responsePtr(), "\r")) {
        LOG_INFO("Detected non-verbose");
        if (autoflag) verbose = false;
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
  LOG_TRACE("Finished parsing AT command response");
  return cmd_error;
}

at_error_t AtClient::lastErrorCode(bool clear) {
  at_error_t last = cmd_error;
  if (clear) cmd_error = AT_OK;
  return last;
}

parse_state_t AtClient::parsingOk() {
  parse_state_t next_state = PARSE_OK;
  cmd_result_ok = true;
  LOG_DEBUG("Result OK");
  LOG_TRACE("Assessing pending command for CRC toggle: ", commandPtr());
  if (!this->crc) {
    if (includes(commandPtr(), (const char*)"CRC=1\r") ||
        includes(commandPtr(), (const char*)"crc=1\r")) {
      LOG_INFO("CRC enabled by pending command - set flag");
      this->crc = true;
      next_state = PARSE_CRC;
    }
  } else {
    if ((includes(commandPtr(), (const char*)"CRC=0\r") ||
        includes(commandPtr(), (const char*)"crc=0\r")) ||
        includes(commandPtr(), 'Z') && serial.available() == 0) {
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
    if (verbose) {
      LOG_WARN("Short response code found");
      if (autoflag) verbose = false;
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

/**
 * @brief Attempts to read the next serial character
 * @returns false if character is invalid else true (success or no data)
*/
bool AtClient::readSerialChar(bool ignore_unprintable) {
  bool success = true;
  if (serial.available() > 0) {
    if (!isRxBufferFull()) {
      char c = serial.read();
      if (!printableChar(c, LOG_GET_LEVEL() > DebugLogLevel::LVL_DEBUG)) {
        if (!ignore_unprintable) success = false;
      } else {
        responsePtr()[strlen(responsePtr())] = c;
      }
    }
  }
  return success;
}

}   // namespace at
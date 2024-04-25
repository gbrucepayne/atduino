#include <atclient.h>

namespace at {

static const char rx_trace_tag[] = "[V][RAW RX <<<] ";
static const char tx_trace_tag[] = "[V][RAW TX >>>] ";

/**
 * @brief Add a preamble to raw character debug or newline for other debug
 * @param raw Adds a preamble for raw character logging
*/
void AtClient::toggleRaw(bool raw) {
  if (ardebugGetLevel() > ARDEBUG_D) {
    if (raw) {
      if (!debug_raw)
        ardprintf("%s", rx_trace_tag);
      debug_raw = true;
    } else {
      if (debug_raw)
        ardprintf("\n");
      debug_raw = false;
    }
  }
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
  response_ready = false;
}

void AtClient::getResponse(char* response, const char* prefix, size_t buffersize,
                           bool clean) {
  if (clean) cleanResponse(prefix);
  strncpy(response, responsePtr(), buffersize);
  response_ready = false;
}

void AtClient::getResponse(String& response, const char* prefix, bool clean) {
  response = sgetResponse(prefix);
}

String AtClient::sgetResponse(const char* prefix, bool clean) {
  if (clean) cleanResponse(prefix);
  response_ready = false;
  return String(responsePtr());
}

void AtClient::clearPendingCommand() {
  memset(commandPtr(), 0, tx_buffer_size);
}

bool AtClient::setPendingCommand(const char *at_command) {
  if (strlen(at_command) > tx_buffer_size) {
    AR_LOGE("Command %s too long for Tx buffer", at_command);
    return false;
  }
  char tmp[tx_buffer_size];
  // strncpy(tmp, at_command, tx_buffer_size);
  // if (crc) applyCrc(tmp, tx_buffer_size);
  snprintf(commandPtr(), tx_buffer_size, "%s\r", at_command);
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
  // TODO: semaphore lock
  if (wait_ms == 0 && serial.available() == 0) {
    // if (strlen(commandPtr()) > 0) AR_LOGW("AT command pending");
    // if (serial.available() == 0) AR_LOGE("No data");
    // if (busy) AR_LOGW("Busy with prior operation");
    return false;
  }
  if (read_until == nullptr)
    read_until = terminator;
  timeout_ms += wait_ms;
#ifndef ARDEBUG_DISABLED
  AR_LOGV("Processing URC until %s or %d",
      debugString(read_until).c_str(), timeout_ms);
#endif
  toggleRaw(true);
  clearRxBuffer();
  bool urc_found = false;
  for (uint32_t start = millis(); (millis() - start) < timeout_ms;) {
    if (!readSerialChar() && urc_found) {
      toggleRaw(false);
      AR_LOGW("Bad serial byte while parsing URC");
      cmd_error = AT_ERR_BAD_BYTE;
      break;
    }
    if (!urc_found) {
      if (lastCharRead() == prefix) {
        urc_found = true;
        if (!startsWith(responsePtr(), terminator) &&
            !startsWith(responsePtr(), prefix)) {
          toggleRaw(false);
#ifndef ARDEBUG_DISABLED
          AR_LOGW("Dumping pre-URC data: %s", sDbgRes().c_str());
#endif
          clearRxBuffer();
          responsePtr()[0] = prefix;
          toggleRaw(true);
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
    if (strlen(responsePtr()) > 0)
      AR_LOGW("URC timeout no prefix and/or terminator: %s", sDbgRes().c_str());
    clearRxBuffer();
  }
  // busy = false;
  // AR_LOGV("Finished parsing URC");
  return response_ready;
}

at_error_t AtClient::sendAtCommand(const char *at_command, uint16_t timeout_ms) {
  // TODO: semaphore lock, possible URC event
  if (serial.available() > 0) {
    while (serial.available() > 0) {
      readSerialChar();
    }
#ifndef ARDEBUG_DISABLED
    AR_LOGW("Dumping unsolicited Rx data: %s", sDbgRes().c_str());
  }
  AR_LOGD("Sending command: %s", at_command);
#endif
  clearRxBuffer();
  serial.flush();   // Wait for any prior outgoing data to complete
  setPendingCommand(at_command);
#ifndef ARDEBUG_DISABLED
  if (ardebugGetLevel() > ARDEBUG_D)
    ardprintf("%s%s\n", tx_trace_tag, sDbgReq().c_str());
#endif
  size_t wrote = serial.print(commandPtr());
  if (wrote < strlen(commandPtr())) {
    AR_LOGE("Failed to write all bytes");
    cmd_error = AT_ERR_BAD_BYTE;
    return cmd_error;
  }
  serial.flush();
  return readAtResponse(timeout_ms);
}

at_error_t AtClient::sendAtCommand(const String &at_command, uint16_t timeout_ms) {
  return sendAtCommand(at_command.c_str(), timeout_ms);
}

at_error_t AtClient::readAtResponse(uint16_t timeout_ms) {
  // busy = true;   // should be redundant
#ifndef ARDEBUG_DISABLED
  AR_LOGV("Parsing response to %s for %d ms", sDbgReq().c_str(), timeout_ms);
#endif
  cmd_parsing = echo ? PARSE_ECHO : PARSE_RESPONSE;
  cmd_error = AT_ERR_GENERIC;
  uint16_t countdown = (uint16_t)(timeout_ms / 1000);
  uint32_t tick = ardebugGetLevel() > ARDEBUG_D ? 1 : 0;
  AR_LOGV("Timeout: %d ms; Countdown: %d s", timeout_ms, countdown);
  for (uint32_t start = millis(); millis() - start < timeout_ms;) {
    while (serial.available() > 0 && cmd_parsing < PARSE_OK) {
      toggleRaw(true);
      if (!readSerialChar()) {
        cmd_error = AT_ERR_BAD_BYTE;
        cmd_parsing = PARSE_ERROR;
        toggleRaw(false);
        AR_LOGE("Bad byte received in response");
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
#ifndef ARDEBUG_DISABLED
            AR_LOGW("Unexpected response data removed: %s",
                debugString(res).c_str());
#endif
            clearRxBuffer();
          }
        }
        if (endsWith(res, vres_ok)) {
          toggleRaw(false);
          cmd_parsing = parsingOk();
          verbose = true;
        } else if (endsWith(res, vres_err) || startsWith(res, cme_err)) {
          toggleRaw(false);
          cmd_parsing = parsingError();
          verbose = true;
        } else if (cmd_parsing == PARSE_CRC) {
          toggleRaw(false);
          AR_LOGV("CRC parsing complete");
          if (!cmd_result_ok) {
            cmd_parsing = PARSE_ERROR;
          } else {
            if (validateCrc(res)) {
              cmd_parsing = PARSE_OK;
            } else {
              AR_LOGW("Invalid CRC");
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
#ifndef ARDEBUG_DISABLED
          if (!startsWith(res, commandPtr())) {
            String xtra = debugString(res, 0, strlen(res) - strlen(commandPtr()));
            AR_LOGW("Unexpected pre-echo data removed: %s", xtra.c_str());
          }
          AR_LOGV("Echo received - clearing RX buffer: %s", sDbgRes().c_str());
#endif
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
      break;   // don't wait for timeout
    }
    if (tick > 0 && strlen(responsePtr()) == 0) {
      if ((millis() - start) / 1000 >= tick) {
        tick++;
        countdown--;
        toggleRaw(false);
        AR_LOGV("[%d] Countdown: %d", millis(), countdown);
      }
    }
  }   // parsing timeout loop
  toggleRaw(false);
  if (cmd_parsing < PARSE_OK) {
    if (cmd_result_ok) {
      if (verbose && endsWith(responsePtr(), "\r")) {
        AR_LOGI("Detected non-verbose");
        if (autoflag) verbose = false;
      } else if (crc && !cmd_crc_found) {
        AR_LOGI("CRC expected but not found - clearing flag");
        crc = false;
        cmd_error = AT_ERR_CRC_CONFIG;
      }
    } else {
      AR_LOGW("AT command timeout during parsing");
      cmd_error = AT_ERR_TIMEOUT;
    }
  } else if (cmd_parsing == PARSE_ERROR) {
    if (!crc && cmd_crc_found) {
      AR_LOGW("CRC detected but not expected");
      crc = true;
      cmd_error = AT_ERR_CRC_CONFIG;
    } else if (startsWith(responsePtr(), cme_err)) {
      const size_t cme_errno_buffer = 24;
      if (strlen(responsePtr()) < cme_errno_buffer) {
        char tmp[cme_errno_buffer];
        strncpy(tmp, responsePtr(), strlen(responsePtr()));
        replace(tmp, cme_err, "", cme_errno_buffer);
        if (isNumber(tmp)) {
          AR_LOGD("Found CME ERROR code - clearing response buffer");
          cmd_error = atoi(tmp);
          clearRxBuffer();
        }
      } else {
        response_ready = true; // Verbose response available to retrieve
#ifndef ARDEBUG_DISABLED
        String tmp = String(responsePtr());
        tmp.trim();
        AR_LOGE("%s", tmp.c_str());
#endif
      }
    }
  } else {
    response_ready = true;
    cmd_error = AT_OK;
  }
#ifndef ARDEBUG_DISABLED
  AR_LOGV("Parsing complete (error code %d) - clearing pending command: %s",
      cmd_error, sDbgReq().c_str());
  if (response_ready) AR_LOGV("Response: %s", sDbgRes().c_str());
#endif
  clearPendingCommand();
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
#ifndef ARDEBUG_DISABLED
  AR_LOGD("Result OK: %s", sDbgRes().c_str());
  AR_LOGV("Assessing pending command for CRC toggle: %s", sDbgReq().c_str());
#endif
  if (!this->crc) {
    if (includes(commandPtr(), (const char*)"CRC=1\r") ||
        includes(commandPtr(), (const char*)"crc=1\r")) {
      AR_LOGI("CRC enabled by pending command - set flag");
      this->crc = true;
      next_state = PARSE_CRC;
    }
  } else {
    if ((includes(commandPtr(), (const char*)"CRC=0\r") ||
        includes(commandPtr(), (const char*)"crc=0\r")) ||
        includes(commandPtr(), 'Z') && serial.available() == 0) {
      AR_LOGI("CRC disabled by pending command - clear flag");
      this->crc = false;
    } else {
      next_state = PARSE_CRC;
    }
  }
  if (next_state == PARSE_CRC) {
    AR_LOGV("Parsing CRC...");
  }
  return next_state;
}

parse_state_t AtClient::parsingError() {
  parse_state_t next_state = PARSE_ERROR;
  AR_LOGE("Result ERROR");
  delay(CHAR_DELAY);
  if (this->crc || serial.available() > 0) {
    next_state = PARSE_CRC;
    AR_LOGV("Parsing CRC...");
  }
  return next_state;
}

parse_state_t AtClient::parsingShort(uint8_t current) {
  parse_state_t next_state = current;
  AR_LOGV("Checking candidate short response code");
  char* res = responsePtr();
  if (!startsWith(res, terminator)) {
    if (verbose) {
      AR_LOGW("Short response code found");
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
  if (strlen(responsePtr()) == 0) {
    AR_LOGD("No response to clean");
    return;
  }
  if (crc) {
    AR_LOGV("Removing CRC");
    unsigned short int crc_length = 1 + CRC_LEN + strlen(terminator);
    size_t crc_offset = strlen(responsePtr()) - crc_length;
    remove(responsePtr(), crc_offset, crc_length);
  }
  const char* to_remove = this->verbose ? vres_ok : res_ok;
#ifndef ARDEBUG_DISABLED
  AR_LOGV("Removing result code: %s", debugString(to_remove).c_str());
#endif
  replace(responsePtr(), to_remove, "", rx_buffer_size);
  if (prefix != nullptr && strcmp(prefix, "") != 0) {
    AR_LOGV("Removing prefix: %s", prefix);
    replace(responsePtr(), prefix, "", rx_buffer_size);
  }
  trim(responsePtr(), rx_buffer_size);
  replace(responsePtr(), "\r\n", "\n", rx_buffer_size);
  replace(responsePtr(), "\n\n", "\n", rx_buffer_size);
#ifndef ARDEBUG_DISABLED
  AR_LOGV("Trimmed and consolidated line feeds: %s", sDbgRes().c_str());
#endif
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
      if (!printableChar(c, ardebugGetLevel() > ARDEBUG_D)) {
        if (!ignore_unprintable) success = false;
      } else {
        size_t index = strlen(responsePtr());
        responsePtr()[index] = c;
        responsePtr()[index + 1] = '\0';
      }
    }
  }
  return success;
}

}   // namespace at
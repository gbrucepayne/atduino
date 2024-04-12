#include "atserver.h"

namespace at {

bool AtServer::handleCommand() {
  bool success = false;
  char* req = commandPtr();
  bool crc_valid = (!crc || (crc && at::validateCrc(req)));
  AR_LOGV("CRC enabled? %d; valid? %d", crc, crc_valid);
  if (!crc_valid) {
    last_error_code = AT_ERR_CRC;
  } else {
    if (at::startsWith(req, "AT") || at::startsWith(req, "at")) {
      // Check for multiple commands based on AT_COMMAND_SEPARATORS
      // For each command loop through this->commands and process each
      if (crc) {
        AR_LOGV("Removing CRC");
        req[(strlen(req) - 1) - (CRC_LEN + 1)] = '\0';
      }
      const char* attn = at::startsWith(req, "AT") ? "AT" : "at";
      at::replace(req, attn, "", AT_SERVER_RX_BUFFERSIZE, 1);
      at::trim(req, AT_SERVER_RX_BUFFERSIZE);
      int req_count = 1;
      req_count += at::instancesOf((const char*)req, AT_SEP);
      for (int i = 0; i < req_count; ++i) {
        while (*req == ' ') {
          // AR_LOGV("Ignoring spaces per V.25");
          req++;
        }
        int req_length = strlen(req);
        if (req_length == 0) {  // basic AT
          success = true;
          break;
        } else if (at::indexOf(req, AT_SEP) != -1) {
          req_length = at::indexOf(req, AT_SEP);
        }
        if (req_length > sizeof(working_buffer)) {
          AR_LOGW("Request length too long");
          break;
        }
        at::substring(working_buffer, req, 0, req_length);
        for (const AtCommand& cmd : commands) {
          char* cur = &working_buffer[0];
          success = false;
          if (strcmp(cur, "E0") == 0 || strcmp(cur, "e0") == 0 ||
              strcmp(cur, "E1") == 0 || strcmp(cur, "e1") == 0) {
            echo = at::endsWith(cur, "1") ? true : false;
            success = true;
          } else if (strcmp(cur, "V0") == 0 || strcmp(cur, "v0") == 0 ||
                     strcmp(cur, "V1") == 0 || strcmp(cur, "v1") == 0) {
            verbose = at::endsWith(cur, "1") ? true : false;
            success = true;
          } else if (at::endsWith(cur, "CRC=0") || at::endsWith(cur, "crc=0") ||
                     at::endsWith(cur, "CRC=1") || at::endsWith(cur, "crc=1")) {
            crc = at::endsWith(cur, "1") ? true : false;
            success = true;
          } else if (at::startsWith(cur, cmd.name)) {
            if (strlen(cur) == strlen(cmd.name)) {
              if (cmd.run != nullptr) {
                cmd.run();
                success = true;
              }
            } else {
              cur += strlen(cmd.name);
              if (*cur == '=') {
                if (*(cur+1) == '?') {
                  if (cmd.test != nullptr) {
                    cmd.test();
                  }
                  success = true;
                } else {
                  if (cmd.write != nullptr) {
                    last_error_code = cmd.write((const char*)cur + 1);
                    success = last_error_code == 0;
                  }
                }
              } else if (*cur == '?') {
                if (cmd.read != nullptr) {
                  cmd.read();
                  success = true;
                }
              }
            }
            break;
          }
        }
        if (i < req_count - 1)
          req += req_length + 1;
      }
    }
  }
  if (!success) {
    last_error_code = AT_ERR_CMD_UNKNOWN;
    sendError();
  } else {
    last_error_code = AT_OK;
    sendOk();
  }
  clearRxBuffer();
  return success;
}

bool AtServer::addCommand(AtCommand* new_cmd, bool replace) {
  int index = -1;
  for (size_t i = 0; i < commands.size(); ++i) {
    if (commands[i].name == new_cmd->name) {
      index = i;
      break;
    }
  }
  if (index > -1) {
    if (!replace)
      return false;
    AR_LOGW("Replacing command %s", new_cmd->name);
    commands.erase(commands.begin() + index);
  }
  commands.push_back(*new_cmd);
  return true;
}

AtServer::AtServer(Stream& serial) : serial(serial) {
  snprintf(terminator, 3, "%c%c", AT_CR, AT_LF);
  snprintf(vres_ok, 7, "%sOK%s", terminator, terminator);
  snprintf(vres_err, 10, "%sERROR%s", terminator, terminator);
  snprintf(res_ok, 3, "0%c", AT_CR);
  snprintf(res_err, 3, "4%c", AT_CR);
}

at_error_t AtServer::readSerial() {
  if (readSerialChar()) {
    char c = lastCharRead();
    if (c == AT_BS) {
      char* cmd = commandPtr();
      int len = strlen(cmd);
      cmd[len - 1] = '\0';   // remove the backspace
      if (len > 2)
        cmd[len - 2] = '\0';   // remove the character prior to backspace
    }
    if (echo)
      serial.write(c);
    if (parsing == PARSE_NONE)
      parsing = PARSE_COMMAND;
    if (c == AT_CR) {
      AR_LOGV("Processing: %s", debugString(commandPtr()));
      bool handled = handleCommand();
      parsing = PARSE_NONE;
      return handled ? 0 : last_error_code;
    }
  }
  return 0;
}

void AtServer::getTerminator(char* buffer, unsigned short buffer_size) {
  strncpy(buffer, terminator, buffer_size);
}

void AtServer::getOk(char* buffer, unsigned short buffer_size) {
  strncpy(buffer, verbose ? vres_ok : res_ok, buffer_size);
}

void AtServer::getError(char* buffer, unsigned short buffer_size) {
  strncpy(buffer, verbose ? vres_err : res_err, buffer_size);
}

void AtServer::send(const char* str, bool ok, bool error) {
  if (strlen(str) > 0)
    serial.write(str);
  if (ok) {
    sendOk();
  } else if (error) {
    sendError();
  } else {
    serial.write(terminator);
  }
}

void AtServer::send(String& str, bool ok, bool error) {
  send(str.c_str(), ok, error);
}

void AtServer::sendOk() {
  int buffersize = 16;
  char to_write[buffersize];
  strncpy(to_write, verbose ? vres_ok : res_ok, buffersize);
  if (crc) {
    at::applyCrc(to_write, buffersize);
    at::append(to_write, "\r\n", buffersize);
  }
  serial.write(to_write);
}

void AtServer::sendError() {
  int buffersize = 24;
  char to_write[buffersize];
  strncpy(to_write, verbose ? vres_err : res_err, buffersize);
  if (crc) {
    AR_LOGV("Pre-CRC length: %d", strlen(to_write));
    at::applyCrc(to_write, buffersize);
    AR_LOGV("Post-CRC length: %d", strlen(to_write));
    at::append(to_write, "\r\n", buffersize);
  }
  AR_LOGV("to_write length: %d", strlen(to_write));
  serial.write(to_write);
}

bool AtServer::readSerialChar(bool ignore_unprintable) {
  bool success = false;
  if (serial.available() > 0) {
    if (!isRxBufferFull()) {
      char c = serial.read();
      if (!printableChar(c, ardebugGetLevel() > ARDEBUG_D)) {
        if (ignore_unprintable) {
          success = true;
        }
      } else {
        char* buf = commandPtr();
        buf[strlen(buf)] = c;
        success = true;
      }
    }
  }
  return success;
}

bool AtServer::isRxBufferFull() {
  char* cmd = commandPtr();
  return strlen(cmd) >= AT_SERVER_RX_BUFFERSIZE - 1;
}

void AtServer::clearRxBuffer() {
  #if defined(__AVR__) || defined(ESP8266)
  memset(rx_buffer_P, 0, AT_SERVER_RX_BUFFERSIZE);
  #else
  memset(rx_buffer, 0, AT_SERVER_RX_BUFFERSIZE);
  #endif
}

char* AtServer::commandPtr() {
  #if defined(__AVR__)
  return &rx_buffer_P[0];   // may need to extract to SRAM first?
  #else  
  return &rx_buffer[0];
  #endif
}

char AtServer::lastCharRead(size_t n) {
  if (n <= 0)
    return -1;
  char* cmd = commandPtr();
  return cmd[strlen(cmd) - n];
}

}   // namespace at
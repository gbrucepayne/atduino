/**
 * @brief CRC-CCITT (0xFFFF) utility
 * 
 * Acknowledgement: https://stackoverflow.com/questions/25239423/crc-ccitt-16-bit-python-manual-calculation
 * 
 */
#include "crcxmodem.h"

namespace at {

static const char* HEX_CHARSET = "0123456789ABCDEF";

/**
 * @brief Generate initial crc
 * @private
 * 
 * @param c 
 * @return int 
 */
static int initialCrc_(int c) {
  int crc = 0;
  c = c << 8;
  for (uint8_t i = 0; i < 8; i++) {
    if ((crc ^ c) & 0x8000) {
      crc = (crc << 1) ^ POLYNOMIAL;
    } else {
      crc = crc << 1;
    }
    c = c << 1;
  }
  return crc;
}

// TODO: better way to initialize this table of constants?
static int crcxmodem_table [256];
static bool crcxmodem_table_is_initialized = false;
static bool initializeCrcTable_() {
  if (!crcxmodem_table_is_initialized) {
    AR_LOGV("Initializing CRC table");
    // Unclear why this fails at i=255
    for (uint8_t i = 0; i < 255; i++) {
      crcxmodem_table[i] = initialCrc_(i);
      // AR_LOGV("CRC table [%d]=%d", i, crcxmodem_table[i]);
    }
    crcxmodem_table[255] = initialCrc_(255);
    // AR_LOGV("CRC table [255]=%d", crcxmodem_table[255]);
    crcxmodem_table_is_initialized = true;
  } else {
    AR_LOGV("CRC table already initialized");
  }
  return crcxmodem_table_is_initialized;
}

static int updateCrc_(int crc, int c) {
  int cc = 0xFF & c;
  int tmp = (crc >> 8) ^ cc;
  crc = (crc << 8) ^ crcxmodem_table[tmp & 0xFF];
  crc = crc & 0xFFFF;
  return crc;
}

static size_t sepPos_(const char* crc_string, const char sep = CRC_SEP) {
  size_t str_len = strlen(crc_string);
  size_t sep_pos = str_len;
  if (includes(crc_string, sep)) {
    for (size_t i = str_len - 1; i >= 0; i--) {
      if (crc_string[i] == sep) {
        sep_pos = i;
        break;
      }
    }
  }
  return sep_pos;
}

static int calculateCrc_(const char *crc_string,
                         int initial_value = 0xFFFF,
                         const char sep = CRC_SEP) {
  initializeCrcTable_();
  int crc = initial_value;
  size_t sep_pos = sepPos_(crc_string, sep);
  for (size_t i = 0; i < sep_pos; i++) {
    crc = updateCrc_(crc, crc_string[i]);
  }
  return crc;
}

static int calculateCrc_(const String &crc_string,
                         int initial_value = 0xFFFF,
                         const char sep = CRC_SEP) {
  return calculateCrc_(crc_string.c_str(), initial_value, sep);
}

bool applyCrc(char *at_command, size_t tx_buffersize, const char sep) {
  size_t offset = strlen(at_command);
  size_t applied_length = offset + 1 + CRC_LEN;   // includes separator
  if (tx_buffersize <= applied_length)
    return false;
  int crc = calculateCrc_(at_command);
  char hex_string[tx_buffersize];
  intToHex(hex_string, crc, CRC_LEN, applied_length);
  for (size_t i = offset; i < applied_length; i++) {
    if (i == offset) {
      at_command[i] = sep;
    } else {
      at_command[i] = hex_string[i - offset - 1];
    }
  }
  at_command[applied_length] = '\0';
  return true;
}

bool applyCrc(String &at_command, const char sep) {
  int crc = calculateCrc_(at_command);
  String hex_crc = "";
  intToHex(hex_crc, crc, CRC_LEN);
  AR_LOGD("Applying CRC: %d -> %s", crc, hex_crc);
  at_command = at_command + sep + hex_crc;
  return true;
}

bool validateCrc(const char *response, const char sep) {
#ifndef ARDEBUG_DISABLED
  AR_LOGD("Validating CRC for %s", debugString(response).c_str());
#endif
  size_t res_len = strlen(response);
  size_t crc_start = sepPos_(response);
  if (crc_start == res_len)
    return false;   // No CRC found
  char res[res_len];
  char res_crc[1 + CRC_LEN];
  substring(res, response, 0, crc_start);
  substring(res_crc, response, crc_start + 1, crc_start + 1 + CRC_LEN);
  return calculateCrc_(res) == hexToInt(res_crc);
}

bool validateCrc(const String& response, const char sep) {
  return validateCrc(response.c_str(), sep);
}

}   // namespace at
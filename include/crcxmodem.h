/**
 * @brief CRC-CCITT (0xFFFF) utility
 * 
 * Acknowledgement: https://stackoverflow.com/questions/25239423/crc-ccitt-16-bit-python-manual-calculation
 * 
 */
#ifndef CRCXMODEM_H
#define CRCXMODEM_H

#include <Arduino.h>
#include "atdebuglog.h"
#include "atstringutils.h"

#define POLYNOMIAL 0x1021
#define PRESET 0
#define CRC_SEP '*'
#define CRC_LEN 4

/**
 * @brief Applies CRC to the supplied AT command for submission to the modem
 * 
 * @param at_command The original AT command
 * @param sep (Optional) Separator uses default `*` if not supplied
 * @return true if successful
 */
bool applyCrc(char* at_command, size_t at_buffersize, const char sep = CRC_SEP);
bool applyCrc(String& at_command, const char sep = CRC_SEP);

/**
 * @brief Validates an AT command response with CRC
 * 
 * @param response The response from the modem
 * @param sep (Optional) Separator uses default `*` if not supplied
 * @return true If received matches expected
 */
bool validateCrc(const char* response, const char sep = CRC_SEP);
bool validateCrc(const String& response, const char sep = CRC_SEP);

#endif
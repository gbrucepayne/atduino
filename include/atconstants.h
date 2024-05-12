#ifndef AT_CONSTANTS_H
#define AT_CONSTANTS_H

#ifndef AT_BAUDRATE
#define AT_BAUDRATE 9600
#endif
#define AT_CHAR_DELAY_MS 10   // default inter-character milliseconds
#define AT_TIMEOUT_MS 1000   // default timeout for command responses
#define AT_URC_TIMEOUT_MS 250   // default timeout checking for unsolicited

#define AT_CLIENT_RX_BUFFERSIZE 32768
#define AT_CLIENT_TX_BUFFERSIZE 32768

#define AT_SERVER_RX_BUFFERSIZE 256

#define AT_CR '\r'   // line terminator (default 0x0C)
#define AT_LF '\n'   // response line formatter (default 0x0A)
#define AT_BS '\b'   // backspace (default 0x08)
#define AT_SEP ';'   // v.25 mandated command separator

// Error codes for AT communications
typedef unsigned short at_error_t;
// V.25 compatible
#define AT_OK 0   // V.25 standard
#define AT_URC 2   // repurpose `RING` for unsolicited result codes (URC)
#define AT_ERR_TIMEOUT 3   // repurpose `NO CARRIER` for modem unavailable
#define AT_ERROR 4   // V.25 standard
// Orbcomm satellite-modem compatible
#define AT_ERR_CMD_CRC 100   // Bad CRC on received command
// Custom-defined for this library
#define AT_ERR_BAD_BYTE 255   // Non-ASCII character received on serial
#define AT_ERR_CRC_CONFIG 254   // CRC expected but not found or vice versa

// Internal use within this library
typedef unsigned short parse_state_t;
#define AT_PARSE_NONE 0
#define AT_PARSE_ECHO 1
#define AT_PARSE_RESPONSE 2
#define AT_PARSE_CRC 3
#define AT_PARSE_OK 4
#define AT_PARSE_ERROR 5
#define AT_PARSE_COMMAND 6   // Server-side

#endif   // AT_CONSTANTS_H
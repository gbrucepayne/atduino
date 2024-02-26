#ifndef AT_CONSTANTS_H
#define AT_CONSTANTS_H

#ifndef AT_BAUDRATE
#define AT_BAUDRATE 9600
#endif
#define CHAR_DELAY 10   // milliseconds 
#define AT_TIMEOUT_MS 1000

#define AT_CLIENT_RX_BUFFERSIZE 32768
#define AT_CLIENT_TX_BUFFERSIZE 32768

#define AT_SERVER_RX_BUFFERSIZE 256

#define AT_CR '\r'   // line terminator (default 0x0C)
#define AT_LF '\n'   // response line formatter (default 0x0A)
#define AT_BS '\b'   // backspace (default 0x08)
#define AT_SEP ';'   // v.25 mandated command separator

// Error codes for AT communications
typedef unsigned short at_error_t;
#define AT_OK 0
#define AT_URC 1
#define AT_ERR_GENERIC 4
// Orbcomm modem error codes
#define AT_ERR_CRC 100
#define AT_ERR_CMD_UNKNOWN 101
#define AT_ERR_CMD_INVALID 102
#define AT_ERR_MSG_SIZE 103
#define AT_ERR_DATA_MODE 104
#define AT_ERR_SYSTEM 105
#define AT_ERR_TX_QUEUE_FULL 106
#define AT_ERR_MSG_NAME_EXISTS 107
#define AT_ERR_GNSS_TIMEOUT 108
#define AT_ERR_MSG_UNAVAILABLE 109
#define AT_ERR_RESOURCE_BUSY 111
#define AT_ERR_READ_ONLY 112

#define AT_ERR_TIMEOUT 255
#define AT_ERR_CRC_CONFIG 254
#define AT_ERR_REENTRANT 253
#define AT_BUSY 252

typedef unsigned short parse_state_t;
#define PARSE_NONE 0
#define PARSE_ECHO 1
#define PARSE_RESPONSE 2
#define PARSE_CRC 3
#define PARSE_OK 4
#define PARSE_ERROR 5
#define PARSE_COMMAND 6

#endif   // AT_CONSTANTS_H
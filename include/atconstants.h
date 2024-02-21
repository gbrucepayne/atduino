#ifndef AT_CONSTANTS_H
#define AT_CONSTANTS_H

#ifndef AT_BAUDRATE
#define AT_BAUDRATE 9600
#endif
#define CHAR_DELAY 10   // milliseconds 
#define AT_TIMEOUT_MS 1000

#define AT_RXBUFFER_MAX_SIZE 16384  // B64 10000 byte payload = 13336 + wrapper
#define AT_TXBUFFER_MAX_SIZE 8192   // B64 6400 byte payload = 8536 + wrapper

#define AT_CR '\r'   // line terminator (default \r)
#define AT_LF '\n'   // response line formatter (default \n)
// Error codes for AT communications
typedef unsigned short at_error_t;
#define AT_OK 0
#define AT_URC 1
#define AT_ERR_GENERIC 4
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

#endif   // AT_CONSTANTS_H
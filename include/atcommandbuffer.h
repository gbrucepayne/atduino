/**
 * @file atcommandbuffer.h
 * @author G.Bruce-Payne (gbrucepayne@hotmail.com)
 * @brief Buffer for AT command/response processing
 * @version 0.1
 * @date 2024-02-13
 * 
 */
#ifndef AT_COMMAND_BUFFER_H
#define AT_COMMAND_BUFFER_H

#include <Arduino.h>
#include <vector>
#include "atdebuglog.h"
#include "atstringutils.h"
#include "crcxmodem.h"
#if defined(__AVR__)
// progmem?
#endif

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

typedef uint8_t parse_state_t;
#define PARSE_NONE 0
#define PARSE_ECHO 1
#define PARSE_RESPONSE 2
#define PARSE_CRC 3
#define PARSE_OK 4
#define PARSE_ERROR 5

/**
 * @brief A buffer class for managing AT command responses
 * 
 */
class AtCommandBuffer {
  private:
    bool echo = true;
    bool verbose = true;
    bool quiet = false;
    bool crc = false;
    char v1PreSuffix[3];
    char vres_ok[7];
    char vres_err[10];
    char res_ok[3];
    char res_err[3];
    #if defined(__AVR__)
    char res_buffer_P[AT_RXBUFFER_MAX_SIZE] PROGMEM;
    #else
    char res_buffer[AT_RXBUFFER_MAX_SIZE];
    #endif
    char pending_command[AT_TXBUFFER_MAX_SIZE];
    bool response_ready = false;
    bool cmd_result_ok = false;
    bool cmd_crc_found = false;
    at_error_t cmd_error = AT_OK;
    bool debug_raw = false;
    void toggleRaw(bool raw);
    bool isRxBufferFull();
    bool setPendingCommand(const char* at_command);
    bool readAtResponse(uint16_t timeout = AT_TIMEOUT_MS);
    parse_state_t parsingOk();
    parse_state_t parsingError();
    parse_state_t parsingShort(uint8_t current);
    void cleanResponse(const char* prefix = nullptr);
    
  public:
    /**
     * @brief Construct a new At Command Buffer object
     * 
     * @param serial The Stream reference associated with the serial port
     */
    AtCommandBuffer(Stream &serial) : serial(serial) {
      snprintf(v1PreSuffix, 3, "%c%c", AT_CR, AT_LF);
      snprintf(vres_ok, 7, "%c%cOK%c%c", AT_CR, AT_LF, AT_CR, AT_LF);
      snprintf(vres_err, 10, "%c%cERROR%c%c", AT_CR, AT_LF, AT_CR, AT_LF);
      snprintf(res_ok, 3, "0%c", AT_CR);
      snprintf(res_err, 3, "4%c", AT_CR);
    };
    
    /**
     * @brief Remove previous data from the serial receive buffer
     */
    void clearRxBuffer();
    
    /**
     * @brief Remove any prior pending AT command
     */
    void clearPendingCommand();
    
    /**
     * @brief Send an AT command on the serial port
     * 
     * @param at_command The AT command to send
     * @return true if accepted
     */
    bool sendAtCommand(const char* at_command, uint16_t timeout = AT_TIMEOUT_MS);
    bool sendAtCommand(const String& at_command, uint16_t timeout = AT_TIMEOUT_MS);

    /**
     * @brief Put the AT command response into a string
     * 
     * @param response The target string
     * @param buffer_size The buffer size of the target string
     */
    void getResponse(char* response, const char* prefix = nullptr,
                     size_t buffer_size = AT_RXBUFFER_MAX_SIZE);
    void getResponse(String& response, const char* prefix);
    String sgetResponse(const char* prefix = nullptr);

    /**
     * @brief Check the serial line for unsolicited data
     * 
     * @param read_until The line terminator (default <cr><lf>)
     * @param timeout The time to wait for terminator in msec
     * @return false if busy processing a command or no data found 
     * @return true if unsolicited data found
     */
    bool checkUrc(const char* read_until=nullptr, time_t timeout = AT_TIMEOUT_MS);

    /**
     * @brief Check if the response or URC is ready for retrieval
    */
    bool responseReady() { return response_ready; }

    /**
     * @brief Get the last error code
    */
    at_error_t lastErrorCode() { return cmd_error; }

    /**
     * @brief Register a callback function for response/URC indicator
    */
    bool setResponseCallback(void (&callback)(at_error_t));

  protected:
    Stream &serial;
    static const size_t rx_buffer_size = AT_RXBUFFER_MAX_SIZE;
    static const size_t tx_buffer_size = AT_TXBUFFER_MAX_SIZE;
    bool reentrant = false;
    uint8_t cmd_parsing = 0;
    bool data_mode = false;
    bool data_mode_echo = false;
    bool readSerialChar(bool ignore_unprintable = false, bool is_locked = false);
    char lastCharRead(size_t n = 1);
    char* commandPtr();
    char* responsePtr();
    void (*cb_ptr)(at_error_t) = nullptr;

};

// void atDebugRxBuffer(bool start = true);

#endif
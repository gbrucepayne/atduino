/**
 * @file atclient.h
 * @author G.Bruce-Payne (gbrucepayne@hotmail.com)
 * @brief Client for AT command/response processing
 * @version 0.1
 * @date 2024-02-13
 * 
 */
#ifndef AT_CLIENT_H
#define AT_CLIENT_H

#include <Arduino.h>
#include <vector>
#include "atdebug.h"
#include "atstringutils.h"
#include "atconstants.h"
#include "crcxmodem.h"
#if defined(__AVR__)
// progmem?
#endif

namespace at {

/**
 * @brief A class for managing client AT command responses
 * 
 */
class AtClient {
  private:
    bool echo = true;
    bool verbose = true;
    bool quiet = false;
    bool crc = false;
    char terminator[3];
    char vres_ok[7];
    char vres_err[10];
    char res_ok[3];
    char res_err[3];
    #if defined(__AVR__)
    char res_buffer_P[AT_CLIENT_RX_BUFFERSIZE] PROGMEM;
    #else
    char res_buffer[AT_CLIENT_RX_BUFFERSIZE];
    #endif
    char pending_command[AT_CLIENT_TX_BUFFERSIZE];
    bool response_ready = false;
    bool cmd_result_ok = false;
    bool cmd_crc_found = false;
    at_error_t cmd_error = AT_OK;
    bool debug_raw = false;
    bool isRxBufferFull();
    bool setPendingCommand(const char* at_command);
    bool readSerialChar(bool ignore_unprintable = true);
    char lastCharRead(size_t n = 1);
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
    AtClient(Stream &serial) : serial(serial) {
      snprintf(terminator, 3, "%c%c", AT_CR, AT_LF);
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
     * @param timeout The timeout in milliseconds (default 1 second)
     * @return true if accepted
     */
    bool sendAtCommand(const char* at_command, uint16_t timeout_ms = AT_TIMEOUT_MS);
    bool sendAtCommand(const String& at_command, uint16_t timeout_ms = AT_TIMEOUT_MS);

    /**
     * @brief Put the AT command response into a string
     * 
     * @param response The target string
     * @param buffer_size The buffer size of the target string
     */
    void getResponse(char* response, const char* prefix = nullptr,
                     size_t buffer_size = AT_CLIENT_RX_BUFFERSIZE);
    void getResponse(String& response, const char* prefix);
    String sgetResponse(const char* prefix = nullptr);

    /**
     * @brief Check the serial line for unsolicited data
     * 
     * @param read_until The line terminator (default <cr><lf>)
     * @param timeout_ms Maximum time to wait for terminator in milliseconds (default 1 second)
     * @return false if busy processing a command or no data found 
     * @return true if unsolicited data found
     */
    bool checkUrc(const char* read_until=nullptr, time_t timeout_ms = AT_TIMEOUT_MS);

    /**
     * @brief Check if the response or URC is ready for retrieval
    */
    bool responseReady() { return response_ready; }

    /**
     * @brief Get the last error code.
     * May be overridden by derived class for modems supporting such a query.
    */
    virtual at_error_t lastErrorCode() { return cmd_error; }

  protected:
    Stream& serial;
    static const size_t rx_buffer_size = AT_CLIENT_RX_BUFFERSIZE;
    static const size_t tx_buffer_size = AT_CLIENT_TX_BUFFERSIZE;
    bool busy = false;
    uint8_t cmd_parsing = 0;
    bool data_mode = false;
    bool data_mode_echo = false;
    void toggleRaw(bool raw);
    char* commandPtr();
    char* responsePtr();

};

}   // namespace at

#endif   // AT_CLIENT_H
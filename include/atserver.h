/**
 * @file atserver.h
 * @author G.Bruce-Payne (gbrucepayne@hotmail.com)
 * @brief Server/host for AT command/response processing
 * @version 0.1
 * @date 2024-02-20
 * 
 */
#ifndef AT_SERVER_H
#define AT_SERVER_H

#include <Arduino.h>
#include <vector>
#include "atdebug.h"
#include "atconstants.h"
#include "atstringutils.h"
#include "crcxmodem.h"

namespace at {

struct AtCommand {
  char name[32];
  void (*read)(void);
  void (*run)(void);
  void (*test)(void);
  at_error_t (*write)(const char* params);
};

/**
 * @brief A class for serving AT responses to a client
*/
class AtServer {
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
    char rx_buffer_P[AT_SERVER_RX_BUFFERSIZE] PROGMEM;
    #else
    char rx_buffer[AT_SERVER_RX_BUFFERSIZE];
    #endif
    char working_buffer[128];
    bool initialized = false;
    parse_state_t parsing = PARSE_NONE;
    at_error_t last_error_code = 0;
    std::vector<AtCommand> commands = {};
    bool readSerialChar(bool ignore_unprintable = true);
    char lastCharRead(size_t n = 1);
    bool isRxBufferFull();
    void clearRxBuffer();
    char* commandPtr();
    bool handleCommand();
  
  protected:
    Stream& serial;
  
  public:
    /**
     * @brief Construct at AT command server using a serial stream
    */
    AtServer(Stream& serial);

    /**
     * @brief Add a command to the supported serving list
     * 
     * @param new_cmd The AtCommand structure
     * @param replace Set to replace any pre-existing command of same name
     * @returns true if no conflict exists
    */
    bool addCommand(AtCommand* new_cmd, bool replace = false);

    /**
     * @brief Check the serial stream for an incoming command to parse
     * 
     * @returns AT_OK (0) if successful or nothing to read
     * @returns an AT error code if any problem parsing the command
    */
    at_error_t readSerial();

    /**
     * @brief Get the currently configured terminator (default \r\n)
     * 
     * @param buffer The character buffer to store in
     * @param buffer_size The size of the buffer (minimum/default 3)
    */
    void getTerminator(char* buffer, unsigned short buffer_size = 3);

    /**
     * @brief Get the OK result string, dependent on verbose setting
     * 
     * @param buffer The character buffer to store in
     * @param buffer_size The size of the buffer (minimum/default 7)
    */
    void getOk(char* buffer, unsigned short buffer_size = 7);

    /**
     * @brief Get the ERROR result string, dependent on verbose setting
     * 
     * @param buffer The character buffer to store in
     * @param buffer_size The size of the buffer (minimum/default 10)
    */
    void getError(char* buffer, unsigned short buffer_size = 10);

    /**
     * @brief Send a string to the serial stream.
     * If both `ok` and `error` are set, ok will take precedence.
     * 
     * @param str The string/char array to send
     * @param ok Set to append the OK result
     * @param error Set to append the ERROR result
    */
    void send(const char* str, bool ok = false, bool error = false);
    void send(String& str, bool ok = false, bool error = false);

    /**
     * @brief Send the OK result, dependent on verbose & crc settings
    */
    void sendOk();

    /**
     * @brief Send the ERROR result, dependent on verbose & crc settings
    */
    void sendError();
};

}   // namespace at

#endif   // AT_SERVER_H
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
#include "cat.h"
#if __INTELLISENSE__
#pragma diag_suppress 144
#endif

namespace at {

typedef int(*write_function)(char);
typedef int(*read_function)(char*);

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
    char serial_command_buffer[AT_SERVER_BUFFER_MAX_SIZE];
    bool initialized = false;
    at_error_t last_error_code = 0;
    write_function w_ptr;
    read_function r_ptr;
    // int writeChar(char c);
    // int readChar(char* c);
    // int (AtServer::*wPtr)(char) = &AtServer::writeChar;
    // int (AtServer::*rPtr)(char*) = &AtServer::readChar;
    // struct cat_io_interface iface = {
    //   .write = &AtServer::writeChar,
    //   .read = &AtServer::readChar,
    // };
    struct cat_io_interface iface;
    std::vector<struct cat_variable> vars = {};
    std::vector<struct cat_command> cmds = {};
    static uint8_t working_buf[128];
    static struct cat_command_group cmd_group;
    std::vector<struct cat_command_group*> cmd_desc = {};
    static struct cat_descriptor desc;
  
  protected:
    Stream& serial;
    struct cat_object cat;
  
  public:
    AtServer(Stream& serial, write_function w_ptr, read_function r_ptr)
      : serial(serial),
        w_ptr(w_ptr),
        r_ptr(r_ptr)
    {
      snprintf(terminator, 3, "%c%c", AT_CR, AT_LF);
      snprintf(vres_ok, 7, "%sOK%s", terminator, terminator);
      snprintf(vres_err, 10, "%sERROR%s", terminator, terminator);
      snprintf(res_ok, 3, "0%c", AT_CR);
      snprintf(res_err, 3, "4%c", AT_CR);
      iface.write = w_ptr;
      iface.read = r_ptr;
    };

    bool begin();

    bool addVar(cat_variable* var);

    bool addCmd(cat_command* cmd);

    int readSerial();

    void getTerminator(char* buffer, unsigned short buffer_size = 3);

    void getOk(char* buffer, unsigned short buffer_size = 7);

    void getError(char* buffer, unsigned short buffer_size = 10);

    void send(const char* str, bool ok = false, bool error = false);

    void sendOk();

    void sendError();
};

}   // namespace at

#endif   // AT_SERVER_H
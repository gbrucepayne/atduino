/**
 * @brief Illustrates use of the AT server with custom commands
*/
#define ARDEBUG_ENABLED
#include <Arduino.h>
#include "atserver.h"

#define MicroSerial Serial2
#define MICRO_BAUD 9600

bool running = false;

at::AtServer host(MicroSerial);

void readHello() {
  char terminator[3];
  host.getTerminator(terminator);
  char outbuffer[100];
  sprintf(outbuffer, "%sHELLO!%s", terminator, terminator);
  MicroSerial.write(outbuffer);
}

void runHello() {
  Serial.println("Running hello");
}

void testHello() {
  char terminator[3];
  host.getTerminator(terminator);
  char outbuffer[100];
  sprintf(outbuffer, "%s(name)%s", terminator, terminator);
  MicroSerial.write(outbuffer);
}

at_error_t writeHello(const char* params) {
  char terminator[3];
  host.getTerminator(terminator);
  char outbuffer[100];
  sprintf(outbuffer, "%sHello, %s!%s", terminator, params, terminator);
  MicroSerial.write(outbuffer);
  return AT_OK;
}

static at::AtCommand hello_cmd = {"+HELLO", readHello, runHello, testHello, writeHello};

void setup() {
  ardebugSetLevel(ARDEBUG_V);
  Serial.begin(115200);
  MicroSerial.begin(MICRO_BAUD);
  delay(3000);
  Serial.println("Starting...");
  running = true;
  host.addCommand(&hello_cmd);
  Serial.println("Added hello");
  Serial.println("Listening...");
  MicroSerial.write("\r\nRDY\r\n");
}

void loop() {
  int result = host.readSerial();
  if (result != 0)
    AR_LOGW("Issue reading serial: %d", result);
}
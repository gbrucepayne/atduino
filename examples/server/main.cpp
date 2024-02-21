/**
 * @brief Broken example - not sure yet why it is crashing on readSerial
*/

#include <Arduino.h>
#include "atserver.h"

#define MicroSerial Serial2
#define MICRO_BAUD 9600

static int serialWrite(char c) {
  MicroSerial.write(c);
  return 1;
}

static int serialRead(char* c) {
  char r = MicroSerial.read();
  if (r > -1) {
    c[0] = r;
    return 1;
  }
  return 0;
}

bool running = false;

at::AtServer host(MicroSerial, serialWrite, serialRead);

static cat_return_state runHello(const struct cat_command *cmd) {
  Serial.println("Running hello");
  return CAT_RETURN_STATE_OK;
}

static cat_command hello_cmd = {
  .name = "+HELLO",
  .description = "A simple demo",
  .run = runHello,
};

static cat_return_state runQuit(const struct cat_command *cmd) {
  Serial.println("Qutting demo");
  return CAT_RETURN_STATE_OK;
}

static cat_command quit_cmd = {
  .name = "+QUIT",
  .description = "Quit the demo",
  .run = runQuit,
};

void setup() {
  LOG_SET_LEVEL(DebugLogLevel::LVL_TRACE);
  Serial.begin(115200);
  MicroSerial.begin(MICRO_BAUD);
  delay(3000);
  Serial.println("Starting...");
  running = true;
  host.addCmd(&hello_cmd);
  Serial.println("Added hello");
  host.addCmd(&quit_cmd);
  Serial.println("Added quit");
}

void loop() {
  Serial.println("This example is broken - don't use");
  delay(3000);
  // if (!running || host.readSerial() != 0)
  //   exit(0);
}
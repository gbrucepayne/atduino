/**
 * @brief Example of AT client querying a modem and listening for
 * unsolicited result codes
*/
// #define ARDEBUG_DISABLED
#include <Arduino.h>
#include "atclient.h"

#define ModemSerial Serial2
#define MODEM_BAUD 9600
#ifndef LED_BUILTIN
#define LED_BUILTIN (2)
#endif

at::AtClient modem(ModemSerial);
size_t command_count = 0;
uint32_t command_interval_ms = 15000;
int max_commands = -1;
int urc_count = 0;
uint32_t start_time = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  ModemSerial.begin(MODEM_BAUD);
  delay(500);
  Serial.println("\r\n>>> Starting basic example");
#ifndef ARDEBUG_DISABLED
  ardebugSetLevel(ARDEBUG_V);
  ardebugBegin(&Serial, nullptr, nullptr);
  Serial.print("ardebug log level:");
  Serial.println(ardebugGetLevel());
#endif
  start_time = millis();
}

void loop() {
  if (modem.checkUrc()) {
    if (!modem.responseReady()) {
      Serial.println("Unable to parse URC");
      return;
    }
    urc_count++;
    String urc = modem.sgetResponse();
    Serial.println("Found unsolicited response: ");
    Serial.println(at::debugString(urc));
    Serial.print("More? ");
    Serial.println(ModemSerial.available());
  } else if ((millis() - start_time) % command_interval_ms == 0) {
    if (ModemSerial.available() == 0 &&
        (max_commands == -1 || command_count < max_commands)) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("\r\nSending basic AT command");
      boolean success = modem.sendAtCommand("AT") == AT_OK;
      if (success) {
        command_count++;
      } else {
        Serial.println("WARNING: Problem sending command");
      }
      Serial.print("Last error code: ");
      Serial.println(modem.lastErrorCode());
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      Serial.println("Serial Rx data pending");
    }
  }
}
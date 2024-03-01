#include <Arduino.h>
#include "atclient.h"

#define ModemSerial Serial2
#define MODEM_BAUD 9600

at::AtClient modem(ModemSerial);
size_t command_count = 0;
int urc_count = 0;
time_t start_time = 0;

void setup() {
  LOG_SET_LEVEL(DebugLogLevel::LVL_TRACE);
  Serial.begin(115200);
  ModemSerial.begin(MODEM_BAUD);
  delay(3000);
  Serial.print("***\nStarting basic example. DebugLogLevel");
  Serial.println((int)LOG_GET_LEVEL());
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
  } else if ((millis() - start_time) % 5000 == 0) {
    if (ModemSerial.available() == 0 && command_count == 0) {
      Serial.println("Sending basic AT command");
      bool success = modem.sendAtCommand("AT");
      if (success) {
        command_count++;
      } else {
        Serial.println("WARNING: Problem sending command");
      }
      Serial.print("Last error code: ");
      Serial.println(modem.lastErrorCode());
    }
  }
}
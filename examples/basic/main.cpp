#include <Arduino.h>
#include "atcommandbuffer.h"

#define ModemSerial Serial2
#define MODEM_BAUD 9600

AtCommandBuffer modem(ModemSerial);
size_t cmdCount = 0;
size_t tick = 1;
String test_command = "AT";

void setup() {
  LOG_SET_LEVEL(DebugLogLevel::LVL_TRACE);
  Serial.begin(115200);
  ModemSerial.begin(MODEM_BAUD);
  delay(3000);
  Serial.println("Starting basic example...");
  Serial.print("DebugLog level: ");
  Serial.println((int)LOG_GET_LEVEL());
}

void loop() {
  while (modem.checkUrc()) {
    Serial.println("Found unsolicited response!");
    while (!modem.responseReady()) {}
    String urc = modem.sgetResponse();
    Serial.println(atDebugString(urc));
  }
  if (cmdCount == 0) {
    ++cmdCount;
    Serial.print("Sending AT command: ");
    Serial.println(test_command);
    if (!modem.sendAtCommand(test_command, 3000)) {
      Serial.print("Could not send AT command - Error code: ");
      Serial.println(modem.lastErrorCode());
    } else {
      while (!modem.responseReady()) {}
      String res = modem.sgetResponse();
      if (res.length() == 0) res = "<No response>";
      Serial.print("\nResponse: ");
      Serial.println(atDebugString(res));
    }
  }
}
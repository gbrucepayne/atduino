#include <Arduino.h>
#include "atcommandbuffer.h"

AtCommandBuffer modem(Serial1);
size_t cmdCount = 0;
String test_command = "AT";

void setup() {
  Serial.begin(115200);
  delay(3000);
}

void loop() {
  while (modem.checkUrc()) {
    while (!modem.responseReady()) {}
    String urc = modem.sgetResponse();
    Serial.println(atDebugString(urc));
  }
  if (cmdCount == 0) {
    if (!modem.sendAtCommand(test_command)) {
      Serial.println("Could not send AT command!");
    } else {
      while (!modem.responseReady()) {}
      String res = modem.sgetResponse();
      Serial.println(atDebugString(res));
      ++cmdCount;
    }
  }
}
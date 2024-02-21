#include <Arduino.h>
#include "atclient.h"

#define ModemSerial Serial2
#define MODEM_BAUD 9600

at::AtClient modem(ModemSerial);
size_t cmdCount = 0;
size_t rx_buffer_size = 0;
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
  size_t new_rx_buffer_size = ModemSerial.available();
  if (new_rx_buffer_size > rx_buffer_size) {
    rx_buffer_size = new_rx_buffer_size;
    char c = ModemSerial.peek();
    Serial.print("UART Rx buffer size: ");
    Serial.print(rx_buffer_size);
    Serial.print(" ; new: ");
    Serial.println(c, HEX);
  }
  if (ModemSerial.available() > 0) {
    char c = ModemSerial.read();
    if (c >= 0) {
      at::printableChar(c, true);
    } else {
      Serial.println("No byte read?");
    }
  }
  // if (modem.checkUrc()) {
    // if (!modem.responseReady()) {
    //   Serial.println("Unable to parse URC");
    //   return;
    // }
    // String urc = modem.sgetResponse();
    // Serial.println("Found unsolicited response: ");
    // Serial.println(debugString(urc));
  //   Serial.print("More? ");
  //   Serial.println(ModemSerial.available());
  // }
  // if (cmdCount == 0) {
  //   ++cmdCount;
  //   Serial.print("Sending AT command: ");
  //   Serial.println(test_command);
  //   if (!modem.sendAtCommand(test_command, 3000)) {
  //     Serial.print("Could not send AT command - Error code: ");
  //     Serial.println(modem.lastErrorCode());
  //   } else {
  //     while (!modem.responseReady()) {}
  //     String res = modem.sgetResponse();
  //     if (res.length() == 0) res = "<No response>";
  //     Serial.print("\nResponse: ");
  //     Serial.println(debugString(res));
  //   }
  // }
}
# Developer Notes

I initially got a ESP32-WROOM-32 CP2102 from KeeYees on Amazon.
Some time later I got the ESP-PROG board hoping to plug n play with the
KeeYees but no such luck. Seems some WROOM chips disable JTAG support by
repurposing pins and require custom firmware builds,
but other variants just need the right configuration.

I had best intentions of also being able to run on a smaller AVR board like
a Nano ATMEGA128, but was limited by the performance of SoftwareSerial.
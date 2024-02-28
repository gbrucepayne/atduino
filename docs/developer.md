# Developer Notes

This project initially started as a follow-on to some work-related
skunkworks projects I was doing around interfacing with satellite IoT modems.
My initial work was done in Python on Linux boards like Raspberry Pi.
However in many cases the newer modems will be physically integrated with
microcontrollers and tend to want to have significant power savings in
production design.

There are several AT command projects/libraries within the community but none
quite fit what I needed.

I initially got a ESP32-WROOM-32 CP2102 from KeeYees on Amazon.
Some time later I got the ESP-PROG board hoping to plug n play with the
KeeYees but no such luck. Distractions reading that some WROOM chips disable
JTAG support by repurposing pins and require custom firmware builds...this
turned out to be a red herring and just needed the right
`platformio.ini` configuration.

I had best intentions of also being able to run on a smaller AVR board like
a Nano ATMEGA128, but was limited by the performance of `SoftwareSerial`,
which has a bunch of known issues within the community.

Ideally the library can also be set up to support the ESP-IDF framework,
though a bit too complex for my level as of Feb 2024. Other longer-term
aspirations include use with other microcontrollers like STM32 or RP2040.

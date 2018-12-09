# Firmware for trump.me badge

Code is written in C using Atmel Studio 7.0 which is free to download and use.

The target processor is an ATtiny814, but will also work with the 414, 614, 1614

I am using an Atmel ICE programmer, these chips use the UPDI programming interface
and do not currently have any board support packages for Arduino.

The 814 was chosen for this project because they are really cheap (like $0.35USD) and
contain a lot of periphrials including a 12bit DAC which is used to drive the
speaker / amplifier currently
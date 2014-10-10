LinuxCNC HAL driver for Teensy 3.X, with matching Teensy firmware
=================================================================

This project provides software for plugging a Teensy 3.x into LinuxCNC's
HAL.  It supports digital inputs and outputs, analog input, analog output
(via the Teensy DAC) and PWM output (via the Teensy "analog output" pins).

This project was forked from apmorton's teensy-template,
https://github.com/apmorton/teensy-template


Using
-----

Install the Teensy udev rule: `sudo cp tools/49-teensy.rules /etc/udev/rules.d/`

Then unplug your Teensy and plug it back in.

Possibly edit the TEENSY variable in the Makefile to set your Teensy
version (3.0 or 3.1).

Run `make upload` to compile the firmware and upload it to the Teensy.

Run `./hal-teensy` to start the hal driver (linuxcnc must be running
for this to work).


Protocol
--------

The Teensy and the PC trade packets with 3 bits of address and 16 bits
of data, using this format:

    byte 0:  0 A02 A01 A00  D15 D14 D13 D12
    byte 1:  1   0 D11 D10  D09 D08 D07 D06
    byte 2:  1   1 D05 D04  D03 D02 D01 D00

The leading bits make the protocol self-synchronizing.  Inspired by Jeff
Epler's Arduino/HAL protocol: http://emergent.unpythonic.net/01198594294

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

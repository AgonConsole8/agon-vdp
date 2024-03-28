# agon-vdp

Part of the official firmware for the [Agon Console8](https://www.heber.co.uk/agon-console8)

This firmware is intended for use on all Agon Light hardware.  It is primarily developed and tested on the Agon Console8, but is also tested on the Olimex Agon Light 2 and is fully compatible with all Agon Light compatible computers.

This version of agon-vdp may differ from the [Quark firmware](https://github.com/breakintoprogram/agon-vdp) and contain some extensions.  Software written for the Quark firmware should be fully compatible with this version.

### What is the Agon

Agon is a modern, fully open-source, 8-bit microcomputer and microcontroller in one small, low-cost board. As a computer, it is a standalone device that requires no host PC: it puts out its own video (VGA), audio (2 identical mono channels), accepts a PS/2 keyboard and has its own mass-storage in the form of a µSD card.  The Agon Console8 adds to this a second PS/2 port for attaching a keyboard, and two Atari-style joystick ports.

https://www.thebyteattic.com/p/agon.html

### What is the VDP

The VDP is a serial graphics terminal that takes a BBC Micro-style text output stream as input. The output is via the VGA connector on the Agon.

It will process any valid BBC BASIC VDU commands (starting with a character between 0 and 31).

For example:

`VDU 25, mode, x; y;` is equivalent to the BBC BASIC statement `PLOT mode, x, y`

The VDP also handles the interfaces to the PS/2 keyboard and mouse, and the audio output.

### Documentation

The AGON documentation can now be found on the [Community Documentation](https://agonconsole8.github.io/agon-docs/) site.  This site provides extensive documentation about the Agon platform firmware, covering both Quark and Console8 firmware releases.

### Building

This project makes use of [PlatformIO](https://platformio.org) to build the firmware.  Using the PlatformIO IDE with Visual Studio Code is recommended, but it is also possible to use the PlatformIO CLI.

Previously, it was also possible to use the Arduino IDE to build the firmware, but this is no longer supported.  This is because this project makes use of an updated version of vdp-gl, which is not directly able to be used with the Arduino IDE.  (It is technically still possible to use the Arduino IDE, but it is not recommended, as you would need to manually download the applicable vdp-gl version.)

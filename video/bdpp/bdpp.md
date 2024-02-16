# Bidirectional Packet Protocol (BDPP)

The Agon, in all of its derivatives, runs by cooperation between its EZ80 CPU
and its ESP32 CPU. While the former runs the main application program, the
latter provides peripheral support for video, audio, keyboard, and mouse.

The EZ80 (MOS) talks to the ESP32 (VDP) through a direct serial link, at 1152000 baud,
with the EZ80 using its UART0 peripheral, and the ESP32 using its UART2 peripheral.

Traditionally, and still by default, the EZ80 performs serial input by handling
UART0 receiver interrupts, but does not use interrupts for data transmission.
The ESP32 uses the Arduino Serial2 driver, which implies using interrupts for
its receiver and its transmitter; however, it does not make use of hardware DMA.

Regarding data content, the EZ80 sends a stream of VDU commands and printable
characters to the ESP32, with no special wrappers around those characters. Each
character output by statements such as VDU, PRINT, PLOT, RST &10, and RST &18
is output directly to the UART0 transmitter holding register in a synchronous
manner. The application waits for each byte to be sent out before writing the
next data byte to that register.

When the ESP32 sends data, such as keyboard key state changes, to the EZ80, it
sends small data packets, which are interpreted by MOS in the EZ80. MOS modifies
certain system variables according to the contents of those small packets. The
packets are limited to 16 bytes in size.

Some small packets are sent by the ESP32 in response to specific requests. For
example, MOS may ask the VDP for the current cursor position on the screen.
The VDP will reply with a small packet containing that information, and MOS then
stores the results into system variables, so that the EZ80 application can
read those variables to obtain the cursor position.

The EZ80 application has no knowledge of the small packets, and no ability to
get data from the ESP32 in larger quantities. If, for instance, the application
wanted to read the screen buffer contents, it would have to do so by asking for
the color of each pixel, one-by-one.

## Purpose of BDPP

The bidirectional packet protocol (BDPP) is designed to change how the communication
between the EZ80 (MOS) and the ESP32 (VDP) works, in order to make it more flexible
and less taxing on the CPUs (i.e., to give the CPUs more time to do other things
than to handle the data transfers). BDPP involves these topics:

* <i>Bidirectional packet flow</i> - This involves data transfers in both directions using potentially larger data packets than the Agon uses by default.
* <i>Multiple stream support</i> - With multiple stream processors (VDU command interpreters) on the ESP32 end, the application can work on multiple jobs on the EZ80 end, and intermingle
any corresponding data packets required for those jobs. For example, the application can
load a large image or set of images from a file (or files), and send those to VDP stream buffers, while updating a progress indicator on the screen.
* <i>Automatic packet division</i> - The output from statements such as VDU, PRINT, PLOT,
RST &10, and RST &18 can be divided into packets automatically, and transmitted to VDP
when the packets get full. Non-full packets can also be flushed (output) on purpose.
* <i>Driver-owned packets</i> - Automatic packet division is performed using driver-owned packets, which are managed by the BDPP driver, and can contain up to 32 bytes of data
in each packet.
* <i>App-owned packets</i> - The application can allocate memory space for large packets, and
tell the BDPP driver that it would like to transmit data from or receive data into
such packets. Each packet can contain up to 4072 bytes of data.

## Packet Flow Diagram

## Creating Driver-owned Packets

## Transmitting Driver-owned Packets

## Receiving Driver-owned Packets

## Processing Driver-owned Packets

## Creating App-owned Packets

## Transmitting App-owned Packets

## Receiving App-owned Packets

## Processing App-owned Packets

## EZ80 Function Call Tree

## ESP32 Function Call Tree

## BDPP VDU Commands

VDU 23, 31, 1 : Get Memory Statistics

VDU 23, 31, 2, addressLo; addressHi; count; : Read Memory Area

VDU 23, 31, 3, addressLo; addressHi; count; data0, ... : Write Memory Area

VDU 23, 31, 4, addressLo; addressHi; count; data : Fill Memory Area

VDU 23, 31, 5, count; : Allocate  DRAM

VDU 23, 31, 6, count; : Allocate IRAM

VDU 23, 31, 7, count; : Allocate PSRAM

VDU 23, 31, 8, addressLo; addressHi; : Free DRAM

VDU 23, 31, 9 , addressLo; addressHi; : Free IRAM

VDU 23, 31, 10, addressLo; addressHi; : Free PSRAM

VDU 23, 31, 11, addressLo; addressHi; : Execute Code Function

VDU 23, 31, 12, addressLo; addressHi; count; data0, ...: Execute Code Function with Params

VDU 23, 31, 13, count; data0,  ... : Echo Data


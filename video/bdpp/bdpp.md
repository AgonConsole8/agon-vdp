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
The VDP will reply with a small packet containing that information, and again, MOS
stores the results into system variables, so that the EZ80 application can
read those variables to obtain the cursor position.

The EZ80 application has no direct knowledge of the small packets, and no ability to
get data from the ESP32 in larger quantities. If, for instance, the application
wanted to read the screen buffer contents, it would have to do so by asking for
the color of each pixel, one-by-one.

## Purpose of BDPP

The bidirectional packet protocol (BDPP) is designed to change how the communication
between the EZ80 (MOS) and the ESP32 (VDP) works, in order to make it more flexible
and less taxing on the CPUs (i.e., to give the CPUs more time to do other things
than to handle the data transfers). BDPP involves these topics:

* <i>Bidirectional packet flow</i> - This means data transfers in both directions can use potentially larger data packets than the Agon uses by default.
* <i>Multiple stream support</i> - With multiple stream processors (VDU command interpreters) on the ESP32 end, the application can work on multiple jobs on the EZ80 end, and intermingle
any corresponding data packets required for those jobs. For example, the application can
load a large image or set of images from a file (or files), and send those to VDP stream buffers, while updating a progress indicator on the screen.
* <i>Automatic packet division</i> - The output from statements such as VDU, PRINT, PLOT,
RST &10, and RST &18 can be divided into packets automatically, and transmitted to VDP
when the packets get full. Non-full packets can also be flushed (output) on purpose.
* <i>Driver-owned packets</i> - Automatic packet division is performed using driver-owned packets, which are managed by the BDPP driver, and can contain up to 32 bytes of _data_
in each packet, not counting packet overhead.
* <i>App-owned packets</i> - The application can allocate memory space for large packets, and
tell the BDPP driver that it would like to transmit data from or receive data into
such packets. Each packet can contain up to 256 bytes of _data_.

## Hardware versus Software

In the EZ80, BDPP uses the UART0 hardware FIFOs, and related TX and RX interrupts, but the TX and RX packet state machines are managed by software in the interrupt handler. The EZ80 UART does not use DMA, so the CPU may be interrupted often, as bytes go out or come in.

In the ESP32, BDPP uses the UART2 hardware in conjunction with the UHCI0 DMA controller. UHCI manages the UART FIFOs, and interrupts the CPU after an entire packet has come in or gone out. Thus, the TX and RX packet state machines are managed by hardware. That is why the packet structure described below is important.

## Packet Structure

Each BDPP packet has the following physical structure:

Start Marker - 0x89, indicates the start of a new packet in the serial stream.

Flags - a single byte with these bits:

Indexes - a single byte with the packet index (0..15) in the lower nibble, and the stream index (0..15) in the upper nibble.

Size - a single byte (1..255, or 0) designating the size (1..255, or 256) of the data portion of the packet. If the size byte is zero, it means the actual data size is 256.
Otherwise, the actual data size equals the given size.

Data - contains whatever print data, VDU commands, bitmap data, keyboard data, mouse data, etc., are appropriate.

End Marker - 0x89, indicates the end of the new packet in the serial stream.

The applications on either end do not use or care about the markers. They are handled by BDPP.

Because the bytes between the two markers _might_ equal the value of the markers (0x89), the packet protocol supports using escape sequences. The value 0x89 is encoded as 0x8B, 0x8A. To prevent misinterpreting a real value of 0x8B, that value is encoded as 0x8B, 0x8D.

For example, the following packet information:

Flags: 1C<br>
Indexes: 62<br>
Size: 05<br>
Data: <b>03 89 E4 8B C7</b><br>

Would be marked, encoded, and transferred as:

89 1C 62 05 <b>03 8B 8A E4 8B 8D C7</b> 89

It would be decoded into:

1C 62 05 <b>03 89 E4 8B C7</b>

In the EZ80, the software state machines in the interrupt handler take care of marking, encoding, and decoding, so the EZ80 application does not need to do so. In the ESP32, the UHCI hardware does this, and the ESP32 application is unaware of it.

## Enabling BDPP

On startup (reboot), neither MOS nor VDP are in BDPP (packet) mode. They communicate
using the existing (legacy) methods. MOS uses UART0 interrupts for receiving data
from the UART FIFO, but does not use interrupts for transmitting data. VDP uses the
Arduino Serial2 stream, which is interrupt-driven, but may interrupt the CPU often,
as characters arrive or depart. Also, the data transfers are in raw (unpacketed)
mode. For example, "VDU a, b" sends just 2 bytes (a and b). The a and b values are
not encapsulated within a packet.

The firmware in <i>both</i> CPUs must support BDPP in order for BDPP mode to be allowed. This is determined during the startup handshake, where the version bytes
are shared between the CPUs. Just because BDPP is allowed, does not imply that it
is <i>enabled</i>, and on startup, it is disabled.

If both firmwares support it,
BDPP may be enabled by executing the "bdpp" command in MOS, either at the MOS command prompt, at the BASIC command prompt via "*bdpp", or in the "autoexec.txt" file.
Once BDPP is enabled, <i>it cannot be disabled,</i> except by resetting (rebooting) the Agon.

When the "bdpp" command is executed, MOS will send a VDU command (equivalent to
VDU 23,0,&A2) to VDP, telling VDP to switch into BDPP (packet) mode. MOS will then wait a short time before sending any other data to VDP, because once that switch is
made, all communication is in packet mode, not the default raw mode.

## Packet Flow Diagram

## Driver-owned Packets

Driver-owned packets are limited to 32 bytes of _data_, not including the packet
overhead bytes, or any special escape characters (inserted and removed by the drivers).
When MOS uses printf(), putch(), RST &10, or RST &18, or when BASIC uses PRINT,
VDU, PLOT, etc., or those RST calls, driver-owned packets are used automatically. Packet are obtained (from the free packet list), filled, queued for
transmission, transmitted, and returned to the free packet list.

Since packet data are buffered in driver-owned packets, the packets are not queued for transmission
until they are filled or flushed. MOS has various points in its code where it flushes
packets, in case data bytes are waiting to go out, but have not yet been sent. When
BDPP is disabled, the above functions and commands send data bytes to VDP immediately,
or at least relatively soon. When BDPP is enabled, data bytes are buffered, and at some
point, need to be flushed. For those cases where neither MOS nor BASIC (nor any other language for apps) knows what the programmer intends, and thus do not know when to flush, BDPP publicizes its flush functions, so that an app can flush intentionally. 

## Creating Driver-owned Packets

### <i>From within EZ80 MS itself (in C)</i>

### <i>From within an EZ80 BASIC app</i>

### <i>From within an EZ80 C app</i>

### <i>From within an EZ80 Assembler app</i>

### <i>From within ESP32 VDP</i>

## Transmitting Driver-owned Packets

### <i>From within EZ80 MS itself (in C)</i>

### <i>From within an EZ80 BASIC app</i>

### <i>From within an EZ80 C app</i>

### <i>From within an EZ80 Assembler app</i>

### <i>From within ESP32 VDP</i>

## Receiving Driver-owned Packets

## Processing Driver-owned Packets

## Application-owned Packets

Application-owned packets differ from driver-owned packets in two primary ways.
First, they are managed by the application, not by the driver. Second, they can contain
up to 256 data bytes, as opposed to just 32 data bytes.

The application is responsible for allocating, and possibly deallocating, memory for app-owned packets. The app tells BDPP what to do with the app-owned packets, because
the app is in charge; the driver is not.

## Creating App-owned Packets

### <i>From within an EZ80 BASIC app</i>

### <i>From within an EZ80 C app</i>

### <i>From within an EZ80 Assembler app</i>

### <i>From within ESP32 VDP</i>

## Transmitting App-owned Packets

### <i>From within an EZ80 BASIC app</i>

### <i>From within an EZ80 C app</i>

### <i>From within an EZ80 Assembler app</i>

### <i>From within ESP32 VDP</i>

## Receiving App-owned Packets

### <i>From within an EZ80 BASIC app</i>

### <i>From within an EZ80 C app</i>

### <i>From within an EZ80 Assembler app</i>

### <i>From within ESP32 VDP</i>

## Processing App-owned Packets

### <i>From within an EZ80 BASIC app</i>

### <i>From within an EZ80 C app</i>

### <i>From within an EZ80 Assembler app</i>

### <i>From within ESP32 VDP</i>

## EZ80 Function Call Tree

## ESP32 Function Call Tree

## BDPP VDU Commands


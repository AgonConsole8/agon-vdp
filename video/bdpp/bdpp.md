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
```
#define BDPP_PKT_FLAG_PRINT				0x00	// Indicates packet contains printable data
#define BDPP_PKT_FLAG_COMMAND			0x01	// Indicates packet contains a command or request

#define BDPP_PKT_FLAG_RESPONSE			0x02	// Indicates packet contains a response

#define BDPP_PKT_FLAG_FIRST				0x04	// Indicates packet is first part of a message
#define BDPP_PKT_FLAG_MIDDLE			0x00	// Indicates packet is middle part of a message
#define BDPP_PKT_FLAG_LAST				0x08	// Indicates packet is last part of a message

#define BDPP_PKT_FLAG_ENHANCED			0x10	// Indicates packet is enhanced in some way

#define BDPP_PKT_FLAG_DONE				0x20	// Indicates packet was transmitted or received

#define BDPP_PKT_FLAG_FOR_RX			0x40	// Indicates packet is for reception, not transmission

#define BDPP_PKT_FLAG_DRIVER_OWNED		0x00	// Indicates packet is owned by the driver
#define BDPP_PKT_FLAG_APP_OWNED			0x80	// Indicates packet is owned by the application
```

Indexes - a single byte with the packet index (0..15) in the lower nibble, and the stream index (0..15) in the upper nibble.

Size - a single byte (1..255, or 0) designating the size (1..255, or 256) of the data portion of the packet. If the size byte is zero, it means the actual data size is 256.
Otherwise, the actual data size equals the given size.

Data - contains whatever print data, VDU commands, bitmap data, keyboard data, mouse data, etc., are appropriate.

End Marker - 0x89, indicates the end of the new packet in the serial stream.

The applications on either end do not use or care about the markers. They are handled by BDPP.

Because the bytes between the two markers _might_ equal the value of the markers (0x89), the packet protocol supports using escape sequences. The value 0x89 is encoded as 0x8B, 0x8A. To prevent misinterpreting a real value of 0x8B, that value is encoded as 0x8B, 0x8D.

For example, the following packet information:

Flags: 0C<br>
Indexes: 62<br>
Size: 05<br>
Data: <b>03 89 E4 8B C7</b><br>

Would be marked, encoded, and transferred as:

89 0C 62 05 <b>03 8B 8A E4 8B 8D C7</b> 89

It would be decoded into:

0C 62 05 <b>03 89 E4 8B C7</b>

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
VDU 23,0,&A2,0) to VDP, telling VDP to switch into BDPP (packet) mode. MOS will then wait a short time before sending any other data to VDP, because once that switch is
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

## BDPP Functions in MOS

The following functions are compiled into the MOS source code,
and are available within MOS by direct function calls. Most
are available outside of MOS via the BDPP API, by using RST &20.
Many of the "foreground"
functions are called automatically when using driver-owned
packets via calls to printf(), putch(), RST &10, or RST &18.

### <i>*** OVERALL MANAGEMENT FROM FOREGROUND ***</i>
NOTE: These functions will disable and enable interrupts as needed in order
to protect certain global variables managed by BDPP. Do not call any of
these functions with interrupts turned OFF!
```
// Initialize the BDPP driver.
void bdpp_fg_initialize_driver();

// Get whether BDPP is allowed (both CPUs have it)
// [BDPP API function code 0x00, signature 1]
BOOL bdpp_fg_is_allowed();

// Get whether BDPP is presently enabled
// [BDPP API function code 0x01, signature 1]
BOOL bdpp_fg_is_enabled();

// Get whether BDPP is presently busy (TX or RX)
// [BDPP API function code 0x10, signature 1]
BOOL bdpp_fg_is_busy();

// Enable BDDP mode for a specific stream
// [BDPP API function code 0x02, signature 2]
BOOL bdpp_fg_enable(BYTE stream);

// Disable BDDP mode
// [BDPP API function code 0x03, signature 1]
BOOL bdpp_fg_disable();
```

### <i>*** PACKET RECEPTION (RX) FROM FOREGROUND (MAIN THREAD) ***</i>
NOTE: These functions will disable and enable interrupts as needed in order
to protect certain global variables managed by BDPP. Do not call any of
these functions with interrupts turned OFF!
```
// Prepare an app-owned packet for reception
// This function can fail if the packet is presently involved in a data transfer.
// The given size is a maximum, based on app memory allocation, and the
// actual size of an incoming packet may be smaller, but not larger.
// [BDPP API function code 0x05, signature 6]
BOOL bdpp_fg_prepare_rx_app_packet(BYTE index, BYTE* data, WORD size);

// Check whether an incoming app-owned packet has been received
// [BDPP API function code 0x07, signature 2]
BOOL bdpp_fg_is_rx_app_packet_done(BYTE index);

// Get the flags for a received app-owned packet.
// [BDPP API function code 0x08, signature 2]
BYTE bdpp_fg_get_rx_app_packet_flags(BYTE index);

// Get the data size for a received app-owned packet.
// [BDPP API function code 0x09, signature 8]
WORD bdpp_fg_get_rx_app_packet_size(BYTE index);
```

### <i>*** PACKET TRANSMISSION (TX) FROM FOREGROUND (MAIN THREAD) ***</i>
NOTE: These functions will disable and enable interrupts as needed in order
to protect certain global variables managed by BDPP. Do not call any of
these functions with interrupts turned OFF!
```
// Initialize an outgoing driver-owned packet, if one is available
// Returns NULL if no packet is available
volatile BDPP_PACKET* bdpp_fg_init_tx_drv_packet(BYTE flags, BYTE stream);

// Queue an app-owned packet for transmission
// The packet is expected to be full when this function is called.
// This function can fail if the packet is presently involved in a data transfer.
// [BDPP API function code 0x04, signature 7]
BOOL bdpp_fg_queue_tx_app_packet(BYTE indexes, BYTE flags, const BYTE* data, WORD size);

// Check whether an outgoing app-owned packet has been transmitted
// [BDPP API function code 0x06, signature 2]
BOOL bdpp_fg_is_tx_app_packet_done(BYTE index);

// Free the driver from using an app-owned packet
// This function can fail if the packet is presently involved in a data transfer.
// [BDPP API function code 0x0A, signature 2]
BOOL bdpp_fg_stop_using_app_packet(BYTE index);

// Start building a driver-owned, outgoing packet.
// If there is an existing packet being built, it will be flushed first.
// This returns NULL if there is no packet available.
volatile BDPP_PACKET* bdpp_fg_start_drv_tx_packet(BYTE flags, BYTE stream);

// Append a data byte to a driver-owned, outgoing packet.
// This is a blocking call, and might wait for room for data.
// [BDPP API function code 0x0B, signature 4]
void bdpp_fg_write_byte_to_drv_tx_packet(BYTE data);

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a blocking call, and might wait for room for data.
// [BDPP API function code 0x0C, signature 5]
void bdpp_fg_write_bytes_to_drv_tx_packet(const BYTE* data, WORD count);

// Append a single data byte to a driver-owned, outgoing packet.
// This is a potentially blocking call, and might wait for room for data.
// If necessary this function initializes and uses a new packet. It
// decides whether to use "print" data (versus "non-print" data) based on
// the value of the data. To guarantee that the packet usage flags are
// set correctly, be sure to flush the packet before switching from "print"
// to "non-print", or vice versa.
// [BDPP API function code 0x0D, signature 4]
void bdpp_fg_write_drv_tx_byte_with_usage(BYTE data);

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a potentially blocking call, and might wait for room for data.
// If necessary this function initializes and uses additional packets. It
// decides whether to use "print" data (versus "non-print" data) based on
// the first byte in the data. To guarantee that the packet usage flags are
// set correctly, be sure to flush the packet before switching from "print"
// to "non-print", or vice versa.
// [BDPP API function code 0x0E, signature 5]
void bdpp_fg_write_drv_tx_bytes_with_usage(const BYTE* data, WORD count);

// Flush the currently-being-built, driver-owned, outgoing packet, if any exists.
// [BDPP API function code 0x0F, signature 3]
void bdpp_fg_flush_drv_tx_packet();
```

### <i>*** OVERALL MANAGEMENT FROM BACKGROUND ***</i>
NOTE: These functions expect interrupts to be turned off already,j
and may modify certain global variables managed by BDPP. Do not call
any of these functions with interrupts turned ON!
```
// Get whether BDPP is presently busy (TX or RX)
// [BDPP API function code 0x1D, signature 1]
BOOL bdpp_bg_is_busy();
```

### <i>*** PACKET RECEPTION (RX) FROM BACKGROUND (ISR) ***</i>
NOTE: These functions expect interrupts to be turned off already,j
and may modify certain global variables managed by BDPP. Do not call
any of these functions with interrupts turned ON!
```
// Initialize an incoming driver-owned packet, if one is available
// Returns NULL if no packet is available
volatile BDPP_PACKET* bdpp_bg_init_rx_drv_packet();

// Prepare an app-owned packet for reception
// This function can fail if the packet is presently involved in a data transfer.
// The given size is a maximum, based on app memory allocation, and the
// actual size of an incoming packet may be smaller, but not larger.
// [BDPP API function code 0x12, signature 6]
BOOL bdpp_bg_prepare_rx_app_packet(BYTE index, BYTE* data, WORD size);

// Check whether an incoming app-owned packet has been received
// [BDPP API function code 0x14, signature 2]
BOOL bdpp_bg_is_rx_app_packet_done(BYTE index);

// Get the flags for a received app-owned packet.
// [BDPP API function code 0x15, signature 2]
BYTE bdpp_bg_get_rx_app_packet_flags(BYTE index);

// Get the data size for a received app-owned packet.
// [BDPP API function code 0x16, signature 8]
WORD bdpp_bg_get_rx_app_packet_size(BYTE index);
```

### <i>*** PACKET TRANSMISSION (TX) FROM BACKGROUND (ISR) ***</i>
NOTE: These functions expect interrupts to be turned off already,j
and may modify certain global variables managed by BDPP. Do not call
any of these functions with interrupts turned ON!
```
// Initialize an outgoing driver-owned packet, if one is available
// Returns NULL if no packet is available
volatile BDPP_PACKET* bdpp_bg_init_tx_drv_packet(BYTE flags, BYTE stream);

// Queue an app-owned packet for transmission
// The packet is expected to be full when this function is called.
// This function can fail if the packet is presently involved in a data transfer.
// [BDPP API function code 0x11, signature 7]
BOOL bdpp_bg_queue_tx_app_packet(BYTE indexes, BYTE flags, const BYTE* data, WORD size);

// Check whether an outgoing app-owned packet has been transmitted
// [BDPP API function code 0x13, signature 2]
BOOL bdpp_bg_is_tx_app_packet_done(BYTE index);

// Free the driver from using an app-owned packet
// This function can fail if the packet is presently involved in a data transfer.
// [BDPP API function code 0x17, signature 2]
BOOL bdpp_bg_stop_using_app_packet(BYTE index);

// Start building a driver-owned, outgoing packet.
// If there is an existing packet being built, it will be flushed first.
// This returns NULL if there is no packet available.
volatile BDPP_PACKET* bdpp_bg_start_drv_tx_packet(BYTE flags, BYTE stream);

// Append a data byte to a driver-owned, outgoing packet.
// This is a blocking call, and might wait for room for data.
// [BDPP API function code 0x18, signature 4]
void bdpp_bg_write_byte_to_drv_tx_packet(BYTE data);

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a blocking call, and might wait for room for data.
// [BDPP API function code 0x19, signature 5]
void bdpp_bg_write_bytes_to_drv_tx_packet(const BYTE* data, WORD count);

// Append a single data byte to a driver-owned, outgoing packet.
// This is a potentially blocking call, and might wait for room for data.
// If necessary this function initializes and uses a new packet. It
// decides whether to use "print" data (versus "non-print" data) based on
// the value of the data. To guarantee that the packet usage flags are
// set correctly, be sure to flush the packet before switching from "print"
// to "non-print", or vice versa.
// [BDPP API function code 0x1A, signature 4]
void bdpp_bg_write_drv_tx_byte_with_usage(BYTE data);

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a potentially blocking call, and might wait for room for data.
// If necessary this function initializes and uses additional packets. It
// decides whether to use "print" data (versus "non-print" data) based on
// the first byte in the data. To guarantee that the packet usage flags are
// set correctly, be sure to flush the packet before switching from "print"
// to "non-print", or vice versa.
// [BDPP API function code 0x1B, signature 5]
void bdpp_bg_write_drv_tx_bytes_with_usage(const BYTE* data, WORD count);

// Flush the currently-being-built, driver-owned, outgoing packet, if any exists.
// [BDPP API function code 0x1C, signature 3]
void bdpp_bg_flush_drv_tx_packet();
```

## BDPP API in MOS

On the EZ80, most BDPP functions may be access via the BDPP API, using RST &20.
The function codes for the available functions are listed in the comments above.
Functions are grouped by their type signatures, making it possible to reuse
assembler code, order to call functions with similar type signatures.

The various BDPP API functions use some or all of the following EZ80
registers as inputs and/or outputs.
In other words, set these registers before using the RST &20 instruction. The "BOOL"
and "BYTE" types (seen above) are both 8 bits in size, so some functions that return
"BOOL" use the same type signature as similar functions that return "BYTE".

Inputs:<br>
IX: Data address<br>
IY: Size of buffer or Count of bytes<br>
D: Data byte<br>
C: Packet Flags<br>
B: Packet/Stream Index(es)<br>
A: BDPP function code<br>

Outputs:<br>
C: BOOL indicator<br>
BC: WORD size<br>

The following sections detail each type signature:

### Function type signature 1
BOOL fcn();
BYTE fcn();
```
```

### Function type signature 2
BOOL fcn(BYTE index);
BYTE fcn(BYTE index);
```
```

### Function type signature 3
void fcn();
```
```

### Function type signature 4
void fcn(BYTE data);
```
```

### Function type signature 5
void fcn(const BYTE* data, WORD size);
```
```

### Function type signature 6
BOOL fcn(BYTE index, BYTE* data, WORD size);
```
```

### Function type signature 7
BOOL fcn(BYTE indexes, BYTE flags, const BYTE* data, WORD size);
```
```

### Function type signature 8
WORD fcn(BYTE index);
```
```

## BDPP Functions in VDP

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


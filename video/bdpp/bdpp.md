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
BDPP may be enabled by executing the "bdpp" command in MOS, either at the MOS command prompt, or in the "autoexec.txt" file.
BDPP may be enabled at the BASIC command prompt via "*bdpp".
From within a BASIC program, BDPP may be enabled via the OSCLI statement,
with a parameter of "bdpp".

NOTE: Once BDPP is enabled, <i>it cannot be disabled,</i> except by resetting (rebooting) the Agon.

When the "bdpp" command is executed, MOS will send a VDU command (equivalent to
VDU 23,0,&A2,0) to VDP, telling VDP to switch into BDPP (packet) mode. MOS will then wait a short time before sending any other data to VDP, because once that switch is
made, all communication is in packet mode, not the default raw mode.

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

## Application-owned Packets

Application-owned packets differ from driver-owned packets in two primary ways.
First, they are managed by the application, not by the driver. Second, they can contain
up to 256 data bytes, as opposed to just 32 data bytes.

The application is responsible for allocating, and possibly deallocating, memory for app-owned packets. The app tells BDPP what to do with the app-owned packets, because
the app is in charge; the driver is not.

Typically, app-owned packets will be used to receive response data from VDP. It is
essential for each VDU request command that will respond with data to include a
packet index (0..15) in the request parameters, so that VDU will know which packet
index to provide in the response.

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
NOTE: These functions expect interrupts to be turned off already,
and may modify certain global variables managed by BDPP. Do not call
any of these functions with interrupts turned ON!
```
// Get whether BDPP is presently busy (TX or RX)
// [BDPP API function code 0x1D, signature 1]
BOOL bdpp_bg_is_busy();
```

### <i>*** PACKET RECEPTION (RX) FROM BACKGROUND (ISR) ***</i>
NOTE: These functions expect interrupts to be turned off already,
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
NOTE: These functions expect interrupts to be turned off already,
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

On the EZ80, most BDPP functions may be accessed via the BDPP API, using RST &20.
The function codes for the available functions are listed in the comments above.
Functions are grouped by their type signatures, making it possible to reuse
assembler code, order to call functions with similar type signatures.

The RST &20 interfaces to various BDPP API functions use some or all of the following EZ80
registers as inputs and/or outputs.
In other words, set the appropriate input registers before using the RST &20 instruction, and check the proper output register, if any, after using RST. The "BOOL"
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

The following sections detail each type signature, and provide sample RST calls in
BASIC-24 format, but without the line numbers. Also shown are some common definitions
that are used throughout the examples. It is assumed that this code runs in EZ80 ADL
(24-bit registers) mode.

### Common definitions

This code reserves space for parameter variables that can be set as needed in
the BASIC app, and will be read in the assembler code further below, to setup
paramer registers for the RST calls. You can omit the PRINT statements,
if you do not want theetir output on the screen. The procedure call to assemble
the code for the various type signatures is required.
````
DIM code% 200
DIM fcn% 1: PRINT "fcn%: ";~fcn%
DIM index% 1: PRINT "index%: ";~index%
DIM flags% 1: PRINT "flags%: ";~flags%
DIM size% 4: PRINT "size%: ";~size%
DIM count% 4: PRINT "count%: ";~count%
DIM data% 4: PRINT "data%: ";~data%
PROC_assemble_bdpp
````

### Function type signature 1
BOOL bdpp_fg_is_allowed();<br>
BOOL bdpp_fg_is_enabled();<br>
BOOL bdpp_fg_is_busy();<br>
BOOL bdpp_fg_disable();<br>
BOOL bdpp_bg_is_busy();<br>

This (type signature 1) is the first section of a single BASIC procedure that
assembles code for all of the type signatures. Further below,
you will see the end of this procedure (in type signature 8).

```
DEF PROC_assemble_bdpp
P%=code%: bdppSig1%=P%
[OPT 2
LD HL,fcn%: LD A,(HL)
RST.LIL &20
LD L,A
LD H,0
XOR A
EXX
XOR A
LD L,0
LD H,0
LD C,A
RET
]
```

### Function type signature 2
BOOL bdpp_fg_enable(BYTE stream);<br>
BOOL bdpp_fg_is_rx_app_packet_done(BYTE index);<br>
BYTE bdpp_fg_get_rx_app_packet_flags(BYTE index);<br>
BOOL bdpp_fg_is_tx_app_packet_done(BYTE index);<br>
BOOL bdpp_fg_stop_using_app_packet(BYTE index);<br>
BOOL bdpp_bg_is_rx_app_packet_done(BYTE index);<br>
BYTE bdpp_bg_get_rx_app_packet_flags(BYTE index);<br>
BOOL bdpp_bg_is_tx_app_packet_done(BYTE index);<br>
BOOL bdpp_bg_stop_using_app_packet(BYTE index);<br>
```
bdppSig2%=P%
[OPT 2
LD HL,index%: LD B,(HL)
LD HL,fcn%: LD A,(HL)
RST.LIL &20
LD L,A
LD H,0
XOR A
EXX
XOR A
LD L,0
LD H,0
LD C,A
RET
]
```

### Function type signature 3
void bdpp_fg_flush_drv_tx_packet();<br>
void bdpp_bg_flush_drv_tx_packet();<br>
```
bdppSig3%=P%
[OPT 2
LD HL,fcn%: LD A,(HL)
RST.LIL &20
RET
]
```

### Function type signature 4
void bdpp_fg_write_byte_to_drv_tx_packet(BYTE data);<br>
void bdpp_fg_write_drv_tx_byte_with_usage(BYTE data);<br>
void bdpp_bg_write_byte_to_drv_tx_packet(BYTE data);<br>
void bdpp_bg_write_drv_tx_byte_with_usage(BYTE data);<br>
```
bdppSig4%=P%
[OPT 2
LD HL,fcn%: LD A,(HL)
LD HL,data%: LD D,(HL)
RST.LIL &20
RET
]
```

### Function type signature 5
void bdpp_fg_write_bytes_to_drv_tx_packet(const BYTE* data, WORD count);<br>
void bdpp_fg_write_drv_tx_bytes_with_usage(const BYTE* data, WORD count);<br>
void bdpp_bg_write_bytes_to_drv_tx_packet(const BYTE* data, WORD count);<br>
void bdpp_bg_write_drv_tx_bytes_with_usage(const BYTE* data, WORD count);<br>
```
bdppSig5%=P%
[OPT 2
LD HL,count%: DEFB &ED: DEFB &31
LD HL,fcn%: LD A,(HL)
LD HL,data%: DEFB &ED: DEFB &37
RST.LIL &20
RET
]
```

### Function type signature 6
BOOL bdpp_fg_prepare_rx_app_packet(BYTE index, BYTE* data, WORD size);<br>
BOOL bdpp_bg_prepare_rx_app_packet(BYTE index, BYTE* data, WORD size);<br>
```
bdppSig6%=P%
[OPT 2
LD HL,size%: DEFB &ED: DEFB &31
LD HL,index%: LD B,(HL)
LD HL,fcn%: LD A,(HL)
LD HL,data%: DEFB &ED: DEFB &37
RST.LIL &20
LD L,A
LD H,0
XOR A
EXX
XOR A
LD L,0
LD H,0
LD C,A
RET
]
```

### Function type signature 7
BOOL bdpp_fg_queue_tx_app_packet(BYTE indexes, BYTE flags, const BYTE* data, WORD size);<br>
BOOL bdpp_bg_queue_tx_app_packet(BYTE indexes, BYTE flags, const BYTE* data, WORD size);<br>
```
bdppSig7%=P%
[OPT 2
LD HL,size%: DEFB &ED: DEFB &31
LD HL,flags%: LD D,(HL)
LD HL,index%: LD B,(HL)
LD HL,fcn%: LD A,(HL)
LD HL,data%: DEFB &ED: DEFB &37
RST.LIL &20
LD L,A
LD H,0
XOR A
EXX
XOR A
LD L,0
LD H,0
LD C,A
RET
]
```

### Function type signature 8
WORD bdpp_fg_get_rx_app_packet_size(BYTE index);<br>
WORD bdpp_bg_get_rx_app_packet_size(BYTE index);<br>
```
bdppSig8%=P%
[OPT 2
LD HL,index%: LD B,(HL)
LD HL,fcn%: LD A,(HL)
RST.LIL &20
RET
]
ENDPROC
```

## Sample BDPP Function calls in BASIC-24

These samples are shown without line numbers. For most of these calls, BDPP
must be both allowed (both CPUs support it), and enabled.

Print whether BDPP is allowed, and whether it is enabled. If BDPP is enabled,
the data sent to VDP will be enclosed in packets. If BDPP is disabled, the
data will not be enclosed in packets. Notice that this code does not set the
value of "fcn%"; it sets a memory byte at the location pointed to by
the address contained in "fcn%"!
```
?fcn%=0: rc%=USR(bdppSig1%): PRINT "is_allowed -> ";rc%
?fcn%=1: rc%=USR(bdppSig1%): PRINT "is_enabled -> ";rc%
```
Assuming that BDPP is enabled, this example shows a mixture of PRINT statements
with a call to buffer a single byte in the current driver-owned packet.
Notice that this code does not set the
value of "data%"; it sets a memory byte at the location pointed to by
the address contained in "data%"! That is typical of the input parameters
for the RST calls, used in BASIC, as detailed in this document.
```
PRINT "<<BDPP mode: This is in a packet!>>"
PRINT "Printing a 'Q' as one byte:";
?fcn%=&0D: ?data%=&51: CALL bdppSig4%
PRINT ""
PRINT "This needs ESC characters:";
PRINT CHR$(&89);CHR$(&8B);"!"
```

This code prints some text, then flushes any remaining data, to make sure
that it gets displayed.
```
PRINT "some goofy text here"
?fcn%=&F: CALL bdppSig3%
```

This code allocates RAM for an app-owned packet, makes an "echo" request to VDP,
waits for the response packet, and displays the resulting "echo data". Line
numbers are shown, because of illustrating the loop (via GOTO), but some other
loop structure could have been used.
```
300 DIM app_pkt% 1: PRINT "app_pkt%: ";~app_pkt%
310 ?app_pkt%=0: ?fcn%=5: ?index%=5: !data%=app_pkt%: !size%=1
320 rc%=USR(bdppSig6%): PRINT "rx pkt setup -> ";rc%
322 ?fcn%=&F: CALL bdppSig3%
330 VDU 23,0,&A2,1,5,&61
335 ?fcn%=&F: CALL bdppSig3%
340 ?index%=5: ?fcn%=7: rc%=USR(bdppSig2%)
350 IF rc%=0 GOTO 340
360 PRINT "echo returned -> ";~?app_pkt%
365 ?fcn%=&F: CALL bdppSig3%
```
## BDPP VDU Commands

The following VDU commands are handled by the BDPP section of the system command handler in the ESP32 stream processor.
Except the first way (command code 0), these commands all return data via app-owned packets, and thus take a packet index
as one of the command parameters. The EZ80 app must have setup an RX buffer with the same packet index before invoking
one of these commands.

### VDU 23, 0, &A2, 0 - enable BDPP by initializing its driver

This command is automatically invoked when the MOS "bdpp" command is executed. Do not invoke this VDU command independently,
because it only affects the VDP and not MOS itself, and will cause the two CPUs to stop communicating properly.

###  VDU 23, 0, &A2, 1, pkt_idx, echo_data - echo a single byte back to the EZ80

This command takes a single data byte as input and echos it back as output.

###  VDU 23, 0, &A2, 2, pkt_idx, n, caps0; caps1; ... - get amount of free ESP32 RAM with given capabilties

This command inputs one or more sets of ESP32 memory capabilities flags, and determines the amount
of free memory, based on those flags. It returns a list of 32-bit values with the resulting free memory sizes. Within each flags set (e.g., 'caps0' parameter), flags may be ORed to indicate
more than one capability. Here is the list of flags from 'esp_heap_caps.h':

```
#define MALLOC_CAP_EXEC             (1<<0)  ///< Memory must be able to run executable code
#define MALLOC_CAP_32BIT            (1<<1)  ///< Memory must allow for aligned 32-bit data accesses
#define MALLOC_CAP_8BIT             (1<<2)  ///< Memory must allow for 8/16/...-bit data accesses
#define MALLOC_CAP_DMA              (1<<3)  ///< Memory must be able to accessed by DMA
#define MALLOC_CAP_PID2             (1<<4)  ///< Memory must be mapped to PID2 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID3             (1<<5)  ///< Memory must be mapped to PID3 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID4             (1<<6)  ///< Memory must be mapped to PID4 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID5             (1<<7)  ///< Memory must be mapped to PID5 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID6             (1<<8)  ///< Memory must be mapped to PID6 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID7             (1<<9)  ///< Memory must be mapped to PID7 memory space (PIDs are not currently used)
#define MALLOC_CAP_SPIRAM           (1<<10) ///< Memory must be in SPI RAM
#define MALLOC_CAP_INTERNAL         (1<<11) ///< Memory must be internal; specifically it should not disappear when flash/spiram cache is switched off
#define MALLOC_CAP_DEFAULT          (1<<12) ///< Memory can be returned in a non-capability-specific memory allocation (e.g. malloc(), calloc()) call
#define MALLOC_CAP_IRAM_8BIT        (1<<13) ///< Memory must be in IRAM and allow unaligned access
#define MALLOC_CAP_RETENTION        (1<<14) ///< Memory must be able to accessed by retention DMA
#define MALLOC_CAP_RTCRAM           (1<<15) ///< Memory must be in RTC fast memory
```

###  VDU 23, 0, &A2, 3, pkt_idx, addr_lo; addr_hi; n; - get ESP32 RAM contents as bytes (8 bits each)

This command copies data from the ESP32 memory, starting at the given 32-bit address, copying 1 data byte at a time
into the output packet. The total number of bytes returned equals the value of n.

###  VDU 23, 0, &A2, 4, pkt_idx, addr_lo; addr_hi; n; - get ESP32 RAM contents as words (16 bits each)

This command copies data from the ESP32 memory, starting at the given 32-bit address, copying 2 data bytes at a time
into the output packet. The total number of bytes returned equals the value of n times 2.

###  VDU 23, 0, &A2, 5, pkt_idx, addr_lo; addr_hi; n; - get ESP32 RAM contents as dwords (32 bits each)

This command copies data from the ESP32 memory, starting at the given 32-bit address, copying 4 data bytes at a time
into the output packet. The total number of bytes returned equals the value of n times 4.

### VDU 23, 0, &A2, 6, pkt_idx - get VDP version information

This command returns the following VDP version information:

* Major number (1 byte)
* Minor number (1 byte)
* Patch number (1 byte)
* Candidate number (1 byte)
* Type (null-terminated text string)
* Variant (null-terminated text string)

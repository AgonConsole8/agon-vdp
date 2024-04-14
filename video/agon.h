//
// Title:			Agon Video BIOS - Function prototypes
// Author:			Dean Belfield
// Created:			05/09/2022
// Last Updated:	13/08/2023
//
// Modinfo:
// 04/03/2023:		Added LOGICAL_SCRW and LOGICAL_SCRH
// 17/03/2023:		Added PACKET_RTC, EPOCH_YEAR, MAX_SPRITES, MAX_BITMAPS
// 21/03/2023:		Added PACKET_KEYSTATE
// 22/03/2023:		Added VDP codes
// 23/03/2023:		Increased baud rate to 1152000
// 09/08/2023:		Added VDP_SWITCHBUFFER
// 13/08/2023:		Added additional modelines

#pragma once

#define EPOCH_YEAR				1980	// 1-byte dates are offset from this (for FatFS)
#define MAX_SPRITES				256		// Maximum number of sprites
#define MAX_BITMAPS				256		// Maximum number of bitmaps

// #define VDP_USE_WDT						// Use the esp watchdog timer (experimental)

#define UART_BR					1152000	// Max baud rate; previous stable value was 384000
#define UART_NA					-1
#define UART_TX					2
#define UART_RX					34
#define UART_RTS				13		// The ESP32 RTS pin (eZ80 CTS)
#define UART_CTS	 			14		// The ESP32 CTS pin (eZ80 RTS)

#define COMMS_TIMEOUT			200		// Timeout for VDP commands (ms)
#define FAST_COMMS_TIMEOUT		10		// Fast timeout for VDP commands (ms)

#define UART_RX_SIZE			256		// The RX buffer size
#define UART_RX_THRESH			128		// Point at which RTS is toggled

#define GPIO_ITRP				17		// VSync Interrupt Pin - for reference only

#define CURSOR_PHASE			640		// Cursor blink phase (ms)
#define CURSOR_FAST_PHASE		320		// Cursor blink phase (ms)

// Commands for VDU 23, 0, n
//
#define VDP_CURSOR_VSTART		0x0A	// Cursor start line offset (0-15)
#define VDP_CURSOR_VEND			0x0B	// Cursor end line offset
#define VDP_GP					0x80	// General poll data
#define VDP_KEYCODE				0x81	// Keyboard data
#define VDP_CURSOR				0x82	// Cursor positions
#define VDP_SCRCHAR				0x83	// Character read from screen
#define VDP_SCRPIXEL			0x84	// Pixel read from screen
#define VDP_AUDIO				0x85	// Audio commands
#define VDP_MODE				0x86	// Get screen dimensions
#define VDP_RTC					0x87	// RTC
#define VDP_KEYSTATE			0x88	// Keyboard repeat rate and LED status
#define VDP_MOUSE				0x89	// Mouse data
#define VDP_CURSOR_HSTART		0x8A	// Cursor start row offset (0-15)
#define VDP_CURSOR_HEND			0x8B	// Cursor end row offset
#define VDP_CURSOR_MOVE			0x8C	// Cursor relative move
#define VDP_UDG					0x90	// User defined characters
#define VDP_UDG_RESET			0x91	// Reset UDGs
#define VDP_MAP_CHAR_TO_BITMAP	0x92	// Map a character to a bitmap
#define VDP_SCRCHAR_GRAPHICS	0x93	// Character read from screen at graphics coordinates
#define VDP_READ_COLOUR			0x94	// Read colour
#define VDP_FONT				0x95	// Font management commands
#define VDP_CONTROLKEYS			0x98	// Control keys on/off
#define VDP_TEXT_VIEWPORT		0x9C	// Set text viewport using current graphics coordinates
#define VDP_GRAPHICS_VIEWPORT	0x9D	// Set graphics viewport using current graphics coordinates
#define VDP_GRAPHICS_ORIGIN		0x9E	// Set graphics origin using current graphics coordinates
#define VDP_SHIFT_ORIGIN		0x9F	// Move origin to new position from graphics coordinates, and viewports too
#define VDP_BUFFERED			0xA0	// Buffered commands
#define VDP_UPDATER				0xA1	// Update VDP
#define VDP_LOGICALCOORDS		0xC0	// Switch BBC Micro style logical coords on and off
#define VDP_LEGACYMODES			0xC1	// Switch VDP 1.03 compatible modes on and off
#define VDP_SWITCHBUFFER		0xC3	// Double buffering control
#define VDP_CONTEXT				0xC8	// Context management commands
#define VDP_FLUSH_DRAWING_QUEUE	0xCA	// Flush the drawing queue
#define VDP_PATTERN_LENGTH		0xF2	// Set pattern length (*FX 163,242,n)
#define VDP_CONSOLEMODE			0xFE	// Switch console mode on and off
#define VDP_TERMINALMODE		0xFF	// Switch to terminal mode

// And the corresponding return packets
// By convention, these match their VDP counterpart, but with the top bit reset
//
#define PACKET_GP				0x00	// General poll data
#define PACKET_KEYCODE			0x01	// Keyboard data
#define PACKET_CURSOR			0x02	// Cursor positions
#define PACKET_SCRCHAR			0x03	// Character read from screen
#define PACKET_SCRPIXEL			0x04	// Pixel read from screen
#define PACKET_AUDIO			0x05	// Audio acknowledgement
#define PACKET_MODE				0x06	// Get screen dimensions
#define PACKET_RTC				0x07	// RTC
#define PACKET_KEYSTATE			0x08	// Keyboard repeat rate and LED status
#define PACKET_MOUSE			0x09	// Mouse data

#define AUDIO_CHANNELS			3		// Default number of audio channels
#define AUDIO_DEFAULT_SAMPLE_RATE	16384	// Default sample rate
#define MAX_AUDIO_CHANNELS		32		// Maximum number of audio channels
#define PLAY_SOUND_PRIORITY		3		// Sound driver task priority with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest

// Audio command definitions
//
#define AUDIO_CMD_PLAY			0		// Play a sound
#define AUDIO_CMD_STATUS		1		// Get the status of a channel
#define AUDIO_CMD_VOLUME		2		// Set the volume of a channel
#define AUDIO_CMD_FREQUENCY		3		// Set the frequency of a channel
#define AUDIO_CMD_WAVEFORM		4		// Set the waveform type for a channel
#define AUDIO_CMD_SAMPLE		5		// Sample management
#define AUDIO_CMD_ENV_VOLUME	6		// Define/set a volume envelope
#define AUDIO_CMD_ENV_FREQUENCY	7		// Define/set a frequency envelope
#define AUDIO_CMD_ENABLE		8		// Enables a channel
#define AUDIO_CMD_DISABLE		9		// Disables (destroys) a channel
#define AUDIO_CMD_RESET			10		// Reset audio channel
#define AUDIO_CMD_SEEK			11		// Seek to a position in a sample
#define AUDIO_CMD_DURATION		12		// Set the duration of a channel
#define AUDIO_CMD_SAMPLERATE	13		// Set the samplerate for channel or underlying audio system
#define AUDIO_CMD_SET_PARAM		14		// Set a waveform parameter

#define AUDIO_WAVE_DEFAULT		0		// Default waveform (Square wave)
#define AUDIO_WAVE_SQUARE		0		// Square wave
#define AUDIO_WAVE_TRIANGLE		1		// Triangle wave
#define AUDIO_WAVE_SAWTOOTH		2		// Sawtooth wave
#define AUDIO_WAVE_SINE			3		// Sine wave
#define AUDIO_WAVE_NOISE		4		// Noise (simple, no frequency support)
#define AUDIO_WAVE_VICNOISE		5		// VIC-style noise (supports frequency)
#define AUDIO_WAVE_SAMPLE		8		// Sample playback, explicit buffer ID sent in following 2 bytes
// negative values for waveforms indicate a sample number

#define AUDIO_SAMPLE_LOAD		0		// Send a sample to the VDP
#define AUDIO_SAMPLE_CLEAR		1		// Clear/delete a sample
#define AUDIO_SAMPLE_FROM_BUFFER				2	// Load a sample from a buffer
#define AUDIO_SAMPLE_SET_FREQUENCY				3	// Set the base frequency of a sample
#define AUDIO_SAMPLE_BUFFER_SET_FREQUENCY		4	// Set the base frequency of a sample (using buffer ID)
#define AUDIO_SAMPLE_SET_REPEAT_START			5	// Set the repeat start point of a sample
#define AUDIO_SAMPLE_BUFFER_SET_REPEAT_START	6	// Set the repeat start point of a sample (using buffer ID)
#define AUDIO_SAMPLE_SET_REPEAT_LENGTH			7	// Set the repeat length of a sample
#define AUDIO_SAMPLE_BUFFER_SET_REPEAT_LENGTH	8	// Set the repeat length of a sample (using buffer ID)
#define AUDIO_SAMPLE_DEBUG_INFO 0x10	// Get debug info about a sample

#define AUDIO_DEFAULT_FREQUENCY	523		// Default sample frequency (C5, or C above middle C)

#define AUDIO_FORMAT_8BIT_SIGNED	0	// 8-bit signed sample
#define AUDIO_FORMAT_8BIT_UNSIGNED	1	// 8-bit unsigned sample
#define AUDIO_FORMAT_DATA_MASK		7	// data bit mask for format
#define AUDIO_FORMAT_WITH_RATE		8	// OR this with the format to indicate a sample rate follows
#define AUDIO_FORMAT_TUNEABLE		16	// OR this with the format to indicate sample can be tuned (frequency adjustable)

#define AUDIO_ENVELOPE_NONE			0		// No envelope
#define AUDIO_ENVELOPE_ADSR			1		// Simple ADSR volume envelope
#define AUDIO_ENVELOPE_MULTIPHASE_ADSR		2		// Multi-phase ADSR envelope

#define AUDIO_FREQUENCY_ENVELOPE_STEPPED	1		// Stepped frequency envelope

#define AUDIO_FREQUENCY_REPEATS		0x01	// Repeat/loop the frequency envelope
#define AUDIO_FREQUENCY_CUMULATIVE	0x02	// Reset frequency envelope when looping
#define AUDIO_FREQUENCY_RESTRICT	0x04	// Restrict frequency envelope to the range 0-65535

#define AUDIO_PARAM_DUTY_CYCLE		0		// Square wave duty cycle
#define AUDIO_PARAM_VOLUME			2		// Volume
#define AUDIO_PARAM_FREQUENCY		3		// Frequency
#define AUDIO_PARAM_16BIT			0x80	// 16-bit value
#define AUDIO_PARAM_MASK			0x0F	// Parameter mask

#define AUDIO_STATUS_ACTIVE		0x01	// Has an active waveform
#define AUDIO_STATUS_PLAYING	0x02	// Playing a note (not in release phase)
#define AUDIO_STATUS_INDEFINITE	0x04	// Indefinite duration sound playing
#define AUDIO_STATUS_HAS_VOLUME_ENVELOPE	0x08	// Channel has a volume envelope set
#define AUDIO_STATUS_HAS_FREQUENCY_ENVELOPE	0x10	// Channel has a frequency envelope set

// Mouse commands
#define MOUSE_ENABLE			0		// Enable mouse
#define MOUSE_DISABLE			1		// Disable mouse
#define MOUSE_RESET				2		// Reset mouse
#define MOUSE_SET_CURSOR		3		// Set cursor
#define MOUSE_SET_POSITION		4		// Set mouse position
#define MOUSE_SET_AREA			5		// Set mouse area
#define MOUSE_SET_SAMPLERATE	6		// Set mouse sample rate
#define MOUSE_SET_RESOLUTION	7		// Set mouse resolution
#define MOUSE_SET_SCALING		8		// Set mouse scaling
#define MOUSE_SET_ACCERATION	9		// Set mouse acceleration (1-2000)
#define MOUSE_SET_WHEELACC		10		// Set mouse wheel acceleration

#define MOUSE_DEFAULT_CURSOR		0		// Default mouse cursor
#define MOUSE_DEFAULT_SAMPLERATE	60		// Default mouse sample rate
#define MOUSE_DEFAULT_RESOLUTION	2		// Default mouse resolution (4 counts/mm)
#define MOUSE_DEFAULT_SCALING		1		// Default mouse scaling (1:1)
#define MOUSE_DEFAULT_ACCELERATION	180		// Default mouse acceleration 
#define MOUSE_DEFAULT_WHEELACC		60000	// Default mouse wheel acceleration

// Font management commands
#define FONT_SELECT						0		// Select a font (by buffer ID, 65535 for system font)
#define FONT_FROM_BUFFER				1		// Load/define a font from a buffer
#define FONT_SET_INFO					2		// Set font information
#define FONT_SET_NAME					3		// Set font name
#define FONT_CLEAR						4		// Clear a font
#define FONT_COPY_SYSTEM				5		// Copy system font to a buffer
#define FONT_SELECT_BY_NAME				0x10	// Select a font by name
#define FONT_DEBUG_INFO					0x20	// Get debug info about a font
// Future commands may include ability to search for fonts based on their info settings

#define FONT_INFO_WIDTH					0		// Font width
#define FONT_INFO_HEIGHT				1		// Font height
#define FONT_INFO_ASCENT				2		// Font ascent
#define FONT_INFO_FLAGS					3		// Font flags
#define FONT_INFO_CHARPTRS_BUFFER		4		// Font character pointers (for variable width fonts)
#define FONT_INFO_POINTSIZE				5		// Font point size
#define FONT_INFO_INLEADING				6		// Font inleading
#define FONT_INFO_EXLEADING				7		// Font exleading
#define FONT_INFO_WEIGHT				8		// Font weight
#define FONT_INFO_CHARSET				9		// Font character set ??
#define FONT_INFO_CODEPAGE				10		// Font code page

#define FONT_SELECTFLAG_ADJUSTBASE		0x01	// Adjust font baseline, based on ascent

// Context management commands
#define CONTEXT_SELECT					0		// Select a context stack
#define CONTEXT_DELETE					1		// Delete a context stack
#define CONTEXT_RESET					2		// Reset current context
#define CONTEXT_SAVE					3		// Save a context to stack
#define CONTEXT_RESTORE					4		// Restore a context from stack
#define CONTEXT_SAVE_AND_SELECT			5		// Save and get a copy of topmost context from numbered stack
#define CONTEXT_RESTORE_ALL				6		// Clear stack and restore to first context in stack
#define CONTEXT_CLEAR_STACK				7		// Clear stack, keeping current context

#define CONTEXT_RESET_GPAINT			0x01	// graphics painting options
#define CONTEXT_RESET_GPOS				0x02	// graphics positioning incl graphics viewport
#define CONTEXT_RESET_TPAINT			0x04	// text painting options
#define CONTEXT_RESET_TCURSOR			0x08	// text cursor incl text viewport
#define CONTEXT_RESET_TBEHAVIOUR		0x10	// text cursor behaviour
#define CONTEXT_RESET_FONTS				0x20	// fonts
#define CONTEXT_RESET_CHAR2BITMAP		0x40	// char-to-bitmap mappings
#define CONTEXT_RESET_RESERVED			0x80	// reserved for future use

// Buffered commands
#define BUFFERED_WRITE					0x00	// Write to a numbered buffer
#define BUFFERED_CALL					0x01	// Call buffered commands
#define BUFFERED_CLEAR					0x02	// Clear buffered commands
#define BUFFERED_CREATE					0x03	// Create a new empty buffer
#define BUFFERED_SET_OUTPUT				0x04	// Set the output buffer
#define BUFFERED_ADJUST					0x05	// Adjust buffered commands
#define BUFFERED_COND_CALL				0x06	// Conditionally call a buffer
#define BUFFERED_JUMP					0x07	// Jump to a buffer
#define BUFFERED_COND_JUMP				0x08	// Conditionally jump to a buffer
#define BUFFERED_OFFSET_JUMP			0x09	// Jump to a buffer with an offset
#define BUFFERED_OFFSET_COND_JUMP		0x0A	// Conditionally jump to a buffer with an offset
#define BUFFERED_OFFSET_CALL			0x0B	// Call a buffer with an offset
#define BUFFERED_OFFSET_COND_CALL		0x0C	// Conditionally call a buffer with an offset
#define BUFFERED_COPY					0x0D	// Copy blocks from multiple buffers into one buffer
#define BUFFERED_CONSOLIDATE			0x0E	// Consolidate blocks inside a buffer into one
#define BUFFERED_SPLIT					0x0F	// Split a buffer into multiple blocks
#define BUFFERED_SPLIT_INTO				0x10	// Split a buffer into multiple blocks to new buffer(s)
#define BUFFERED_SPLIT_FROM				0x11	// Split to new buffers from a target bufferId onwards
#define BUFFERED_SPLIT_BY				0x12	// Split a buffer into multiple blocks by width (columns)
#define BUFFERED_SPLIT_BY_INTO			0x13	// Split by width into new buffer(s)
#define BUFFERED_SPLIT_BY_FROM			0x14	// Split by width to new buffers from a target bufferId onwards
#define BUFFERED_SPREAD_INTO			0x15	// Spread blocks from a buffer to multiple target buffers
#define BUFFERED_SPREAD_FROM			0x16	// Spread blocks from target buffer ID onwards
#define BUFFERED_REVERSE_BLOCKS			0x17	// Reverse the order of blocks in a buffer
#define BUFFERED_REVERSE				0x18	// Reverse the order of data in a buffer
#define BUFFERED_COPY_REF				0x19	// Copy references to blocks from multiple buffers into one buffer
#define BUFFERED_COPY_AND_CONSOLIDATE	0x1A	// Copy blocks from multiple buffers into one buffer and consolidate them
#define BUFFERED_COMPRESS				0x40	// Compress blocks from multiple buffers into one buffer
#define BUFFERED_DECOMPRESS				0x41	// Decompress blocks from multiple buffers into one buffer

#define BUFFERED_DEBUG_INFO				0x80	// Get debug info about a buffer

// Adjust operation codes
#define ADJUST_NOT				0x00	// Adjust: NOT
#define ADJUST_NEG				0x01	// Adjust: Negative
#define ADJUST_SET				0x02	// Adjust: set new value (replace)
#define ADJUST_ADD				0x03	// Adjust: add
#define ADJUST_ADD_CARRY		0x04	// Adjust: add with carry
#define ADJUST_AND				0x05	// Adjust: AND
#define ADJUST_OR				0x06	// Adjust: OR
#define ADJUST_XOR				0x07	// Adjust: XOR

// Adjust operation flags
#define ADJUST_OP_MASK			0x0F	// operation code mask
#define ADJUST_ADVANCED_OFFSETS	0x10	// advanced, 24-bit offsets (16-bit block offset follows if top bit set)
#define ADJUST_BUFFER_VALUE		0x20	// operand is a buffer fetched value
#define ADJUST_MULTI_TARGET		0x40	// multiple target values will be adjusted
#define ADJUST_MULTI_OPERAND	0x80	// multiple operand values used for adjustments

// Conditional operation codes
#define COND_EXISTS				0x00	// Conditional: exists (non-zero value)
#define COND_NOT_EXISTS			0x01	// Conditional: NOT exists (zero value)
#define COND_EQUAL				0x02	// Conditional: equal
#define COND_NOT_EQUAL			0x03	// Conditional: not equal
#define COND_LESS				0x04	// Conditional: less than
#define COND_GREATER			0x05	// Conditional: greater than
#define COND_LESS_EQUAL			0x06	// Conditional: less than or equal
#define COND_GREATER_EQUAL		0x07	// Conditional: greater than or equal
#define COND_AND				0x08	// Conditional: AND
#define COND_OR					0x09	// Conditional: OR

// Conditional operation flags
#define COND_OP_MASK			0x0F	// conditional operation code mask
#define COND_ADVANCED_OFFSETS	0x10	// advanced offset values
#define COND_BUFFER_VALUE		0x20	// value to compare is a buffer-fetched value

// Reverse operation flags
#define REVERSE_16BIT			0x01	// 16-bit value length
#define REVERSE_32BIT			0x02	// 32-bit value length
#define REVERSE_SIZE			0x03	// when both length flags are set, a 16-bit length value follows
#define REVERSE_CHUNKED			0x04	// chunked reverse, 16-bit size value follows
#define REVERSE_BLOCK			0x08	// reverse block order
#define REVERSE_UNUSED_BITS		0xF0	// unused bits

// Buffered bitmap and sample info
#define BUFFERED_BITMAP_BASEID	0xFA00	// Base ID for buffered bitmaps
#define BUFFERED_SAMPLE_BASEID	0xFB00	// Base ID for buffered samples

#define LOGICAL_SCRW			1280	// As per the BBC Micro standard
#define LOGICAL_SCRH			1024

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE	0
#else
#define ARDUINO_RUNNING_CORE	1
#endif

// Function Prototypes
//
void debug_log(const char *format, ...);

// Terminal states
//
enum TerminalState {
	Disabled,
	Disabling,
	Enabling,
	Enabled,
	Suspending,
	Suspended,
	Resuming
};

// Additional modelines
//
#ifndef VGA_640x240_60Hz
#define VGA_640x240_60Hz	"\"640x240@60Hz\" 25.175 640 656 752 800 240 245 246 262 -HSync -VSync DoubleScan"
#endif

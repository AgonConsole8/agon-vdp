//
// Title:			Bidirectional Packet Protocol
// Author:			Curtis Whitley
// Contributors:	Curtis Whitley
// Created:			29/01/2024
// Last Updated:	29/01/2024
//

#ifndef BDPP_PROTOCOL_H
#define BDPP_PROTOCOL_H

#include <stdint.h>

#define ESP32_COMM_PROTOCOL_VERSION		0x04	// Range is 0x04 to 0x0F, for future enhancements

#define BDPP_FLAG_ALLOWED				0x01 	// Whether bidirectional protocol is allowed (both CPUs have it)
#define BDPP_FLAG_ENABLED				0x02 	// Whether bidirectional protocol is presently enabled

#define BDPP_SMALL_DATA_SIZE			32		// Maximum payload data length for small packet
#define BDPP_MAX_DRIVER_PACKETS			8		// Maximum number of driver-owned small packets
#define BDPP_MAX_APP_PACKETS			8		// Maximum number of app-owned packets

#define BDPP_PACKET_START_MARKER		0x8C
#define BDPP_PACKET_ESCAPE				0x9D
#define BDPP_PACKET_END_MARKER			0xAE

#define BDPP_RX_STATE_AWAIT_START		0x01	// Waiting for the packet start marker
#define BDPP_RX_STATE_AWAIT_FLAGS		0x02	// Waiting for the packet flags
#define BDPP_RX_STATE_AWAIT_INDEX		0x03	// Waiting for the packet index
#define BDPP_RX_STATE_AWAIT_SIZE_1		0x04	// Waiting for the packet size, part 1
#define BDPP_RX_STATE_AWAIT_SIZE_2		0x05	// Waiting for the packet size, part 2
#define BDPP_RX_STATE_AWAIT_DATA_ESC	0x06	// Waiting for a packet data byte or escape
#define BDPP_RX_STATE_AWAIT_DATA		0x07	// Waiting for a packet data byte only
#define BDPP_RX_STATE_AWAIT_END			0x08	// Waiting for the packet end marker

#define BDPP_TX_STATE_IDLE				0x20	// Doing nothing (not transmitting)
#define BDPP_TX_STATE_SENT_START		0x21	// Recently sent the packet start marker
#define BDPP_TX_STATE_SENT_ESC_FLAGS	0x22	// Recently sent escape for packet flags
#define BDPP_TX_STATE_SENT_FLAGS		0x23	// Recently sent the packet flags
#define BDPP_TX_STATE_SENT_INDEX		0x24	// Recently sent the packet index
#define BDPP_TX_STATE_SENT_ESC_SIZE_1	0x25	// Recently sent escape for packet size, part 1
#define BDPP_TX_STATE_SENT_SIZE_1		0x26	// Recently sent the packet size, part 1
#define BDPP_TX_STATE_SENT_ESC_SIZE_2	0x27	// Recently sent escape for packet size, part 2
#define BDPP_TX_STATE_SENT_SIZE_2		0x28	// Recently sent the packet size, part 2
#define BDPP_TX_STATE_SENT_ESC_DATA		0x29	// Recently sent escape for data byte
#define BDPP_TX_STATE_SENT_DATA			0x2A	// Recently sent a packet data byte
#define BDPP_TX_STATE_SENT_ALL_DATA		0x2B	// Recently sent the last packet data byte

#define BDPP_PKT_FLAG_PRINT				0x00	// Indicates packet contains printable data
#define BDPP_PKT_FLAG_COMMAND			0x01	// Indicates packet contains a command or request
#define BDPP_PKT_FLAG_RESPONSE			0x02	// Indicates packet contains a response
#define BDPP_PKT_FLAG_FIRST				0x04	// Indicates packet is first part of a message
#define BDPP_PKT_FLAG_MIDDLE			0x00	// Indicates packet is middle part of a message
#define BDPP_PKT_FLAG_LAST				0x08	// Indicates packet is last part of a message
#define BDPP_PKT_FLAG_READY				0x10	// Indicates packet is ready for transmission or reception
#define BDPP_PKT_FLAG_DONE				0x20	// Indicates packet is was transmitted or received
#define BDPP_PKT_FLAG_FOR_RX			0x40	// Indicates packet is for reception, not transmission
#define BDPP_PKT_FLAG_DRIVER_OWNED		0x00	// Indicates packet is owned by the driver
#define BDPP_PKT_FLAG_APP_OWNED			0x80	// Indicates packet is owned by the application
#define BDPP_PKT_FLAG_USAGE_BITS		0x0F	// Flag bits that describe packet usage
#define BDPP_PKT_FLAG_PROCESS_BITS		0xF0	// Flag bits that affect packet processing

// Defines one packet in the transmit or receive list
//
typedef struct tag_BDPP_PACKET {
	uint8_t			flags;	// Flags describing the packet
	uint8_t			index;	// Index of the packet
	uint16_t		max_size; // Maximum size of the data portion
	uint16_t		act_size; // Actual size of the data portion
	uint8_t*		data;	// Address of the data bytes
	struct tag_BDPP_PACKET* next; // Points to the next packet in the list
} BDPP_PACKET;

// Initialize the BDPP driver.
void bdpp_initialize_driver();

// Get whether BDPP is allowed (both CPUs have it)
bool_t bdpp_is_allowed();

// Get whether BDPP is presently enabled
bool_t bdpp_is_enabled();

// Get whether BDPP is presently busy (TX or RX)
bool_t bdpp_is_busy();

// Enable BDDP mode
bool_t bdpp_enable();

// Disable BDDP mode
bool_t bdpp_disable();

// Initialize an outgoing driver-owned packet, if one is available
// Returns NULL if no packet is available
BDPP_PACKET* bdpp_init_tx_drv_packet(uint8_t flags);

// Initialize an incoming driver-owned packet, if one is available
// Returns NULL if no packet is available
BDPP_PACKET* bdpp_init_rx_drv_packet();

// Queue an app-owned packet for transmission
// This function can fail if the packet is presently involved in a data transfer.
bool_t bdpp_queue_tx_app_packet(uint8_t index, uint8_t flags, const uint8_t* data, uint16_t size);

// Prepare an app-owned packet for reception
// This function can fail if the packet is presently involved in a data transfer.
// The given size is a maximum, based on app memory allocation, and the
// actual size of an incoming packet may be smaller, but not larger.
bool_t bdpp_prepare_rx_app_packet(uint8_t index, uint8_t* data, uint16_t size);

// Check whether an outgoing app-owned packet has been transmitted
bool_t bdpp_is_tx_app_packet_done(uint8_t index);

// Check whether an incoming app-owned packet has been received
bool_t bdpp_is_rx_app_packet_done(uint8_t index);

// Get the flags for a received app-owned packet.
uint8_t bdpp_get_rx_app_packet_flags(uint8_t index);

// Get the data size for a received app-owned packet.
uint16_t bdpp_get_rx_app_packet_size(uint8_t index);

// Free the driver from using an app-owned packet
// This function can fail if the packet is presently involved in a data transfer.
bool_t bdpp_stop_using_app_packet(uint8_t index);

// Start building a driver-owned, outgoing packet.
// If there is an existing packet being built, it will be flushed first.
// This returns NULL if there is no packet available.
BDPP_PACKET* bdpp_start_drv_tx_packet(uint8_t flags);

// Append a data byte to a driver-owned, outgoing packet.
// This is a blocking call, and might wait for room for data.
void bdpp_write_byte_to_drv_tx_packet(uint8_t data);

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a blocking call, and might wait for room for data.
void bdpp_write_bytes_to_drv_tx_packet(const uint8_t* data, uint16_t count);

// Append a single data byte to a driver-owned, outgoing packet.
// This is a potentially blocking call, and might wait for room for data.
// If necessary this function initializes and uses a new packet. It
// decides whether to use "print" data (versus "non-print" data) based on
// the value of the data. To guarantee that the packet usage flags are
// set correctly, be sure to flush the packet before switching from "print"
// to "non-print", or vice versa.
void bdpp_write_drv_tx_byte_with_usage(uint8_t data);

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a potentially blocking call, and might wait for room for data.
// If necessary this function initializes and uses additional packets. It
// decides whether to use "print" data (versus "non-print" data) based on
// the first byte in the data. To guarantee that the packet usage flags are
// set correctly, be sure to flush the packet before switching from "print"
// to "non-print", or vice versa.
void bdpp_write_drv_tx_bytes_with_usage(const uint8_t* data, uint16_t count);

// Flush the currently-being-built, driver-owned, outgoing packet, if any exists.
void bdpp_flush_drv_tx_packet();

#endif // BDPP_PROTOCOL_H

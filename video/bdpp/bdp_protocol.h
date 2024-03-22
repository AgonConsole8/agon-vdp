//
// Title:			Agon Video BIOS
// Author:			Dean Belfield
// Subtitle:		Bidirectional Packet Protocol
// Author:			Curtis Whitley
// Contributors:	
// Created:			29/01/2024
// Last Updated:	29/01/2024
//

#ifndef BDPP_PROTOCOL_H
#define BDPP_PROTOCOL_H

#include "packet.h"

typedef uint8_t bool_t;	// To match what the EZ80 is using for this

#define ESP32_COMM_PROTOCOL_VERSION		0x04	// Range is 0x04 to 0x0F, for future enhancements

// Initialize the BDPP driver.
void bdpp_initialize_driver();

// Get whether the driver has been initialized.
bool bdpp_is_initialized();

// Queue a packet for transmission to the EZ80.
// The packet is expected to be full (to contain all data that
// VDP wants to place into it) when this function is called.
void bdpp_queue_tx_packet(Packet* packet);

// Check for a received packet being available.
bool bdpp_rx_packet_available(uint8_t stream_index);

// Get a received packet.
UhciPacket* bdpp_get_rx_packet(uint8_t stream_index);

#endif // BDPP_PROTOCOL_H

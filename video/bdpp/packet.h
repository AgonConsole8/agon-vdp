//
// Title:			Agon Video BIOS
// Author:			Dean Belfield
// Subtitle:		Packet for Bidirectional Packet Protocol
// Author:			Curtis Whitley
// Contributors:	
// Created:			13/02/2024
// Last Updated:	13/02/2024
//

#pragma once

#include <stdint.h>
#include <string.h>
#include "esp_heap_caps.h"

#define BDPP_MAX_PACKET_DATA_SIZE       256     // Maximum size of the data in one packet
#define BDPP_SMALL_PACKET_DATA_SIZE		32		// Maximum payload data length for small packet
#define BDPP_MAX_DRIVER_PACKETS			16		// Maximum number of driver-owned small packets
#define BDPP_MAX_APP_PACKETS			16		// Maximum number of app-owned packets
#define BDPP_MAX_STREAMS				16		// Maximum number of command/data streams
#define BDPP_MAX_RX_PACKETS             32      // Maximum number of packets setup for DMA RX

#define BDPP_STREAM_INDEX_BITS			0xF0	// Upper nibble used for stream index
#define BDPP_PACKET_INDEX_BITS			0x0F	// Lower nibble used for packet index

#define BDPP_PKT_FLAG_PRINT				0x00	// Indicates packet contains printable data
#define BDPP_PKT_FLAG_COMMAND			0x01	// Indicates packet contains a command or request
#define BDPP_PKT_FLAG_RESPONSE			0x02	// Indicates packet contains a response
#define BDPP_PKT_FLAG_FIRST				0x04	// Indicates packet is first part of a message
#define BDPP_PKT_FLAG_MIDDLE			0x00	// Indicates packet is middle part of a message
#define BDPP_PKT_FLAG_LAST				0x08	// Indicates packet is last part of a message
#define BDPP_PKT_FLAG_ENHANCED  		0x10	// Indicates packet is enhanced in some way
#define BDPP_PKT_FLAG_DONE				0x20	// Indicates packet was transmitted or received
#define BDPP_PKT_FLAG_FOR_RX			0x40	// Indicates packet is for reception, not transmission
#define BDPP_PKT_FLAG_DRIVER_OWNED		0x00	// Indicates packet is owned by the driver
#define BDPP_PKT_FLAG_APP_OWNED			0x80	// Indicates packet is owned by the application
#define BDPP_PKT_FLAG_USAGE_BITS		0x0F	// Flag bits that describe packet usage
#define BDPP_PKT_FLAG_PROCESS_BITS		0xF0	// Flag bits that affect packet processing

// This structure represents the UHCI data for a packet.
// It does not include the enclosing separator characters or escape characters.
#pragma pack(push, 1)
typedef struct tagUhciPacket {
	uint8_t     flags;	// Flags describing the packet
	uint8_t     indexes; // Index of the packet (lower nibble) & stream (upper nibble)
	uint8_t	    act_size; // Actual size of the data portion
	uint8_t     data[BDPP_MAX_PACKET_DATA_SIZE]; // The real data bytes
    uint8_t     dummy; // Padding to even multiple of 4 bytes

    // Test whether a flag is set.
    inline bool is_flag_set(uint8_t flag) { return ((flags & flag) != 0); }

    // Test whether a flag is clear.
    inline bool is_flag_clear(uint8_t flag) { return ((flags & flag) == 0); }

    // Get the flags.
    inline uint8_t get_flags() { return flags; }

    // Get the packet index.
    inline uint8_t get_packet_index() { return (indexes & 0x0F); }

    // Get the stream index.
    inline uint8_t get_stream_index() { return (indexes >> 4); }

    // Get the actual data size.
    inline uint16_t get_actual_data_size() { return (act_size ? act_size : 256); }

    // Get the transfer packet size.
    uint16_t get_transfer_size() {
        return (get_actual_data_size() + 3);
    }

    // Get a pointer to the data.
    inline uint8_t* get_data() { return data; }

    // Set one or more flags.
    inline void set_flags(uint8_t flags) { this->flags |= flags; }

    // Clear one or more flags.
    inline void clear_flags(uint8_t flags) { this->flags &= ~flags; }

    // Append a data byte to the packet.
    void append_data(uint8_t data_byte) {
        // The size will wrap to zero if it becomes 256.
        data[act_size++] = data_byte;
    }

    // Append multiple data bytes to the packet.
    void append_data(const uint8_t* data_bytes, uint16_t count) {
        memcpy((void*) &data[act_size], data_bytes, count);
        // The size will wrap to zero if it becomes 256.
        act_size += count;
    }

    // Set the actual size.
    //
    // This may be used after using get_data() to write to the packet directly.
    inline void set_size(uint16_t size) {
        act_size = size;
    }
} UhciPacket;
#pragma pack(pop)

// This class represents one BDPP data packet.
//
class Packet {
    public:
    // Create a new, empty, driver-owned packet.
    static Packet* create_driver_tx_packet(uint8_t flags, uint8_t packet_index, uint8_t stream_index) {
        return new Packet(flags & BDPP_PKT_FLAG_USAGE_BITS, packet_index, stream_index);
    }

    // Create a new, empty, app-owned packet.
    static Packet* create_app_tx_packet(uint8_t flags, uint8_t packet_index, uint8_t stream_index) {
        return new Packet((flags & BDPP_PKT_FLAG_USAGE_BITS) | BDPP_PKT_FLAG_APP_OWNED, packet_index, stream_index);
    }

    // Create a new, empty packet.
    Packet(uint8_t flags, uint8_t packet_index, uint8_t stream_index) {
        auto max_size = (flags & BDPP_PKT_FLAG_APP_OWNED) ? BDPP_MAX_PACKET_DATA_SIZE : BDPP_SMALL_PACKET_DATA_SIZE;
        auto alloc_size = get_alloc_size(max_size);
        uhci_packet = (UhciPacket *) heap_caps_calloc(1, alloc_size, MALLOC_CAP_DMA|MALLOC_CAP_8BIT);
        uhci_packet->flags = flags;
        uhci_packet->indexes = (packet_index | (stream_index << 4));
        this->max_size = max_size;
        uhci_packet->act_size = 0;
    }

    // Destroy the packet.
    ~Packet() {
        heap_caps_free((void*) uhci_packet);
    }

    // Get a pointer to the allocated UHCI packet data memory.
    inline uint8_t* get_uhci_data() { return (uint8_t*) uhci_packet; }

    // Set the maximum data size.
    inline void set_maximum_data_sze(uint16_t max_size) { this->max_size = max_size; }

    // Get the maximum data size.
    inline uint16_t get_maximum_data_size() { return max_size; }

    // Get the allocated memory size.
    static uint16_t get_alloc_size(uint16_t max_size) {
        return ((3 + max_size + 3) & 0xFFFFFFFC) + 4;
    }

    // Determine whether the packet is full of data.
    inline bool is_full() { return uhci_packet->get_actual_data_size() >= get_maximum_data_size(); }

    // Get a pointer to the UHCI structure.
    inline UhciPacket* get_uhci_packet() { return uhci_packet; }
    
    protected:
	uint16_t        max_size; // Maximum size of the data portion
    UhciPacket*     uhci_packet; // Pointer to UHCI data for the packet
};

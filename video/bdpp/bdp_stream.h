//
// Title:			Agon Video BIOS
// Author:			Dean Belfield
// Subtitle:		Bidirectional Packet Protocol
// Author:			Curtis Whitley
// Contributors:	
// Created:			13/02/2024
// Last Updated:	13/02/2024
//

#pragma once
#include <Stream.h>
#include "bdpp/bdp_protocol.h"

#define DEBUG_BDPP 0

#if DEBUG_BDPP
extern void debug_log(const char* f, ...);
#endif

// This class represents a stream of data coming from BDPP.
//
class BdppStream : public Stream {
    public:
    // Construct a stream
    //
    // This defaults to stream #0, and is intended for creating
    // an array of streams. To set the proper stream index, use
    // the set_stream_index() function, below.
    //
    BdppStream() {
        packet_index = 0;
        stream_index = 0;
        rx_packet = NULL;
        tx_packet = NULL;
        data_index = 0;
    }

    // Destroy the stream.
    //
    virtual ~BdppStream() {
        if (rx_packet) {
            delete rx_packet;
        }
        if (tx_packet) {
            delete tx_packet;
        }
    }

    // Set the stream index for this stream.
    //
    void set_stream_index(uint8_t stream_index) {
        this->stream_index = stream_index;
    }


#if DEBUG_BDPP
void show_rx_packet(UhciPacket* packet) {
    auto act_size = packet->get_actual_data_size();
    auto data = packet->get_data();
    debug_log("RX pkt: %02hX %02hX %02hX (%u): ",
        packet->get_flags(),
        packet->indexes,
        packet->act_size, act_size);
    for (uint16_t i = 0; i < act_size; i++) {
        auto ch = data[i];
        if (ch == 0x20) {
            debug_log("-");
        } else if (ch > 0x20 && ch < 0x7E) {
            debug_log("%c", ch);
        } else {
            debug_log("[%02hX]", ch);
        }
    }
    debug_log("\n");
}
#endif

    // Check for available data
    //
    // Data is available if we have a current packet where
    // not all data bytes have been consumed (we discard
    // packets that have a zero actual data size).
    //
    // If there is no current packet, but we can pull one from
    // the packet queue, and the packet has a non-zero size,
    // then data is available.
    //
    virtual int available() {
        while (true) {
            if (rx_packet) {
                return rx_packet->get_actual_data_size() - data_index;
            }
            if (bdpp_rx_packet_available(stream_index)) {
                rx_packet = bdpp_get_rx_packet(stream_index);
#if DEBUG_BDPP
                show_rx_packet(rx_packet);
#endif
                auto act_size = rx_packet->get_actual_data_size();
                if (act_size) {
                    data_index = 0;
                    return act_size;
                }
                // ignore empty packet
                rx_packet = NULL;
            } else {
                return 0; // no data available
            }
        }
    }

    // Read data from the current packet, if any.
    //
    // After reading a data byte, if the current packet has no more
    // data left in it, we delete that packet (trusting that the
    // "available()" function will grab another one from the queue later).
    //
    virtual int read() {
        if (available()) {
            auto result = (int)(uint)(rx_packet->get_data()[data_index++]);
            if (data_index >= rx_packet->get_actual_data_size()) {
                rx_packet = NULL;
            }
            return result;
        } else {
            return -1;
        }
    }

    // Peek at available data, if any exists.
    //
    // This will return an available data byte, but will not remove
    // that byte from the current packet.
    //
    virtual int peek() {
        if (available()) {
            return (int)(uint)(rx_packet->get_data()[data_index]);
        } else {
            return -1;
        }
    }

    // Write data to the stream.
    //
    // This builds an outgoing (TX) packet, and sends it when full.
    //
    virtual size_t write(uint8_t data_byte) {
        if (!tx_packet) {
            tx_packet = Packet::create_driver_tx_packet(
                BDPP_PKT_FLAG_PRINT | BDPP_PKT_FLAG_MIDDLE,
                packet_index++, stream_index);
            if (packet_index >= BDPP_MAX_DRIVER_PACKETS) {
                packet_index = 0;
            }
        }
        tx_packet->get_uhci_packet()->append_data(data_byte);
        if (tx_packet->is_full()) {
            bdpp_queue_tx_packet(tx_packet);
            tx_packet = NULL;
        }
        return 1;
    }

    // Make available for writing.
    //
    virtual int availableForWrite() {
        return true;
    }

    // Flush the stream.
    //
    // This will flush any packet currently being built.
    //
    virtual void flush() {
        if (tx_packet) {
            bdpp_queue_tx_packet(tx_packet);
            tx_packet = NULL;
        }
    }

    // Start an app-owned packet
    //
    // This will flush any existing TX packet, and start a new one,
    // intended for a particular app-owned packet index at the EZ80.
    //
    Packet* start_app_response_packet(uint8_t packet_index) {
        flush();
        tx_packet = Packet::create_app_tx_packet(
            BDPP_PKT_FLAG_PRINT | BDPP_PKT_FLAG_FIRST | BDPP_PKT_FLAG_RESPONSE,
            packet_index, stream_index);
        return tx_packet;
    }

    protected:
    uint8_t     packet_index; // index of an outgoing packet
    uint8_t     stream_index; // index of this string (0..BDPP_MAX_STREAMS-1)
    uint16_t    data_index; // index into data portion of current packet
    UhciPacket* rx_packet; // current packet used to extract data
    Packet*     tx_packet; // current packet used to output data
};

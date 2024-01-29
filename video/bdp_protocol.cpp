//
// Title:			Bidirectional Packet Protocol
// Author:			Curtis Whitley
// Contributors:	Curtis Whitley
// Created:			29/01/2024
// Last Updated:	29/01/2024
//

#include "bdp_protocol.h"
#include <string>

#define bool_t uint8_t;

uint8_t bdpp_driver_flags;	// Flags controlling the driver

BDPP_PACKET* bdpp_free_drv_pkt_head; // Points to head of free driver packet list
BDPP_PACKET* bdpp_free_drv_pkt_tail; // Points to tail of free driver packet list

uint8_t bdpp_tx_state; // Driver transmitter state
BDPP_PACKET* bdpp_tx_packet; // Points to the packet being transmitted
BDPP_PACKET* bdpp_tx_build_packet; // Points to the packet being built
uint16_t bdpp_tx_byte_count; // Number of data bytes transmitted
uint8_t bdpp_tx_next_pkt_flags; // Flags for the next transmitted packet, possibly
BDPP_PACKET* bdpp_tx_pkt_head; // Points to head of transmit packet list
BDPP_PACKET* bdpp_tx_pkt_tail; // Points to tail of transmit packet list

uint8_t bdpp_rx_state; // Driver receiver state
BDPP_PACKET* bdpp_rx_packet; // Points to the packet being received
uint16_t bdpp_rx_byte_count; // Number of data bytes left to receive
uint8_t bdpp_rx_hold_pkt_flags; // Flags for the received packet

BDPP_PACKET* bdpp_rx_pkt_head; // Points to head of receive packet list
BDPP_PACKET* bdpp_rx_pkt_tail; // Points to tail of receive packet list

// Header information for driver-owned small packets (TX and RX)
BDPP_PACKET bdpp_drv_pkt_header[BDPP_MAX_DRIVER_PACKETS];

// Data bytes for driver-owned small packets
uint8_t bdpp_drv_pkt_data[BDPP_MAX_DRIVER_PACKETS][BDPP_SMALL_DATA_SIZE];

// Header information for app-owned packets (TX and RX)
BDPP_PACKET bdpp_app_pkt_header[BDPP_MAX_APP_PACKETS];


void uart_write_thr(uint8_t data) {
	printf("uart_write_thr(%02hX)\n", data);
	uart_thr = data;
}

bool_t any_more_incoming() {
	return ((uart_rbr >= drv_incoming &&
			uart_rbr - drv_incoming < sizeof(drv_incoming)) ||
			(uart_rbr >= app_incoming &&
			uart_rbr - app_incoming < sizeof(app_incoming)));
}

uint8_t uart_read_lsr() {
	uint8_t data;
	if (any_more_incoming()) {
		data = uart_lsr | UART_LSR_DATA_READY;
	} else {
		data = uart_lsr;
	}
	printf("uart_read_lsr() -> %02hX\n", data);
	return data;
}

uint8_t uart_read_rbr() {
	uint8_t data = *uart_rbr++;
	printf("uart_read_rbr() -> %02hX\n", data);
	if (!any_more_incoming()) {
		uart_lsr &= ~UART_LSR_DATA_READY;
	}
	return data;
}

uint8_t uart_read_iir() {
	uint8_t data = 0;
	if (any_more_incoming()) {
		data = UART_IIR_LINESTATUS;
	} else if (uart_ier & UART_IER_TRANSMITINT) {
		data = UART_IIR_TRANSBUFFEREMPTY;
	}
	printf("uart_read_iir() -> %02hX\n", data);
	return data;
}

void uart_enable_interrupt(uint8_t flag) {
	printf("uart_enable_interrupt(%02hX)\n", flag);
	uart_ier |= flag;
}

void uart_disable_interrupt(uint8_t flag) {
	printf("uart_disable_interrupt(%02hX)\n", flag);
	uart_ier &= ~flag;
}

void disable_interrupts() {
	printf("disable_interrupts\n");
}

void enable_interrupts() {
	printf("enable_interrupts\n");
}

// Push (append) a packet to a list of packets
static void push_to_list(BDPP_PACKET** head, BDPP_PACKET** tail, BDPP_PACKET* packet) {
#if DEBUG_STATE_MACHINE
	printf("push_to_list(%p,%p,%p)\n", head, tail, packet);
#endif
	if (*tail) {
		(*tail)->next = packet;
	} else {
		*head = packet;
	}
	*tail = packet;
}

// Pull (remove) a packet from a list of packets
static BDPP_PACKET* pull_from_list(BDPP_PACKET** head, BDPP_PACKET** tail) {
	BDPP_PACKET* packet = *head;
	if (packet) {
		*head = packet->next;
		if (!packet->next) {
			*tail = NULL;
		}
		packet->next = NULL;
	}
#if DEBUG_STATE_MACHINE
	printf("pull_from_list(%p,%p) -> %p\n", head, tail, packet);
#endif
	return packet;
}

// Reset the receiver state
static void reset_receiver() {
#if DEBUG_STATE_MACHINE
	printf("reset_receiver()\n");
#endif
	bdpp_rx_state = BDPP_RX_STATE_AWAIT_START;
	bdpp_rx_packet = NULL;
}

// Initialize the BDPP driver.
//
void bdpp_initialize_driver() {
#if DEBUG_STATE_MACHINE
	printf("bdpp_initialize_driver()\n");
#endif
	int i;

	reset_receiver();
	bdpp_driver_flags = BDPP_FLAG_ALLOWED;
	bdpp_tx_state = BDPP_TX_STATE_IDLE;
	bdpp_tx_packet = NULL;
	bdpp_tx_build_packet = NULL;
	bdpp_free_drv_pkt_head = NULL;
	bdpp_free_drv_pkt_tail = NULL;
	bdpp_tx_pkt_head = NULL;
	bdpp_tx_pkt_tail = NULL;
	bdpp_rx_pkt_head = NULL;
	bdpp_rx_pkt_tail = NULL;
	bdpp_tx_next_pkt_flags = 0;
	memset(bdpp_drv_pkt_header, 0, sizeof(bdpp_drv_pkt_header));
	memset(bdpp_app_pkt_header, 0, sizeof(bdpp_app_pkt_header));
	memset(bdpp_drv_pkt_data, 0, sizeof(bdpp_drv_pkt_data));
	
	// Initialize the free driver-owned packet list
	for (i = 0; i < BDPP_MAX_DRIVER_PACKETS; i++) {
		bdpp_drv_pkt_header[i].index = (uint8_t)i;
		bdpp_drv_pkt_header[i].data = bdpp_drv_pkt_data[i];
		push_to_list(&bdpp_free_drv_pkt_head, &bdpp_free_drv_pkt_tail,
						&bdpp_drv_pkt_header[i]);
	}
	
	// Initialize the free app-owned packet list
	for (i = 0; i < BDPP_MAX_APP_PACKETS; i++) {
		bdpp_app_pkt_header[i].index = (uint8_t)i;
		bdpp_app_pkt_header[i].flags |= BDPP_PKT_FLAG_APP_OWNED;
	}
}

// Get whether BDPP is allowed (both CPUs have it)
//
bool_t bdpp_is_allowed() {
	return ((bdpp_driver_flags & BDPP_FLAG_ALLOWED) != 0);
}

// Get whether BDPP is presently enabled
//
bool_t bdpp_is_enabled() {
	return ((bdpp_driver_flags & BDPP_FLAG_ENABLED) != 0);
}

// Get whether the BDPP driver is busy (TX or RX)
//
bool_t bdpp_is_busy() {
	bool_t rc;
	DI();
	rc = (bdpp_tx_state != BDPP_TX_STATE_IDLE ||
		bdpp_rx_state != BDPP_RX_STATE_AWAIT_START ||
		bdpp_tx_packet != NULL ||
		bdpp_rx_packet != NULL ||
		bdpp_tx_pkt_head != NULL ||
		bdpp_tx_build_packet != NULL);
	EI();
	return rc;
}

// Enable BDDP mode
//
bool_t bdpp_enable() {
	if (bdpp_driver_flags & BDPP_FLAG_ALLOWED) {
		if (!(bdpp_driver_flags & BDPP_FLAG_ENABLED)) {
			bdpp_driver_flags |= BDPP_FLAG_ENABLED;
			set_vector(uart_IVECT, bdpp_handler);
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

// Disable BDDP mode
//
bool_t bdpp_disable() {
	if (bdpp_driver_flags & BDPP_FLAG_ALLOWED) {
		if (bdpp_driver_flags & BDPP_FLAG_ENABLED) {
			while (bdpp_is_busy()); // wait for BDPP to be fully idle
			bdpp_driver_flags &= ~BDPP_FLAG_ENABLED;
			set_vector(uart_IVECT, uart0_handler);
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

// Initialize an outgoing driver-owned packet, if one is available
// Returns NULL if no packet is available.
//
BDPP_PACKET* bdpp_init_tx_drv_packet(uint8_t flags) {
	BDPP_PACKET* packet = pull_from_list(&bdpp_free_drv_pkt_head, &bdpp_free_drv_pkt_tail);
	if (packet) {
		packet->flags = flags & BDPP_PKT_FLAG_USAGE_BITS;
		packet->max_size = BDPP_SMALL_DATA_SIZE;
		packet->act_size = 0;
	}
#if DEBUG_STATE_MACHINE
	printf("bdpp_init_tx_drv_packet() -> %p\n", packet);
#endif
	return packet;
}

// Initialize an incoming driver-owned packet, if one is available
// Returns NULL if no packet is available.
//
BDPP_PACKET* bdpp_init_rx_drv_packet() {
	BDPP_PACKET* packet = pull_from_list(&bdpp_free_drv_pkt_head, &bdpp_free_drv_pkt_tail);
	if (packet) {
		packet->flags = 0;
		packet->max_size = BDPP_SMALL_DATA_SIZE;
		packet->act_size = 0;
	}
#if DEBUG_STATE_MACHINE
	printf("bdpp_init_rx_drv_packet() -> %p\n", packet);
#endif
	return packet;
}

// Queue an app-owned packet for transmission
// This function can fail if the packet is presently involved in a data transfer.
//
bool_t bdpp_queue_tx_app_packet(uint8_t index, uint8_t flags, const uint8_t* data, uint16_t size) {
#if DEBUG_STATE_MACHINE
	printf("bdpp_queue_tx_app_packet(%02hX,%02hX,%p,%04hX)\n", index, flags, data, size);
#endif
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		DI();
		if (bdpp_rx_packet == packet || bdpp_tx_packet == packet) {
			EI();
			return FALSE;
		}
		flags &= ~(BDPP_PKT_FLAG_DONE|BDPP_PKT_FLAG_FOR_RX);
		flags |= BDPP_PKT_FLAG_APP_OWNED|BDPP_PKT_FLAG_READY;
		packet->flags = flags;
		packet->max_size = size;
		packet->act_size = size;
		packet->data = (uint8_t*)data;
		push_to_list(&bdpp_tx_pkt_head, &bdpp_tx_pkt_tail, packet);
		uart_enable_interrupt(UART_IER_TRANSMITINT);
		EI();
		return TRUE;
	}
	return FALSE;
}

// Prepare an app-owned packet for reception
// This function can fail if the packet is presently involved in a data transfer.
// The given size is a maximum, based on app memory allocation, and the
// actual size of an incoming packet may be smaller, but not larger.
//
bool_t bdpp_prepare_rx_app_packet(uint8_t index, uint8_t* data, uint16_t size) {
#if DEBUG_STATE_MACHINE
	printf("bdpp_prepare_rx_app_packet(%02hX,%p,%04hX)\n", index, data, size);
#endif
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		DI();
		if (bdpp_rx_packet == packet || bdpp_tx_packet == packet) {
			EI();
			return FALSE;
		}
		packet->flags &= ~BDPP_PKT_FLAG_DONE;
		packet->flags |= BDPP_PKT_FLAG_APP_OWNED|BDPP_PKT_FLAG_READY|BDPP_PKT_FLAG_FOR_RX;
		packet->max_size = size;
		packet->act_size = 0;
		packet->data = data;
		EI();
		return TRUE;
	}
	return FALSE;
}

// Check whether an outgoing app-owned packet has been transmitted
//
bool_t bdpp_is_tx_app_packet_done(uint8_t index) {
	bool_t rc;
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		DI();
		rc = (((packet->flags & BDPP_PKT_FLAG_DONE) != 0) &&
				((packet->flags & BDPP_PKT_FLAG_FOR_RX) == 0));
		EI();
#if DEBUG_STATE_MACHINE
		printf("bdpp_is_tx_app_packet_done(%02hX) -> %hX\n", index, rc);
#endif
		return rc;
	}
#if DEBUG_STATE_MACHINE
	printf("bdpp_is_tx_app_packet_done(%02hX) -> %hX\n", index, FALSE);
#endif
	return FALSE;
}

// Check whether an incoming app-owned packet has been received
//
bool_t bdpp_is_rx_app_packet_done(uint8_t index) {
	bool_t rc;
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		DI();
		rc = ((packet->flags & (BDPP_PKT_FLAG_FOR_RX|BDPP_PKT_FLAG_DONE)) ==
				(BDPP_PKT_FLAG_FOR_RX|BDPP_PKT_FLAG_DONE));
		EI();
#if DEBUG_STATE_MACHINE
		printf("bdpp_is_rx_app_packet_done(%02hX) -> %hX\n", index, rc);
#endif
		return rc;
	}
#if DEBUG_STATE_MACHINE
	printf("bdpp_is_rx_app_packet_done(%02hX) -> %hX\n", index, FALSE);
#endif
	return FALSE;
}

// Get the flags for a received app-owned packet.
uint8_t bdpp_get_rx_app_packet_flags(uint8_t index) {
	uint8_t flags = 0;
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		DI();
		flags = packet->flags;
		EI();
	}
#if DEBUG_STATE_MACHINE
	printf("bdpp_get_rx_app_packet_flags(%02hX) -> %02hX\n", index, flags);
#endif
	return flags;
}

// Get the data size for a received app-owned packet.
uint16_t bdpp_get_rx_app_packet_size(uint8_t index) {
	uint16_t size = 0;
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		DI();
		size = packet->act_size;
		EI();
	}
#if DEBUG_STATE_MACHINE
	printf("bdpp_get_rx_app_packet_size(%02hX) -> %04hX\n", index, size);
#endif
	return size;
}

// Free the driver from using an app-owned packet
// This function can fail if the packet is presently involved in a data transfer.
//
bool_t bdpp_stop_using_app_packet(uint8_t index) {
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		DI();
		if (bdpp_rx_packet == packet || bdpp_tx_packet == packet) {
			EI();
			return FALSE;
		}
		packet->flags &= ~(BDPP_PKT_FLAG_DONE|BDPP_PKT_FLAG_READY|BDPP_PKT_FLAG_FOR_RX);
		EI();
#if DEBUG_STATE_MACHINE
		printf("bdpp_stop_using_app_packet(%02hX) -> %hX\n", index, TRUE);
#endif
		return TRUE;
	}
#if DEBUG_STATE_MACHINE
	printf("bdpp_stop_using_app_packet(%02hX) -> %hX\n", index, FALSE);
#endif
	return FALSE;
}

// Start building a device-owned, outgoing packet.
// If there is an existing packet being built, it will be flushed first.
// This returns NULL if there is no packet available.
//
BDPP_PACKET* bdpp_start_drv_tx_packet(uint8_t flags) {
	BDPP_PACKET* packet;
	bdpp_flush_drv_tx_packet();
	packet = bdpp_init_tx_drv_packet(flags);
#if DEBUG_STATE_MACHINE
	printf("bdpp_start_drv_tx_packet(%02hX) -> %p\n", flags, packet);
#endif
	return packet;
}

// Flush the currently-being-built, driver-owned, outgoing packet, if any exists.
//
static void bdpp_internal_flush_drv_tx_packet() {
	if (bdpp_tx_build_packet) {
#if DEBUG_STATE_MACHINE
		printf("bdpp_internal_flush_drv_tx_packet() flushing %p\n", bdpp_tx_build_packet);
#endif
		DI();
			bdpp_tx_build_packet->flags |= BDPP_PKT_FLAG_READY;
			push_to_list(&bdpp_tx_pkt_head, &bdpp_tx_pkt_tail, bdpp_tx_build_packet);
			bdpp_tx_build_packet = NULL;
			uart_enable_interrupt(UART_IER_TRANSMITINT);
		EI();
	}
}

// Append a data byte to a driver-owned, outgoing packet.
// This is a blocking call, and might wait for room for data.
static void bdpp_internal_write_byte_to_drv_tx_packet(uint8_t data) {
#if DEBUG_STATE_MACHINE
	printf("bdpp_internal_write_byte_to_drv_tx_packet(%02hX)\n", data);
#endif
	while (TRUE) {
		if (bdpp_tx_build_packet) {
			uint8_t* pdata = bdpp_tx_build_packet->data;
			pdata[bdpp_tx_build_packet->act_size++] = data;
			if (bdpp_tx_build_packet->act_size >= bdpp_tx_build_packet->max_size) {
				if (bdpp_tx_build_packet->flags & BDPP_PKT_FLAG_LAST) {
					bdpp_tx_next_pkt_flags = 0;
				} else {
					bdpp_tx_next_pkt_flags = bdpp_tx_build_packet->flags & ~BDPP_PKT_FLAG_FIRST;
				}
				bdpp_internal_flush_drv_tx_packet();
			}
			break;
		} else {
			bdpp_tx_build_packet = bdpp_init_tx_drv_packet(bdpp_tx_next_pkt_flags);
		}
	}
}

// Append a data byte to a driver-owned, outgoing packet.
// This is a blocking call, and might wait for room for data.
void bdpp_write_byte_to_drv_tx_packet(uint8_t data) {
#if DEBUG_STATE_MACHINE
	printf("bdpp_write_byte_to_drv_tx_packet(%02hX)\n", data);
#endif
	if (bdpp_is_allowed()) {
		bdpp_internal_write_byte_to_drv_tx_packet(data);
	}
}

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a blocking call, and might wait for room for data.
void bdpp_write_bytes_to_drv_tx_packet(const uint8_t* data, uint16_t count) {
#if DEBUG_STATE_MACHINE
	printf("bdpp_write_bytes_to_drv_tx_packet(%p, %04hX)\n", data, count);
#endif
	if (bdpp_is_allowed()) {
		while (count > 0) {
			bdpp_internal_write_byte_to_drv_tx_packet(*data++);
			count--;
		}
	}
}

// Append a single data byte to a driver-owned, outgoing packet.
// This is a potentially blocking call, and might wait for room for data.
// If necessary this function initializes and uses a new packet. It
// decides whether to use "print" data (versus "non-print" data) based on
// the value of the data. To guarantee that the packet usage flags are
// set correctly, be sure to flush the packet before switching from "print"
// to "non-print", or vice versa.
void bdpp_write_drv_tx_byte_with_usage(uint8_t data) {
#if DEBUG_STATE_MACHINE
	printf("bdpp_write_drv_tx_byte_with_usage(%02hX)\n", data);
#endif
	if (bdpp_is_allowed()) {
		if (!bdpp_tx_build_packet) {
			if (data >= 0x20 && data <= 0x7E) {
				bdpp_tx_next_pkt_flags = BDPP_PKT_FLAG_FIRST|BDPP_PKT_FLAG_PRINT;
			} else {
				bdpp_tx_next_pkt_flags = BDPP_PKT_FLAG_FIRST|BDPP_PKT_FLAG_COMMAND;
			}
		}
		bdpp_internal_write_byte_to_drv_tx_packet(data);
	}
}

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a blocking call, and might wait for room for data.
// If necessary this function initializes and uses additional packets. It
// decides whether to use "print" data (versus "non-print" data) based on
// the first byte in the data. To guarantee that the packet usage flags are
// set correctly, be sure to flush the packet before switching from "print"
// to "non-print", or vice versa.
void bdpp_write_drv_tx_bytes_with_usage(const uint8_t* data, uint16_t count) {
#if DEBUG_STATE_MACHINE
	printf("bdpp_write_drv_tx_bytes_with_usage(%p, %04hX) [%02hX]\n", data, count, *data);
#endif
	if (bdpp_is_allowed()) {
		if (!bdpp_tx_build_packet) {
			if (*data >= 0x20 && *data <= 0x7E) {
				bdpp_tx_next_pkt_flags = BDPP_PKT_FLAG_FIRST|BDPP_PKT_FLAG_PRINT;
			} else {
				bdpp_tx_next_pkt_flags = BDPP_PKT_FLAG_FIRST|BDPP_PKT_FLAG_COMMAND;
			}
		}
		bdpp_write_bytes_to_drv_tx_packet(data, count);
	}
}

// Flush the currently-being-built, driver-owned, outgoing packet, if any exists.
//
void bdpp_flush_drv_tx_packet() {
	if (bdpp_tx_build_packet) {
#if DEBUG_STATE_MACHINE
		printf("bdpp_flush_drv_tx_packet(%p)\n", bdpp_tx_build_packet);
#endif
		bdpp_tx_build_packet->flags |= BDPP_PKT_FLAG_LAST;
		bdpp_internal_flush_drv_tx_packet();
		bdpp_tx_next_pkt_flags = 0;
	}
}

// Process the BDPP receiver (RX) state machine
//
void bdpp_run_rx_state_machine() {
	uint8_t incoming_byte;
	BDPP_PACKET* packet;
#if DEBUG_STATE_MACHINE
	printf("\nbdpp_run_rx_state_machine() state:[%02hX]\n", bdpp_rx_state);
#endif

	while (uart_read_lsr() & UART_LSR_DATA_READY) {
		incoming_byte = uart_read_rbr();
#if DEBUG_STATE_MACHINE
		printf(" RX state:[%02hX], incoming:[%02hX]\n", bdpp_rx_state, incoming_byte);
#endif
		switch (bdpp_rx_state) {
			case BDPP_RX_STATE_AWAIT_START: {
				if (incoming_byte == BDPP_PACKET_START_MARKER) {
					bdpp_rx_state = BDPP_RX_STATE_AWAIT_FLAGS;
				}
			} break;

			case BDPP_RX_STATE_AWAIT_FLAGS: {
				bdpp_rx_hold_pkt_flags =
					(incoming_byte & BDPP_PKT_FLAG_USAGE_BITS) |
					(BDPP_PKT_FLAG_FOR_RX | BDPP_PKT_FLAG_READY);
				if (incoming_byte & BDPP_PKT_FLAG_APP_OWNED) {
					// An index will be received for an app-owned packet.
					bdpp_rx_state = BDPP_RX_STATE_AWAIT_INDEX;
				} else {
					// No index will be received for a driver-owned packet.
					if (bdpp_rx_packet = bdpp_init_rx_drv_packet()) {
						bdpp_rx_packet->flags = bdpp_rx_hold_pkt_flags;
						bdpp_rx_state = BDPP_RX_STATE_AWAIT_SIZE_1;
					} else {
						reset_receiver();
					}
				}
			} break;

			case BDPP_RX_STATE_AWAIT_INDEX: {
				if (incoming_byte < BDPP_MAX_APP_PACKETS) {
					packet = &bdpp_app_pkt_header[incoming_byte];
					if (packet->flags & BDPP_PKT_FLAG_DONE) {
						reset_receiver();
					} else {
						bdpp_rx_packet = packet;
						bdpp_rx_packet->flags = bdpp_rx_hold_pkt_flags;
						bdpp_rx_state = BDPP_RX_STATE_AWAIT_SIZE_1;
					}
				} else {
					reset_receiver();
				}
				
			} break;

			case BDPP_RX_STATE_AWAIT_SIZE_1: {
				bdpp_rx_byte_count = (uint16_t)incoming_byte;
				bdpp_rx_state = BDPP_RX_STATE_AWAIT_SIZE_2;
			} break;

			case BDPP_RX_STATE_AWAIT_SIZE_2: {
				bdpp_rx_byte_count |= (((uint16_t)incoming_byte) << 8);
				if (bdpp_rx_byte_count > bdpp_rx_packet->max_size) {
					reset_receiver();
				} else if (bdpp_rx_byte_count == 0) {
					bdpp_rx_state = BDPP_RX_STATE_AWAIT_END;
				} else {
					bdpp_rx_state = BDPP_RX_STATE_AWAIT_DATA_ESC;
				}
			} break;

			case BDPP_RX_STATE_AWAIT_DATA_ESC: {
				if (incoming_byte == BDPP_PACKET_ESCAPE) {
					bdpp_rx_state = BDPP_RX_STATE_AWAIT_DATA;
				} else {
					(bdpp_rx_packet->data)[bdpp_rx_packet->act_size++] = incoming_byte;
					if (--bdpp_rx_byte_count == 0) {
						// All data bytes received
						bdpp_rx_state = BDPP_RX_STATE_AWAIT_END;
					}
				}
			} break;

			case BDPP_RX_STATE_AWAIT_DATA: {
				(bdpp_rx_packet->data)[bdpp_rx_packet->act_size++] = incoming_byte;
				if (--bdpp_rx_byte_count == 0) {
					// All data bytes received
					bdpp_rx_state = BDPP_RX_STATE_AWAIT_END;
				}
			} break;

			case BDPP_RX_STATE_AWAIT_END: {
				if (incoming_byte == BDPP_PACKET_END_MARKER) {
					// Packet is complete
					bdpp_rx_packet->flags &= ~BDPP_PKT_FLAG_READY;
					bdpp_rx_packet->flags |= BDPP_PKT_FLAG_DONE;
					if ((bdpp_rx_packet->flags & BDPP_PKT_FLAG_APP_OWNED) == 0) {
						push_to_list(&bdpp_rx_pkt_head, &bdpp_rx_pkt_head, bdpp_rx_packet);
					}
				}
				reset_receiver();
			} break;
		}
	}
}

// Process the BDPP transmitter (TX) state machine
//
void bdpp_run_tx_state_machine() {
	uint8_t outgoing_byte;
#if DEBUG_STATE_MACHINE
	printf("\nbdpp_run_tx_state_machine() state:[%02hX]\n", bdpp_tx_state);
#endif
	
	while (uart_read_lsr() & UART_LSR_THREMPTY) {
#if DEBUG_STATE_MACHINE
		printf(" TX state:[%02hX]\n", bdpp_tx_state);
#endif
		switch (bdpp_tx_state) {
			case BDPP_TX_STATE_IDLE: {
				if (bdpp_tx_packet = pull_from_list(&bdpp_tx_pkt_head, &bdpp_tx_pkt_tail)) {
					uart_write_thr(BDPP_PACKET_START_MARKER);
					bdpp_tx_state = BDPP_TX_STATE_SENT_START;
				} else {
					uart_disable_interrupt(UART_IER_TRANSMITINT);
					return;
				}
			} break;

			case BDPP_TX_STATE_SENT_START: {
				outgoing_byte = bdpp_tx_packet->flags;
				if (outgoing_byte == BDPP_PACKET_START_MARKER ||
					outgoing_byte == BDPP_PACKET_ESCAPE ||
					outgoing_byte == BDPP_PACKET_END_MARKER) {
					uart_write_thr(BDPP_PACKET_ESCAPE);
					bdpp_tx_state = BDPP_TX_STATE_SENT_ESC_FLAGS;
				} else {
					uart_write_thr(outgoing_byte);
					bdpp_tx_state = BDPP_TX_STATE_SENT_FLAGS;
				}
			} break;
			
			case BDPP_TX_STATE_SENT_ESC_FLAGS: {
				uart_write_thr(bdpp_tx_packet->flags);
				bdpp_tx_state = BDPP_TX_STATE_SENT_FLAGS;
			} break;

			case BDPP_TX_STATE_SENT_FLAGS: {
				if (bdpp_tx_packet->flags & BDPP_PKT_FLAG_APP_OWNED) {
					uart_write_thr(bdpp_tx_packet->index);
					bdpp_tx_state = BDPP_TX_STATE_SENT_INDEX;
				} else {
					uart_write_thr((uint8_t)bdpp_tx_packet->act_size);
					bdpp_tx_state = BDPP_TX_STATE_SENT_SIZE_1;
				}
			} break;

			case BDPP_TX_STATE_SENT_INDEX: {
				outgoing_byte = (uint8_t)(bdpp_tx_packet->act_size);
				if (outgoing_byte == BDPP_PACKET_START_MARKER ||
					outgoing_byte == BDPP_PACKET_ESCAPE ||
					outgoing_byte == BDPP_PACKET_END_MARKER) {
					uart_write_thr(BDPP_PACKET_ESCAPE);
					bdpp_tx_state = BDPP_TX_STATE_SENT_ESC_SIZE_1;
				} else {
					uart_write_thr(outgoing_byte);
					bdpp_tx_state = BDPP_TX_STATE_SENT_SIZE_1;
				}
			} break;

			case BDPP_TX_STATE_SENT_ESC_SIZE_1: {
				uart_write_thr((uint8_t)(bdpp_tx_packet->act_size));
				bdpp_tx_state = BDPP_TX_STATE_SENT_SIZE_1;
			} break;
			
			case BDPP_TX_STATE_SENT_SIZE_1: {
				outgoing_byte = (uint8_t)(bdpp_tx_packet->act_size >> 8);
				if (outgoing_byte == BDPP_PACKET_START_MARKER ||
					outgoing_byte == BDPP_PACKET_ESCAPE ||
					outgoing_byte == BDPP_PACKET_END_MARKER) {
					uart_write_thr(BDPP_PACKET_ESCAPE);
					bdpp_tx_state = BDPP_TX_STATE_SENT_ESC_SIZE_2;
				} else {
					uart_write_thr(outgoing_byte);
					bdpp_tx_state = BDPP_TX_STATE_SENT_SIZE_2;
				}
			} break;
			
			case BDPP_TX_STATE_SENT_ESC_SIZE_2: {
				uart_write_thr((uint8_t)(bdpp_tx_packet->act_size >> 8));
				bdpp_tx_state = BDPP_TX_STATE_SENT_SIZE_2;
			} break;
			
			case BDPP_TX_STATE_SENT_SIZE_2: {
				if (bdpp_tx_packet->act_size == 0) {
					bdpp_tx_state = BDPP_TX_STATE_SENT_ALL_DATA;
				} else {
					bdpp_tx_byte_count = 0;
					bdpp_tx_state = BDPP_TX_STATE_SENT_DATA;
				}
			} break;

			case BDPP_TX_STATE_SENT_ESC_DATA: {
				outgoing_byte = (bdpp_tx_packet->data)[bdpp_tx_byte_count];
				uart_write_thr(outgoing_byte);
				if (++bdpp_tx_byte_count >= bdpp_tx_packet->act_size) {
					bdpp_tx_state = BDPP_TX_STATE_SENT_ALL_DATA;
				} else {
					bdpp_tx_state = BDPP_TX_STATE_SENT_DATA;
				}
			} break;

			case BDPP_TX_STATE_SENT_DATA: {
				outgoing_byte = (bdpp_tx_packet->data)[bdpp_tx_byte_count];
				if (outgoing_byte == BDPP_PACKET_START_MARKER ||
					outgoing_byte == BDPP_PACKET_ESCAPE ||
					outgoing_byte == BDPP_PACKET_END_MARKER) {
					uart_write_thr(BDPP_PACKET_ESCAPE);
					bdpp_tx_state = BDPP_TX_STATE_SENT_ESC_DATA;
				} else {
					uart_write_thr(outgoing_byte);
					if (++bdpp_tx_byte_count >= bdpp_tx_packet->act_size) {
						bdpp_tx_state = BDPP_TX_STATE_SENT_ALL_DATA;
					}
				}
			} break;
			
			case BDPP_TX_STATE_SENT_ALL_DATA: {
				uart_write_thr(BDPP_PACKET_END_MARKER);
				bdpp_tx_packet->flags &= ~BDPP_PKT_FLAG_READY;
				bdpp_tx_packet->flags |= BDPP_PKT_FLAG_DONE;
				if (!(bdpp_tx_packet->flags & BDPP_PKT_FLAG_APP_OWNED)) {
					push_to_list(&bdpp_free_drv_pkt_head, &bdpp_free_drv_pkt_tail, bdpp_tx_packet);
				}
				bdpp_tx_packet = NULL;
				bdpp_tx_state = BDPP_TX_STATE_IDLE;
			} break;
		}
	}
}

// The real guts of the bidirectional packet protocol!
// This function processes the TX and RX state machines.
// It is called by the uart_ interrupt handler, so it assumes
// that interrupts are always turned off during this function.
//
void bdp_protocol() {
#if DEBUG_STATE_MACHINE
	printf("bdp_protocol()\n");
#endif
	uint8_t dummy = uart_read_iir();
	bdpp_run_rx_state_machine();
	bdpp_run_tx_state_machine();
}

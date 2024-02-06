//
// Title:			Agon Video BIOS
// Author:			Dean Belfield
// Subtitle:		Bidirectional Packet Protocol
// Author:			Curtis Whitley
// Contributors:	
// Created:			29/01/2024
// Last Updated:	29/01/2024
//

#include "bdp_protocol.h"
#include <HardwareSerial.h>
#include <string.h>
#include <queue>
#include "soc/uhci_struct.h"
#include "soc/uart_struct.h"
#include "soc/uart_periph.h"
#include "driver/uart.h"
#include "uhci_driver.h"

#define UHCI_NUM  UHCI_NUM_0
#define UART_NUM  UART_NUM_2
#define DEBUG_BDPP 1

extern void debug_log(const char* fmt, ...);

uint8_t bdpp_driver_flags;	// Flags controlling the driver

BDPP_PACKET* bdpp_free_drv_pkt_head; // Points to head of free driver packet list
BDPP_PACKET* bdpp_free_drv_pkt_tail; // Points to tail of free driver packet list

uint8_t bdpp_tx_state; 				// Driver transmitter state
BDPP_PACKET* bdpp_tx_packet; 		// Points to the packet being transmitted
BDPP_PACKET* bdpp_tx_build_packet; 	// Points to the packet being built
uint16_t bdpp_tx_byte_count; 		// Number of data bytes transmitted
uint8_t bdpp_tx_next_pkt_flags; 	// Flags for the next transmitted packet, possibly
uint8_t bdpp_tx_next_stream;  		// Index of the next stream to use

std::queue<BDPP_PACKET*> bdpp_tx_queue; // Transmit packet queue
std::queue<BDPP_PACKET*> bdpp_rx_queue; // Receive packet queue
std::queue<BDPP_PACKET*> bdpp_free_queue; // Free packet queue

uint8_t bdpp_rx_state; 				// Driver receiver state
BDPP_PACKET* bdpp_rx_packet; 		// Points to the packet being received
uint16_t bdpp_rx_byte_count; 		// Number of data bytes left to receive
uint8_t bdpp_rx_hold_pkt_flags; 	// Flags for the received packet


// Header information for driver-owned small packets (TX and RX)
BDPP_PACKET bdpp_drv_pkt_header[BDPP_MAX_DRIVER_PACKETS];

// Data bytes for driver-owned small packets
uint8_t bdpp_drv_pkt_data[BDPP_MAX_DRIVER_PACKETS][BDPP_SMALL_DATA_SIZE];

// Header information for app-owned packets (TX and RX)
BDPP_PACKET bdpp_app_pkt_header[BDPP_MAX_APP_PACKETS];

//--------------------------------------------------

void UART0_write_thr(uint8_t data) {
	debug_log("UART0_write_thr(%02hX)\n", data);
	//uart_thr = data;
}

bool_t any_more_incoming() {
	return false;
	/*return ((uart_rbr >= drv_incoming &&
			uart_rbr - drv_incoming < sizeof(drv_incoming)) ||
			(uart_rbr >= app_incoming &&
			uart_rbr - app_incoming < sizeof(app_incoming)));*/
}

uint8_t UART0_read_lsr() {
	/*uint8_t data;
	if (any_more_incoming()) {
		data = uart_lsr | UART_LSR_DATA_READY;
	} else {
		data = uart_lsr;
	}
	debug_log("UART0_read_lsr() -> %02hX\n", data);
	return data;*/
	return 0;
}

uint8_t UART0_read_rbr() {
	/*uint8_t data = *uart_rbr++;
	debug_log("UART0_read_rbr() -> %02hX\n", data);
	if (!any_more_incoming()) {
		uart_lsr &= ~UART_LSR_DATA_READY;
	}
	return data;*/
	return 0;
}

uint8_t UART0_read_iir() {
	/*uint8_t data = 0;
	if (any_more_incoming()) {
		data = UART_IIR_LINESTATUS;
	} else if (uart_ier & UART_IER_TRANSMITINT) {
		data = UART_IIR_TRANSBUFFEREMPTY;
	}
	debug_log("UART0_read_iir() -> %02hX\n", data);
	return data;*/
	return 0;
}

void UART0_enable_interrupt(uint8_t flag) {
	debug_log("UART0_enable_interrupt(%02hX)\n", flag);
	//uart_ier |= flag;
}

void UART0_disable_interrupt(uint8_t flag) {
	debug_log("UART0_disable_interrupt(%02hX)\n", flag);
	//uart_ier &= ~flag;
}

void disable_interrupts() {
	debug_log("disable_interrupts\n");
}

void enable_interrupts() {
	debug_log("enable_interrupts\n");
}

//--------------------------------------------------

// Reset the receiver state
static void reset_receiver() {
#if DEBUG_BDPP
	debug_log("reset_receiver()\n");
#endif
	bdpp_rx_state = BDPP_RX_STATE_AWAIT_START;
	bdpp_rx_packet = NULL;
}

// Initialize the BDPP driver.
//
void bdpp_initialize_driver() {
#if DEBUG_BDPP
	debug_log("bdpp_initialize_driver()\n");
#endif
	int i;

	reset_receiver();
	bdpp_driver_flags = BDPP_FLAG_ALLOWED;
	bdpp_tx_state = BDPP_TX_STATE_IDLE;
	bdpp_tx_packet = NULL;
	bdpp_tx_build_packet = NULL;
	bdpp_free_drv_pkt_head = NULL;
	bdpp_free_drv_pkt_tail = NULL;
	bdpp_tx_next_pkt_flags = 0;
	memset(bdpp_drv_pkt_header, 0, sizeof(bdpp_drv_pkt_header));
	memset(bdpp_app_pkt_header, 0, sizeof(bdpp_app_pkt_header));
	memset(bdpp_drv_pkt_data, 0, sizeof(bdpp_drv_pkt_data));
	
	// Initialize the free driver-owned packet list
	for (i = 0; i < BDPP_MAX_DRIVER_PACKETS; i++) {
		bdpp_drv_pkt_header[i].indexes = (uint8_t)i;
		bdpp_drv_pkt_header[i].data = bdpp_drv_pkt_data[i];
		bdpp_free_queue.push(&bdpp_drv_pkt_header[i]);
	}
	
	// Initialize the free app-owned packet list
	for (i = 0; i < BDPP_MAX_APP_PACKETS; i++) {
		bdpp_app_pkt_header[i].indexes = (uint8_t)i;
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
	disable_interrupts();
	rc = (bdpp_tx_state != BDPP_TX_STATE_IDLE ||
		bdpp_rx_state != BDPP_RX_STATE_AWAIT_START ||
		bdpp_tx_packet != NULL ||
		bdpp_rx_packet != NULL ||
		bdpp_tx_queue.size() != 0 ||
		bdpp_tx_build_packet != NULL);
	enable_interrupts();
	return rc;
}

// Enable BDDP mode for a specific stream
//
bool_t bdpp_enable(uint8_t stream) {
	if (bdpp_driver_flags & BDPP_FLAG_ALLOWED && stream < BDPP_MAX_STREAMS) {
		bdpp_flush_drv_tx_packet();
		bdpp_tx_next_stream = stream;
		if (!(bdpp_driver_flags & BDPP_FLAG_ENABLED)) {
			bdpp_driver_flags |= BDPP_FLAG_ENABLED;
			//set_vector(UART0_IVECT, bdpp_handler);
		}
		return true;
	} else {
		return false;
	}
}

// Disable BDDP mode
//
bool_t bdpp_disable() {
	if (bdpp_driver_flags & BDPP_FLAG_ALLOWED) {
		if (bdpp_driver_flags & BDPP_FLAG_ENABLED) {
			while (bdpp_is_busy()); // wait for BDPP to be fully idle
			bdpp_driver_flags &= ~BDPP_FLAG_ENABLED;
			//set_vector(UART0_IVECT, uart0_handler);
		}
		return true;
	} else {
		return false;
	}
}

// Initialize an outgoing driver-owned packet, if one is available
// Returns NULL if no packet is available.
//
BDPP_PACKET* bdpp_init_tx_drv_packet(uint8_t flags, uint8_t stream) {
	if (!bdpp_free_queue.size()) {
		return NULL;
	}

	BDPP_PACKET* packet = bdpp_free_queue.front();
	bdpp_free_queue.pop();
	packet->flags = flags & BDPP_PKT_FLAG_USAGE_BITS;
	packet->indexes = (packet->indexes & BDPP_PACKET_INDEX_BITS) | (stream << 4);
	packet->max_size = BDPP_SMALL_DATA_SIZE;
	packet->act_size = 0;
#if DEBUG_BDPP
	debug_log("bdpp_init_tx_drv_packet() -> %p\n", packet);
#endif
	return packet;
}

// Initialize an incoming driver-owned packet, if one is available
// Returns NULL if no packet is available.
//
BDPP_PACKET* bdpp_init_rx_drv_packet() {
	if (!bdpp_free_queue.size()) {
		return NULL;
	}

	BDPP_PACKET* packet = bdpp_free_queue.front();
	bdpp_free_queue.pop();
	packet->flags = 0;
	packet->max_size = BDPP_SMALL_DATA_SIZE;
	packet->act_size = 0;
#if DEBUG_BDPP
	debug_log("bdpp_init_rx_drv_packet() -> %p\n", packet);
#endif
	return packet;
}

// Queue an app-owned packet for transmission
// The packet is expected to be full when this function is called.
// This function can fail if the packet is presently involved in a data transfer.
//
bool_t bdpp_queue_tx_app_packet(uint8_t indexes, uint8_t flags, const uint8_t* data, uint16_t size) {
	uint8_t index;
#if DEBUG_BDPP
	debug_log("bdpp_queue_tx_app_packet(%02hX,%02hX,%p,%04hX)\n", indexes, flags, data, size);
#endif
	index = indexes & BDPP_PACKET_INDEX_BITS;
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		disable_interrupts();
		if (bdpp_rx_packet == packet || bdpp_tx_packet == packet) {
			enable_interrupts();
			return false;
		}
		flags &= ~(BDPP_PKT_FLAG_DONE|BDPP_PKT_FLAG_FOR_RX);
		flags |= BDPP_PKT_FLAG_APP_OWNED|BDPP_PKT_FLAG_READY;
		packet->flags = flags;
		packet->indexes = indexes;
		packet->max_size = size;
		packet->act_size = size;
		packet->data = (uint8_t*)data;
		bdpp_tx_queue.push(packet);
		//UART0_enable_interrupt(UART_IER_TRANSMITINT);
		enable_interrupts();
		return true;
	}
	return false;
}

// Check whether an incoming app-owned packet has been received
//
bool_t bdpp_is_rx_app_packet_done(uint8_t index) {
	bool_t rc;
	if (bdpp_is_allowed() && (index < BDPP_MAX_APP_PACKETS)) {
		BDPP_PACKET* packet = &bdpp_app_pkt_header[index];
		disable_interrupts();
		rc = ((packet->flags & (BDPP_PKT_FLAG_FOR_RX|BDPP_PKT_FLAG_DONE)) ==
				(BDPP_PKT_FLAG_FOR_RX|BDPP_PKT_FLAG_DONE));
		enable_interrupts();
#if DEBUG_BDPP
		debug_log("bdpp_is_rx_app_packet_done(%02hX) -> %hX\n", index, rc);
#endif
		return rc;
	}
#if DEBUG_BDPP
	debug_log("bdpp_is_rx_app_packet_done(%02hX) -> %hX\n", index, false);
#endif
	return false;
}

// Start building a device-owned, outgoing packet.
// If there is an existing packet being built, it will be flushed first.
// This returns NULL if there is no packet available.
//
BDPP_PACKET* bdpp_start_drv_tx_packet(uint8_t flags, uint8_t stream) {
	BDPP_PACKET* packet;
	bdpp_flush_drv_tx_packet();
	packet = bdpp_init_tx_drv_packet(flags, stream);
#if DEBUG_BDPP
	debug_log("bdpp_start_drv_tx_packet(%02hX, %02hX) -> %p\n", flags, stream, packet);
#endif
	return packet;
}

// Flush the currently-being-built, driver-owned, outgoing packet, if any exists.
//
static void bdpp_internal_flush_drv_tx_packet() {
	if (bdpp_tx_build_packet) {
#if DEBUG_BDPP
		debug_log("bdpp_internal_flush_drv_tx_packet() flushing %p\n", bdpp_tx_build_packet);
#endif
		disable_interrupts();
			bdpp_tx_build_packet->flags |= BDPP_PKT_FLAG_READY;
			bdpp_tx_queue.push(bdpp_tx_build_packet);
			bdpp_tx_build_packet = NULL;
			//UART0_enable_interrupt(UART_IER_TRANSMITINT);
		enable_interrupts();
	}
}

// Append a data byte to a driver-owned, outgoing packet.
// This is a blocking call, and might wait for room for data.
static void bdpp_internal_write_byte_to_drv_tx_packet(uint8_t data) {
#if DEBUG_BDPP
	debug_log("bdpp_internal_write_byte_to_drv_tx_packet(%02hX)\n", data);
#endif
	while (true) {
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
			bdpp_tx_build_packet = bdpp_init_tx_drv_packet(bdpp_tx_next_pkt_flags, bdpp_tx_next_stream);
		}
	}
}

// Append a data byte to a driver-owned, outgoing packet.
// This is a blocking call, and might wait for room for data.
void bdpp_write_byte_to_drv_tx_packet(uint8_t data) {
#if DEBUG_BDPP
	debug_log("bdpp_write_byte_to_drv_tx_packet(%02hX)\n", data);
#endif
	if (bdpp_is_allowed()) {
		bdpp_internal_write_byte_to_drv_tx_packet(data);
	}
}

// Append multiple data bytes to one or more driver-owned, outgoing packets.
// This is a blocking call, and might wait for room for data.
void bdpp_write_bytes_to_drv_tx_packet(const uint8_t* data, uint16_t count) {
#if DEBUG_BDPP
	debug_log("bdpp_write_bytes_to_drv_tx_packet(%p, %04hX)\n", data, count);
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
#if DEBUG_BDPP
	debug_log("bdpp_write_drv_tx_byte_with_usage(%02hX)\n", data);
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
#if DEBUG_BDPP
	debug_log("bdpp_write_drv_tx_bytes_with_usage(%p, %04hX) [%02hX]\n", data, count, *data);
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
#if DEBUG_BDPP
		debug_log("bdpp_flush_drv_tx_packet(%p)\n", bdpp_tx_build_packet);
#endif
		bdpp_tx_build_packet->flags |= BDPP_PKT_FLAG_LAST;
		bdpp_tx_next_stream = (bdpp_tx_build_packet->indexes >> 4);
		bdpp_internal_flush_drv_tx_packet();
		bdpp_tx_next_pkt_flags = 0;
	}
}

uhci_event_t event;
uint8_t* pr = NULL;
uint8_t* pw = NULL;

extern uint32_t dma_data_len[2];
extern uint32_t hold_intr_mask;
extern volatile uint8_t* dma_data_in[2];
#define PACKET_DATA_SIZE (26*2+1+3)

int uart_dma_read(int uhci_num, uint8_t *addr, size_t read_size, TickType_t ticks_to_wait);
int uart_dma_write(int uhci_num, uint8_t *pbuf, size_t wr);

void read_task(void *param)
{
	dma_data_len[0] = 0;
	dma_data_len[1] = 0;
	dma_data_in[0] = 0;
	dma_data_in[1] = 0;
	memset(pr, 0, PACKET_DATA_SIZE+1);
	auto len = uart_dma_read(0, pr, PACKET_DATA_SIZE, (portTickType)100);
	for (int n = 0; n < 100; n++) {
		int i;
		debug_log(".");
		int total = 0;
		for (i=0;i<600;i++) {
			total = dma_data_len[0]+dma_data_len[1];
			if (total >= PACKET_DATA_SIZE) {
				break;
			}
			vTaskDelay(2);
		}

		if (hold_intr_mask) {
			debug_log("/intr %X/",hold_intr_mask);
			hold_intr_mask = 0;
		}

		for (int j=0; j<2;j++) {
			if (dma_data_in[j]) {
				debug_log("/len %d/ ", total);
				dma_data_in[j][total]=0;
				debug_log("%s\n",dma_data_in[j]);
				dma_data_in[j]=0;
				dma_data_len[j]=0;
			}
		}
	}
    while(1) {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void write_task(void *param)
{
	debug_log("enter write_task\n");
    for(int i = 0; i < PACKET_DATA_SIZE; i++) {
        pw[i] = 'a'+ i;
		debug_log(" [%i] %02hX",i,pw[i]);
    }
	debug_log("\n");
    vTaskDelay(100/portTICK_PERIOD_MS);
    for(int i = 0; i < 20; i++) {
		debug_log("%i: call uart_dma_write %p\n", i, pw);
        uart_dma_write(UHCI_NUM, pw, 6);
        vTaskDelay(2);
		if (hold_intr_mask) {
			debug_log("\n/intr %X/",hold_intr_mask);
			hold_intr_mask = 0;
		}
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
	debug_log("finished write_task\n");
    while(1) {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

#include "hal/uart_ll.h"

void dump_uart_regs() {
	return;
	auto dev = UART_LL_GET_HW(UART_NUM);
	auto size = sizeof(*dev) & 0xFFFFFFFC;
	auto p = (uint32_t*)dev;
	for (int i = 0; i < size; i+=sizeof(uint32_t)) {
		debug_log("[%04X] %08X\n", i, *p++);
	}
	debug_log("\n");
}

void bdpp_run_test() {
	debug_log("@%i enter bdpp_run_test\n", __LINE__);
	debug_log("\n\n--- Before Serial2.end() ---\n");
	dump_uart_regs();
	Serial2.end(); // stop existing communication
	debug_log("\n\n--- After Serial2.end() ---\n");
	dump_uart_regs();
	debug_log("@%i\n", __LINE__);

    uart_config_t uart_config = {
        .baud_rate = 1152000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 120,
		.source_clk = UART_SCLK_APB
    };

    uhci_driver_install(UHCI_NUM, 1024*2, 1024*2, 0);
	debug_log("@%i\n", __LINE__);

    uhci_attach_uart_port(UHCI_NUM, UART_NUM, &uart_config);
	debug_log("@%i\n", __LINE__);

	debug_log("\n\n--- After uhci_attach_uart_port() ---\n");
	dump_uart_regs();

    pr = (uint8_t *)heap_caps_calloc(1, 1024, MALLOC_CAP_DMA|MALLOC_CAP_8BIT);
    if(pr == NULL) {
        debug_log("SRAM RX malloc fail\n");
        return;
    }

	debug_log("@%i\n", __LINE__);
    pw = (uint8_t *)heap_caps_calloc(1, 1024, MALLOC_CAP_DMA|MALLOC_CAP_8BIT);
    if(pw == NULL) {
        debug_log("SRAM TX malloc fail\n");
        return;
    }

	debug_log("@%i\n", __LINE__);
    xTaskCreate(read_task, "read_task", 2048*4, NULL, 12, NULL);
    vTaskDelay(20/portTICK_PERIOD_MS);
	debug_log("@%i\n", __LINE__);

    //xTaskCreate(write_task, "write_task", 2048, NULL, 12, NULL);
	debug_log("@%i\n", __LINE__);

	debug_log("@%i leave bdpp_run_test\n", __LINE__);
    while(1) {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}
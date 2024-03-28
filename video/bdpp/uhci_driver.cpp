//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <string.h>
#include "esp_types.h"
#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "esp_err.h"
#include "malloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/xtensa_api.h"
#include "freertos/ringbuf.h"
#include "bdpp/uhci_hal.h"
#include "driver/uart.h"
#include "bdpp/uhci_driver.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "sdkconfig.h"
#include "soc/periph_defs.h"
#include "rom/lldesc.h"
#include "hal/uart_hal.h"
#include "bdp_protocol.h"
#include <queue>

extern std::queue<Packet*> bdpp_tx_queue; // Transmit (TX) packet queue
extern std::queue<UhciPacket*> bdpp_rx_queue[BDPP_MAX_STREAMS]; // Receive (RX) packet queue

typedef struct {
    Packet*             tx_pkt;

    union {
    uint32_t            align1;
    lldesc_t            tx_dma;
    };

    union {
    uint32_t            align2;
    lldesc_t            rx_dma[BDPP_MAX_RX_PACKETS];
    };

    union {
    uint32_t            align3;
    UhciPacket          rx_pkt[BDPP_MAX_RX_PACKETS];
    };

    uart_hal_context_t  uart_hal;
    uhci_hal_context_t  uhci_hal;
    intr_handle_t       intr_handle;
    int                 uhci_num;
} uhci_obj_t;

uhci_obj_t* uhci_obj = NULL;

static void IRAM_ATTR uhci_isr_handler_for_bdpp(void *param)
{
    while (1) {
        uint32_t intr_mask = uhci_hal_get_intr(&(uhci_obj->uhci_hal));
        if (intr_mask == 0) {
            break;
        }
        uhci_hal_clear_intr(&(uhci_obj->uhci_hal), intr_mask);

        // handle RX interrupt */
        if (intr_mask & (UHCI_INTR_IN_DONE | UHCI_INTR_IN_SUC_EOF | UHCI_INTR_TX_HUNG|UHCI_INTR_RX_HUNG)) {
            lldesc_t* descr = (lldesc_t*) uhci_obj->uhci_hal.dev->dma_in_suc_eof_des_addr;
            if (descr) {
                int dma_index = descr - uhci_obj->rx_dma;
                if (dma_index <= BDPP_MAX_RX_PACKETS) {
                    if (descr->length >= sizeof(UhciPacket)-BDPP_MAX_PACKET_DATA_SIZE) {
                        // provide this packet to the app
                        auto packet = &uhci_obj->rx_pkt[dma_index];
                        //show_rx_packet(packet);
                        packet->set_flags(BDPP_PKT_FLAG_DONE);
                        bdpp_rx_queue[packet->get_stream_index()].push(packet);                
                    }
                }
            }
        }

        /* handle TX interrupt */
        if (intr_mask & (UHCI_INTR_OUT_EOF)) {
            auto packet = uhci_obj->tx_pkt;
            if (packet) {
                packet->get_uhci_packet()->set_flags(BDPP_PKT_FLAG_DONE);
                delete packet;
                uhci_obj->tx_pkt = NULL;
            }
            if (bdpp_tx_queue.size()) {
                    auto packet = bdpp_tx_queue.front();
                    bdpp_tx_queue.pop();
                    uhci_obj->tx_pkt = packet;
                    uart_dma_write(UHCI_NUM_0, packet->get_uhci_data(),
                        packet->get_uhci_packet()->get_transfer_size()); 
            } else {
                    uhci_hal_disable_intr(&uhci_obj->uhci_hal, UHCI_INTR_OUT_EOF);
            }
        }
    }
}

void uart_dma_read()
{
    for (uint32_t i = 0; i < BDPP_MAX_RX_PACKETS; i++) {
        auto dma = &uhci_obj->rx_dma[i];
        auto packet = &uhci_obj->rx_pkt[i];
        auto alloc_size = Packet::get_alloc_size(BDPP_MAX_PACKET_DATA_SIZE);
        dma->buf = (volatile uint8_t *) packet;
        dma->eof = 1;
        dma->owner = 1;
        dma->size = alloc_size;
        dma->length = 0;
        dma->offset = 0;
        dma->sosf = 0;
        auto next = (i >= BDPP_MAX_RX_PACKETS-1 ? 0 : i + 1);
        dma->empty = (uint32_t) &uhci_obj->rx_dma[next]; // actually, 'qe' field
        packet->flags = BDPP_PKT_FLAG_FOR_RX;
    }

    auto hal = &(uhci_obj->uhci_hal);
    uhci_hal_rx_dma_restart(hal);
    uhci_hal_set_rx_dma(hal, (uint32_t)(uhci_obj->rx_dma));

    uhci_enable_interrupts(UHCI_INTR_IN_DONE|UHCI_INTR_IN_SUC_EOF|UHCI_INTR_TX_HUNG|UHCI_INTR_RX_HUNG);
    uhci_hal_rx_dma_start(hal);
}

int uart_dma_write(int uhci_num, uint8_t *pbuf, size_t wr)
{
    uhci_obj->tx_dma.owner = 1;
    uhci_obj->tx_dma.eof = 1;
    uhci_obj->tx_dma.buf = pbuf;
    uhci_obj->tx_dma.length = wr;
    uhci_obj->tx_dma.size = (wr+3)&0xFFFFFFFC;
    uhci_obj->tx_dma.empty = 0; // actually 'qe' (ptr to next descr)
    auto hal = &(uhci_obj->uhci_hal);
    uhci_hal_set_tx_dma(hal, (uint32_t)(&(uhci_obj->tx_dma)));
    uhci_hal_tx_dma_start(hal);
    return 0;
}

void uart_dma_start_transmitter() {
        auto old_int = uhci_disable_interrupts();
        if (!uhci_obj->tx_pkt) {
                if (bdpp_tx_queue.size()) {
                        auto packet = bdpp_tx_queue.front();
                        bdpp_tx_queue.pop();
                        uhci_obj->tx_pkt = packet;
                        uart_dma_write(UHCI_NUM_0, packet->get_uhci_data(),
                            packet->get_uhci_packet()->get_transfer_size());             
                        old_int |= UHCI_INTR_OUT_EOF;
                }
        }
        uhci_enable_interrupts(old_int);
}

esp_err_t uhci_driver_install(int uhci_num, int intr_flag)
{
    uhci_obj = (uhci_obj_t*) heap_caps_malloc(sizeof(uhci_obj_t), MALLOC_CAP_DMA | MALLOC_CAP_32BIT | MALLOC_CAP_8BIT);
    memset(uhci_obj, 0, sizeof(uhci_obj_t));
    uhci_obj->uhci_num = uhci_num;
    periph_module_enable(PERIPH_UHCI0_MODULE);
    return esp_intr_alloc(ETS_UHCI0_INTR_SOURCE, intr_flag, &uhci_isr_handler_for_bdpp,
                            uhci_obj, &uhci_obj->intr_handle);
}

esp_err_t uhci_attach_uart_port(int uhci_num, int uart_num, const uart_config_t *uart_config)
{
    {
        // Configure UART params
        auto hal = &(uhci_obj->uart_hal);
        hal->dev = UART_LL_GET_HW(uart_num);
        uart_hal_init(hal, uart_num);
        periph_module_enable(uart_periph_signal[uart_num].module);
        uart_hal_disable_intr_mask(hal, ~0);
        uart_param_config(uart_num, uart_config);
        uart_hal_set_loop_back(hal, false);
        //uart_ll_set_rx_tout(hal->dev, 255*8); // 2040 baud bit times (1.5 character times?)
        uart_ll_set_rx_tout(hal->dev, 0); // no timeout
        uart_set_pin(uart_num, 2, 34, 13, 14);
    }
    {
        //Configure UHCI param
        uhci_seper_chr_t seper_char = { 0x89, 0x8B, 0x8A, 0x8B, 0x8D, 0x01 };
        auto hal = &(uhci_obj->uhci_hal);
        uhci_hal_init(hal, uhci_num);
        uhci_hal_disable_intr(hal, UHCI_INTR_MASK);
        uhci_hal_set_eof_mode(hal, 0);
        uhci_hal_attach_uart_port(hal, uart_num);
        uhci_hal_set_seper_chr(hal, &seper_char);
        uhci_hal_clear_intr(hal, UHCI_INTR_MASK);
    }
    return ESP_OK;
}

uint32_t uhci_disable_interrupts() {
	auto hal = &(uhci_obj->uhci_hal);
	auto old_int = uhci_hal_get_enabled_intr(hal);
        uhci_hal_disable_intr(hal, ~0);
        return old_int;
}

void uhci_enable_interrupts(uint32_t old_int) {
	auto hal = &(uhci_obj->uhci_hal);
        uhci_hal_enable_intr(hal, old_int);
}

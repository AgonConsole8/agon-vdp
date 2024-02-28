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
extern std::queue<Packet*> bdpp_rx_queue[BDPP_MAX_STREAMS]; // Receive (RX) packet queue
extern std::queue<Packet*> bdpp_free_queue; // Free packet queue for RX

#define DMA_TX_IDLE_NUM (0)
#define DMA_RX_IDLE_THRD (20)
#define DMA_RX_DESC_CNT (4)
#define DMA_TX_DESC_CNT (1)
#define DMA_RX_BUF_SZIE (128)
#define DMA_TX_BUF_SIZE  (256)

typedef struct {
    uart_hal_context_t uart_hal;
    uhci_hal_context_t uhci_hal;
    intr_handle_t intr_handle;
    int uhci_num;
    lldesc_t rx_dma[BDPP_MAX_RX_PACKETS];
    lldesc_t tx_dma;
    lldesc_t* rx_cur;
    Packet* rx_pkt[BDPP_MAX_RX_PACKETS];
    Packet* tx_pkt;
} uhci_obj_t;

uhci_obj_t uhci_obj = {0};

static void IRAM_ATTR uhci_isr_handler_for_bdpp(void *param)
{
    while (1) {
        uint32_t intr_mask = uhci_hal_get_intr(&(uhci_obj.uhci_hal));
        if (intr_mask == 0) {
            break;
        }
        uhci_hal_clear_intr(&(uhci_obj.uhci_hal), intr_mask);

        // handle RX interrupt */
        if (intr_mask & (UHCI_INTR_IN_DONE | UHCI_INTR_IN_SUC_EOF | UHCI_INTR_TX_HUNG|UHCI_INTR_RX_HUNG)) {
            lldesc_t* descr = uhci_obj.rx_cur;
            //lldesc_t* descr = (lldesc_t*) uhci_obj.uhci_hal.dev->?;
            int dma_index = descr - uhci_obj.rx_dma;
            auto packet = uhci_obj.rx_pkt[dma_index];
            bdpp_rx_queue[packet->get_stream_index()].push(packet);

            packet->set_flags(BDPP_PKT_FLAG_DONE);
            packet->clear_flags(BDPP_PKT_FLAG_READY);
            if (dma_index < 3) {
                (uhci_obj.rx_cur)++;
            } else {
                uhci_obj.rx_cur = uhci_obj.rx_dma;
            }
            packet = Packet::create_rx_packet();
            uhci_obj.rx_pkt[dma_index] = packet;
            auto dma = &uhci_obj.rx_dma[dma_index];
            dma->buf = packet->get_uhci_data();
            dma->size = packet->get_maximum_data_size();
            dma->length = 0;
        }

        /* handle TX interrupt */
        if (intr_mask & (UHCI_INTR_OUT_EOF)) {
            auto packet = uhci_obj.tx_pkt;
            if (packet) {
                packet->clear_flags(BDPP_PKT_FLAG_READY);
                packet->set_flags(BDPP_PKT_FLAG_DONE);
                delete packet;
                uhci_obj.tx_pkt = NULL;
            }
               if (bdpp_tx_queue.size()) {
                        auto packet = bdpp_tx_queue.front();
                        bdpp_tx_queue.pop();
                        uhci_obj.tx_pkt = packet;
                        uart_dma_write(UHCI_NUM_0, packet->get_uhci_data(), packet->get_transfer_size()); 
                } else {
                        uhci_hal_disable_intr(&uhci_obj.uhci_hal, UHCI_INTR_OUT_EOF);
                }
        }
    }
}

int uart_dma_read(int uhci_num)
{
    auto hal = &(uhci_obj.uhci_hal);

    uhci_obj.rx_cur = uhci_obj.rx_dma;
    uhci_obj.rx_pkt[0] = Packet::create_rx_packet();
    auto alloc_size = Packet::get_alloc_size(uhci_obj.rx_pkt[0]->get_maximum_data_size());
    uhci_obj.rx_dma[0].owner = 1;
    uhci_obj.rx_dma[0].eof = 1;
    uhci_obj.rx_dma[0].size = alloc_size;
    uhci_obj.rx_dma[0].length = 0;
    uhci_obj.rx_dma[0].empty = (uint32_t)&uhci_obj.rx_dma[1]; // actually 'qe' (ptr to next descr)
    uhci_obj.rx_dma[0].buf = uhci_obj.rx_pkt[0]->get_uhci_data();
    uhci_obj.rx_dma[0].offset = 0;
    uhci_obj.rx_dma[0].sosf = 0;

    uhci_obj.rx_pkt[1] = Packet::create_rx_packet();
    uhci_obj.rx_dma[1].owner = 1;
    uhci_obj.rx_dma[1].eof = 1;
    uhci_obj.rx_dma[1].size = alloc_size;
    uhci_obj.rx_dma[1].length = 0;
    uhci_obj.rx_dma[1].empty = (uint32_t)&uhci_obj.rx_dma[2]; // actually 'qe' (ptr to next descr)
    uhci_obj.rx_dma[1].buf = uhci_obj.rx_pkt[1]->get_uhci_data();
    uhci_obj.rx_dma[1].offset = 0;
    uhci_obj.rx_dma[1].sosf = 0;

    uhci_obj.rx_pkt[2] = Packet::create_rx_packet();
    uhci_obj.rx_dma[2].owner = 1;
    uhci_obj.rx_dma[2].eof = 1;
    uhci_obj.rx_dma[2].size = alloc_size;
    uhci_obj.rx_dma[2].length = 0;
    uhci_obj.rx_dma[2].empty = (uint32_t)&uhci_obj.rx_dma[3]; // actually 'qe' (ptr to next descr)
    uhci_obj.rx_dma[2].buf = uhci_obj.rx_pkt[2]->get_uhci_data();
    uhci_obj.rx_dma[2].offset = 0;
    uhci_obj.rx_dma[2].sosf = 0;

    uhci_obj.rx_pkt[3] = Packet::create_rx_packet();
    uhci_obj.rx_dma[3].owner = 1;
    uhci_obj.rx_dma[3].eof = 1;
    uhci_obj.rx_dma[3].size = alloc_size;
    uhci_obj.rx_dma[3].length = 0;
    uhci_obj.rx_dma[3].empty = (uint32_t)&uhci_obj.rx_dma[0]; // actually 'qe' (ptr to next descr)
    uhci_obj.rx_dma[3].buf = uhci_obj.rx_pkt[3]->get_uhci_data();
    uhci_obj.rx_dma[3].offset = 0;
    uhci_obj.rx_dma[3].sosf = 0;

    uhci_hal_rx_dma_restart(hal);
    uhci_hal_set_rx_dma(hal, (uint32_t)(&(uhci_obj.rx_dma[0])));

    uhci_enable_interrupts(UHCI_INTR_IN_DONE|UHCI_INTR_IN_SUC_EOF|UHCI_INTR_TX_HUNG|UHCI_INTR_RX_HUNG);
    uhci_hal_rx_dma_start(hal);
    return 0;
}

int uart_dma_write(int uhci_num, uint8_t *pbuf, size_t wr)
{
    uhci_obj.tx_dma.owner = 1;
    uhci_obj.tx_dma.eof = 1;
    uhci_obj.tx_dma.buf = pbuf;
    uhci_obj.tx_dma.length = wr;
    uhci_obj.tx_dma.size = (wr+3)&0xFFFFFFFC;
    uhci_obj.tx_dma.empty = 0; // actually 'qe' (ptr to next descr)
    auto hal = &(uhci_obj.uhci_hal);
    uhci_hal_set_tx_dma(hal, (uint32_t)(&(uhci_obj.tx_dma)));
    uhci_hal_tx_dma_start(hal);
    return 0;
}

void uart_dma_start_transmitter() {
        auto old_int = uhci_disable_interrupts();
        if (!uhci_obj.tx_pkt) {
                if (bdpp_tx_queue.size()) {
                        auto packet = bdpp_tx_queue.front();
                        bdpp_tx_queue.pop();
                        uhci_obj.tx_pkt = packet;
                        uart_dma_write(UHCI_NUM_0, packet->get_uhci_data(), packet->get_transfer_size());             
                        old_int |= UHCI_INTR_OUT_EOF;
                }
        }
        uhci_enable_interrupts(old_int);
}

esp_err_t uhci_driver_install(int uhci_num, int intr_flag)
{
    uhci_obj.uhci_num = uhci_num;
    periph_module_enable(PERIPH_UHCI0_MODULE);
    return esp_intr_alloc(ETS_UHCI0_INTR_SOURCE, intr_flag, &uhci_isr_handler_for_bdpp,
                            &uhci_obj, &uhci_obj.intr_handle);
}

esp_err_t uhci_attach_uart_port(int uhci_num, int uart_num, const uart_config_t *uart_config)
{
    {
        // Configure UART params
        auto hal = &(uhci_obj.uart_hal);
        hal->dev = UART_LL_GET_HW(uart_num);
        uart_hal_init(hal, uart_num);
        periph_module_enable(uart_periph_signal[uart_num].module);
        uart_hal_disable_intr_mask(hal, ~0);
        uart_param_config(uart_num, uart_config);
        uart_hal_set_loop_back(hal, false);
        uart_ll_set_rx_tout(hal->dev, 255*8); // 2040 baud bit times (1.5 character times?)
        //uart_ll_set_rx_tout(hal->dev, 0); // no timeout
        uart_set_pin(uart_num, 2, 34, 13, 14);
    }
    {
        //Configure UHCI param
        uhci_seper_chr_t seper_char = { 0x89, 0x8B, 0x8A, 0x8B, 0x8D, 0x01 };
        auto hal = &(uhci_obj.uhci_hal);
        uhci_hal_init(hal, uhci_num);
        uhci_hal_disable_intr(hal, UHCI_INTR_MASK);
        uhci_hal_set_eof_mode(hal, UHCI_RX_EOF_MAX);
        uhci_hal_attach_uart_port(hal, uart_num);
        uhci_hal_set_seper_chr(hal, &seper_char);
        hal->dev->conf0.len_eof_en = 1;
        //hal->dev->conf0.uart_idle_eof_en = 1;
        uhci_hal_clear_intr(hal, UHCI_INTR_MASK);
    }
    return ESP_OK;
}

uint32_t uhci_disable_interrupts() {
	auto hal = &(uhci_obj.uhci_hal);
	auto old_int = uhci_hal_get_enabled_intr(hal);
        uhci_hal_disable_intr(hal, ~0);
        return old_int;
}

void uhci_enable_interrupts(uint32_t old_int) {
	auto hal = &(uhci_obj.uhci_hal);
        uhci_hal_enable_intr(hal, old_int);
}

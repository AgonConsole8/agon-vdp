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
extern std::queue<Packet*> bdpp_rx_queue; // Receive (RX) packet queue
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

extern void debug_log(const char* fmt, ...);

static void IRAM_ATTR uhci_isr_handler_for_bdpp(void *param)
{
    while (1) {
        uint32_t intr_mask = uhci_hal_get_intr(&(uhci_obj.uhci_hal));
        if (intr_mask == 0) {
            break;
        }
        uhci_hal_clear_intr(&(uhci_obj.uhci_hal), intr_mask);
        //debug_log("* INT %X *\n", intr_mask);

        // handle RX interrupt */
        if (intr_mask & (UHCI_INTR_IN_DONE | UHCI_INTR_IN_SUC_EOF)) {
            lldesc_t* descr = uhci_obj.rx_cur;
            int index = descr - uhci_obj.rx_dma;
            bdpp_rx_queue.push(uhci_obj.rx_pkt[index]);

            //debug_log("irq %i %u %u\n",index,descr->size,descr->length);
            auto packet = uhci_obj.rx_pkt[index];
            packet->set_flags(BDPP_PKT_FLAG_DONE);
            packet->clear_flags(BDPP_PKT_FLAG_READY);
            /*debug_log("irq Packet: %X, %02hX, %02hx, %02hX, %u\n",
                packet,
                packet->get_flags(),
                packet->get_packet_index(),
                packet->get_stream_index(),
                packet->get_actual_data_size());*/
            if (index < 3) {
                (uhci_obj.rx_cur)++;
            } else {
                uhci_obj.rx_cur = uhci_obj.rx_dma;
            }
            //debug_log("@%i\n",__LINE__);
            uhci_obj.rx_pkt[index] = Packet::create_rx_packet();
            //debug_log("@%i\n",__LINE__);
            uhci_obj.rx_dma[index].buf = uhci_obj.rx_pkt[index]->get_uhci_data();
            //debug_log("@%i\n",__LINE__);
            uhci_obj.rx_dma[index].size = uhci_obj.rx_pkt[index]->get_maximum_data_size();
            //debug_log("@%i\n",__LINE__);
            uhci_obj.rx_dma[index].length = 0;
            //debug_log("@%i\n",__LINE__);
        }

        /* handle TX interrupt */
        if (intr_mask & (UHCI_INTR_OUT_EOF)) {
            //debug_log("@%i\n",__LINE__);
            //uart_dma_write(int uhci_num, uint8_t *pbuf, size_t wr)
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

    //debug_log("rx dma 0 %X\n", &(uhci_obj.rx_dma[0]));
    //debug_log("rx dma 1 %X\n", &(uhci_obj.rx_dma[1]));
    uhci_hal_rx_dma_restart(hal);
    uhci_hal_set_rx_dma(hal, (uint32_t)(&(uhci_obj.rx_dma[0])));

    uhci_enable_interrupts(UHCI_INTR_IN_DONE);
    uhci_hal_rx_dma_start(hal);
    return 0;
}

int uart_dma_write(int uhci_num, uint8_t *pbuf, size_t wr)
{
    debug_log("enter uart_dma_write\n");

    uhci_obj.tx_dma.owner = 1;
    uhci_obj.tx_dma.eof = 1;
    uhci_obj.tx_dma.buf = pbuf;
    uhci_obj.tx_dma.length = wr;
    uhci_obj.tx_dma.size = (wr+3)&0xFFFFFFFC;
    uhci_obj.tx_dma.empty = 0; // actually 'qe' (ptr to next descr)
    auto hal = &(uhci_obj.uhci_hal);
    uhci_hal_set_tx_dma(hal, (uint32_t)(&(uhci_obj.tx_dma)));
    uhci_hal_tx_dma_start(hal);

    debug_log("leave uart_dma_write %u\n", (uint32_t)wr);
    return 0;
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
	debug_log("@%i\n", __LINE__);
//        periph_module_enable(uart_periph_signal[uart_num].module);
	debug_log("@%i\n", __LINE__);
        auto hal = &(uhci_obj.uart_hal);
        hal->dev = UART_LL_GET_HW(uart_num);
	debug_log("@%i uart hal %X, dev %X\n", __LINE__, hal, hal->dev);
        uart_hal_init(hal, uart_num);

	debug_log("@%i\n", __LINE__);
    /*periph_module_disable(uhci_periph_signal[uhci_num].module);
	debug_log("@%i\n", __LINE__);
    vTaskDelay(5);*/
	debug_log("@%i\n", __LINE__);
    periph_module_enable(uart_periph_signal[uart_num].module);

	debug_log("@%i\n", __LINE__);
        uart_hal_disable_intr_mask(hal, ~0);
	debug_log("@%i\n", __LINE__);
//        uart_hal_set_sclk(hal, uart_config->source_clk);
	debug_log("@%i\n", __LINE__);
/*        uart_hal_set_baudrate(hal, uart_config->baud_rate);
	debug_log("@%i\n", __LINE__);
        uart_hal_set_parity(hal, uart_config->parity);
	debug_log("@%i\n", __LINE__);
        uart_hal_set_data_bit_num(hal, uart_config->data_bits);
	debug_log("@%i\n", __LINE__);
        uart_hal_set_stop_bits(hal, uart_config->stop_bits);
	debug_log("@%i\n", __LINE__);
        uart_hal_set_tx_idle_num(hal, DMA_TX_IDLE_NUM);
	debug_log("@%i\n", __LINE__);
        uart_hal_set_hw_flow_ctrl(hal, uart_config->flow_ctrl, uart_config->rx_flow_ctrl_thresh);
        uart_hal_set_rts(hal, 0);
	debug_log("@%i\n", __LINE__);
*/
        // UART2 has no reset register for the FIFO, but may be affected via UART1.
        //auto dev1 = UART_LL_GET_HW(UART_NUM_1);
        //dev1->conf0.rxfifo_rst = 1;
        //dev1->conf0.txfifo_rst = 1;

	debug_log("@%i\n", __LINE__);
        uart_param_config(uart_num, uart_config);
	debug_log("@%i\n", __LINE__);
        uart_hal_set_loop_back(hal, false);
        uart_ll_set_rx_tout(hal->dev, 0); // use no timeout
        //uart_ll_set_rx_tout(hal->dev, 3*8); // 24 baud bit times (1.5 character times?)
	debug_log("@%i\n", __LINE__);
        //uart_hal_set_hw_flow_ctrl(hal, uart_config->flow_ctrl, uart_config->rx_flow_ctrl_thresh);
        //uart_hal_set_rts(hal, 1);
	debug_log("@%i\n", __LINE__);

    debug_log("@%i\n", __LINE__);
    uart_set_pin(uart_num, 2, 34, 13, 14);

    }
    {
        //Configure UHCI param
	debug_log("@%i\n", __LINE__);
        uhci_seper_chr_t seper_char = { 0x89, 0x8B, 0x8A, 0x8B, 0x8D, 0x01 };
	debug_log("@%i\n", __LINE__);
        auto hal = &(uhci_obj.uhci_hal);
        uhci_hal_init(hal, uhci_num);
	debug_log("@%i uhci hal %X, dev %X\n", __LINE__, hal, hal->dev);
	debug_log("@%i\n", __LINE__);
        uhci_hal_disable_intr(hal, UHCI_INTR_MASK);
	debug_log("@%i\n", __LINE__);
        uhci_hal_set_eof_mode(hal, UHCI_RX_EOF_MAX);
	debug_log("@%i\n", __LINE__);
        uhci_hal_attach_uart_port(hal, uart_num);
	debug_log("@%i\n", __LINE__);
        uhci_hal_set_seper_chr(hal, &seper_char);
	debug_log("@%i\n", __LINE__);
        //uhci_hal_set_rx_dma(hal,(uint32_t)(&(uhci_obj.rx_dma)));
	debug_log("@%i\n", __LINE__);
        //uhci_hal_set_tx_dma(hal,(uint32_t)(&(uhci_obj.tx_dma)));
	debug_log("@%i\n", __LINE__);
        uhci_hal_clear_intr(hal, UHCI_INTR_MASK);
	debug_log("@%i\n", __LINE__);
        uhci_hal_enable_intr(hal, UHCI_INTR_IN_DONE | UHCI_INTR_IN_SUC_EOF | UHCI_INTR_TX_HUNG);
        //uhci_hal_enable_intr(hal, 0x0001FFFF);
	debug_log("@%i\n", __LINE__);
        //uhci_hal_rx_dma_start(hal);
	debug_log("@%i\n", __LINE__);
        //uhci_hal_tx_dma_start(hal);
	debug_log("@%i\n", __LINE__);
    }
    return ESP_OK;
}

uint32_t uhci_disable_interrupts() {
	//debug_log("uhci_disable_interrupts\n");
	auto hal = &(uhci_obj.uhci_hal);
	auto old_int = uhci_hal_get_enabled_intr(hal);
    uhci_hal_disable_intr(hal, ~0);
    return old_int;
}

void uhci_enable_interrupts(uint32_t old_int) {
	//debug_log("uhci_enable_interrupts\n");
	auto hal = &(uhci_obj.uhci_hal);
    uhci_hal_enable_intr(hal, old_int);
}


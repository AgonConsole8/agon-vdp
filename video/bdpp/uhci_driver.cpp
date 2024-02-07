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

#define DMA_TX_IDLE_NUM (0)
#define DMA_RX_IDLE_THRD (20)
#define DMA_RX_DESC_CNT (4)
#define DMA_TX_DESC_CNT (1)
#define DMA_RX_BUF_SZIE (128)
#define DMA_TX_BUF_SIZE  (256)

#define UHCI_ENTER_CRITICAL_ISR(uhci_num)   portENTER_CRITICAL_ISR(&uhci_spinlock[uhci_num])
#define UHCI_EXIT_CRITICAL_ISR(uhci_num)    portEXIT_CRITICAL_ISR(&uhci_spinlock[uhci_num])
#define UHCI_ENTER_CRITICAL(uhci_num)       portENTER_CRITICAL(&uhci_spinlock[uhci_num])
#define UHCI_EXIT_CRITICAL(uhci_num)        portEXIT_CRITICAL(&uhci_spinlock[uhci_num])

static const char* UHCI_TAG = "uhci";

#define UHCI_CHECK(a, str, ret_val) \
    if (!(a)) { \
        ESP_LOGE(UHCI_TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret_val); \
    }

typedef struct {
    uint8_t *buf;
    size_t size;
} rx_stash_item_t;

typedef struct {
    uint8_t *p;
    size_t size;
} uhci_wr_item;

typedef struct {
    uart_hal_context_t uart_hal;
    uhci_hal_context_t uhci_hal;
    intr_handle_t intr_handle;
    int uhci_num;
    lldesc_t rx_dma[4];
    lldesc_t tx_dma;
    lldesc_t* rx_cur;
} uhci_obj_t;

uhci_obj_t *uhci_obj[UHCI_NUM_MAX] = {0};

uart_dev_t uart_dev;
uhci_dev_t uhci_dev;

typedef uhci_obj_t uhci_handle_t;

static portMUX_TYPE uhci_spinlock[UHCI_NUM_MAX] = {
    portMUX_INITIALIZER_UNLOCKED,
#if UART_NUM_MAX > 1
    portMUX_INITIALIZER_UNLOCKED
#endif
};

typedef struct {
    const uint8_t irq;
    const periph_module_t module;
} uhci_signal_conn_t;

/*
 Bunch of constants for every UHCI peripheral: hw addr of registers and isr source
*/
static const uhci_signal_conn_t uhci_periph_signal[UHCI_NUM_MAX] = {
    {
        .irq = ETS_UHCI0_INTR_SOURCE,
        .module = PERIPH_UHCI0_MODULE,
    },
    {
        .irq = ETS_UHCI1_INTR_SOURCE,
        .module = PERIPH_UHCI1_MODULE,
    },
};

extern void debug_log(const char* fmt, ...);

uint32_t rx_count;
bool hung;
uint32_t dma_data_len[4];
uint32_t hold_intr_mask;
volatile uint8_t* dma_data_in[4];
#define PACKET_DATA_SIZE ((26+1+3)*2+2)

static void IRAM_ATTR uhci_isr_default_new(void *param)
{
    uhci_obj_t *p_obj = (uhci_obj_t *)param;
    while (1) {
        uint32_t intr_mask = uhci_hal_get_intr(&(p_obj->uhci_hal));
        if (intr_mask == 0) {
            break;
        }
        uhci_hal_clear_intr(&(p_obj->uhci_hal), intr_mask);
        hold_intr_mask |= intr_mask;

        // handle RX interrupt */
        if (intr_mask & (UHCI_INTR_IN_DONE | UHCI_INTR_IN_SUC_EOF)) {
            lldesc_t* descr = p_obj->rx_cur;
            int index = descr - p_obj->rx_dma;
            dma_data_len[index] = descr->length;
            //debug_log("i %i %u %u\n",index,descr->size,descr->length);
            if (index < 3) {
                (p_obj->rx_cur)++;
            } else {
                p_obj->rx_cur = p_obj->rx_dma;
            }
            rx_count++;
        }
        if (intr_mask & UHCI_INTR_TX_HUNG) {
            hung = true;
        }

        /* handle TX interrupt */
        if (intr_mask & (UHCI_INTR_OUT_EOF)) {
        }
    }
}

int uart_dma_read(int uhci_num)
{
    auto hal = &(uhci_obj[uhci_num]->uhci_hal);

    uhci_obj[uhci_num]->rx_cur = uhci_obj[uhci_num]->rx_dma;
    uhci_obj[uhci_num]->rx_dma[0].owner = 1;
    uhci_obj[uhci_num]->rx_dma[0].eof = 1;
    uhci_obj[uhci_num]->rx_dma[0].size = (PACKET_DATA_SIZE+7)&0xFFFFFFFC;
    uhci_obj[uhci_num]->rx_dma[0].length = 0;
    uhci_obj[uhci_num]->rx_dma[0].empty = (uint32_t)&uhci_obj[uhci_num]->rx_dma[1]; // actually 'qe' (ptr to next descr)
    uhci_obj[uhci_num]->rx_dma[0].buf = dma_data_in[0];
    uhci_obj[uhci_num]->rx_dma[0].offset = 0;
    uhci_obj[uhci_num]->rx_dma[0].sosf = 0;

    uhci_obj[uhci_num]->rx_dma[1].owner = 1;
    uhci_obj[uhci_num]->rx_dma[1].eof = 1;
    uhci_obj[uhci_num]->rx_dma[1].size = (PACKET_DATA_SIZE+7)&0xFFFFFFFC;
    uhci_obj[uhci_num]->rx_dma[1].length = 0;
    uhci_obj[uhci_num]->rx_dma[1].empty = (uint32_t)&uhci_obj[uhci_num]->rx_dma[2]; // actually 'qe' (ptr to next descr)
    uhci_obj[uhci_num]->rx_dma[1].buf = dma_data_in[1];
    uhci_obj[uhci_num]->rx_dma[1].offset = 0;
    uhci_obj[uhci_num]->rx_dma[1].sosf = 0;

    uhci_obj[uhci_num]->rx_dma[2].owner = 1;
    uhci_obj[uhci_num]->rx_dma[2].eof = 1;
    uhci_obj[uhci_num]->rx_dma[2].size = (PACKET_DATA_SIZE+7)&0xFFFFFFFC;
    uhci_obj[uhci_num]->rx_dma[2].length = 0;
    uhci_obj[uhci_num]->rx_dma[2].empty = (uint32_t)&uhci_obj[uhci_num]->rx_dma[3]; // actually 'qe' (ptr to next descr)
    uhci_obj[uhci_num]->rx_dma[2].buf = dma_data_in[2];
    uhci_obj[uhci_num]->rx_dma[2].offset = 0;
    uhci_obj[uhci_num]->rx_dma[2].sosf = 0;

    uhci_obj[uhci_num]->rx_dma[3].owner = 1;
    uhci_obj[uhci_num]->rx_dma[3].eof = 1;
    uhci_obj[uhci_num]->rx_dma[3].size = (PACKET_DATA_SIZE+7)&0xFFFFFFFC;
    uhci_obj[uhci_num]->rx_dma[3].length = 0;
    uhci_obj[uhci_num]->rx_dma[3].empty = (uint32_t)&uhci_obj[uhci_num]->rx_dma[0]; // actually 'qe' (ptr to next descr)
    uhci_obj[uhci_num]->rx_dma[3].buf = dma_data_in[3];
    uhci_obj[uhci_num]->rx_dma[3].offset = 0;
    uhci_obj[uhci_num]->rx_dma[3].sosf = 0;

    //debug_log("rx dma 0 %X\n", &(uhci_obj[uhci_num]->rx_dma[0]));
    //debug_log("rx dma 1 %X\n", &(uhci_obj[uhci_num]->rx_dma[1]));
    uhci_hal_rx_dma_restart(hal);
    uhci_hal_set_rx_dma(hal, (uint32_t)(&(uhci_obj[uhci_num]->rx_dma[0])));

    uhci_hal_rx_dma_start(hal);
    return 0;
}

int uart_dma_write(int uhci_num, uint8_t *pbuf, size_t wr)
{
    debug_log("enter uart_dma_write\n");

    uhci_obj[uhci_num]->tx_dma.owner = 1;
    uhci_obj[uhci_num]->tx_dma.eof = 1;
    uhci_obj[uhci_num]->tx_dma.buf = pbuf;
    uhci_obj[uhci_num]->tx_dma.length = wr;
    uhci_obj[uhci_num]->tx_dma.size = (wr+3)&0xFFFFFFFC;
    uhci_obj[uhci_num]->tx_dma.empty = 0; // actually 'qe' (ptr to next descr)
    auto hal = &(uhci_obj[uhci_num]->uhci_hal);
    uhci_hal_set_tx_dma(hal, (uint32_t)(&(uhci_obj[uhci_num]->tx_dma)));
    uhci_hal_tx_dma_start(hal);

    debug_log("leave uart_dma_write %u\n", (uint32_t)wr);
    return 0;
}

esp_err_t uhci_driver_install(int uhci_num, size_t tx_buf_size, size_t rx_buf_size, int intr_flag)
{
    if (uhci_obj[uhci_num] != NULL) {
        return ESP_FAIL;
    }
    uhci_obj_t *puhci = (uhci_obj_t *)heap_caps_calloc(1, sizeof(uhci_obj_t), MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
    if (!puhci) {
        ESP_LOGE(UHCI_TAG, "UHCI driver malloc error");
        return ESP_FAIL;
    }
    memset(puhci, 0, sizeof(uhci_obj_t));
    puhci->uhci_num = uhci_num;
    uhci_obj[uhci_num] = puhci;
    periph_module_enable(uhci_periph_signal[uhci_num].module);
    return esp_intr_alloc(uhci_periph_signal[uhci_num].irq, intr_flag, &uhci_isr_default_new, uhci_obj[uhci_num], &uhci_obj[uhci_num]->intr_handle);
}

static void dump_special_chars() {
    uhci_dev_t* dev = UHCI_LL_GET_HW(0);
    debug_log("\n");
	debug_log("seper_en:        %02hX\n", dev->conf0.seper_en);
	debug_log("seper_char:      %02hX\n", dev->esc_conf0.seper_char);
	debug_log("seper_esc_char0: %02hX\n", dev->esc_conf0.seper_esc_char0);
	debug_log("seper_esc_char1: %02hX\n", dev->esc_conf0.seper_esc_char1);
	debug_log("seq0:            %02hX\n", dev->esc_conf1.seq0);
	debug_log("seq0_char0:      %02hX\n", dev->esc_conf1.seq0_char0);
	debug_log("seq0_char1:      %02hX\n", dev->esc_conf1.seq0_char1);
	debug_log("seq1:            %02hX\n", dev->esc_conf2.seq1);
	debug_log("seq1_char0:      %02hX\n", dev->esc_conf2.seq1_char0);
	debug_log("seq1_char1:      %02hX\n", dev->esc_conf2.seq1_char1);
	debug_log("seq2:            %02hX\n", dev->esc_conf3.seq2);
	debug_log("seq2_char0:      %02hX\n", dev->esc_conf3.seq2_char0);
	debug_log("seq2_char1:      %02hX\n", dev->esc_conf3.seq2_char1);
    debug_log("\n");
}

esp_err_t uhci_attach_uart_port(int uhci_num, int uart_num, const uart_config_t *uart_config)
{
    {
        // Configure UART params
	debug_log("@%i\n", __LINE__);
//        periph_module_enable(uart_periph_signal[uart_num].module);
	debug_log("@%i\n", __LINE__);
        auto hal = &(uhci_obj[uhci_num]->uart_hal);
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
        //uhci_seper_chr_t seper_char = { 0x8B, 0x9D, 0xAE, 0x01 };
        uhci_seper_chr_t seper_char = { 0xC0, 0xDB, 0xDC, 0x01 };
	debug_log("@%i\n", __LINE__);
        auto hal = &(uhci_obj[uhci_num]->uhci_hal);
        uhci_hal_init(hal, uhci_num);
	debug_log("@%i uhci hal %X, dev %X\n", __LINE__, hal, hal->dev);
	debug_log("@%i\n", __LINE__);
        uhci_hal_disable_intr(hal, UHCI_INTR_MASK);
	debug_log("@%i\n", __LINE__);
        uhci_hal_set_eof_mode(hal, UHCI_RX_EOF_MAX);
	debug_log("@%i\n", __LINE__);
        uhci_hal_attach_uart_port(hal, uart_num);
	debug_log("@%i\n", __LINE__);
        dump_special_chars();
        uhci_hal_set_seper_chr(hal, &seper_char);
        dump_special_chars();
	debug_log("@%i\n", __LINE__);
        //uhci_hal_set_rx_dma(hal,(uint32_t)(&(uhci_obj[uhci_num]->rx_dma)));
	debug_log("@%i\n", __LINE__);
        //uhci_hal_set_tx_dma(hal,(uint32_t)(&(uhci_obj[uhci_num]->tx_dma)));
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

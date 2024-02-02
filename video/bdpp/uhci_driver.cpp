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
    uint8_t *pread_cur;
    uint8_t *preturn;
    size_t stash_len;
    lldesc_t rx_dma[DMA_RX_DESC_CNT];
    lldesc_t tx_dma[DMA_TX_DESC_CNT];
    lldesc_t *rx_head;
    lldesc_t *rx_real;
    lldesc_t *rx_cur;
    int buffered_len;
    SemaphoreHandle_t rx_mux;
    SemaphoreHandle_t tx_mux;
    QueueHandle_t event_Queue;
    QueueHandle_t stash_Queue;
    QueueHandle_t tx_Queue;
    RingbufHandle_t rx_ring_buf;
    RingbufHandle_t tx_ring_buf;
    bool rx_buffer_full;
    bool rx_idle;
    bool tx_idle;
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

static void uhci_rx_start_dma_s(int uhci_num)
{
    for (int i = 0; i < DMA_RX_DESC_CNT; i++) {
        uhci_obj[uhci_num]->rx_dma[i].owner = 1;
        uhci_obj[uhci_num]->rx_dma[i].length = 0;
        uhci_obj[uhci_num]->rx_dma[i].empty = (uint32_t)&(uhci_obj[uhci_num]->rx_dma[(i + 1) % DMA_RX_DESC_CNT]);
    }
    uhci_obj[uhci_num]->rx_cur = &uhci_obj[uhci_num]->rx_dma[0];
    auto hal = &(uhci_obj[uhci_num]->uhci_hal);
    uhci_hal_set_rx_dma(hal, (uint32_t)(&(uhci_obj[uhci_num]->rx_dma[0])));
    uhci_hal_rx_dma_start(hal);
}

static void uhci_tx_start_dma_s(int uhci_num)
{
    uhci_obj[uhci_num]->tx_dma[0].size = DMA_TX_BUF_SIZE;
    uhci_obj[uhci_num]->tx_dma[0].eof = 1;
    uhci_obj[uhci_num]->tx_dma[0].owner = 1;
    auto hal = &(uhci_obj[uhci_num]->uhci_hal);
    uhci_hal_set_tx_dma(hal, (uint32_t)(&(uhci_obj[uhci_num]->tx_dma[0])));
    uhci_hal_tx_dma_start(hal);
}

static void IRAM_ATTR uhci_isr_default_new(void *param)
{
    uhci_obj_t *p_obj = (uhci_obj_t *)param;
    portBASE_TYPE HPTaskAwoken = 0;
    while (1) {
        uint32_t intr_mask = uhci_hal_get_intr(&(p_obj->uhci_hal));
        if (intr_mask == 0) {
            break;
        }
        uhci_hal_clear_intr(&(p_obj->uhci_hal), intr_mask);
        // handle RX interrupt */
        if (intr_mask & (UHCI_INTR_IN_DONE | UHCI_INTR_IN_DSCR_EMPTY)) {
            uhci_event_t event;
            event.type = p_obj->rx_cur->eof ? UHCI_EVENT_EOF : UHCI_EVENT_DATA;
            event.len = p_obj->rx_cur->length;
            if (p_obj->rx_buffer_full == 0) {
                if (pdFALSE == xRingbufferSendFromISR(p_obj->rx_ring_buf, (void *)p_obj->rx_cur->buf, p_obj->rx_cur->length, &HPTaskAwoken)) {
                    p_obj->rx_buffer_full = true;
                    rx_stash_item_t stash_item = {0};
                    stash_item.buf = (uint8_t *)p_obj->rx_cur->buf;
                    stash_item.size = p_obj->rx_cur->length;
                    xQueueSendFromISR(p_obj->stash_Queue, (void * )&stash_item, &HPTaskAwoken);
                    event.type = UHCI_EVENT_BUF_FULL;
                    p_obj->rx_head = p_obj->rx_cur;
                    int front = (p_obj->rx_head - &(p_obj->rx_dma[0]) + DMA_RX_DESC_CNT -1) % DMA_RX_DESC_CNT;
                    p_obj->rx_real = &(p_obj->rx_dma[front]);
                    p_obj->rx_real->empty = 0;
                } else {
                    p_obj->buffered_len += p_obj->rx_cur->length;
                }
            } else {
                if (p_obj->rx_cur->length > 0) {
                    rx_stash_item_t stash_item = {0};
                    stash_item.buf = (uint8_t *)p_obj->rx_cur->buf;
                    stash_item.size = p_obj->rx_cur->length;
                    xQueueSendFromISR(p_obj->stash_Queue, (void * )&stash_item, &HPTaskAwoken);
                }
                if (p_obj->rx_cur->eof || (intr_mask & UHCI_INTR_IN_DSCR_EMPTY)) {
                    uhci_hal_rx_dma_stop(&(p_obj->uhci_hal));
                    p_obj->rx_idle = true;
                }
            }
            p_obj->rx_cur->eof = 0;
            p_obj->rx_cur->owner = 1;
            p_obj->rx_real = p_obj->rx_cur;
            p_obj->rx_cur = (lldesc_t *)p_obj->rx_cur->empty;
            if (p_obj->event_Queue && xQueueSendFromISR(p_obj->event_Queue, (void * )&event, &HPTaskAwoken) == pdFALSE) {
                ESP_EARLY_LOGV(UHCI_TAG, "UHCI EVENT Queue full");
            }
        }

        /* handle TX interrupt */
        if (intr_mask & (UHCI_INTR_OUT_DSCR_ERR | UHCI_INTR_OUT_TOT_EOF)) {
            if (intr_mask & UHCI_INTR_OUT_TOT_EOF) {
                vRingbufferReturnItemFromISR(p_obj->tx_ring_buf, (void *)p_obj->tx_dma[0].buf, &HPTaskAwoken);
            }
            size_t pxItemSize = 0;
            uint8_t *rbp = (uint8_t*) xRingbufferReceiveUpToFromISR(p_obj->tx_ring_buf, &pxItemSize, DMA_TX_BUF_SIZE);
            if (rbp != NULL) {
                p_obj->tx_dma[0].length = pxItemSize;
                p_obj->tx_dma[0].buf = rbp;
                p_obj->tx_dma[0].eof = 1;
                p_obj->tx_dma[0].owner = 1;
                uhci_tx_start_dma_s(p_obj->uhci_num);
            } else {
                uhci_hal_disable_intr(&(p_obj->uhci_hal), UHCI_INTR_OUT_DSCR_ERR);
                p_obj->tx_idle = true;
            }
        }
    }
    if (HPTaskAwoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

extern void debug_log(const char* fmt, ...);

//TODO cpoy DMA buffer stashed data to ringbuffer and restart RX DMA.
//If timeout. return false
static bool dma_buf_to_ringbuf(int uhci_num, TickType_t ticks_to_wait)
{
    debug_log("enter dma_buf_to_ringbuf\n");
    while(1) {
        rx_stash_item_t stash_item = {0};
        if (uhci_obj[uhci_num]->rx_idle == true) {
            if (xQueueReceive(uhci_obj[uhci_num]->stash_Queue, &stash_item, 0) == pdFALSE) {
                uhci_obj[uhci_num]->rx_buffer_full = false;
                uhci_obj[uhci_num]->rx_idle = false;
                /* restart DMA */
                UHCI_ENTER_CRITICAL(uhci_num);
                uhci_rx_start_dma_s(uhci_num);
                UHCI_EXIT_CRITICAL(uhci_num);
                debug_log("leave dma_buf_to_ringbuf 1\n");
                return true;
            }
        } else {
            if (xQueueReceive(uhci_obj[uhci_num]->stash_Queue, (void *)&stash_item, (TickType_t)ticks_to_wait) == pdFALSE) {
                //TOUT
                debug_log("leave dma_buf_to_ringbuf 0\n");
                return false;
            }
        }
        xRingbufferSend(uhci_obj[uhci_num]->rx_ring_buf, (void *)stash_item.buf, stash_item.size, (TickType_t)0);
        UHCI_ENTER_CRITICAL(uhci_num);
        uhci_obj[uhci_num]->buffered_len += stash_item.size;
        UHCI_EXIT_CRITICAL(uhci_num);
    }
}

int uart_dma_read(int uhci_num, uint8_t *addr, size_t read_size, TickType_t ticks_to_wait)
{
    //debug_log("enter uart_dma_read\n");
    UHCI_CHECK(addr != NULL,  "Read buffer null", ESP_FAIL)
    uint8_t *rd_ptr = addr;
    size_t rd_rem = read_size;

    //When `ticks_to_wait == portMAX_DELAY`, We may encounter a situation where the ringbuffer is empty and the `rx_buffer_full == true`,
    //the data in the DMA buffer needs to be cpoy to the ringbuffer in this function. in order to avoid blocking at `xRingbufferReceive`, 
    //we set `tick_rem = 0` at the begin.
    TickType_t tick_rem = 0;
    TickType_t tick_end = xTaskGetTickCount() + ticks_to_wait;
    if (xSemaphoreTake(uhci_obj[uhci_num]->rx_mux,(portTickType)ticks_to_wait) != pdTRUE) {
        return 0;
    }
    while (rd_rem && (tick_rem <= ticks_to_wait)) {
        if (uhci_obj[uhci_num]->stash_len == 0) {
            uhci_obj[uhci_num]->pread_cur = (uint8_t*)xRingbufferReceive(uhci_obj[uhci_num]->rx_ring_buf, &(uhci_obj[uhci_num]->stash_len), (TickType_t)tick_rem);
            if (uhci_obj[uhci_num]->pread_cur != NULL) {
                uhci_obj[uhci_num]->preturn = uhci_obj[uhci_num]->pread_cur;
                tick_rem = 0;
     debug_log("mismatch\n");
           } else { //RingBuffer is empty
                tick_rem = (ticks_to_wait == portMAX_DELAY ) ? portMAX_DELAY : tick_end - xTaskGetTickCount();
                if (uhci_obj[uhci_num]->rx_buffer_full == true && dma_buf_to_ringbuf(uhci_num, tick_rem) == false) {
    debug_log("goto exit\n");
                    goto exit;
                }
                continue;
            }
        }
        size_t rd_size = uhci_obj[uhci_num]->stash_len > rd_rem ? rd_rem : uhci_obj[uhci_num]->stash_len;
        memcpy(rd_ptr, uhci_obj[uhci_num]->pread_cur, rd_size);
        UHCI_ENTER_CRITICAL(uhci_num);
        uhci_obj[uhci_num]->buffered_len -= rd_size;
        UHCI_EXIT_CRITICAL(uhci_num);
        uhci_obj[uhci_num]->stash_len -= rd_size;
        uhci_obj[uhci_num]->pread_cur += rd_size;
        rd_ptr += rd_size;
        rd_rem -= rd_size;
    debug_log("read %u bytes\n", (uint)rd_size);
        if (uhci_obj[uhci_num]->stash_len == 0) {
            vRingbufferReturnItem(uhci_obj[uhci_num]->rx_ring_buf, (void *)uhci_obj[uhci_num]->preturn);
        }
    }
exit:
    xSemaphoreGive(uhci_obj[uhci_num]->rx_mux);
    //debug_log("leave uart_dma_read\n");
    return (read_size - rd_rem);
}

int uart_dma_write(int uhci_num, uint8_t *pbuf, size_t wr)
{
    debug_log("enter uart_dma_write\n");
    size_t size_rem = wr;
    size_t size_tmp = 0;
    uint8_t *pr = pbuf;
    xSemaphoreTake(uhci_obj[uhci_num]->tx_mux, (portTickType)0);
    while (size_rem) {
        size_tmp = size_rem > DMA_TX_BUF_SIZE ? DMA_TX_BUF_SIZE : size_rem;
        xRingbufferSend(uhci_obj[uhci_num]->tx_ring_buf, (void *)pr, size_tmp, (portTickType)portMAX_DELAY);
        if (uhci_obj[uhci_num]->tx_idle == true) {
            uhci_obj[uhci_num]->tx_idle = false;
            auto hal = &(uhci_obj[uhci_num]->uhci_hal);
            uhci_hal_tx_dma_start(hal);
            uhci_hal_enable_intr(hal, UHCI_INTR_OUT_DSCR_ERR);
        }
        size_rem -= size_tmp;
        pr += size_tmp;
    }
    xSemaphoreGive(uhci_obj[uhci_num]->tx_mux);
    debug_log("leave uart_dma_write %u\n", (uint32_t)wr);
    return 0;
}

static void uhci_dma_desc_init(int uhci_num)
{
    uhci_obj_t *p_obj = uhci_obj[uhci_num];
    for (int i = 0; i < DMA_RX_DESC_CNT; i++) {
        p_obj->rx_dma[i].eof = 1;
        p_obj->rx_dma[i].owner = 1;
        p_obj->rx_dma[i].length = 0;
        p_obj->rx_dma[i].size = DMA_RX_BUF_SZIE;
        p_obj->rx_dma[i].buf = (volatile uint8_t*) malloc(DMA_RX_BUF_SZIE);
        p_obj->rx_dma[i].empty = (uint32_t)&(p_obj->rx_dma[(i + 1) % DMA_RX_DESC_CNT]);
    }
    for (int i = 0; i < DMA_TX_DESC_CNT; i++) {
        p_obj->tx_dma[i].eof = 1;
        p_obj->tx_dma[i].owner = 1;
        p_obj->tx_dma[i].length = 0;
        p_obj->tx_dma[i].size = DMA_TX_BUF_SIZE;
    }
}

esp_err_t uhci_driver_install(int uhci_num, size_t tx_buf_size, size_t rx_buf_size, int intr_flag, QueueHandle_t *uart_queue, int queue_cnt)
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
    puhci->tx_Queue = xQueueCreate(4, sizeof(uhci_wr_item));
    puhci->stash_Queue = xQueueCreate(DMA_RX_DESC_CNT, sizeof(rx_stash_item_t));
    if (uart_queue != NULL && queue_cnt > 0) {
        puhci->event_Queue = xQueueCreate(queue_cnt, sizeof(uhci_event_t));
        *uart_queue = puhci->event_Queue;
    }
    puhci->tx_mux = xSemaphoreCreateMutex();
    puhci->rx_mux = xSemaphoreCreateMutex();
    puhci->rx_ring_buf = xRingbufferCreate(rx_buf_size, RINGBUF_TYPE_BYTEBUF);
    puhci->tx_ring_buf = xRingbufferCreate(tx_buf_size, RINGBUF_TYPE_BYTEBUF);
    puhci->tx_idle = true;
    puhci->rx_cur = &(puhci->rx_dma[0]);
    puhci->uhci_num = uhci_num;
    uhci_obj[uhci_num] = puhci;
    uhci_dma_desc_init(uhci_num);
    periph_module_enable(uhci_periph_signal[uhci_num].module);
    return esp_intr_alloc(uhci_periph_signal[uhci_num].irq, intr_flag, &uhci_isr_default_new, uhci_obj[uhci_num], &uhci_obj[uhci_num]->intr_handle);
}

static void dump_special_chars() {
    uhci_dev_t* dev = UHCI_LL_GET_HW(0);
    debug_log("\n");
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
        periph_module_enable(uart_periph_signal[uart_num].module);
	debug_log("@%i\n", __LINE__);
        auto hal = &(uhci_obj[uhci_num]->uart_hal);
	debug_log("@%i\n", __LINE__);
        hal->dev = UART_LL_GET_HW(uart_num);
        uart_hal_init(hal, uart_num);
	debug_log("@%i\n", __LINE__);
        uart_hal_disable_intr_mask(hal, ~0);
	debug_log("@%i\n", __LINE__);
        uart_hal_set_sclk(hal, uart_config->source_clk);
	debug_log("@%i\n", __LINE__);
        uart_hal_set_baudrate(hal, uart_config->baud_rate);
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
	debug_log("@%i\n", __LINE__);
        uart_hal_rxfifo_rst(hal);
	debug_log("@%i\n", __LINE__);
        uart_param_config(uart_num, uart_config);
	debug_log("@%i\n", __LINE__);
    }
    {
        //Configure UHCI param
	debug_log("@%i\n", __LINE__);
        uhci_seper_chr_t seper_char = { 0x8B, 0x9D, 0xAE, 0x01 };
	debug_log("@%i\n", __LINE__);
        auto hal = &(uhci_obj[uhci_num]->uhci_hal);
	debug_log("@%i\n", __LINE__);
        uhci_hal_init(hal, uhci_num);
	debug_log("@%i\n", __LINE__);
        uhci_hal_disable_intr(hal, UHCI_INTR_MASK);
	debug_log("@%i\n", __LINE__);
        uhci_hal_set_eof_mode(hal, UHCI_RX_IDLE_EOF);
	debug_log("@%i\n", __LINE__);
        uhci_hal_attach_uart_port(hal, uart_num);
	debug_log("@%i\n", __LINE__);
        dump_special_chars();
        //uhci_hal_set_seper_chr(hal, &seper_char);
        //dump_special_chars();
	debug_log("@%i\n", __LINE__);
        uhci_hal_set_rx_dma(hal,(uint32_t)(&(uhci_obj[uhci_num]->rx_dma[0])));
	debug_log("@%i\n", __LINE__);
        uhci_hal_set_tx_dma(hal,(uint32_t)(&(uhci_obj[uhci_num]->tx_dma[0])));
	debug_log("@%i\n", __LINE__);
        uhci_hal_clear_intr(hal, UHCI_INTR_MASK);
	debug_log("@%i\n", __LINE__);
        uhci_hal_enable_intr(hal, UHCI_INTR_IN_DONE | UHCI_INTR_IN_DSCR_EMPTY | UHCI_INTR_OUT_DONE | UHCI_INTR_OUT_TOT_EOF);
	debug_log("@%i\n", __LINE__);
        uhci_hal_rx_dma_start(hal);
	debug_log("@%i\n", __LINE__);
        uhci_hal_tx_dma_start(hal);
	debug_log("@%i\n", __LINE__);
    }
    return ESP_OK;
}

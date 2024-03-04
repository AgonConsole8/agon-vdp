//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// The LL layer for UHCI register operations.
// Note that most of the register operations in this layer are non-atomic operations.


#pragma once
#include "bdpp/uhci_types.h"
#include "soc/uhci_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UHCI_LL_GET_HW(num) (((num) == 0) ? (&UHCI0) : (&UHCI1))

static inline void uhci_ll_init(uhci_dev_t *hw)
{
    typeof(hw->conf0) conf0_reg;
    hw->conf0.clk_en = 1;
    conf0_reg.val = 0;
    conf0_reg.clk_en = 1;
    hw->conf0.val = conf0_reg.val;
    hw->conf0.in_rst = 1;
    hw->conf0.in_rst = 0;
    hw->conf0.out_rst = 1;
    hw->conf0.out_rst = 0;
    hw->conf1.val = 0;
    hw->conf1.dma_in_fifo_full_thrs = 100;
    hw->conf1.check_owner = 0;//realy need?
    hw->escape_conf.val = 0;
    hw->hung_conf.val = 0;
}

static inline void uhci_ll_attach_uart_port(uhci_dev_t *hw, int uart_num)
{
    typeof(hw->conf0) conf0_reg;
    conf0_reg.val = hw->conf0.val;
    conf0_reg.val &= 0xfffff1ff;
    conf0_reg.val |= (0x1 << (9+uart_num));
    hw->conf0.val = conf0_reg.val;
}

static inline void uhci_ll_set_seper_chr(uhci_dev_t *hw, uhci_seper_chr_t *seper_char)
{
    if (seper_char->sub_chr_en) {
        typeof(hw->esc_conf0) esc_conf0_reg;
        esc_conf0_reg.val = hw->esc_conf0.val;
        esc_conf0_reg.seper_char = seper_char->seper_chr;
        esc_conf0_reg.seper_esc_char0 = seper_char->sub_chr1;
        esc_conf0_reg.seper_esc_char1 = seper_char->sub_chr2;
        hw->esc_conf0.val = esc_conf0_reg.val;

        typeof(hw->esc_conf1) esc_conf1_reg;
        esc_conf1_reg.val = hw->esc_conf1.val;
        esc_conf1_reg.seq0 = seper_char->sub_chr1;
        esc_conf1_reg.seq0_char0 = seper_char->sub_chr1b;
        esc_conf1_reg.seq0_char1 = seper_char->sub_chr2b;
        hw->esc_conf1.val = esc_conf1_reg.val;

        hw->escape_conf.tx_c0_esc_en = 1;
        hw->escape_conf.tx_db_esc_en = 1;
        hw->escape_conf.rx_c0_esc_en = 1;
        hw->escape_conf.rx_db_esc_en = 1;

        hw->conf0.seper_en = 1;
        hw->conf0.uart_idle_eof_en = 0;
        hw->conf0.len_eof_en = 0;
        hw->conf0.indscr_burst_en = 0;

        hw->hung_conf.txfifo_timeout_ena = 1;
        hw->hung_conf.txfifo_timeout = 0xFF;
        hw->hung_conf.txfifo_timeout_shift = 4;
    } else {
        hw->escape_conf.tx_c0_esc_en = 0;
        hw->escape_conf.tx_db_esc_en = 0;
        hw->escape_conf.rx_c0_esc_en = 0;
        hw->escape_conf.rx_db_esc_en = 0;
        hw->conf0.seper_en = 0;
    }
}

static inline void uhci_ll_get_seper_chr(uhci_dev_t *hw, uhci_seper_chr_t *seper_chr)
{
    ;
}

static inline void uhci_ll_set_swflow_ctrl_sub_chr(uhci_dev_t *hw, uhci_swflow_ctrl_sub_chr_t *sub_ctr)
{
    typeof(hw->escape_conf) escape_conf_reg;
    escape_conf_reg.val = hw->escape_conf.val;
    if (sub_ctr->flow_en == 1) {
        typeof(hw->esc_conf2) esc_conf2_reg;
        esc_conf2_reg.val = hw->esc_conf2.val;
        typeof(hw->esc_conf3) esc_conf3_reg;
        esc_conf3_reg.val = hw->esc_conf3.val;
        esc_conf2_reg.seq1 = sub_ctr->xon_chr;
        esc_conf2_reg.seq1_char0 = sub_ctr->xon_sub1;
        esc_conf2_reg.seq1_char1 = sub_ctr->xon_sub2;
        esc_conf3_reg.seq2 = sub_ctr->xoff_chr;
        esc_conf3_reg.seq2_char0 = sub_ctr->xoff_sub1;
        esc_conf3_reg.seq2_char1 = sub_ctr->xoff_sub2;
        escape_conf_reg.tx_11_esc_en = 1;
        escape_conf_reg.tx_13_esc_en = 1;
        escape_conf_reg.rx_11_esc_en = 1;
        escape_conf_reg.rx_13_esc_en = 1;
        hw->esc_conf2.val = esc_conf2_reg.val;
        hw->esc_conf3.val = esc_conf3_reg.val;
    } else {
        escape_conf_reg.tx_11_esc_en = 0;
        escape_conf_reg.tx_13_esc_en = 0;
        escape_conf_reg.rx_11_esc_en = 0;
        escape_conf_reg.rx_13_esc_en = 0;
    }
    hw->escape_conf.val = escape_conf_reg.val;
}

static inline void uhci_ll_dma_in_reset(uhci_dev_t *hw)
{
    hw->conf0.in_rst = 1;
    hw->conf0.in_rst = 0;
}

static inline void uhci_ll_dma_out_reset(uhci_dev_t *hw)
{
    hw->conf0.out_rst = 1;
    hw->conf0.out_rst = 0;
}

static inline void uhci_ll_enable_intr(uhci_dev_t *hw, uint32_t intr_mask)
{
    hw->int_ena.val |= intr_mask;
}

static inline uint32_t uhci_ll_get_enabled_intr(uhci_dev_t *hw)
{
    return hw->int_ena.val;
}

static inline void uhci_ll_disable_intr(uhci_dev_t *hw, uint32_t intr_mask)
{
    hw->int_ena.val &= (~intr_mask);
}

static inline void uhci_ll_clear_intr(uhci_dev_t *hw, uint32_t intr_mask)
{
    hw->int_clr.val = intr_mask;
}

static inline uint32_t uhci_ll_get_intr(uhci_dev_t *hw)
{
    return hw->int_st.val;
}

static inline void uhci_ll_set_rx_dma(uhci_dev_t *hw, uint32_t addr)
{
    hw->dma_in_link.addr = addr;
}

static inline void uhci_ll_set_tx_dma(uhci_dev_t *hw, uint32_t addr)
{
    hw->dma_out_link.addr = addr;
}

static inline void uhci_ll_rx_dma_start(uhci_dev_t *hw)
{
    hw->dma_in_link.start = 1;
}

static inline void uhci_ll_tx_dma_start(uhci_dev_t *hw)
{
    hw->dma_out_link.start = 1;
}

static inline void uhci_ll_rx_dma_restart(uhci_dev_t *hw)
{
    hw->dma_in_link.restart = 1;
}

static inline void uhci_ll_rx_dma_stop(uhci_dev_t *hw)
{
    hw->dma_in_link.stop = 1;
}

static inline void uhci_ll_tx_dma_stop(uhci_dev_t *hw)
{
    hw->dma_out_link.stop = 1;
}

static inline void uhci_ll_set_eof_mode(uhci_dev_t *hw, uint32_t eof_mode)
{
    if (eof_mode & UHCI_RX_BREAK_CHR_EOF) {
        hw->conf0.uart_rx_brk_eof_en = 1;
    }
    if (eof_mode & UHCI_RX_IDLE_EOF) {
        hw->conf0.uart_idle_eof_en = 1;
    }
    if (eof_mode & UHCI_RX_LEN_EOF) {
        hw->conf0.len_eof_en = 1;
    }
}

#ifdef __cplusplus
} // extern "C"
#endif

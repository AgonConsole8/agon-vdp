// di_timing.h - Video timing constant data structures.
//
// Copyright (c) 2023 Curtis Whitley
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 

#pragma once

#include "di_constants.h"
#include <stdint.h>

typedef struct {
    const char* m_mode_line;
    uint32_t m_active_lines;
    uint32_t m_vfp_lines;
    uint32_t m_vs_lines;
    uint32_t m_vbp_lines;
    uint32_t m_hfp_pixels;
    uint32_t m_hs_pixels;
    uint32_t m_active_pixels;
    uint32_t m_hbp_pixels;
    uint32_t m_dma_clock_freq;
    uint32_t m_dma_total_lines;
    uint32_t m_dma_total_descr;
    uint32_t m_hs_on;
    uint32_t m_hs_off;
    uint32_t m_vs_on;
    uint32_t m_vs_off;
    uint32_t m_syncs_on;
    uint32_t m_syncs_off;
    uint32_t m_syncs_off_x4;
    uint32_t m_scan_count;

    //void dump();
} OtfVideoParams;

extern OtfVideoParams otf_video_params;

// di_timing.cpp - Video timing data structures.
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

#include "di_timing.h"

OtfVideoParams otf_video_params;

/*
void OtfVideoParams::dump() {
    debug_log("m_active_lines: %u\n", m_active_lines);
    debug_log("m_vfp_lines: %u\n", m_vfp_lines);
    debug_log("m_vs_lines: %u\n", m_vs_lines);
    debug_log("m_vbp_lines: %u\n", m_vbp_lines);
    debug_log("m_hfp_pixels: %u\n", m_hfp_pixels);
    debug_log("m_hs_pixels: %u\n", m_hs_pixels);
    debug_log("m_active_pixels: %u\n", m_active_pixels);
    debug_log("m_hbp_pixels: %u\n", m_hbp_pixels);
    debug_log("m_dma_clock_freq: %u\n", m_dma_clock_freq);
    debug_log("m_dma_total_lines: %u\n", m_dma_total_lines);
    debug_log("m_dma_total_descr: %u\n", m_dma_total_descr);
    debug_log("m_hs_on: %X\n", m_hs_on);
    debug_log("m_hs_off: %X\n", m_hs_off);
    debug_log("m_vs_on: %X\n", m_vs_on);
    debug_log("m_vs_off: %X\n", m_vs_off);
    debug_log("m_syncs_on: %X\n", m_syncs_on);
    debug_log("m_syncs_off: %X\n", m_syncs_off);
    debug_log("m_syncs_off_x4: %X\n", m_syncs_off_x4);
    debug_log("m_scan_count: %u\n", m_scan_count);
}
*/

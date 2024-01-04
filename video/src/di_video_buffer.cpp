// di_video_buffer.cpp - Function definitions for painting video scan lines
//
// A a video buffer is a set of 1-pixel-high video scan lines that are equal
// in length (number of pixels) to the total width of the video screen and
// horizontal synchronization pixels.
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

#include <string.h>
#include <math.h>
#include <vector>
#include "di_video_buffer.h"
#include "di_timing.h"
#include "freertos/FreeRTOS.h"

DiVideoScanLine::DiVideoScanLine(uint32_t num_lines) {
  m_num_lines = num_lines;
  auto new_size = (size_t)(get_buffer_size() * num_lines);
  auto p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_scan_line = (uint32_t *)p;
}

DiVideoScanLine::~DiVideoScanLine() {
  if (m_scan_line) {
    delete [] m_scan_line;
  }
}

volatile uint32_t * DiVideoScanLine::get_buffer_ptr(uint32_t index) volatile {
  uint32_t offset = get_buffer_size() * index;
  return (volatile uint32_t *) (((uint8_t*) m_scan_line) + offset);
}

uint32_t DiVideoScanLine::get_buffer_size()
{
  return otf_video_params.m_active_pixels +
    otf_video_params.m_hfp_pixels +
    otf_video_params.m_hs_pixels +
    otf_video_params.m_hbp_pixels;
}

uint32_t DiVideoScanLine::get_total_size() {
  return get_buffer_size() * m_num_lines;
}

volatile uint32_t* DiVideoScanLine::get_active_pixels() {
  return m_scan_line;
}

volatile uint32_t* DiVideoScanLine::get_hfp_pixels() {
  return (volatile uint32_t*)
    (((volatile uint8_t*) m_scan_line) +
      otf_video_params.m_active_pixels);
}

volatile uint32_t* DiVideoScanLine::get_hs_pixels() {
  return (volatile uint32_t*)
    (((volatile uint8_t*) m_scan_line) +
      otf_video_params.m_active_pixels +
      otf_video_params.m_hfp_pixels);
}

volatile uint32_t* DiVideoScanLine::get_hbp_pixels() {
  return (volatile uint32_t*)
    (((volatile uint8_t*) m_scan_line) +
      otf_video_params.m_active_pixels +
      otf_video_params.m_hfp_pixels +  
      otf_video_params.m_hs_pixels);
}

volatile uint32_t* DiVideoScanLine::get_active_pixels(uint32_t index) {
  uint32_t offset = get_buffer_size() * index;
  return (volatile uint32_t*)(((volatile uint8_t*)get_active_pixels()) + offset);
}

volatile uint32_t* DiVideoScanLine::get_hfp_pixels(uint32_t index) {
  uint32_t offset = get_buffer_size() * index;
  return (volatile uint32_t*)(((volatile uint8_t*)get_hfp_pixels()) + offset);
}

volatile uint32_t* DiVideoScanLine::get_hs_pixels(uint32_t index) {
  uint32_t offset = get_buffer_size() * index;
  return (volatile uint32_t*)(((volatile uint8_t*)get_hs_pixels()) + offset);
}

volatile uint32_t* DiVideoScanLine::get_hbp_pixels(uint32_t index) {
  uint32_t offset = get_buffer_size() * index;
  return (volatile uint32_t*)(((volatile uint8_t*)get_hbp_pixels()) + offset);
}

void DiVideoScanLine::init_to_black() {
  for (uint32_t i = 0; i < m_num_lines; i++) {
    memset((void*)get_active_pixels(i), otf_video_params.m_syncs_off, otf_video_params.m_active_pixels);
    memset((void*)get_hfp_pixels(i), otf_video_params.m_syncs_off, otf_video_params.m_hfp_pixels);
    memset((void*)get_hs_pixels(i), (otf_video_params.m_hs_on|otf_video_params.m_vs_off), otf_video_params.m_hs_pixels);
    memset((void*)get_hbp_pixels(i), otf_video_params.m_syncs_off, otf_video_params.m_hbp_pixels);
  }
}

void DiVideoScanLine::init_for_vsync() {
  for (uint32_t i = 0; i < m_num_lines; i++) {
    memset((void*)get_active_pixels(i), (otf_video_params.m_hs_off|otf_video_params.m_vs_on), otf_video_params.m_active_pixels);
    memset((void*)get_hfp_pixels(i), (otf_video_params.m_hs_off|otf_video_params.m_vs_on), otf_video_params.m_hfp_pixels);
    memset((void*)get_hs_pixels(i), otf_video_params.m_syncs_on, otf_video_params.m_hs_pixels);
    memset((void*)get_hbp_pixels(i), (otf_video_params.m_hs_off|otf_video_params.m_vs_on), otf_video_params.m_hbp_pixels);
  }
}

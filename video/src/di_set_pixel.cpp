// di_set_pixel.cpp - Function definitions for setting individual pixels
//
// A pixel is the smallest visible dot on the screen.
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

#include "di_set_pixel.h"
#include "di_timing.h"

DiSetPixel::DiSetPixel(uint16_t flags, int32_t x, int32_t y, uint8_t color) : DiPrimitive(flags) {
  m_opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
  m_rel_x = x;
  m_rel_y = y;
  m_width = 1;
  m_height = 1;
  m_color = PIXEL_COLOR_X4(color) | otf_video_params.m_syncs_off_x4;
  m_paint_code.enter_and_leave_outer_function();
}

void DiSetPixel::generate_instructions() {
  delete_instructions();
  EspFixups fixups;
  uint16_t width = 1;
  DiLineSections sections;
  sections.add_piece(1, 0, 1, false);
  auto x_offset = m_abs_x & 3;
  m_paint_code.draw_line(fixups, x_offset, 0, width, &sections, m_flags, m_opaqueness, true);
  m_paint_code.do_fixups(fixups);
}

void IRAM_ATTR DiSetPixel::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  m_paint_code.call(this, p_scan_line, line_index);
}

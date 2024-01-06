// di_rectangle.cpp - Function definitions for drawing rectangle outlines
//
// A rectangle outline is a thin rectangle that is left unfilled.
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

#include "di_rectangle.h"
#include "di_timing.h"

DiRectangle::DiRectangle(uint16_t flags) : DiPrimitive(flags) {
}

void DiRectangle::make_rectangle_outline(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
  m_rel_x = x;
  m_rel_y = y;
  m_width = width;
  m_height = height;
  m_opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
  m_color = PIXEL_COLOR_X4(color) | otf_video_params.m_syncs_off_x4;
}

void DiRectangle::generate_instructions() {
  /*
  delete_instructions();
  auto width = (uint16_t)m_width;
  {
    EspFixups fixups;
    DiLineSections sections;
    sections.add_piece(1, 0, width, false);
    m_paint_code[0].draw_line(fixups, m_draw_x, m_draw_x, &sections, m_flags, m_opaqueness, true);
    m_paint_code[0].do_fixups(fixups);
  }
  {
    EspFixups fixups;
    DiLineSections sections;
    sections.add_piece(1, 0, 1, false);
    sections.add_piece(1, width-1, 1, false);
    m_paint_code[1].draw_line(fixups, m_draw_x, m_draw_x, &sections, m_flags, m_opaqueness, true);
    m_paint_code[1].do_fixups(fixups);
  }
  */
}

void IRAM_ATTR DiRectangle::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  /*
  if (line_index == m_abs_y || line_index + 1 == m_y_extent) {
    m_paint_code[0].call(this, p_scan_line, line_index);
  } else {
    m_paint_code[1].call(this, p_scan_line, line_index);
  }
  */
}

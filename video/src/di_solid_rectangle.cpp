// di_solid_rectangle.cpp - Function definitions for drawing solid rectangles
//
// A solid rectangle is filled with a single color.
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

#include "di_solid_rectangle.h"
#include "di_timing.h"

DiSolidRectangle::DiSolidRectangle(uint16_t flags) : DiPrimitive(flags) {
  m_flags |= PRIM_FLAGS_X;
}

void DiSolidRectangle::make_rectangle(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
  m_opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
  m_rel_x = x;
  m_rel_y = y;
  m_width = width;
  m_height = height;
  m_color = PIXEL_COLOR_X4(color) | otf_video_params.m_syncs_off_x4;
  m_paint_code.enter_and_leave_outer_function();
}

void DiSolidRectangle::generate_instructions() {
  delete_instructions();
  EspFixups fixups;
  generate_code_for_positions(fixups, m_width, m_height);
  m_paint_code.do_fixups(fixups);
  set_current_paint_pointer(m_width, m_height);
}

void DiSolidRectangle::generate_code_for_left_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_left_edge(fixups, x_offset, width, height, hidden, visible);
  auto draw_width = (m_draw_x_extent - m_draw_x) - hidden;
  DiLineSections sections;
  sections.add_piece(1, 0, (uint16_t)draw_width, false);
  m_paint_code.draw_line(fixups, x_offset, hidden, visible, &sections, m_flags, m_opaqueness, true);
}

void DiSolidRectangle::generate_code_for_right_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_right_edge(fixups, x_offset, width, height, hidden, visible);
  auto draw_width = (m_draw_x_extent - m_draw_x) - hidden;
  DiLineSections sections;
  sections.add_piece(1, 0, (uint16_t)draw_width, false);
  m_paint_code.draw_line(fixups, x_offset, 0, visible, &sections, m_flags, m_opaqueness, true);
}

void DiSolidRectangle::generate_code_for_draw_area(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_draw_area(fixups, x_offset, width, height, hidden, visible);
  auto draw_width = m_draw_x_extent - m_draw_x;
  DiLineSections sections;
  sections.add_piece(1, 0, (uint16_t)draw_width, false);
  m_paint_code.draw_line(fixups, x_offset, 0, draw_width, &sections, m_flags, m_opaqueness, true);
}

void IRAM_ATTR DiSolidRectangle::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  (*(m_cur_paint_ptr.m_a5))(this, p_scan_line, line_index, m_abs_x);
}

// di_general_line.cpp - Function definitions for drawing general lines
//
// A general line is 1 pixel thick, and connects any 2 points, except that
// it should not be used for vertical, horizontal, or precisely diagonal
// lines, because there are other optimized classes for those cases.
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

#include "di_general_line.h"
#include "di_timing.h"

static int32_t min3(int32_t a, int32_t b, int32_t c) {
  int32_t m = MIN(a, b);
  return MIN(m, c);
}

static int32_t max3(int32_t a, int32_t b, int32_t c) {
  int32_t m = MAX(a, b);
  return MAX(m, c);
}

static int16_t min_of_pairs(const int16_t* coords, uint16_t n) {
  int16_t m = *coords;
  while (--n) {
    coords += 2;
    auto c = *coords;
    m = MIN(m, c);
  }
  return m;
}

static int16_t max_of_pairs(const int16_t* coords, uint16_t n) {
  int16_t m = *coords;
  while (--n) {
    coords += 2;
    auto c = *coords;
    m = MAX(m, c);
  }
  return m;
}

DiGeneralLine::DiGeneralLine(uint16_t flags) : DiPrimitive(flags) {
  m_flags |= PRIM_FLAGS_X;
}

void DiGeneralLine::make_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                uint8_t color, uint8_t opaqueness) {
  int16_t coords[4];
  coords[0] = x1;
  coords[1] = y1;
  coords[2] = x2;
  coords[3] = y2;
  make_line(coords, color, opaqueness);
}

void DiGeneralLine::make_line(int16_t* coords, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, 2, color, opaqueness);
  m_line_details.make_line(1, coords[0], coords[1], coords[2], coords[3], false);
  create_functions();
}

void DiGeneralLine::make_triangle_outline(int16_t* coords, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, 3, color, opaqueness);
  m_line_details.make_triangle_outline(1, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
  create_functions();
}

void DiGeneralLine::make_solid_triangle(int16_t* coords, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, 3, color, opaqueness);
  m_line_details.make_solid_triangle(1, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
  create_functions();
}

void DiGeneralLine::init_from_coords(int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  m_opaqueness = opaqueness;
  m_rel_x = min_of_pairs(coords, n);
  m_rel_y = min_of_pairs(coords+1, n);
  m_width = max_of_pairs(coords, n) - m_rel_x + 1;
  m_height = max_of_pairs(coords+1, n) - m_rel_y + 1;
  color &= 0x3F; // remove any alpha bits
  m_color = PIXEL_COLOR_X4(color) | otf_video_params.m_syncs_off_x4;

  while (n--) {
    *coords++ -= m_rel_x;
    *coords++ -= m_rel_y;
  }
}

void DiGeneralLine::make_triangle_list_outline(int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n*3, color, opaqueness);

  uint8_t id = 1;
  while (n--) {
    m_line_details.make_triangle_outline(id++, coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5]);
    coords += 6;
  }
  create_functions();
}

void DiGeneralLine::make_solid_triangle_list(int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n*3, color, opaqueness);

  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_triangle(id++, coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5]);
    coords += 6;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::make_triangle_fan_outline(
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n+2, color, opaqueness);
  
  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;

  uint8_t id = 1;
  while (n--) {
    m_line_details.make_triangle_outline(id++, sx0, sy0, sx1, sy1, coords[0], coords[1]);
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 2;
  }
  create_functions();
}

void DiGeneralLine::make_solid_triangle_fan(
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;
  
  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_triangle(id++, sx0, sy0, sx1, sy1, coords[0], coords[1]);
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 2;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::make_triangle_strip_outline(
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;
  
  uint8_t id = 1;
  while (n--) {
    m_line_details.make_triangle_outline(id++, sx0, sy0, sx1, sy1, coords[0], coords[1]);
    sx0 = sx1;
    sy0 = sy1;
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 2;
  }
  create_functions();
}

void DiGeneralLine::make_solid_triangle_strip(
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;
  
  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_triangle(id++, sx0, sy0, sx1, sy1, coords[0], coords[1]);
    sx0 = sx1;
    sy0 = sy1;
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 2;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::make_quad_outline(int16_t* coords, 
          uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, 4, color, opaqueness);
  m_line_details.make_quad_outline(1, coords[0], coords[1], coords[2], coords[3],
    coords[4], coords[5], coords[6], coords[7]);
  create_functions();
}

void DiGeneralLine::make_solid_quad(int16_t* coords,
          uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, 4, color, opaqueness);
  m_line_details.make_solid_quad(1, coords[0], coords[1], coords[2], coords[3],
    coords[4], coords[5], coords[6], coords[7]);
  create_functions();
}

void DiGeneralLine::make_quad_list_outline(int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n*4, color, opaqueness);

  uint8_t id = 1;
  while (n--) {
    m_line_details.make_quad_outline(id++, coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5], coords[6], coords[7]);
    coords += 8;
  }
  create_functions();
}

void DiGeneralLine::make_solid_quad_list(int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n*4, color, opaqueness);

  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_quad(id++, coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5], coords[6], coords[7]);
    coords += 8;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::make_quad_strip_outline(
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n*2+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;

  uint8_t id = 1;
  while (n--) {
    m_line_details.make_quad_outline(id++, sx0, sy0, sx1, sy1, coords[0], coords[1], coords[2], coords[3]);
    sx0 = coords[2];
    sy0 = coords[3];
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 4;
  }
  create_functions();
}

void DiGeneralLine::make_solid_quad_strip(
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(coords, n*2+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;

  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_quad(id++, sx0, sy0, sx1, sy1, coords[0], coords[1], coords[2], coords[3]);
    sx0 = coords[2];
    sy0 = coords[3];
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 4;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::generate_instructions() {
  delete_instructions();
  EspFixups fixups;
  generate_code_for_positions(fixups, m_width, m_height);
  m_paint_code.do_fixups(fixups);
  set_current_paint_pointer(m_width, m_height);
}

void DiGeneralLine::create_functions() {
}

void DiGeneralLine::generate_code_for_left_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_left_edge(fixups, x_offset, width, height, hidden, visible);
  auto num_sections = (uint32_t)m_line_details.m_sections.size();
  uint32_t at_jump_table = m_paint_code.init_jump_table(num_sections);
  for (uint32_t i = 0; i < num_sections; i++) {
    auto sections = &m_line_details.m_sections[i];
    m_paint_code.align32();
    m_paint_code.j_to_here(at_jump_table + i * sizeof(uint32_t));
    m_paint_code.draw_line(fixups, x_offset, hidden, visible, sections, m_flags, m_opaqueness, false);
  }
}

void DiGeneralLine::generate_code_for_right_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_right_edge(fixups, x_offset, width, height, hidden, visible);
  auto num_sections = (uint32_t)m_line_details.m_sections.size();
  uint32_t at_jump_table = m_paint_code.init_jump_table(num_sections);
  for (uint32_t i = 0; i < num_sections; i++) {
    auto sections = &m_line_details.m_sections[i];
    m_paint_code.align32();
    m_paint_code.j_to_here(at_jump_table + i * sizeof(uint32_t));
    m_paint_code.draw_line(fixups, x_offset, 0, visible, sections, m_flags, m_opaqueness, false);
  }
}

void DiGeneralLine::generate_code_for_draw_area(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_draw_area(fixups, x_offset, width, height, hidden, visible);
  auto num_sections = (uint32_t)m_line_details.m_sections.size();
  uint32_t at_jump_table = m_paint_code.init_jump_table(num_sections);
  for (uint32_t i = 0; i < num_sections; i++) {
    auto sections = &m_line_details.m_sections[i];
    m_paint_code.align32();
    m_paint_code.j_to_here(at_jump_table + i * sizeof(uint32_t));
    m_paint_code.draw_line(fixups, x_offset, 0, visible, sections, m_flags, m_opaqueness, false);
  }
}

void IRAM_ATTR DiGeneralLine::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
 (*(m_cur_paint_ptr.m_a5))(this, p_scan_line, line_index, m_abs_x);
}
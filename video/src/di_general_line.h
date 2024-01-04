// di_general_line.h - Function declarations for drawing general lines
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

#pragma once
#include "di_primitive.h"
#include "di_line_pieces.h"
#include "di_code.h"

class DiGeneralLine: public DiPrimitive {
  public:
  DiLineDetails m_line_details; // determines how pixels on each scan line are written
  uint8_t       m_opaqueness;

  // Construct a general line. This requires calling init_params() afterward.
  DiGeneralLine(uint16_t flags);

  // This function constructs a line from two points. The upper 2 bits of
  // the color must be zeros.
  void make_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                  uint8_t color, uint8_t opaqueness);

  // This function constructs a line from two points. The upper 2 bits of
  // the color must be zeros. There must be 2 points (4 coordinates) given.
  void make_line(int16_t* coords, uint8_t color, uint8_t opaqueness);

  // This function constructs a triangle outline from three points.
  // The upper 2 bits of the color must be zeros.
  // There must be 3 points (6 coordinates) given.
  void make_triangle_outline(int16_t* coords, uint8_t color, uint8_t opaqueness);

  // This function constructs a solid (filled) triangle from three points.
  // The upper 2 bits of the color must be zeros.
  // There must be 3 points (6 coordinates) given.
  void make_solid_triangle(int16_t* coords, uint8_t color, uint8_t opaqueness);

  // This function constructs a triangle list outline from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n*3 points (n*6 coordinates) given.
  void make_triangle_list_outline(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a solid triangle list from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n*3 points (n*6 coordinates) given.
  void make_solid_triangle_list(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a triangle fan outline from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n+2 points ((n+2)*2 coordinates) given.
  void make_triangle_fan_outline(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a solid triangle fan from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n+2 points ((n+2)*2 coordinates) given.
  void make_solid_triangle_fan(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a triangle strip outline from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n+2 points ((n+2)*2 coordinates) given.
  void make_triangle_strip_outline(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a solid triangle outline from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n+2 points ((n+2)*2 coordinates) given.
  void make_solid_triangle_strip(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a quad outline from four points.
  // The upper 2 bits of the color must be zeros.
  // There must be 4 points (8 coordinates) given.
  void make_quad_outline(int16_t* coords, 
            uint8_t color, uint8_t opaqueness);

  // This function constructs a solid (filled) quad from four points.
  // The upper 2 bits of the color must be zeros.
  // There must be 4 points (8 coordinates) given.
  void make_solid_quad(int16_t* coords,
            uint8_t color, uint8_t opaqueness);

  // This function constructs a quad list outline from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n*4 points (n*8 coordinates) given.
  void make_quad_list_outline(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a solid quad list from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n*4 points (n*8 coordinates) given.
  void make_solid_quad_list(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a quad strip outline from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n+2 points ((n+2)*2 coordinates) given.
  void make_quad_strip_outline(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  // This function constructs a solid quad outline from multiple points.
  // The upper 2 bits of the color must be zeros.
  // There must be n+2 points ((n+2)*2 coordinates) given.
  void make_solid_quad_strip(int16_t* coords,
            uint16_t n, uint8_t color, uint8_t opaqueness);

  virtual void generate_code_for_left_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);
  virtual void generate_code_for_right_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);
  virtual void generate_code_for_draw_area(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);

  // Reassemble the custom instructions needed to draw the primitive.
  virtual void generate_instructions();
   
  virtual void paint(volatile uint32_t* p_scan_line, uint32_t line_index);

  protected:
  void init_from_coords(int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness);
  void create_functions();
};
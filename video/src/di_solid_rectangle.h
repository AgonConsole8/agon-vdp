// di_solid_rectangle.h - Function declarations for drawing solid rectangles
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

#pragma once
#include "di_primitive.h"
#include "di_code.h"

class DiSolidRectangle: public DiPrimitive {
  public:
  uint8_t   m_opaqueness;

  // Construct a solid rectangle. This requires calling init_params() afterward.
  DiSolidRectangle(uint16_t flags);
  
  // Draws a solid (filled) rectangle on the screen.
  void make_rectangle(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color);

  virtual void generate_code_for_left_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);
  virtual void generate_code_for_right_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);
  virtual void generate_code_for_draw_area(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);

  // Reassemble the custom instructions needed to draw the primitive.
  virtual void generate_instructions();
   
  virtual void IRAM_ATTR paint(volatile uint32_t* p_scan_line, uint32_t line_index);

  protected:
};
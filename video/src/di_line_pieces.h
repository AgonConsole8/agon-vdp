// di_line_pieces.h - Function declarations for generating line pieces
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
#include <stdint.h>
#include <vector>
#include "di_constants.h"

#pragma pack(push,1)

// This structure tells how to draw a section of a line,
// on a single scan line that intersects with that line,
// and represents a (possibly) short section of a larger line.
typedef struct {
  uint8_t   m_id;
  int16_t   m_x;
  uint16_t  m_width;
} DiLinePiece;

#pragma pack(pop)

// This structure tells how to draw sections of one or more lines,
// on a single scan line that intersects with those lines,
// and represents (possibly) short sections of larger lines.
class DiLineSections {
  public:
  std::vector<DiLinePiece> m_pieces;

  // Each added piece represents colored pixels, meaning
  // pixels that are drawn.
  void add_piece(uint8_t id, int16_t x, uint16_t width, bool solid);
};

// This class represents enough details to draw a set of lines,
// based on sections of them, arranged by scan lines.
//
class DiLineDetails {
  public:
  int16_t   m_min_x;
  int16_t   m_min_y;
  int16_t   m_max_x;
  int16_t   m_max_y;
  std::vector<DiLineSections> m_sections;

  // Constructs an empty object. You must call a function below to create the line sections.
  DiLineDetails();

  // Destroys the line sections.
  ~DiLineDetails();

  // This function creates line sections for a line from two points.
  void make_line(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool solid);

  // This function creates a triangle outline from three points.
  void make_triangle_outline(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);

  // This function creates a solid (filled) triangle from three points.
  void make_solid_triangle(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);

  // This function creates a quad outline from four points.
  void make_quad_outline(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
          int16_t x3, int16_t y3, int16_t x4, int16_t y4);

  // This function creates a solid (filled) quad from four points.
  void make_solid_quad(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
          int16_t x3, int16_t y3, int16_t x4, int16_t y4);

  // Each added piece represents colored pixels, meaning
  // pixels that are drawn.
  void add_piece(uint8_t id, int16_t x, int16_t y, uint16_t width, bool solid);

  // This function merges the given set of details with this set.
  void merge(const DiLineDetails& details);
};

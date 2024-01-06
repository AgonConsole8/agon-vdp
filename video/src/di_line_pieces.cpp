// di_line_points.cpp - Function definitions for generating line points
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

#include "di_line_pieces.h"
#include <cstddef>
#include <string.h>

typedef union {
  int64_t value64;
  struct {
    uint32_t low;
    int32_t high;
  } value32;
} Overlay;

void DiLineSections::add_piece(uint8_t id, int16_t x, uint16_t width, bool solid) {
  auto xe = x + width;
  auto encloser = m_pieces.end();

  for (auto piece = m_pieces.begin(); piece != m_pieces.end(); piece++) {
    auto px = piece->m_x;
    auto pe = px + piece->m_width;
    // (a)  px----------pe
    //            x-----------xe
    //
    // (b)        px----------pe
    //       x-----------xe
    //
    // (c)       x-------xe
    //              px--------pe
    //
    // (d)   x-------xe
    //     px--------pe
    //
    if (solid ||
        x >= px && x <= pe || // case (a)
        xe >= px && xe <= pe || // case (b)
        px >= x && px <= xe || // case (c)
        pe >= x && pe <= xe) { // case (d)
      // merge the new piece with the old piece
      px = MIN(x, px);
      piece->m_x = px;
      pe = MAX(xe, pe);
      piece->m_width = pe - px;

      // check whether to merge with the next piece
      while (true) {
        auto next_piece = piece + 1;
        if (next_piece == m_pieces.end()) {
          break;
        }
        auto pe = piece->m_x + piece->m_width;
        if (solid || pe >= next_piece->m_x) {
          auto ne = next_piece->m_x + next_piece->m_width;
          auto nw = ne - piece->m_x;
          piece->m_width = MAX(piece->m_width, nw);
          m_pieces.erase(next_piece);
        } else {
          break;
        }
      }
      return;
    } else if (xe < px) {
      // insert a new piece before the old piece
      DiLinePiece new_piece;
      new_piece.m_id = id;
      new_piece.m_x = x;
      new_piece.m_width = width;
      m_pieces.insert(piece, new_piece);
      return;
    }
  }
  // insert a new piece after the old piece
  DiLinePiece new_piece;
  new_piece.m_id = id;
  new_piece.m_x = x;
  new_piece.m_width = width;
  m_pieces.push_back(new_piece);
}

//-------------------------------------------------------------------------------

DiLineDetails::DiLineDetails() {
  m_min_x = 0;
  m_min_y = 0;
  m_max_x = 0;
  m_max_y = 0;
}

DiLineDetails::~DiLineDetails() {
}

void DiLineDetails::make_line(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool solid) {
  auto min_x = MIN(x1, x2);
  auto max_x = MAX(x1, x2);
  auto min_y = MIN(y1, y2);
  auto max_y = MAX(y1, y2);
  auto flip_vertically = (x1<x2 && y1>y2);
  auto flip_horizontally = (x1>x2 && y1<y2);

  int16_t dx = max_x - min_x;
  int16_t dy = max_y - min_y;
  int16_t delta = MAX(dx, dy);

  if (!delta) {
    add_piece(id, x1, y1, 1, solid);
    return;
  }
  
  Overlay x;
  x.value32.low = 0;
  x.value32.high = min_x;
  int64_t delta_x = (((int64_t)dx) << 32) / delta + 1;

  Overlay y;
  y.value32.low = 0;
  y.value32.high = min_y;
  int64_t delta_y = (((int64_t)dy) << 32) / delta + 1;

  int32_t first_x = x.value32.high;
  int32_t first_y = y.value32.high;
  uint16_t i = 0;

  bool x_at_end = (x1 == x2);
  bool y_at_end = (y1 == y2);

  while (i < delta) {
    Overlay nx;
    Overlay ny;
    if (!x_at_end) {
      nx.value64 = x.value64 + delta_x;
      if (nx.value32.high == max_x) {
        x_at_end = true;
      }
    } else {
      nx.value32.high = first_x;
      nx.value32.low = 0;
    }
    
    if (!y_at_end) {
      ny.value64 = y.value64 + delta_y;
      if (ny.value32.high == max_y) {
        y_at_end = true;
      }
    } else {
      ny.value32.high = first_y;
      ny.value32.low = 0;
    }

    if (/*nx.value32.high != first_x ||*/ ny.value32.high != first_y) {
      uint16_t width = (uint16_t)(ABS(nx.value32.high - first_x));
      if (width == 0) {
          width = 1;
      }
      if (flip_vertically) {
        first_y = min_y + (dy - (first_y - min_y));
      } else if (flip_horizontally) {
        // 0123456789
        //       --
        //   --
        // first_x is 6
        // width is 2
        // dx is 9
        // min_x is 0
        // new first_x is 0 + (9 - (6 - 0)) - 2 + 1 = 2
        //
        // old first_x is 0 + (9 - (2 - 0)) - 2 + 1 = 6
        //
        first_x = min_x + (dx - (first_x - min_x)) - width + 1;
      }
      add_piece(id, (int16_t)first_x, (int16_t)first_y, width, solid);
      
      first_x = nx.value32.high;
      first_y = ny.value32.high;
    }

    if (x_at_end && y_at_end) {
      break;
    }

    x.value64 += delta_x;
    y.value64 += delta_y;
  }

  uint16_t width = (int16_t)(ABS(max_x - first_x + 1));
  if (width == 0) {
      width = 1;
  }
  if (flip_vertically) {
    first_y = min_y + (dy - (first_y - min_y));
  } else if (flip_horizontally) {
    first_x = min_x + (dx - (first_x - min_x)) - width + 1;    
  }
  add_piece(id, (int16_t)first_x, (int16_t)first_y, width, solid);

  m_min_x = MIN(m_min_x, min_x);  
  m_min_y = MIN(m_min_y, min_y);  
  m_max_x = MAX(m_max_x, max_x);  
  m_max_y = MAX(m_max_y, max_y);  
}

void DiLineDetails::make_triangle_outline(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
  make_line(id, x1, y1, x2, y2, false);
  make_line(id, x2, y2, x3, y3, false);
  make_line(id, x3, y3, x1, y1, false);

  auto y = m_min_y;
  for (auto sections = m_sections.begin(); sections != m_sections.end(); sections++) {
    for (auto piece = sections->m_pieces.begin();
          piece != sections->m_pieces.end();
          piece++) {
    }
    y++;
  }
}

void DiLineDetails::make_solid_triangle(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
  make_line(id, x1, y1, x2, y2, true);
  make_line(id, x2, y2, x3, y3, true);
  make_line(id, x3, y3, x1, y1, true);
}

void DiLineDetails::make_quad_outline(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
        int16_t x3, int16_t y3, int16_t x4, int16_t y4) {
  make_line(id, x1, y1, x2, y2, false);
  make_line(id, x2, y2, x3, y3, false);
  make_line(id, x3, y3, x4, y4, false);
  make_line(id, x4, y4, x1, y1, false);
}

void DiLineDetails::make_solid_quad(uint8_t id, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
        int16_t x3, int16_t y3, int16_t x4, int16_t y4) {
  make_line(id, x1, y1, x2, y2, true);
  make_line(id, x2, y2, x3, y3, true);
  make_line(id, x3, y3, x4, y4, true);
  make_line(id, x4, y4, x1, y1, true);
}

void DiLineDetails::add_piece(uint8_t id, int16_t x, int16_t y, uint16_t width, bool solid) {
  if (m_sections.size()) {
    // determine whether to add a new section
    if (y < m_min_y) {
      // insert one or more new sections at lower Y values
      auto new_count = m_min_y - y;
      DiLineSections new_sections;
      m_sections.insert(m_sections.begin(), new_count, new_sections);
      m_sections[0].add_piece(id, x, width, solid);
      m_min_y = y;
    } else if (y > m_max_y) {
      // insert one or more new sections at higher Y values
      auto new_count = y - m_max_y;
      DiLineSections new_sections;
      m_sections.resize(m_sections.size() + new_count);
      m_sections[m_sections.size() - 1].add_piece(id, x, width, solid);
      m_max_y = y;
    } else {
      // reuse an existing section with the same Y value
      m_sections[y - m_min_y].add_piece(id, x, width, solid);
    }
    m_min_x = MIN(m_min_x, x);
    m_max_x = MAX(m_max_x, x);
  } else {
    // add the first section
    DiLineSections new_section;
    new_section.add_piece(id, x, width, solid);
    m_sections.push_back(new_section);
    m_min_x = x;
    m_min_y = y;
    m_max_x = x;
    m_max_y = y;
  }
}

void DiLineDetails::merge(const DiLineDetails& details) {
  auto y = details.m_min_y;
  for (auto sections = details.m_sections.begin();
      sections != details.m_sections.end();
      sections++) {
    for (auto piece = sections->m_pieces.begin();
          piece != sections->m_pieces.end();
          piece++) {
      add_piece(piece->m_id, piece->m_x, y, piece->m_width, false);
    }
    y++;
  }
}
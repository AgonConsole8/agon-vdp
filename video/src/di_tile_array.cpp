// di_tile_array.cpp - Function definitions for drawing tile arrays
//
// A tile array is a set of rectangular tiles, where each tile is a bitmap of
// the same size (width and height). Tiles are arranged in a rectangular
// grid, where the entire portion of the grid that fits within the visible
// area of the screen may be displayed at any given moment. In other words
// multiple tiles show at the same time.
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

#include "di_tile_array.h"
#include <cstring>

DiTileArray::DiTileArray(uint32_t screen_width, uint32_t screen_height,
                      uint32_t columns, uint32_t rows,
                      uint32_t tile_width, uint32_t tile_height, uint16_t flags) : DiPrimitive(flags) {
  m_flags |= PRIM_FLAGS_X_SRC;
  m_tile_width = tile_width;
  m_tile_height = tile_height;
  m_rows = rows;
  m_columns = columns;
  uint32_t draw_words_per_line = (tile_width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  uint32_t words_per_line = draw_words_per_line;

  if (flags & PRIM_FLAG_H_SCROLL_1) {
    draw_words_per_line += 2;
  }

  m_bytes_per_line = words_per_line * sizeof(uint32_t);
  uint32_t words_per_position = words_per_line * tile_height;
  m_bytes_per_position = words_per_position * sizeof(uint32_t);

  m_visible_columns = (screen_width + tile_width - 1) / tile_width;
  if (m_visible_columns > columns) {
    m_visible_columns = columns;
  }

  m_visible_rows = (screen_height + tile_height - 1) / tile_height;
  if (m_visible_rows > rows) {
    m_visible_rows = rows;
  }

  m_width = tile_width * columns;
  m_height = tile_height * rows;

  m_tile_pixels = new uint32_t*[rows * columns];
  if (m_tile_pixels) {
    memset(m_tile_pixels, 0, rows * columns * sizeof(uint32_t*));
  }
}

DiTileArray::~DiTileArray() {
  for (auto bitmap_item = m_id_to_bitmap_map.begin(); bitmap_item != m_id_to_bitmap_map.end(); bitmap_item++) {
    delete bitmap_item->second;
  }

  if (m_tile_pixels) {
    delete [] m_tile_pixels;
  }
}

void DiTileArray::generate_instructions() {
  delete_instructions();

  // Painting is done with this parameter list:
  // a0 = return address
  // a1 = stack ptr
  // a2 = p_this
  // a3 = p_scan_line
  // a4 = line_index
  // a5 = a5_value (pointer to array of src_pixel pointers for 1 row)
  // a6 = a6_value (src_pixels_offset)
  // m_paint_code->call_a5_a6(this, p_scan_line, y_offset_within_tile, row_array, src_pixels_offset);

  m_paint_code.entry(REG_STACK_PTR, 32);

  //m_paint_code.movi(a11, 0x3F);
  //m_paint_code.s32i(a11, a3, 4);
  //m_paint_code.retw();

  m_paint_code.movi(a12, m_visible_columns); // a12 <-- loop counter (# of visible columns)
  auto at_loop = m_paint_code.get_code_index();
  m_paint_code.loop(a12, 0); // loop once per column

  m_paint_code.l32i(a10, a5, 0); // a10 <-- points to start of pixels for 1 bitmap
  auto at_branch = m_paint_code.get_code_index();
  m_paint_code.beqz(a10, 0); // go if the tile cell is empty (null)
  m_paint_code.add(a10, a10, a6); // a10 <-- points to line of source pixels for 1 bitmap
  for (uint32_t x = 0; x < m_tile_width; x+=4) {
    m_paint_code.l32i(a11, a10, x);
    m_paint_code.s32i(a11, a3, x);
  }
  uint32_t x = m_tile_width;
  while (x) {
    if (x < 124) {
      m_paint_code.addi(a3, a3, x);
      break;
    }
    m_paint_code.addi(a3, a3, 124);
    x -= 124;
  }
  m_paint_code.bgez_to_here(a10, at_branch);
  m_paint_code.addi(a5, a5, 4);

  m_paint_code.loop_to_here(a12, at_loop);
  m_paint_code.retw();
}

DiBitmap* DiTileArray::create_bitmap(DiTileBitmapID bm_id) {
  auto bitmap_item = m_id_to_bitmap_map.find(bm_id);
  if (bitmap_item == m_id_to_bitmap_map.end()) {
    auto bitmap = new DiBitmap(m_tile_width, m_tile_height, m_flags, false);
    bitmap->set_custom(bm_id);
    m_id_to_bitmap_map[bm_id] = bitmap;
    return bitmap;
  } else {
    return bitmap_item->second;
  }
}

void DiTileArray::set_pixel(DiTileBitmapID bm_id, int32_t x, int32_t y, uint8_t color) {
  m_id_to_bitmap_map[bm_id]->set_transparent_pixel(x, y, color);
}

void DiTileArray::set_tile(int16_t column, int16_t row, DiTileBitmapID bm_id) {
  if (column >= 0 && column < m_columns && row >= 0 && row < m_rows) {
    auto bitmap_item = m_id_to_bitmap_map.find(bm_id);
    if (bitmap_item != m_id_to_bitmap_map.end()) {
      m_tile_pixels[row * m_columns + column] = bitmap_item->second->get_pixels();
    }
  }
}

void DiTileArray::set_tiles(int16_t column, int16_t row, DiTileBitmapID bm_id,
                                  int16_t columns, int16_t rows) {
  while (rows-- > 0) {
    auto cols = columns;
    auto col = column;
    while (cols-- > 0) {
      set_tile(col++, row, bm_id);
    }
    row++;
  }
}

void DiTileArray::unset_tiles(int16_t column, int16_t row, int16_t columns, int16_t rows) {
  while (rows-- > 0) {
    auto cols = columns;
    auto col = column;
    while (cols-- > 0) {
      unset_tile(col++, row);
    }
    row++;
  }
}

void DiTileArray::unset_tile(int16_t column, int16_t row) {
    m_tile_pixels[row * m_columns + column] = NULL;
}

DiTileBitmapID DiTileArray::get_tile(int16_t column, int16_t row) {
  auto tile_pixels = m_tile_pixels[row * m_columns + column];
  if (tile_pixels) {
    for (auto bitmap_item = m_id_to_bitmap_map.begin(); bitmap_item != m_id_to_bitmap_map.end(); bitmap_item++) {
      if (tile_pixels == bitmap_item->second->get_pixels()) {
        return bitmap_item->second->get_custom();
      }
    }
  }
  return (DiTileBitmapID)0;
}

void DiTileArray::get_rel_tile_coordinates(int16_t column, int16_t row,
          int16_t& x, int16_t& y, int16_t& x_extent, int16_t& y_extent) {
    x = column * m_tile_width;
    x_extent = x + m_tile_width;
    y = row * m_tile_height;
    y_extent = y + m_tile_height;
}

void DiTileArray::get_abs_tile_coordinates(int16_t column, int16_t row,
          int16_t& x, int16_t& y, int16_t& x_extent, int16_t& y_extent) {
    x = column * m_tile_width + m_abs_x;
    x_extent = x + m_tile_width;
    y = row * m_tile_height + m_abs_y;
    y_extent = y + m_tile_height;
}

void IRAM_ATTR DiTileArray::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  p_scan_line += m_abs_x / 4;
  auto y_offset_within_tile_array = (int32_t)line_index - m_abs_y;
  auto y_offset_within_tile = y_offset_within_tile_array % (int32_t)m_tile_height;
  auto row = y_offset_within_tile_array / (int32_t)m_tile_height;
  auto src_pixels_offset = y_offset_within_tile * m_bytes_per_line;
  auto row_array = (uint32_t)(m_tile_pixels + row * m_columns);
  m_paint_code.call_a5_a6(this, p_scan_line, y_offset_within_tile, row_array, src_pixels_offset);
}

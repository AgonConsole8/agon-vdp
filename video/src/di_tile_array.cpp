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

  m_tile_bitmaps.resize(rows * columns);
}

DiTileArray::~DiTileArray() {
  for (auto bitmap_item = m_id_to_bitmap_map.begin(); bitmap_item != m_id_to_bitmap_map.end(); bitmap_item++) {
    delete bitmap_item->second;
  }
}

void DiTileArray::generate_instructions() {
  for (auto bitmap_item = m_id_to_bitmap_map.begin(); bitmap_item != m_id_to_bitmap_map.end(); bitmap_item++) {
    auto bitmap = bitmap_item->second;
    bitmap->compute_absolute_geometry();
    bitmap->generate_instructions();
  }
  compute_paint_parameters();
}

DiBitmap* DiTileArray::create_bitmap(DiTileBitmapID bm_id, bool psram) {
  auto bitmap_item = m_id_to_bitmap_map.find(bm_id);
  if (bitmap_item == m_id_to_bitmap_map.end()) {
    auto bitmap = new DiBitmap(m_tile_width, m_tile_height, m_flags, psram);
    bitmap->set_custom(bm_id);
    m_id_to_bitmap_map[bm_id] = bitmap;
    return bitmap;
  } else {
    return bitmap_item->second;
  }
}

DiBitmap* DiTileArray::get_bitmap(DiTileBitmapID bm_id) {
  return m_id_to_bitmap_map[bm_id];
}

void DiTileArray::set_pixel(DiTileBitmapID bm_id, uint32_t x, uint32_t y, uint8_t color) {
  m_id_to_bitmap_map[bm_id]->set_transparent_pixel(x, y, color);
}
extern void debug_log(const char* f,...);
void DiTileArray::set_tile(int16_t column, int16_t row, DiTileBitmapID bm_id) {
  if (column >= 0 && column < m_columns && row >= 0 && row < m_rows) {
    auto bitmap_item = m_id_to_bitmap_map.find(bm_id);
    if (bitmap_item != m_id_to_bitmap_map.end()) {
      auto index = row * m_columns + column;
      auto bitmap = bitmap_item->second;
      m_tile_bitmaps[index] = bitmap;
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
  auto index = row * m_columns + column;
  m_tile_bitmaps[index] = NULL;
}

DiTileBitmapID DiTileArray::get_tile(int16_t column, int16_t row) {
  auto index = row * m_columns + column;
  auto bitmap = m_tile_bitmaps[index];
  if (bitmap) {
    return bitmap->get_custom();
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
  /* FROM PRIMITIVE:
  uint32_t hidden_left = 0;
  uint32_t hidden_right = 0;
  if (m_abs_x < m_draw_x) {
    hidden_left = m_draw_x - m_abs_x;
  } else if (m_draw_x_extent < m_x_extent) {
    hidden_right = m_x_extent - m_draw_x_extent;
  }
  return get_paint_pointer(width, height, hidden_left, hidden_right);
  */
 /*
  p_scan_line += m_abs_x / 4;
  auto y_offset_within_tile_array = (int32_t)line_index - m_abs_y;
  auto y_offset_within_tile = y_offset_within_tile_array % (int32_t)m_tile_height;
  auto row = y_offset_within_tile_array / (int32_t)m_tile_height;
  auto src_pixels_offset = y_offset_within_tile * m_bytes_per_line;
  auto row_array = (uint32_t)(m_tile_pixels + row * m_columns);
  m_paint_code.call_a5_a6(this, p_scan_line, y_offset_within_tile, row_array, src_pixels_offset);
  */

  p_scan_line += m_abs_x / 4;
  auto y_offset_within_tile_array = (int32_t)line_index - m_abs_y;
  auto y_offset_within_tile = y_offset_within_tile_array % (int32_t)m_tile_height;
  auto row = y_offset_within_tile_array / (int32_t)m_tile_height;
  auto src_pixels_offset = y_offset_within_tile * m_bytes_per_line;
  auto index = row * m_columns + 0;
  for (int i = 0; i < m_visible_columns-1; i++) {
  auto bitmap = m_tile_bitmaps[index];
    if (bitmap) {
      // debug_log("paint %u %u %u %u\n",m_id,bitmap->get_custom(),index,y_offset_within_tile);
      bitmap->paint(p_scan_line, y_offset_within_tile);
      p_scan_line += m_tile_width / 4;
    }
  }
}

void DiTileArray::compute_paint_parameters() {
  m_left_tile_count = 0;
  m_left_src_pixel_offset = 0;
  m_left_paint_fcn_index = 0;
  m_mid_tile_count = 0;
  m_mid_src_pixel_offset = 0;
  m_mid_paint_fcn_index = 0;
  m_right_tile_count = 0;
  m_right_src_pixel_offset = 0;
  m_right_paint_fcn_index = 0;
  m_dst_x_offset = m_abs_x & 3;
  m_dst_pixel_offset = 0;

  debug_log("cpp id %hu ax %i axe %i dx %i dxe %i dw %i\n",m_id,m_abs_x,m_abs_x+m_width,m_draw_x,m_draw_x_extent,m_draw_x_extent-m_draw_x);

  if (m_tile_bitmaps.size()) {
    auto bitmap = *m_tile_bitmaps.begin();

    uint32_t hidden_left = 0;
    uint32_t hidden_right = 0;
    auto draw_width = m_draw_x_extent - m_draw_x;
    debug_log("  dw %i\n", draw_width);

    if (m_abs_x < m_draw_x) {
      hidden_left = (m_draw_x - m_abs_x) % m_tile_width;
    }

    if (m_draw_x_extent < m_x_extent) {
      hidden_right = (m_x_extent - m_draw_x_extent) % m_tile_width;
    }

    if (hidden_left) {
      draw_width -= hidden_left;
      m_left_tile_count = 1;
      m_left_paint_fcn_index = bitmap->get_paint_fcn_index(m_tile_width, m_tile_height, hidden_left, 0);
      debug_log("  dw %i ltc %u lpfi %u\n", draw_width, m_left_tile_count, m_left_paint_fcn_index);
    }

    if (hidden_right) {
      draw_width -= hidden_right;
      m_right_tile_count = 1;
      m_right_paint_fcn_index = bitmap->get_paint_fcn_index(m_tile_width, m_tile_height, 0, hidden_right);
      debug_log("  dw %i rtc %u rpfi %u\n", draw_width, m_right_tile_count, m_right_paint_fcn_index);
    }

    if (draw_width) {
      m_mid_tile_count = draw_width % m_tile_width;
      m_mid_paint_fcn_index = bitmap->get_paint_fcn_index(m_tile_width, m_tile_height, 0, 0);
      debug_log("  mtc %u mpfi %u\n", m_mid_tile_count, m_mid_paint_fcn_index);
    }
  }
  debug_log("  --\n");
}
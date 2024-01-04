// di_text_area.cpp - Function definitions for supporting a character text_area display
//
// A text_area is a specialized tile array, where each tile is a single character
// cell, and the character codes are used as tile image IDs.
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

#include "di_text_area.h"
#include <cstring>
#include "di_timing.h"

DiTextArea::DiTextArea(uint32_t x, uint32_t y, uint8_t flags,
                        uint32_t columns, uint32_t rows, const uint8_t* font) :
  DiTileArray(columns*8, rows*8, columns, rows, 8, 8, flags) {
  m_flags |= PRIM_FLAGS_ALL_SAME;
  m_current_column = 0;
  m_current_row = 0;
  m_fg_color = PIXEL_COLOR_ARGB(3, 1, 1, 0);
  m_bg_color = PIXEL_COLOR_ARGB(3, 0, 0, 0);
  m_font = font;
  m_rel_x = x;
  m_rel_y = y;
}

DiTextArea::~DiTextArea() {
}

void DiTextArea::delete_instructions() {
  DiTileArray::delete_instructions();
  auto cursor = get_first_child();
  if (cursor) {
    cursor->delete_instructions();
  }
}
  
void DiTextArea::generate_instructions() {
  delete_instructions();
  DiTileArray::generate_instructions();
  auto cursor = get_first_child();
  if (cursor) {
    cursor->generate_instructions();
  }
}

void DiTextArea::define_character_range(uint8_t first_char, uint8_t last_char,
                            uint8_t fg_color, uint8_t bg_color) {
  auto ch = (uint16_t) first_char;
  auto last = (uint16_t) last_char;
  while (ch <= last) {
    define_character((uint8_t)ch++, fg_color, bg_color);
  }
}

DiTileBitmapID DiTextArea::get_bitmap_id(uint8_t character) {
  return ((DiTileBitmapID)character) | ((DiTileBitmapID)m_bg_color << 24) | ((DiTileBitmapID)m_fg_color << 16);
}

DiTileBitmapID DiTextArea::get_bitmap_id(uint8_t character, uint8_t fg_color, uint8_t bg_color) {
  return ((DiTileBitmapID)character) | ((DiTileBitmapID)bg_color << 24) | ((DiTileBitmapID)fg_color << 16);
}

DiTileBitmapID DiTextArea::define_character(uint8_t character, uint8_t fg_color, uint8_t bg_color) {
  auto bm_id = get_bitmap_id(character, fg_color, bg_color);
  if (m_id_to_bitmap_map.find(bm_id) == m_id_to_bitmap_map.end()) {
    create_bitmap(bm_id);
    uint32_t char_start = (uint32_t)character * 8;
    for (int y = 0; y < 8; y++) {
      uint8_t pixels = m_font[char_start+y];
      for (int x = 0; x < 8; x++) {
        if (pixels & 0x80) {
          set_pixel(bm_id, x, y, fg_color);
        } else {
          set_pixel(bm_id, x, y, bg_color);
        }
        pixels <<= 1;
      }
    }
  }
  return bm_id;
}

void DiTextArea::set_character_position(int32_t column, int32_t row) {
  m_current_column = column;
  m_current_row = row;
}

void DiTextArea::bring_current_position_into_view() {
  if (m_current_column < 0) {
    // scroll text to the right (insert on the left)
    int32_t move = m_columns + m_current_column;
    int32_t open = -m_current_column;
    move_text(0, 0, move, m_rows, open, 0);
    erase_text(0, 0, open, m_rows);
    m_current_column = 0;
  }
  if (m_current_column >= m_columns) {
    // scroll text to the left (insert on the right)
    int32_t open = m_current_column - m_columns + 1;
    int32_t move = m_columns - open;
    move_text(open, 0, move, m_rows, 0, 0);
    erase_text(move, 0, open, m_rows);
    m_current_column = m_columns - 1;
  }
  if (m_current_row < 0) {
    // scroll text down (insert at the top)
    int32_t move = m_rows + m_current_row;
    int32_t open = -m_current_row;
    move_text(0, 0, m_columns, move, 0, open);
    erase_text(0, 0, m_columns, open);
    m_current_row = 0;
  }
  if (m_current_row >= m_rows) {
    // scroll text up (insert at the bottom)
    int32_t open = m_current_row - m_rows + 1;
    int32_t move = m_rows -  open;
    move_text(0, open, m_columns, move, 0, -open);
    erase_text(0, move, m_columns, open);
    m_current_row = m_rows - 1;
  }
}

void DiTextArea::write_character(uint8_t character) {
  bring_current_position_into_view();

  // Set the tile image ID using the character code.
  set_character(m_current_column, m_current_row, character);

  // Advance the current position
  if (++m_current_column >= m_columns) {
    m_current_column = 0;
    m_current_row++;
  }
}

void DiTextArea::set_character(int32_t column, int32_t row, uint8_t character) {
  auto bm_id = define_character(character, m_fg_color, m_bg_color);
  set_tile(column, row, bm_id);
}

DiTileBitmapID DiTextArea::read_character() {
  return read_character(m_current_column, m_current_row);
}

DiTileBitmapID DiTextArea::read_character(int32_t column, int32_t row) {
  return get_tile(column, row);
}

void DiTextArea::erase_text(int32_t column, int32_t row, int32_t columns, int32_t rows) {
  while (rows-- > 0) {
    int32_t col = column;
    auto bm_id = get_bitmap_id(0x20);
    for (int32_t c = 0; c < columns; c++) {
      set_character(col++, row, bm_id);
    }
    row++;
  }
}

void DiTextArea::move_text(int32_t column, int32_t row, int32_t columns, int32_t rows,
                            int32_t delta_horiz, int32_t delta_vert) {
  if (delta_vert > 0) {
    // moving rows down; copy bottom-up
    row += rows - 1;
    while (rows-- > 0) {
      auto col = column;
      auto n = columns;
      while (n-- > 0) {
        auto bm_id = get_tile(col, row);
        set_tile(col++, row + delta_vert, bm_id);
      }
      row--;
    }
  } else {
    // moving rows up; copy top-down
    while (rows-- > 0) {
      auto col = column;
      auto n = columns;
      while (n-- > 0) {
        auto bm_id = get_tile(col, row);
        set_tile(col++, row + delta_vert, bm_id);
      }
      row++;
    }
  }
}

void DiTextArea::clear_screen() {
  erase_text(0, 0, m_columns, m_rows);
  m_current_column = 0;
  m_current_row = 0;
}

void DiTextArea::move_cursor_left() {
  bring_current_position_into_view();
  if (m_current_column > 0) {
    m_current_column--;
  } else if (m_current_row > 0) {
    m_current_row--;
    m_current_column = m_columns - 1;
  }
}

void DiTextArea::move_cursor_right() {
  bring_current_position_into_view();
  if (m_current_column < m_columns - 1) {
    m_current_column++;
  } else if (m_current_row < m_rows - 1) {
    m_current_row++;
    m_current_column = 0;
  }
}

void DiTextArea::move_cursor_down() {
  bring_current_position_into_view();
  m_current_row++;
}

void DiTextArea::move_cursor_up() {
  bring_current_position_into_view();
  if (m_current_row > 0) {
    m_current_row--;
  }
}

void DiTextArea::move_cursor_home() {
  bring_current_position_into_view();
  m_current_row = 0;
  m_current_column = 0;
}

void DiTextArea::move_cursor_boln() {
  bring_current_position_into_view();
  m_current_column = 0;
}

void DiTextArea::do_backspace() {
  bring_current_position_into_view();
  if (m_current_column > 0) {
    m_current_column--;
    auto bm_id = get_bitmap_id(0x20);
    set_character(m_current_column, m_current_row, bm_id);
  } else if (m_current_row > 0) {
    m_current_row--;
    m_current_column = m_columns - 1;
    auto bm_id = get_bitmap_id(0x20);
    set_character(m_current_column, m_current_row, bm_id);
  }
}

void DiTextArea::move_cursor_tab(uint8_t x, uint8_t y) {
  set_character_position(x, y);
}

void DiTextArea::get_position(uint16_t& column, uint16_t& row) {
  column = m_current_column;
  row = m_current_row;
}

void DiTextArea::set_foreground_color(uint8_t color) {
  m_fg_color = color;
}

void DiTextArea::set_background_color(uint8_t color) {
  m_bg_color = color;
}

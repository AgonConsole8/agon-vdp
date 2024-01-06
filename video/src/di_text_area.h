// di_text_area.h - Function declarations for supporting a character text_area display
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

#pragma once
#include "di_tile_array.h"

class DiTextArea: public DiTileArray {
  public:

  // Construct a text_area. The text_area always shows characters that are 8x8
  // pixels, based on the built-in Agon font.
  //
  // The given x coordinate must be a multiple of 4, to align the text_area on
  // a 4-byte boundary, which saves memory and processing time.
  //
  DiTextArea(uint32_t x, uint32_t y, uint8_t flags, uint32_t columns, uint32_t rows, const uint8_t* font);

  // Destroy a text_area, including its allocated data.
  virtual ~DiTextArea();

  // Clear the custom instructions needed to draw the primitive.
  virtual void delete_instructions();
   
  // Reassemble the custom instructions needed to draw the primitive.
  virtual void generate_instructions();

  // Define a range of characters using given colors and 8x8 font.
  void define_character_range(uint8_t first_char, uint8_t last_char,
                              uint8_t fg_color, uint8_t bg_color);

  // Define an individual character using given colors and 8x8 font.
  DiTileBitmapID define_character(uint8_t character, uint8_t fg_color, uint8_t bg_color);

  // Set the current character position. The position given may be within the text_area
  // display, or may be outside of it. If it is within the display, then the next
  // character written by write_character(ch) will appear at the given position. If the
  // position is outside of the display, then writing the next character will cause the
  // text_area display to scroll far enough to bring the current character position into
  // view, and the current position will be updated accordingly.
  void set_character_position(int32_t column, int32_t row);

  // Write a character at the current character position. This may cause scrolling
  // BEFORE writing the character (not after), if the current character position is
  // off the visible text_area area. This function will advance the current character
  // position. The character is treated as part of a tile image ID, and is not
  // interpreted as a text_area command of any kind.
  void write_character(uint8_t character);

  // Set the image ID to use to draw a character at a specific row and column.
  // This function does not cause scrolling, nor does it change the current
  // character position. The character is treated as part of a tile image ID,
  // and is not interpreted as a text_area command of any kind.
  void set_character(int32_t column, int32_t row, uint8_t character);

  // Read the character code at the current character position. If the current position
  // is outside of the text_area display, this function returns zero.
  DiTileBitmapID read_character();

  // Read the character code at the given character position.
  DiTileBitmapID read_character(int32_t column, int32_t row);

  // Erase an area of text within the text_area display.
  void erase_text(int32_t column, int32_t row, int32_t columns, int32_t rows);

  // Move an area of text within the text_area display. This may be used to scroll
  // text at the character level (not at the pixel level).
  void move_text(int32_t column, int32_t row, int32_t columns, int32_t rows,
                  int32_t delta_horiz, int32_t delta_vert);

  // Set the foreground color.
  void set_foreground_color(uint8_t color);

  // Set the background color.
  void set_background_color(uint8_t color);

  void clear_screen();
  void move_cursor_left();
  void move_cursor_right();
  void move_cursor_down();
  void move_cursor_up();
  void move_cursor_home();
  void move_cursor_boln();
  void move_cursor_tab(uint8_t column, uint8_t row);
  void do_backspace();
  void get_position(uint16_t& column, uint16_t& row);

  // Bring a potentially off-screen position into view.
  void bring_current_position_into_view();

  protected:
  // Get the bitmap ID for a character, based on current colors.
  DiTileBitmapID get_bitmap_id(uint8_t character);

  // Get the bitmap ID for a character, based on given colors.
  DiTileBitmapID get_bitmap_id(uint8_t character, uint8_t fg_color, uint8_t bg_color);

  int32_t   m_current_column;
  int32_t   m_current_row;
  uint8_t   m_fg_color;
  uint8_t   m_bg_color;
  const uint8_t* m_font;
};


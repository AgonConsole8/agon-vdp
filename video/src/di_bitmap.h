// di_bitmap.h - Function declarations for drawing bitmaps 
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
//
// A masked bitmap is a combination of fully opaque pixels of various colors,
// and fully transparent pixels.
//
// A transparent bitmap is a rectangle that is a combination of fully transparent pixels,
// partially transparent pixels, and fully opaque pixels, of various colors. 
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

class DiBitmap : public DiPrimitive {
  public:
  // Construct a bitmap that owns its pixel data.
  DiBitmap(uint32_t width, uint32_t height, uint16_t flags, bool use_psram);

  // Construct a bitmap that references (borrows) its pixel data.
  DiBitmap(uint16_t flags, DiBitmap* ref_bitmap);

  // Destroy a bitmap.
  ~DiBitmap();

  // Set the X, Y position relative to the parent (which may be the screen).
  virtual void IRAM_ATTR set_relative_position(int32_t rel_x, int32_t rel_y);

  // Set the position of the bitmap, and assume using pixels starting at the given line in the bitmap.
  // This makes it possible to use a single (tall) bitmap to support animated sprites.
  void IRAM_ATTR set_slice_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height);

  // Set a single pixel within the allocated bitmap. The upper 2 bits of the color
  // are the transparency level (00BBGGRR is 25% opaque, 01BBGGRR is 50% opaque,
  // 10BBGGRR is 75% opaque, and 11BBGGRR is 100% opaque). If the given color value
  // equals the already-set transparent color, then the pixel will be fully transparent,
  // meaning 0% opaque.
  void set_transparent_pixel(int32_t x, int32_t y, uint8_t color);

  // Set the single 8-bit color value used to represent a transparent pixel. This should be
  // an unused color value in the visible image when designing the image. This does take out
  // 1 of the 256 possible color values. The upper 2 bits of the color are the transparency
  // level (00BBGGRR is 25% opaque, 01BBGGRR is 50% opaque, 10BBGGRR is 75% opaque, and
  // 11BBGGRR is 100% opaque).
  void set_transparent_color(uint8_t color);

  virtual void generate_code_for_left_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);
  virtual void generate_code_for_right_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);
  virtual void generate_code_for_draw_area(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible);

  // Reassemble the custom instructions needed to draw the primitive.
  virtual void generate_instructions();
   
  virtual void paint(volatile uint32_t* p_scan_line, uint32_t line_index);

  // Get a pointer to the pixel data.
  inline uint32_t* get_pixels() { return m_pixels; }

  protected:
  // Set a single pixel with an adjusted color value.
  void set_pixel(int32_t x, int32_t y, uint8_t color);

  uint32_t    m_words_per_line;
  uint32_t    m_bytes_per_line;
  uint32_t    m_words_per_position;
  uint32_t    m_bytes_per_position;
  uint32_t*   m_visible_start;
  uint32_t*   m_pixels;
  uint32_t    m_save_height;
  uint32_t    m_built_width;
  uint8_t     m_transparent_color;
  bool        m_use_psram;
};

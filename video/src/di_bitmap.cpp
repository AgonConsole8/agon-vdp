// di_bitmap.cpp - Function definitions for drawing bitmaps 
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
//
// A masked bitmap is a combination of fully opaque of various colors,
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

#include "di_bitmap.h"
#include "esp_heap_caps.h"
#include <cstring>
#include "di_timing.h"

DiBitmap::DiBitmap(uint32_t width, uint32_t height, uint16_t flags, bool use_psram) : DiPrimitive(flags) {
  m_width = width;
  m_height = height;
  m_save_height = height;
  m_flags |= PRIM_FLAGS_X_SRC;
  m_transparent_color = 0;
  m_use_psram = use_psram;

  if (flags & PRIM_FLAG_H_SCROLL_1) {
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t) + 2);
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);

      if (use_psram) {
        m_pixels = (uint32_t*) heap_caps_malloc(m_bytes_per_position * 4, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT | MALLOC_CAP_32BIT);
      } else {
        m_pixels = new uint32_t[m_words_per_position * 4];
      }

      memset(m_pixels, 0x00, m_bytes_per_position * 4);
  } else {
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t));
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);

      if (use_psram) {
        m_pixels = (uint32_t*) heap_caps_malloc(m_bytes_per_position, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT | MALLOC_CAP_32BIT);
      } else {
        m_pixels = new uint32_t[m_words_per_position];
      }

      memset(m_pixels, 0x00, m_bytes_per_position);
  }
  m_visible_start = m_pixels;
  m_paint_code.enter_and_leave_outer_function();
}

DiBitmap::DiBitmap(uint16_t flags, DiBitmap* ref_bitmap) : DiPrimitive(flags) {
  m_width = ref_bitmap->m_width;
  m_height = ref_bitmap->m_height;
  m_save_height = ref_bitmap->m_save_height;
  m_flags |= (ref_bitmap->m_flags & (PRIM_FLAG_H_SCROLL_1|PRIM_FLAGS_BLENDED)) | (PRIM_FLAGS_X_SRC|PRIM_FLAGS_REF_DATA);
  m_transparent_color = ref_bitmap->m_transparent_color;
  m_words_per_line = ref_bitmap->m_words_per_line;
  m_bytes_per_line = ref_bitmap->m_bytes_per_line;
  m_words_per_position = ref_bitmap->m_words_per_position;
  m_bytes_per_position = ref_bitmap->m_bytes_per_position;
  m_pixels = ref_bitmap->m_pixels;
  m_visible_start = m_pixels;
  m_paint_code.enter_and_leave_outer_function();
}


DiBitmap::~DiBitmap() {
  if (!(m_flags & PRIM_FLAGS_REF_DATA)) {
    if (m_use_psram) {
      heap_caps_free(m_pixels);
    } else {
      delete [] m_pixels;
    }
  }
}

void IRAM_ATTR DiBitmap::set_relative_position(int32_t x, int32_t y) {
  DiPrimitive::set_relative_position(x, y);
  m_visible_start = m_pixels;
}

void DiBitmap::set_slice_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height) {
  DiPrimitive::set_relative_position(x, y);
  m_height = height;
  m_visible_start = m_pixels + start_line * m_words_per_line;
}

void DiBitmap::set_transparent_pixel(uint32_t x, uint32_t y, uint8_t color) {
  // Invert the meaning of the alpha bits.
  set_pixel(x, y, PIXEL_ALPHA_INV_MASK(color));
}

void DiBitmap::set_transparent_color(uint8_t color) {
  m_transparent_color = PIXEL_ALPHA_INV_MASK(color);
}

void DiBitmap::set_pixel(uint32_t x, uint32_t y, uint8_t color) {
  uint32_t* p;
  int32_t index;

  if (m_flags & PRIM_FLAG_H_SCROLL_1) {
    for (uint32_t pos = 0; pos < 4; pos++) {
      p = m_pixels + pos * m_words_per_position + y * m_words_per_line + (FIX_INDEX(pos+x) / 4);
      index = FIX_INDEX((pos+x)&3);
      pixels(p)[index] = color;
    }
  } else {
    p = m_pixels + y * m_words_per_line + (FIX_INDEX(x) / 4);
    index = FIX_INDEX(x&3);
    pixels(p)[index] = color;
  }
}

void DiBitmap::copy_pixels(DiBitmap* from_bitmap, uint8_t flip) {
  auto lines = m_height;
  if (m_flags & PRIM_FLAG_H_SCROLL_1) {
    lines *= 4;
  }
  auto width = m_words_per_line * 4;
  if (flip) {
    auto src_pixels = (uint8_t*) from_bitmap->m_pixels;
    auto dst_pixels = (uint8_t*) m_pixels;
    if (flip & 0x02) {
      dst_pixels = (uint8_t*)(m_pixels + (lines - 1));
    }
    while (lines--) {
      if (flip & 0x01) {
        for (uint32_t x = 0; x < width; x++) {
          auto psrc = src_pixels + FIX_INDEX(x);
          auto pdst = dst_pixels + FIX_INDEX((width - 1 - x));
          *pdst = *psrc;
        }
      } else {
        // pixels in line are not flipped
        memcpy(dst_pixels, src_pixels, width);
      }
    }
    src_pixels += m_bytes_per_line;
    if (flip & 0x02) {
      dst_pixels -= m_bytes_per_line;
    } else {
      dst_pixels += m_bytes_per_line;
    }
  } else {
    memcpy(m_pixels, from_bitmap->m_pixels, lines * width);
  }
}

void DiBitmap::generate_instructions() {
  delete_instructions();
  EspFixups fixups;
  generate_code_for_positions(fixups, m_width, m_height);
  m_paint_code.do_fixups(fixups);
  set_current_paint_pointer(m_width, m_height);
  setup_alpha_bits();
}
extern void debug_log(const char* fmt,...);
void DiBitmap::setup_alpha_bits() {
  // Clear the alpha bits, and replace them with HS & VS bits,
  // so that the bytes may be copied directly to the DMA buffers.
  uint32_t n = m_words_per_position;
  if (m_flags & PRIM_FLAG_H_SCROLL_1) {
    n *= 4;
  }
  debug_log("bmid %04X, custom %08X, set %u alpha words using %08X\n",m_id,m_custom,n,otf_video_params.m_syncs_off_x4);
  uint32_t* src_pixels = m_pixels;
  while (n--) {
    *src_pixels = (*src_pixels & 0x3F3F3F3F) | otf_video_params.m_syncs_off_x4;
    src_pixels++;
  }
}

void DiBitmap::generate_code_for_left_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_left_edge(fixups, x_offset, width, height, hidden, visible);
  auto draw_width = (m_draw_x_extent - m_draw_x) - hidden;
  if (m_flags & PRIM_FLAGS_ALL_SAME) {
    m_paint_code.copy_line(fixups, x_offset, hidden, visible, m_flags, m_transparent_color, m_visible_start, true);
  } else {
    uint32_t at_jump_table = m_paint_code.init_jump_table(m_height);
    uint32_t* src_pixels = m_visible_start;
    for (uint32_t i = 0; i < m_height; i++) {
      m_paint_code.align32();
      m_paint_code.j_to_here(at_jump_table + i * sizeof(uint32_t));
      m_paint_code.copy_line(fixups, x_offset, hidden, visible, m_flags, m_transparent_color, src_pixels, false);
      src_pixels += m_words_per_line;
    }
  }
}

void DiBitmap::generate_code_for_right_edge(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_right_edge(fixups, x_offset, width, height, hidden, visible);
  auto draw_width = (m_draw_x_extent - m_draw_x) - hidden;
  if (m_flags & PRIM_FLAGS_ALL_SAME) {
    m_paint_code.copy_line(fixups, x_offset, 0, visible, m_flags, m_transparent_color, m_visible_start, true);
  } else {
    uint32_t at_jump_table = m_paint_code.init_jump_table(m_height);
    uint32_t* src_pixels = m_visible_start;
    for (uint32_t i = 0; i < m_height; i++) {
      m_paint_code.align32();
      m_paint_code.j_to_here(at_jump_table + i * sizeof(uint32_t));
      m_paint_code.copy_line(fixups, x_offset, 0, visible, m_flags, m_transparent_color, src_pixels, false);
      src_pixels += m_words_per_line;
    }
  }
}

void DiBitmap::generate_code_for_draw_area(EspFixups& fixups, uint32_t x_offset, uint32_t width, uint32_t height, uint32_t hidden, uint32_t visible) {
  DiPrimitive::generate_code_for_draw_area(fixups, x_offset, width, height, hidden, visible);
  auto draw_width = m_draw_x_extent - m_draw_x;
  if (m_flags & PRIM_FLAGS_ALL_SAME) {
    m_paint_code.copy_line(fixups, x_offset, 0, draw_width, m_flags, m_transparent_color, m_visible_start, true);
  } else {
    uint32_t at_jump_table = m_paint_code.init_jump_table(m_height);
    uint32_t* src_pixels = m_visible_start;
    for (uint32_t i = 0; i < m_height; i++) {
      m_paint_code.align32();
      m_paint_code.j_to_here(at_jump_table + i * sizeof(uint32_t));
      m_paint_code.copy_line(fixups, x_offset, 0, draw_width, m_flags, m_transparent_color, src_pixels, false);
      src_pixels += m_words_per_line;
    }
  }
}

void IRAM_ATTR DiBitmap::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  if (!m_cur_paint_ptr.m_address) return;
  auto line_offset = line_index - m_abs_y;
  uint32_t pixels = (uint32_t)(m_visible_start + (m_words_per_line * line_offset));
  (*(m_cur_paint_ptr.m_a5a6))(this, p_scan_line, line_index, m_abs_x, pixels);
}

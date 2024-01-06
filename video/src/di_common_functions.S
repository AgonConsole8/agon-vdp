// di_common_functions.S - Low-level assembler functions for setting pixels
//
// Copyright (c) 2023 Curtis Whitley
// 
// The functions below are intended to be called using the CALL0 or CALLX0
// instructions, and are thus not intended to be called directly from C++,
// which would require using instructions such as CALL8.
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

  .section .iram1.text
  .align 4
  
  .global   fcn_draw_256_pixels_in_loop
  .global   fcn_draw_128_pixels
  .global   fcn_draw_128_pixels_last
  .global   fcn_draw_64_pixels
  .global   fcn_draw_64_pixels_last
  .global   fcn_draw_32_pixels
  .global   fcn_draw_32_pixels_last
  .global   fcn_draw_16_pixels
  .global   fcn_draw_16_pixels_last
  .global   fcn_draw_8_pixels
  .global   fcn_draw_8_pixels_last
  .global   fcn_get_blend_25_for_4_pixels
  .global   fcn_get_blend_50_for_4_pixels
  .global   fcn_get_blend_75_for_4_pixels
  .global   fcn_dummy

  .global   fcn_skip_draw_256_pixels_in_loop
  .global   fcn_skip_draw_128_pixels
  .global   fcn_skip_draw_64_pixels
  .global   fcn_skip_draw_32_pixels
  .global   fcn_skip_draw_16_pixels
  .global   fcn_skip_draw_8_pixels

  .global   fcn_copy_words_in_loop

  .global   fcn_copy_256_pixels_in_loop
  .global   fcn_copy_128_pixels
  .global   fcn_copy_128_pixels_last
  .global   fcn_copy_64_pixels
  .global   fcn_copy_64_pixels_last
  .global   fcn_copy_32_pixels
  .global   fcn_copy_32_pixels_last
  .global   fcn_copy_16_pixels
  .global   fcn_copy_16_pixels_last
  .global   fcn_copy_8_pixels
  .global   fcn_copy_8_pixels_last

  .global   fcn_skip_copy_256_pixels_in_loop
  .global   fcn_skip_copy_128_pixels
  .global   fcn_skip_copy_64_pixels
  .global   fcn_skip_copy_32_pixels
  .global   fcn_skip_copy_16_pixels
  .global   fcn_skip_copy_8_pixels

  .global   fcn_color_blend_25_for_256_pixels_in_loop
  .global   fcn_color_blend_25_for_128_pixels
  .global   fcn_color_blend_25_for_128_pixels_last
  .global   fcn_color_blend_25_for_64_pixels
  .global   fcn_color_blend_25_for_64_pixels_last
  .global   fcn_color_blend_25_for_32_pixels
  .global   fcn_color_blend_25_for_32_pixels_last
  .global   fcn_color_blend_25_for_16_pixels
  .global   fcn_color_blend_25_for_16_pixels_last
  .global   fcn_color_blend_25_for_8_pixels
  .global   fcn_color_blend_25_for_8_pixels_last
  .global   fcn_color_blend_25_for_1_pixel_at_offset_0
  .global   fcn_color_blend_25_for_1_pixel_at_offset_0_last
  .global   fcn_color_blend_25_for_1_pixel_at_offset_1
  .global   fcn_color_blend_25_for_1_pixel_at_offset_1_last
  .global   fcn_color_blend_25_for_1_pixel_at_offset_2
  .global   fcn_color_blend_25_for_1_pixel_at_offset_2_last
  .global   fcn_color_blend_25_for_1_pixel_at_offset_3
  .global   fcn_color_blend_25_for_1_pixel_at_offset_3_last
  .global   fcn_color_blend_25_for_2_pixels_at_offset_0
  .global   fcn_color_blend_25_for_2_pixels_at_offset_0_last
  .global   fcn_color_blend_25_for_2_pixels_at_offset_1
  .global   fcn_color_blend_25_for_2_pixels_at_offset_1_last
  .global   fcn_color_blend_25_for_2_pixels_at_offset_2
  .global   fcn_color_blend_25_for_2_pixels_at_offset_2_last
  .global   fcn_color_blend_25_for_3_pixels_at_offset_0
  .global   fcn_color_blend_25_for_3_pixels_at_offset_0_last
  .global   fcn_color_blend_25_for_3_pixels_at_offset_1
  .global   fcn_color_blend_25_for_3_pixels_at_offset_1_last
  .global   fcn_color_blend_25_for_4_pixels_at_offset_0
  .global   fcn_color_blend_25_for_4_pixels_at_offset_0_last

  .global   fcn_color_blend_50_for_256_pixels_in_loop
  .global   fcn_color_blend_50_for_128_pixels
  .global   fcn_color_blend_50_for_128_pixels_last
  .global   fcn_color_blend_50_for_64_pixels
  .global   fcn_color_blend_50_for_64_pixels_last
  .global   fcn_color_blend_50_for_32_pixels
  .global   fcn_color_blend_50_for_32_pixels_last
  .global   fcn_color_blend_50_for_16_pixels
  .global   fcn_color_blend_50_for_16_pixels_last
  .global   fcn_color_blend_50_for_8_pixels
  .global   fcn_color_blend_50_for_8_pixels_last
  .global   fcn_color_blend_50_for_1_pixel_at_offset_0
  .global   fcn_color_blend_50_for_1_pixel_at_offset_0_last
  .global   fcn_color_blend_50_for_1_pixel_at_offset_1
  .global   fcn_color_blend_50_for_1_pixel_at_offset_1_last
  .global   fcn_color_blend_50_for_1_pixel_at_offset_2
  .global   fcn_color_blend_50_for_1_pixel_at_offset_2_last
  .global   fcn_color_blend_50_for_1_pixel_at_offset_3
  .global   fcn_color_blend_50_for_1_pixel_at_offset_3_last
  .global   fcn_color_blend_50_for_2_pixels_at_offset_0
  .global   fcn_color_blend_50_for_2_pixels_at_offset_0_last
  .global   fcn_color_blend_50_for_2_pixels_at_offset_1
  .global   fcn_color_blend_50_for_2_pixels_at_offset_1_last
  .global   fcn_color_blend_50_for_2_pixels_at_offset_2
  .global   fcn_color_blend_50_for_2_pixels_at_offset_2_last
  .global   fcn_color_blend_50_for_3_pixels_at_offset_0
  .global   fcn_color_blend_50_for_3_pixels_at_offset_0_last
  .global   fcn_color_blend_50_for_3_pixels_at_offset_1
  .global   fcn_color_blend_50_for_3_pixels_at_offset_1_last
  .global   fcn_color_blend_50_for_4_pixels_at_offset_0
  .global   fcn_color_blend_50_for_4_pixels_at_offset_0_last

  .global   fcn_color_blend_75_for_256_pixels_in_loop
  .global   fcn_color_blend_75_for_128_pixels
  .global   fcn_color_blend_75_for_128_pixels_last
  .global   fcn_color_blend_75_for_64_pixels
  .global   fcn_color_blend_75_for_64_pixels_last
  .global   fcn_color_blend_75_for_32_pixels
  .global   fcn_color_blend_75_for_32_pixels_last
  .global   fcn_color_blend_75_for_16_pixels
  .global   fcn_color_blend_75_for_16_pixels_last
  .global   fcn_color_blend_75_for_8_pixels
  .global   fcn_color_blend_75_for_8_pixels_last
  .global   fcn_color_blend_75_for_1_pixel_at_offset_0
  .global   fcn_color_blend_75_for_1_pixel_at_offset_0_last
  .global   fcn_color_blend_75_for_1_pixel_at_offset_1
  .global   fcn_color_blend_75_for_1_pixel_at_offset_1_last
  .global   fcn_color_blend_75_for_1_pixel_at_offset_2
  .global   fcn_color_blend_75_for_1_pixel_at_offset_2_last
  .global   fcn_color_blend_75_for_1_pixel_at_offset_3
  .global   fcn_color_blend_75_for_1_pixel_at_offset_3_last
  .global   fcn_color_blend_75_for_2_pixels_at_offset_0
  .global   fcn_color_blend_75_for_2_pixels_at_offset_0_last
  .global   fcn_color_blend_75_for_2_pixels_at_offset_1
  .global   fcn_color_blend_75_for_2_pixels_at_offset_1_last
  .global   fcn_color_blend_75_for_2_pixels_at_offset_2
  .global   fcn_color_blend_75_for_2_pixels_at_offset_2_last
  .global   fcn_color_blend_75_for_3_pixels_at_offset_0
  .global   fcn_color_blend_75_for_3_pixels_at_offset_0_last
  .global   fcn_color_blend_75_for_3_pixels_at_offset_1
  .global   fcn_color_blend_75_for_3_pixels_at_offset_1_last
  .global   fcn_color_blend_75_for_4_pixels_at_offset_0
  .global   fcn_color_blend_75_for_4_pixels_at_offset_0_last

  .global   fcn_src_blend_25_for_256_pixels_in_loop
  .global   fcn_src_blend_25_for_128_pixels
  .global   fcn_src_blend_25_for_128_pixels_last
  .global   fcn_src_blend_25_for_64_pixels
  .global   fcn_src_blend_25_for_64_pixels_last
  .global   fcn_src_blend_25_for_32_pixels
  .global   fcn_src_blend_25_for_32_pixels_last
  .global   fcn_src_blend_25_for_16_pixels
  .global   fcn_src_blend_25_for_16_pixels_last
  .global   fcn_src_blend_25_for_8_pixels
  .global   fcn_src_blend_25_for_8_pixels_last
  .global   fcn_src_blend_25_for_1_pixel_at_offset_0
  .global   fcn_src_blend_25_for_1_pixel_at_offset_0_last
  .global   fcn_src_blend_25_for_1_pixel_at_offset_1
  .global   fcn_src_blend_25_for_1_pixel_at_offset_1_last
  .global   fcn_src_blend_25_for_1_pixel_at_offset_2
  .global   fcn_src_blend_25_for_1_pixel_at_offset_2_last
  .global   fcn_src_blend_25_for_1_pixel_at_offset_3
  .global   fcn_src_blend_25_for_1_pixel_at_offset_3_last
  .global   fcn_src_blend_25_for_2_pixels_at_offset_0
  .global   fcn_src_blend_25_for_2_pixels_at_offset_0_last
  .global   fcn_src_blend_25_for_2_pixels_at_offset_1
  .global   fcn_src_blend_25_for_2_pixels_at_offset_1_last
  .global   fcn_src_blend_25_for_2_pixels_at_offset_2
  .global   fcn_src_blend_25_for_2_pixels_at_offset_2_last
  .global   fcn_src_blend_25_for_3_pixels_at_offset_0
  .global   fcn_src_blend_25_for_3_pixels_at_offset_0_last
  .global   fcn_src_blend_25_for_3_pixels_at_offset_1
  .global   fcn_src_blend_25_for_3_pixels_at_offset_1_last
  .global   fcn_src_blend_25_for_4_pixels_at_offset_0
  .global   fcn_src_blend_25_for_4_pixels_at_offset_0_last

  .global   fcn_src_blend_50_for_256_pixels_in_loop
  .global   fcn_src_blend_50_for_128_pixels
  .global   fcn_src_blend_50_for_128_pixels_last
  .global   fcn_src_blend_50_for_64_pixels
  .global   fcn_src_blend_50_for_64_pixels_last
  .global   fcn_src_blend_50_for_32_pixels
  .global   fcn_src_blend_50_for_32_pixels_last
  .global   fcn_src_blend_50_for_16_pixels
  .global   fcn_src_blend_50_for_16_pixels_last
  .global   fcn_src_blend_50_for_8_pixels
  .global   fcn_src_blend_50_for_8_pixels_last
  .global   fcn_src_blend_50_for_1_pixel_at_offset_0
  .global   fcn_src_blend_50_for_1_pixel_at_offset_0_last
  .global   fcn_src_blend_50_for_1_pixel_at_offset_1
  .global   fcn_src_blend_50_for_1_pixel_at_offset_1_last
  .global   fcn_src_blend_50_for_1_pixel_at_offset_2
  .global   fcn_src_blend_50_for_1_pixel_at_offset_2_last
  .global   fcn_src_blend_50_for_1_pixel_at_offset_3
  .global   fcn_src_blend_50_for_1_pixel_at_offset_3_last
  .global   fcn_src_blend_50_for_2_pixels_at_offset_0
  .global   fcn_src_blend_50_for_2_pixels_at_offset_0_last
  .global   fcn_src_blend_50_for_2_pixels_at_offset_1
  .global   fcn_src_blend_50_for_2_pixels_at_offset_1_last
  .global   fcn_src_blend_50_for_2_pixels_at_offset_2
  .global   fcn_src_blend_50_for_2_pixels_at_offset_2_last
  .global   fcn_src_blend_50_for_3_pixels_at_offset_0
  .global   fcn_src_blend_50_for_3_pixels_at_offset_0_last
  .global   fcn_src_blend_50_for_3_pixels_at_offset_1
  .global   fcn_src_blend_50_for_3_pixels_at_offset_1_last
  .global   fcn_src_blend_50_for_4_pixels_at_offset_0
  .global   fcn_src_blend_50_for_4_pixels_at_offset_0_last

  .global   fcn_src_blend_75_for_256_pixels_in_loop
  .global   fcn_src_blend_75_for_128_pixels
  .global   fcn_src_blend_75_for_128_pixels_last
  .global   fcn_src_blend_75_for_64_pixels
  .global   fcn_src_blend_75_for_64_pixels_last
  .global   fcn_src_blend_75_for_32_pixels
  .global   fcn_src_blend_75_for_32_pixels_last
  .global   fcn_src_blend_75_for_16_pixels
  .global   fcn_src_blend_75_for_16_pixels_last
  .global   fcn_src_blend_75_for_8_pixels
  .global   fcn_src_blend_75_for_8_pixels_last
  .global   fcn_src_blend_75_for_1_pixel_at_offset_0
  .global   fcn_src_blend_75_for_1_pixel_at_offset_0_last
  .global   fcn_src_blend_75_for_1_pixel_at_offset_1
  .global   fcn_src_blend_75_for_1_pixel_at_offset_1_last
  .global   fcn_src_blend_75_for_1_pixel_at_offset_2
  .global   fcn_src_blend_75_for_1_pixel_at_offset_2_last
  .global   fcn_src_blend_75_for_1_pixel_at_offset_3
  .global   fcn_src_blend_75_for_1_pixel_at_offset_3_last
  .global   fcn_src_blend_75_for_2_pixels_at_offset_0
  .global   fcn_src_blend_75_for_2_pixels_at_offset_0_last
  .global   fcn_src_blend_75_for_2_pixels_at_offset_1
  .global   fcn_src_blend_75_for_2_pixels_at_offset_1_last
  .global   fcn_src_blend_75_for_2_pixels_at_offset_2
  .global   fcn_src_blend_75_for_2_pixels_at_offset_2_last
  .global   fcn_src_blend_75_for_3_pixels_at_offset_0
  .global   fcn_src_blend_75_for_3_pixels_at_offset_0_last
  .global   fcn_src_blend_75_for_3_pixels_at_offset_1
  .global   fcn_src_blend_75_for_3_pixels_at_offset_1_last
  .global   fcn_src_blend_75_for_4_pixels_at_offset_0
  .global   fcn_src_blend_75_for_4_pixels_at_offset_0_last

// Input registers:
#define REG_RETURN_ADDR     a0
#define REG_STACK_PTR       a1
#define REG_THIS_PTR        a2
#define REG_LINE_PTR        a3
#define REG_LINE_INDEX      a4
#define REG_DST_DRAW_X      a5
#define REG_SRC_PIXEL_PTR   a6
// Temporary registers:
#define REG_SAVE_RET_DEEP   a3
#define REG_DST_PIXEL_PTR   a5
#define REG_PIXEL_COLOR     a7
#define REG_LOOP_INDEX      a4
#define REG_SRC_PIXELS      a8
#define REG_SRC_BR_PIXELS   a9
#define REG_DST_BR_PIXELS   a10
#define REG_SRC_G_PIXELS    a8
#define REG_DST_G_PIXELS    a11
#define REG_ABS_Y           a12
#define REG_DOUBLE_COLOR    a12
#define REG_ISOLATE_BR      a13
#define REG_ISOLATE_G       a14
#define REG_JUMP_ADDRESS    a14
#define REG_SAVE_COLOR      a15     // also the transparent color when copying pixels

#define FIX_OFFSET(off)    ((off)^2)

  .macro set_1_pixel_at_offset_0
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro set_1_pixel_at_offset_1
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro set_1_pixel_at_offset_2
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro set_1_pixel_at_offset_3
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3)
  .endm

  .macro set_2_pixels_at_offset_0
    s16i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro set_2_pixels_at_offset_1
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro set_2_pixels_at_offset_2
    s16i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)    
  .endm

  .macro set_3_pixels_at_offset_0
    s16i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro set_3_pixels_at_offset_1
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)    
    s16i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro set_4_pixels_at_offset offset
    s32i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \offset
  .endm

  .macro copy_1_pixel_at_offset_0
    l8ui  REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro copy_1_pixel_at_offset_1
    l8ui  REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(1)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro copy_1_pixel_at_offset_2
    l8ui  REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro copy_1_pixel_at_offset_3
    l8ui  REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(3)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3)
  .endm

  .macro copy_2_pixels_at_offset_0
    l16ui REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0)
    s16i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro copy_2_pixels_at_offset_1
    l16ui REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(1)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
    l16ui REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro copy_2_pixels_at_offset_2
    l16ui REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)    
    s16i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)    
  .endm

  .macro copy_3_pixels_at_offset_0
    l16ui REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0)
    s16i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
    l16ui REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro copy_3_pixels_at_offset_1
    l8ui  REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(1)    
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1) 
    l16ui REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)
    s16i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro copy_4_pixels_at_offset word_offset
    l32i  REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, \word_offset
    s32i  REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \word_offset
  .endm

// In REG_PIXEL_COLOR
// 31..24 23..16 15..08 07..00
// pixel1 pixel0 pixel3 pixel2

  .macro blend_for_1_pixel_at_offset_0 pct src
    .ifc \src\(),src
      l32i  REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_for_1_pixel_at_offset_1 pct src
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_for_1_pixel_at_offset_2 pct src
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro blend_for_1_pixel_at_offset_3 pct src
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 8
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3)
  .endm

  .macro blend_for_2_pixels_at_offset_0 pct src
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_for_2_pixels_at_offset_1 pct src
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_for_2_pixels_at_offset_2 pct src
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro blend_for_3_pixels_at_offset_0 pct src
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_for_3_pixels_at_offset_1 pct src
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_\pct\()_for_4_pixels
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_for_4_pixels_at_offset pct src wo
    .ifc \src\(),src
      l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, \wo
    .else
      mov   REG_SRC_PIXELS, REG_SAVE_COLOR
    .endif
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \wo
    call0  fcn_get_blend_\pct\()_for_4_pixels
    s32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \wo
  .endm

  .macro enter_blend_fcn
    mov   REG_SAVE_RET_DEEP, REG_RETURN_ADDR
  .endm

  .macro leave_blend_fcn
    jx    REG_SAVE_RET_DEEP
  .endm

  .align 4
fcn_draw_256_pixels_in_loop:
begin_loop:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  set_4_pixels_at_offset 64
  set_4_pixels_at_offset 68
  set_4_pixels_at_offset 72
  set_4_pixels_at_offset 76
  set_4_pixels_at_offset 80
  set_4_pixels_at_offset 84
  set_4_pixels_at_offset 88
  set_4_pixels_at_offset 92
  set_4_pixels_at_offset 96
  set_4_pixels_at_offset 100
  set_4_pixels_at_offset 104
  set_4_pixels_at_offset 108
  set_4_pixels_at_offset 112
  set_4_pixels_at_offset 116
  set_4_pixels_at_offset 120
  set_4_pixels_at_offset 124

  set_4_pixels_at_offset 128
  set_4_pixels_at_offset 132
  set_4_pixels_at_offset 136
  set_4_pixels_at_offset 140
  set_4_pixels_at_offset 144
  set_4_pixels_at_offset 148
  set_4_pixels_at_offset 152
  set_4_pixels_at_offset 156
  set_4_pixels_at_offset 160
  set_4_pixels_at_offset 164
  set_4_pixels_at_offset 168
  set_4_pixels_at_offset 172
  set_4_pixels_at_offset 176
  set_4_pixels_at_offset 180
  set_4_pixels_at_offset 184
  set_4_pixels_at_offset 188
  set_4_pixels_at_offset 192
  set_4_pixels_at_offset 196
  set_4_pixels_at_offset 200
  set_4_pixels_at_offset 204
  set_4_pixels_at_offset 208
  set_4_pixels_at_offset 212
  set_4_pixels_at_offset 216
  set_4_pixels_at_offset 220
  set_4_pixels_at_offset 224
  set_4_pixels_at_offset 228
  set_4_pixels_at_offset 232
  set_4_pixels_at_offset 236
  set_4_pixels_at_offset 240
  set_4_pixels_at_offset 244
  set_4_pixels_at_offset 248
  set_4_pixels_at_offset 252

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_loop
  j      begin_loop
end_loop:
  ret

  .align 4
fcn_draw_128_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  set_4_pixels_at_offset 64
  set_4_pixels_at_offset 68
  set_4_pixels_at_offset 72
  set_4_pixels_at_offset 76
  set_4_pixels_at_offset 80
  set_4_pixels_at_offset 84
  set_4_pixels_at_offset 88
  set_4_pixels_at_offset 92
  set_4_pixels_at_offset 96
  set_4_pixels_at_offset 100
  set_4_pixels_at_offset 104
  set_4_pixels_at_offset 108
  set_4_pixels_at_offset 112
  set_4_pixels_at_offset 116
  set_4_pixels_at_offset 120
  set_4_pixels_at_offset 124
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  ret

  .align 4
fcn_draw_128_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  set_4_pixels_at_offset 64
  set_4_pixels_at_offset 68
  set_4_pixels_at_offset 72
  set_4_pixels_at_offset 76
  set_4_pixels_at_offset 80
  set_4_pixels_at_offset 84
  set_4_pixels_at_offset 88
  set_4_pixels_at_offset 92
  set_4_pixels_at_offset 96
  set_4_pixels_at_offset 100
  set_4_pixels_at_offset 104
  set_4_pixels_at_offset 108
  set_4_pixels_at_offset 112
  set_4_pixels_at_offset 116
  set_4_pixels_at_offset 120
  set_4_pixels_at_offset 124
  ret

  .align 4
fcn_draw_64_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  ret

  .align 4
fcn_draw_64_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  ret

  .align 4
fcn_draw_32_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32
  ret

  .align 4
fcn_draw_32_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  ret

  .align 4
fcn_draw_16_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16
  ret

  .align 4
fcn_draw_16_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  ret

  .align 4
fcn_draw_8_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8
  ret

  .align 4
fcn_draw_8_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  ret

  .align 4
fcn_skip_draw_256_pixels_in_loop:
begin_skip_draw_loop:
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_skip_draw_loop
  j      begin_skip_draw_loop
end_skip_draw_loop:
  ret

  .align 4
fcn_skip_draw_128_pixels:
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  ret

  .align 4
fcn_skip_draw_64_pixels:
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  ret

  .align 4
fcn_skip_draw_32_pixels:
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32
  ret

  .align 4
fcn_skip_draw_16_pixels:
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16
  ret

  .align 4
fcn_skip_draw_8_pixels:
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8
  ret

  .align 4
fcn_get_blend_25_for_4_pixels:
  // 00BBGGRR 00BBGGRR 00BBGGRR 00BBGGRR - source or destination pixels
  // 00110011 00110011 00110011 00110011 - isolate blue & red
  // 00001100 00001100 00001100 00001100 - isolate green
  and   REG_SRC_BR_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_BR // src blue & red (no green)
  and   REG_DST_BR_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_BR // dst blue & red (no green)
  slli  REG_DOUBLE_COLOR, REG_DST_BR_PIXELS, 1 // double the dst color values
  add   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_DOUBLE_COLOR // add dst color values again (total of 3X)
  add   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_SRC_BR_PIXELS // sum of blue & sum of red
  srli  REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, 2 // averages of mixed blue & mixed red, in 4 bits each
  and   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_ISOLATE_BR // averages in 2 bits each

  and   REG_SRC_G_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_G // src green (no red or blue)
  and   REG_DST_G_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_G // dst green (no red or blue)
  slli  REG_DOUBLE_COLOR, REG_DST_G_PIXELS, 1 // double the dst color values
  add   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_DOUBLE_COLOR // add dst color values again (total of 3X)
  add   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_SRC_G_PIXELS // sum of green
  srli  REG_DST_G_PIXELS, REG_DST_G_PIXELS, 2 // averages of mixed green, in 4 bits each
  and   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_G // averages in 2 bits each

  or    REG_PIXEL_COLOR, REG_DST_BR_PIXELS, REG_DST_G_PIXELS // 4 mixed pixels
  ret

  .align 4
fcn_get_blend_50_for_4_pixels:
  and   REG_SRC_BR_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_BR // src blue & red (no green)
  and   REG_DST_BR_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_BR // dst blue & red (no green)
  add   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_SRC_BR_PIXELS // sum of blue & sum of red
  srli  REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, 1 // averages of mixed blue & mixed red, in 3 bits each
  and   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_ISOLATE_BR // averages in 2 bits each

  and   REG_SRC_G_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_G // src green (no red or blue)
  and   REG_DST_G_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_G // dst green (no red or blue)
  add   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_SRC_G_PIXELS // sum of green
  srli  REG_DST_G_PIXELS, REG_DST_G_PIXELS, 1 // averages of mixed green, in 3 bits each
  and   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_G // averages in 2 bits each

  or    REG_PIXEL_COLOR, REG_DST_BR_PIXELS, REG_DST_G_PIXELS // 4 mixed pixels
  ret

  .align 4
fcn_get_blend_75_for_4_pixels:
  and   REG_SRC_BR_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_BR // src blue & red (no green)
  and   REG_DST_BR_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_BR // dst blue & red (no green)
  slli  REG_DOUBLE_COLOR, REG_SRC_BR_PIXELS, 1 // double the src color values
  add   REG_SRC_BR_PIXELS, REG_SRC_BR_PIXELS, REG_DOUBLE_COLOR // add src color values again (total of 3X)
  add   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_SRC_BR_PIXELS // sum of blue & sum of red
  srli  REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, 2 // averages of mixed blue & mixed red, in 4 bits each
  and   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_ISOLATE_BR // averages in 2 bits each

  and   REG_SRC_G_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_G // src green (no red or blue)
  and   REG_DST_G_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_G // dst green (no red or blue)
  slli  REG_DOUBLE_COLOR, REG_SRC_G_PIXELS, 1 // double the src color values
  add   REG_SRC_G_PIXELS, REG_SRC_G_PIXELS, REG_DOUBLE_COLOR // add src color values again (total of 3X)
  add   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_SRC_G_PIXELS // sum of green
  srli  REG_DST_G_PIXELS, REG_DST_G_PIXELS, 2 // averages of mixed green, in 4 bits each
  and   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_G // averages in 2 bits each

  or    REG_PIXEL_COLOR, REG_DST_BR_PIXELS, REG_DST_G_PIXELS // 4 mixed pixels
  ret

  .align 4
fcn_dummy:
  ret

// a0: return address
// a1: stack pointer
// a2: destination pointer
// a3: source pointer
// a4: number of words
  .align 4
fcn_copy_words_in_loop:
  entry sp,32
  loop  a4,end_loop_words
  l32i  a5,a3,0
  s32i  a5,a2,0
  addi  a3,a3,4
  addi  a2,a2,4
end_loop_words:
  retw

  .align 4
fcn_copy_256_pixels_in_loop:
begin_copy_loop:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  copy_4_pixels_at_offset 64
  copy_4_pixels_at_offset 68
  copy_4_pixels_at_offset 72
  copy_4_pixels_at_offset 76
  copy_4_pixels_at_offset 80
  copy_4_pixels_at_offset 84
  copy_4_pixels_at_offset 88
  copy_4_pixels_at_offset 92
  copy_4_pixels_at_offset 96
  copy_4_pixels_at_offset 100
  copy_4_pixels_at_offset 104
  copy_4_pixels_at_offset 108
  copy_4_pixels_at_offset 112
  copy_4_pixels_at_offset 116
  copy_4_pixels_at_offset 120
  copy_4_pixels_at_offset 124

  copy_4_pixels_at_offset 128
  copy_4_pixels_at_offset 132
  copy_4_pixels_at_offset 136
  copy_4_pixels_at_offset 140
  copy_4_pixels_at_offset 144
  copy_4_pixels_at_offset 148
  copy_4_pixels_at_offset 152
  copy_4_pixels_at_offset 156
  copy_4_pixels_at_offset 160
  copy_4_pixels_at_offset 164
  copy_4_pixels_at_offset 168
  copy_4_pixels_at_offset 172
  copy_4_pixels_at_offset 176
  copy_4_pixels_at_offset 180
  copy_4_pixels_at_offset 184
  copy_4_pixels_at_offset 188
  copy_4_pixels_at_offset 192
  copy_4_pixels_at_offset 196
  copy_4_pixels_at_offset 200
  copy_4_pixels_at_offset 204
  copy_4_pixels_at_offset 208
  copy_4_pixels_at_offset 212
  copy_4_pixels_at_offset 216
  copy_4_pixels_at_offset 220
  copy_4_pixels_at_offset 224
  copy_4_pixels_at_offset 228
  copy_4_pixels_at_offset 232
  copy_4_pixels_at_offset 236
  copy_4_pixels_at_offset 240
  copy_4_pixels_at_offset 244
  copy_4_pixels_at_offset 248
  copy_4_pixels_at_offset 252

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_copy_loop
  j      begin_copy_loop
end_copy_loop:
  ret

  .align 4
fcn_copy_128_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  copy_4_pixels_at_offset 64
  copy_4_pixels_at_offset 68
  copy_4_pixels_at_offset 72
  copy_4_pixels_at_offset 76
  copy_4_pixels_at_offset 80
  copy_4_pixels_at_offset 84
  copy_4_pixels_at_offset 88
  copy_4_pixels_at_offset 92
  copy_4_pixels_at_offset 96
  copy_4_pixels_at_offset 100
  copy_4_pixels_at_offset 104
  copy_4_pixels_at_offset 108
  copy_4_pixels_at_offset 112
  copy_4_pixels_at_offset 116
  copy_4_pixels_at_offset 120
  copy_4_pixels_at_offset 124
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_copy_128_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  copy_4_pixels_at_offset 64
  copy_4_pixels_at_offset 68
  copy_4_pixels_at_offset 72
  copy_4_pixels_at_offset 76
  copy_4_pixels_at_offset 80
  copy_4_pixels_at_offset 84
  copy_4_pixels_at_offset 88
  copy_4_pixels_at_offset 92
  copy_4_pixels_at_offset 96
  copy_4_pixels_at_offset 100
  copy_4_pixels_at_offset 104
  copy_4_pixels_at_offset 108
  copy_4_pixels_at_offset 112
  copy_4_pixels_at_offset 116
  copy_4_pixels_at_offset 120
  copy_4_pixels_at_offset 124
  ret

  .align 4
fcn_copy_64_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_copy_64_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  ret

  .align 4
fcn_copy_32_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  ret

  .align 4
fcn_copy_32_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  ret

  .align 4
fcn_copy_16_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  ret

  .align 4
fcn_copy_16_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  ret

  .align 4
fcn_copy_8_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  ret

  .align 4
fcn_copy_8_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  ret

  .align 4
fcn_skip_copy_256_pixels_in_loop:
begin_skip_copy_loop:
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_skip_copy_loop
  j      begin_skip_copy_loop
end_skip_copy_loop:
  ret

  .align 4
fcn_skip_copy_128_pixels:
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  ret

  .align 4
fcn_skip_copy_64_pixels:
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  ret

  .align 4
fcn_skip_copy_32_pixels:
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32
  ret

  .align 4
fcn_skip_copy_16_pixels:
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16
  ret

  .align 4
fcn_skip_copy_8_pixels:
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8
  ret

  .align 4
fcn_color_blend_25_for_256_pixels_in_loop:
  enter_blend_fcn
begin_color_blend_25_loop:
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12
  blend_for_4_pixels_at_offset 25 clr 16
  blend_for_4_pixels_at_offset 25 clr 20
  blend_for_4_pixels_at_offset 25 clr 24
  blend_for_4_pixels_at_offset 25 clr 28
  blend_for_4_pixels_at_offset 25 clr 32
  blend_for_4_pixels_at_offset 25 clr 36
  blend_for_4_pixels_at_offset 25 clr 40
  blend_for_4_pixels_at_offset 25 clr 44
  blend_for_4_pixels_at_offset 25 clr 48
  blend_for_4_pixels_at_offset 25 clr 52
  blend_for_4_pixels_at_offset 25 clr 56
  blend_for_4_pixels_at_offset 25 clr 60
  blend_for_4_pixels_at_offset 25 clr 64
  blend_for_4_pixels_at_offset 25 clr 68
  blend_for_4_pixels_at_offset 25 clr 72
  blend_for_4_pixels_at_offset 25 clr 76
  blend_for_4_pixels_at_offset 25 clr 80
  blend_for_4_pixels_at_offset 25 clr 84
  blend_for_4_pixels_at_offset 25 clr 88
  blend_for_4_pixels_at_offset 25 clr 92
  blend_for_4_pixels_at_offset 25 clr 96
  blend_for_4_pixels_at_offset 25 clr 100
  blend_for_4_pixels_at_offset 25 clr 104
  blend_for_4_pixels_at_offset 25 clr 108
  blend_for_4_pixels_at_offset 25 clr 112
  blend_for_4_pixels_at_offset 25 clr 116
  blend_for_4_pixels_at_offset 25 clr 120
  blend_for_4_pixels_at_offset 25 clr 124

  blend_for_4_pixels_at_offset 25 clr 128
  blend_for_4_pixels_at_offset 25 clr 132
  blend_for_4_pixels_at_offset 25 clr 136
  blend_for_4_pixels_at_offset 25 clr 140
  blend_for_4_pixels_at_offset 25 clr 144
  blend_for_4_pixels_at_offset 25 clr 148
  blend_for_4_pixels_at_offset 25 clr 152
  blend_for_4_pixels_at_offset 25 clr 156
  blend_for_4_pixels_at_offset 25 clr 160
  blend_for_4_pixels_at_offset 25 clr 164
  blend_for_4_pixels_at_offset 25 clr 168
  blend_for_4_pixels_at_offset 25 clr 172
  blend_for_4_pixels_at_offset 25 clr 176
  blend_for_4_pixels_at_offset 25 clr 180
  blend_for_4_pixels_at_offset 25 clr 184
  blend_for_4_pixels_at_offset 25 clr 188
  blend_for_4_pixels_at_offset 25 clr 192
  blend_for_4_pixels_at_offset 25 clr 196
  blend_for_4_pixels_at_offset 25 clr 200
  blend_for_4_pixels_at_offset 25 clr 204
  blend_for_4_pixels_at_offset 25 clr 208
  blend_for_4_pixels_at_offset 25 clr 212
  blend_for_4_pixels_at_offset 25 clr 216
  blend_for_4_pixels_at_offset 25 clr 220
  blend_for_4_pixels_at_offset 25 clr 224
  blend_for_4_pixels_at_offset 25 clr 228
  blend_for_4_pixels_at_offset 25 clr 232
  blend_for_4_pixels_at_offset 25 clr 236
  blend_for_4_pixels_at_offset 25 clr 240
  blend_for_4_pixels_at_offset 25 clr 244
  blend_for_4_pixels_at_offset 25 clr 248
  blend_for_4_pixels_at_offset 25 clr 252

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_color_blend_25_loop
  j      begin_color_blend_25_loop
end_color_blend_25_loop:
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_128_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12
  blend_for_4_pixels_at_offset 25 clr 16
  blend_for_4_pixels_at_offset 25 clr 20
  blend_for_4_pixels_at_offset 25 clr 24
  blend_for_4_pixels_at_offset 25 clr 28
  blend_for_4_pixels_at_offset 25 clr 32
  blend_for_4_pixels_at_offset 25 clr 36
  blend_for_4_pixels_at_offset 25 clr 40
  blend_for_4_pixels_at_offset 25 clr 44
  blend_for_4_pixels_at_offset 25 clr 48
  blend_for_4_pixels_at_offset 25 clr 52
  blend_for_4_pixels_at_offset 25 clr 56
  blend_for_4_pixels_at_offset 25 clr 60
  blend_for_4_pixels_at_offset 25 clr 64
  blend_for_4_pixels_at_offset 25 clr 68
  blend_for_4_pixels_at_offset 25 clr 72
  blend_for_4_pixels_at_offset 25 clr 76
  blend_for_4_pixels_at_offset 25 clr 80
  blend_for_4_pixels_at_offset 25 clr 84
  blend_for_4_pixels_at_offset 25 clr 88
  blend_for_4_pixels_at_offset 25 clr 92
  blend_for_4_pixels_at_offset 25 clr 96
  blend_for_4_pixels_at_offset 25 clr 100
  blend_for_4_pixels_at_offset 25 clr 104
  blend_for_4_pixels_at_offset 25 clr 108
  blend_for_4_pixels_at_offset 25 clr 112
  blend_for_4_pixels_at_offset 25 clr 116
  blend_for_4_pixels_at_offset 25 clr 120
  blend_for_4_pixels_at_offset 25 clr 124
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_128_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12
  blend_for_4_pixels_at_offset 25 clr 16
  blend_for_4_pixels_at_offset 25 clr 20
  blend_for_4_pixels_at_offset 25 clr 24
  blend_for_4_pixels_at_offset 25 clr 28
  blend_for_4_pixels_at_offset 25 clr 32
  blend_for_4_pixels_at_offset 25 clr 36
  blend_for_4_pixels_at_offset 25 clr 40
  blend_for_4_pixels_at_offset 25 clr 44
  blend_for_4_pixels_at_offset 25 clr 48
  blend_for_4_pixels_at_offset 25 clr 52
  blend_for_4_pixels_at_offset 25 clr 56
  blend_for_4_pixels_at_offset 25 clr 60
  blend_for_4_pixels_at_offset 25 clr 64
  blend_for_4_pixels_at_offset 25 clr 68
  blend_for_4_pixels_at_offset 25 clr 72
  blend_for_4_pixels_at_offset 25 clr 76
  blend_for_4_pixels_at_offset 25 clr 80
  blend_for_4_pixels_at_offset 25 clr 84
  blend_for_4_pixels_at_offset 25 clr 88
  blend_for_4_pixels_at_offset 25 clr 92
  blend_for_4_pixels_at_offset 25 clr 96
  blend_for_4_pixels_at_offset 25 clr 100
  blend_for_4_pixels_at_offset 25 clr 104
  blend_for_4_pixels_at_offset 25 clr 108
  blend_for_4_pixels_at_offset 25 clr 112
  blend_for_4_pixels_at_offset 25 clr 116
  blend_for_4_pixels_at_offset 25 clr 120
  blend_for_4_pixels_at_offset 25 clr 124
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_64_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12
  blend_for_4_pixels_at_offset 25 clr 16
  blend_for_4_pixels_at_offset 25 clr 20
  blend_for_4_pixels_at_offset 25 clr 24
  blend_for_4_pixels_at_offset 25 clr 28
  blend_for_4_pixels_at_offset 25 clr 32
  blend_for_4_pixels_at_offset 25 clr 36
  blend_for_4_pixels_at_offset 25 clr 40
  blend_for_4_pixels_at_offset 25 clr 44
  blend_for_4_pixels_at_offset 25 clr 48
  blend_for_4_pixels_at_offset 25 clr 52
  blend_for_4_pixels_at_offset 25 clr 56
  blend_for_4_pixels_at_offset 25 clr 60
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_64_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12
  blend_for_4_pixels_at_offset 25 clr 16
  blend_for_4_pixels_at_offset 25 clr 20
  blend_for_4_pixels_at_offset 25 clr 24
  blend_for_4_pixels_at_offset 25 clr 28
  blend_for_4_pixels_at_offset 25 clr 32
  blend_for_4_pixels_at_offset 25 clr 36
  blend_for_4_pixels_at_offset 25 clr 40
  blend_for_4_pixels_at_offset 25 clr 44
  blend_for_4_pixels_at_offset 25 clr 48
  blend_for_4_pixels_at_offset 25 clr 52
  blend_for_4_pixels_at_offset 25 clr 56
  blend_for_4_pixels_at_offset 25 clr 60
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_32_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12
  blend_for_4_pixels_at_offset 25 clr 16
  blend_for_4_pixels_at_offset 25 clr 20
  blend_for_4_pixels_at_offset 25 clr 24
  blend_for_4_pixels_at_offset 25 clr 28
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_32_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12
  blend_for_4_pixels_at_offset 25 clr 16
  blend_for_4_pixels_at_offset 25 clr 20
  blend_for_4_pixels_at_offset 25 clr 24
  blend_for_4_pixels_at_offset 25 clr 28
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_16_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_16_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  blend_for_4_pixels_at_offset 25 clr 8
  blend_for_4_pixels_at_offset 25 clr 12
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_8_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_8_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  blend_for_4_pixels_at_offset 25 clr 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_1_pixel_at_offset_0:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_1_pixel_at_offset_0_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_1_pixel_at_offset_1:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_1_pixel_at_offset_1_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_1_pixel_at_offset_2:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_1_pixel_at_offset_2_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_1_pixel_at_offset_3:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_1_pixel_at_offset_3_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_2_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_2_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_2_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_2_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_2_pixels_at_offset_2:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_2_pixels_at_offset_2_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_3_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_3_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_3_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 25 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_3_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 25 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_4_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_25_for_4_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 clr 0
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_256_pixels_in_loop:
  enter_blend_fcn
begin_color_blend_50_loop:
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12
  blend_for_4_pixels_at_offset 50 clr 16
  blend_for_4_pixels_at_offset 50 clr 20
  blend_for_4_pixels_at_offset 50 clr 24
  blend_for_4_pixels_at_offset 50 clr 28
  blend_for_4_pixels_at_offset 50 clr 32
  blend_for_4_pixels_at_offset 50 clr 36
  blend_for_4_pixels_at_offset 50 clr 40
  blend_for_4_pixels_at_offset 50 clr 44
  blend_for_4_pixels_at_offset 50 clr 48
  blend_for_4_pixels_at_offset 50 clr 52
  blend_for_4_pixels_at_offset 50 clr 56
  blend_for_4_pixels_at_offset 50 clr 60
  blend_for_4_pixels_at_offset 50 clr 64
  blend_for_4_pixels_at_offset 50 clr 68
  blend_for_4_pixels_at_offset 50 clr 72
  blend_for_4_pixels_at_offset 50 clr 76
  blend_for_4_pixels_at_offset 50 clr 80
  blend_for_4_pixels_at_offset 50 clr 84
  blend_for_4_pixels_at_offset 50 clr 88
  blend_for_4_pixels_at_offset 50 clr 92
  blend_for_4_pixels_at_offset 50 clr 96
  blend_for_4_pixels_at_offset 50 clr 100
  blend_for_4_pixels_at_offset 50 clr 104
  blend_for_4_pixels_at_offset 50 clr 108
  blend_for_4_pixels_at_offset 50 clr 112
  blend_for_4_pixels_at_offset 50 clr 116
  blend_for_4_pixels_at_offset 50 clr 120
  blend_for_4_pixels_at_offset 50 clr 124

  blend_for_4_pixels_at_offset 50 clr 128
  blend_for_4_pixels_at_offset 50 clr 132
  blend_for_4_pixels_at_offset 50 clr 136
  blend_for_4_pixels_at_offset 50 clr 140
  blend_for_4_pixels_at_offset 50 clr 144
  blend_for_4_pixels_at_offset 50 clr 148
  blend_for_4_pixels_at_offset 50 clr 152
  blend_for_4_pixels_at_offset 50 clr 156
  blend_for_4_pixels_at_offset 50 clr 160
  blend_for_4_pixels_at_offset 50 clr 164
  blend_for_4_pixels_at_offset 50 clr 168
  blend_for_4_pixels_at_offset 50 clr 172
  blend_for_4_pixels_at_offset 50 clr 176
  blend_for_4_pixels_at_offset 50 clr 180
  blend_for_4_pixels_at_offset 50 clr 184
  blend_for_4_pixels_at_offset 50 clr 188
  blend_for_4_pixels_at_offset 50 clr 192
  blend_for_4_pixels_at_offset 50 clr 196
  blend_for_4_pixels_at_offset 50 clr 200
  blend_for_4_pixels_at_offset 50 clr 204
  blend_for_4_pixels_at_offset 50 clr 208
  blend_for_4_pixels_at_offset 50 clr 212
  blend_for_4_pixels_at_offset 50 clr 216
  blend_for_4_pixels_at_offset 50 clr 220
  blend_for_4_pixels_at_offset 50 clr 224
  blend_for_4_pixels_at_offset 50 clr 228
  blend_for_4_pixels_at_offset 50 clr 232
  blend_for_4_pixels_at_offset 50 clr 236
  blend_for_4_pixels_at_offset 50 clr 240
  blend_for_4_pixels_at_offset 50 clr 244
  blend_for_4_pixels_at_offset 50 clr 248
  blend_for_4_pixels_at_offset 50 clr 252

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_color_blend_50_loop
  j      begin_color_blend_50_loop
end_color_blend_50_loop:
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_128_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12
  blend_for_4_pixels_at_offset 50 clr 16
  blend_for_4_pixels_at_offset 50 clr 20
  blend_for_4_pixels_at_offset 50 clr 24
  blend_for_4_pixels_at_offset 50 clr 28
  blend_for_4_pixels_at_offset 50 clr 32
  blend_for_4_pixels_at_offset 50 clr 36
  blend_for_4_pixels_at_offset 50 clr 40
  blend_for_4_pixels_at_offset 50 clr 44
  blend_for_4_pixels_at_offset 50 clr 48
  blend_for_4_pixels_at_offset 50 clr 52
  blend_for_4_pixels_at_offset 50 clr 56
  blend_for_4_pixels_at_offset 50 clr 60
  blend_for_4_pixels_at_offset 50 clr 64
  blend_for_4_pixels_at_offset 50 clr 68
  blend_for_4_pixels_at_offset 50 clr 72
  blend_for_4_pixels_at_offset 50 clr 76
  blend_for_4_pixels_at_offset 50 clr 80
  blend_for_4_pixels_at_offset 50 clr 84
  blend_for_4_pixels_at_offset 50 clr 88
  blend_for_4_pixels_at_offset 50 clr 92
  blend_for_4_pixels_at_offset 50 clr 96
  blend_for_4_pixels_at_offset 50 clr 100
  blend_for_4_pixels_at_offset 50 clr 104
  blend_for_4_pixels_at_offset 50 clr 108
  blend_for_4_pixels_at_offset 50 clr 112
  blend_for_4_pixels_at_offset 50 clr 116
  blend_for_4_pixels_at_offset 50 clr 120
  blend_for_4_pixels_at_offset 50 clr 124
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_128_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12
  blend_for_4_pixels_at_offset 50 clr 16
  blend_for_4_pixels_at_offset 50 clr 20
  blend_for_4_pixels_at_offset 50 clr 24
  blend_for_4_pixels_at_offset 50 clr 28
  blend_for_4_pixels_at_offset 50 clr 32
  blend_for_4_pixels_at_offset 50 clr 36
  blend_for_4_pixels_at_offset 50 clr 40
  blend_for_4_pixels_at_offset 50 clr 44
  blend_for_4_pixels_at_offset 50 clr 48
  blend_for_4_pixels_at_offset 50 clr 52
  blend_for_4_pixels_at_offset 50 clr 56
  blend_for_4_pixels_at_offset 50 clr 60
  blend_for_4_pixels_at_offset 50 clr 64
  blend_for_4_pixels_at_offset 50 clr 68
  blend_for_4_pixels_at_offset 50 clr 72
  blend_for_4_pixels_at_offset 50 clr 76
  blend_for_4_pixels_at_offset 50 clr 80
  blend_for_4_pixels_at_offset 50 clr 84
  blend_for_4_pixels_at_offset 50 clr 88
  blend_for_4_pixels_at_offset 50 clr 92
  blend_for_4_pixels_at_offset 50 clr 96
  blend_for_4_pixels_at_offset 50 clr 100
  blend_for_4_pixels_at_offset 50 clr 104
  blend_for_4_pixels_at_offset 50 clr 108
  blend_for_4_pixels_at_offset 50 clr 112
  blend_for_4_pixels_at_offset 50 clr 116
  blend_for_4_pixels_at_offset 50 clr 120
  blend_for_4_pixels_at_offset 50 clr 124
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_64_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12
  blend_for_4_pixels_at_offset 50 clr 16
  blend_for_4_pixels_at_offset 50 clr 20
  blend_for_4_pixels_at_offset 50 clr 24
  blend_for_4_pixels_at_offset 50 clr 28
  blend_for_4_pixels_at_offset 50 clr 32
  blend_for_4_pixels_at_offset 50 clr 36
  blend_for_4_pixels_at_offset 50 clr 40
  blend_for_4_pixels_at_offset 50 clr 44
  blend_for_4_pixels_at_offset 50 clr 48
  blend_for_4_pixels_at_offset 50 clr 52
  blend_for_4_pixels_at_offset 50 clr 56
  blend_for_4_pixels_at_offset 50 clr 60
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_64_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12
  blend_for_4_pixels_at_offset 50 clr 16
  blend_for_4_pixels_at_offset 50 clr 20
  blend_for_4_pixels_at_offset 50 clr 24
  blend_for_4_pixels_at_offset 50 clr 28
  blend_for_4_pixels_at_offset 50 clr 32
  blend_for_4_pixels_at_offset 50 clr 36
  blend_for_4_pixels_at_offset 50 clr 40
  blend_for_4_pixels_at_offset 50 clr 44
  blend_for_4_pixels_at_offset 50 clr 48
  blend_for_4_pixels_at_offset 50 clr 52
  blend_for_4_pixels_at_offset 50 clr 56
  blend_for_4_pixels_at_offset 50 clr 60
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_32_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12
  blend_for_4_pixels_at_offset 50 clr 16
  blend_for_4_pixels_at_offset 50 clr 20
  blend_for_4_pixels_at_offset 50 clr 24
  blend_for_4_pixels_at_offset 50 clr 28
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_32_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12
  blend_for_4_pixels_at_offset 50 clr 16
  blend_for_4_pixels_at_offset 50 clr 20
  blend_for_4_pixels_at_offset 50 clr 24
  blend_for_4_pixels_at_offset 50 clr 28
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_16_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_16_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  blend_for_4_pixels_at_offset 50 clr 8
  blend_for_4_pixels_at_offset 50 clr 12
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_8_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_8_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  blend_for_4_pixels_at_offset 50 clr 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_1_pixel_at_offset_0:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_1_pixel_at_offset_0_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_1_pixel_at_offset_1:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_1_pixel_at_offset_1_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_1_pixel_at_offset_2:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_1_pixel_at_offset_2_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_1_pixel_at_offset_3:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_1_pixel_at_offset_3_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_2_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_2_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_2_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_2_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_2_pixels_at_offset_2:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_2_pixels_at_offset_2_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_3_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_3_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_3_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 50 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_3_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 50 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_4_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_50_for_4_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 clr 0
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_256_pixels_in_loop:
  enter_blend_fcn
begin_color_blend_75_loop:
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12
  blend_for_4_pixels_at_offset 75 clr 16
  blend_for_4_pixels_at_offset 75 clr 20
  blend_for_4_pixels_at_offset 75 clr 24
  blend_for_4_pixels_at_offset 75 clr 28
  blend_for_4_pixels_at_offset 75 clr 32
  blend_for_4_pixels_at_offset 75 clr 36
  blend_for_4_pixels_at_offset 75 clr 40
  blend_for_4_pixels_at_offset 75 clr 44
  blend_for_4_pixels_at_offset 75 clr 48
  blend_for_4_pixels_at_offset 75 clr 52
  blend_for_4_pixels_at_offset 75 clr 56
  blend_for_4_pixels_at_offset 75 clr 60
  blend_for_4_pixels_at_offset 75 clr 64
  blend_for_4_pixels_at_offset 75 clr 68
  blend_for_4_pixels_at_offset 75 clr 72
  blend_for_4_pixels_at_offset 75 clr 76
  blend_for_4_pixels_at_offset 75 clr 80
  blend_for_4_pixels_at_offset 75 clr 84
  blend_for_4_pixels_at_offset 75 clr 88
  blend_for_4_pixels_at_offset 75 clr 92
  blend_for_4_pixels_at_offset 75 clr 96
  blend_for_4_pixels_at_offset 75 clr 100
  blend_for_4_pixels_at_offset 75 clr 104
  blend_for_4_pixels_at_offset 75 clr 108
  blend_for_4_pixels_at_offset 75 clr 112
  blend_for_4_pixels_at_offset 75 clr 116
  blend_for_4_pixels_at_offset 75 clr 120
  blend_for_4_pixels_at_offset 75 clr 124

  blend_for_4_pixels_at_offset 75 clr 128
  blend_for_4_pixels_at_offset 75 clr 132
  blend_for_4_pixels_at_offset 75 clr 136
  blend_for_4_pixels_at_offset 75 clr 140
  blend_for_4_pixels_at_offset 75 clr 144
  blend_for_4_pixels_at_offset 75 clr 148
  blend_for_4_pixels_at_offset 75 clr 152
  blend_for_4_pixels_at_offset 75 clr 156
  blend_for_4_pixels_at_offset 75 clr 160
  blend_for_4_pixels_at_offset 75 clr 164
  blend_for_4_pixels_at_offset 75 clr 168
  blend_for_4_pixels_at_offset 75 clr 172
  blend_for_4_pixels_at_offset 75 clr 176
  blend_for_4_pixels_at_offset 75 clr 180
  blend_for_4_pixels_at_offset 75 clr 184
  blend_for_4_pixels_at_offset 75 clr 188
  blend_for_4_pixels_at_offset 75 clr 192
  blend_for_4_pixels_at_offset 75 clr 196
  blend_for_4_pixels_at_offset 75 clr 200
  blend_for_4_pixels_at_offset 75 clr 204
  blend_for_4_pixels_at_offset 75 clr 208
  blend_for_4_pixels_at_offset 75 clr 212
  blend_for_4_pixels_at_offset 75 clr 216
  blend_for_4_pixels_at_offset 75 clr 220
  blend_for_4_pixels_at_offset 75 clr 224
  blend_for_4_pixels_at_offset 75 clr 228
  blend_for_4_pixels_at_offset 75 clr 232
  blend_for_4_pixels_at_offset 75 clr 236
  blend_for_4_pixels_at_offset 75 clr 240
  blend_for_4_pixels_at_offset 75 clr 244
  blend_for_4_pixels_at_offset 75 clr 248
  blend_for_4_pixels_at_offset 75 clr 252

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_color_blend_75_loop
  j      begin_color_blend_75_loop
end_color_blend_75_loop:
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_128_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12
  blend_for_4_pixels_at_offset 75 clr 16
  blend_for_4_pixels_at_offset 75 clr 20
  blend_for_4_pixels_at_offset 75 clr 24
  blend_for_4_pixels_at_offset 75 clr 28
  blend_for_4_pixels_at_offset 75 clr 32
  blend_for_4_pixels_at_offset 75 clr 36
  blend_for_4_pixels_at_offset 75 clr 40
  blend_for_4_pixels_at_offset 75 clr 44
  blend_for_4_pixels_at_offset 75 clr 48
  blend_for_4_pixels_at_offset 75 clr 52
  blend_for_4_pixels_at_offset 75 clr 56
  blend_for_4_pixels_at_offset 75 clr 60
  blend_for_4_pixels_at_offset 75 clr 64
  blend_for_4_pixels_at_offset 75 clr 68
  blend_for_4_pixels_at_offset 75 clr 72
  blend_for_4_pixels_at_offset 75 clr 76
  blend_for_4_pixels_at_offset 75 clr 80
  blend_for_4_pixels_at_offset 75 clr 84
  blend_for_4_pixels_at_offset 75 clr 88
  blend_for_4_pixels_at_offset 75 clr 92
  blend_for_4_pixels_at_offset 75 clr 96
  blend_for_4_pixels_at_offset 75 clr 100
  blend_for_4_pixels_at_offset 75 clr 104
  blend_for_4_pixels_at_offset 75 clr 108
  blend_for_4_pixels_at_offset 75 clr 112
  blend_for_4_pixels_at_offset 75 clr 116
  blend_for_4_pixels_at_offset 75 clr 120
  blend_for_4_pixels_at_offset 75 clr 124
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_128_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12
  blend_for_4_pixels_at_offset 75 clr 16
  blend_for_4_pixels_at_offset 75 clr 20
  blend_for_4_pixels_at_offset 75 clr 24
  blend_for_4_pixels_at_offset 75 clr 28
  blend_for_4_pixels_at_offset 75 clr 32
  blend_for_4_pixels_at_offset 75 clr 36
  blend_for_4_pixels_at_offset 75 clr 40
  blend_for_4_pixels_at_offset 75 clr 44
  blend_for_4_pixels_at_offset 75 clr 48
  blend_for_4_pixels_at_offset 75 clr 52
  blend_for_4_pixels_at_offset 75 clr 56
  blend_for_4_pixels_at_offset 75 clr 60
  blend_for_4_pixels_at_offset 75 clr 64
  blend_for_4_pixels_at_offset 75 clr 68
  blend_for_4_pixels_at_offset 75 clr 72
  blend_for_4_pixels_at_offset 75 clr 76
  blend_for_4_pixels_at_offset 75 clr 80
  blend_for_4_pixels_at_offset 75 clr 84
  blend_for_4_pixels_at_offset 75 clr 88
  blend_for_4_pixels_at_offset 75 clr 92
  blend_for_4_pixels_at_offset 75 clr 96
  blend_for_4_pixels_at_offset 75 clr 100
  blend_for_4_pixels_at_offset 75 clr 104
  blend_for_4_pixels_at_offset 75 clr 108
  blend_for_4_pixels_at_offset 75 clr 112
  blend_for_4_pixels_at_offset 75 clr 116
  blend_for_4_pixels_at_offset 75 clr 120
  blend_for_4_pixels_at_offset 75 clr 124
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_64_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12
  blend_for_4_pixels_at_offset 75 clr 16
  blend_for_4_pixels_at_offset 75 clr 20
  blend_for_4_pixels_at_offset 75 clr 24
  blend_for_4_pixels_at_offset 75 clr 28
  blend_for_4_pixels_at_offset 75 clr 32
  blend_for_4_pixels_at_offset 75 clr 36
  blend_for_4_pixels_at_offset 75 clr 40
  blend_for_4_pixels_at_offset 75 clr 44
  blend_for_4_pixels_at_offset 75 clr 48
  blend_for_4_pixels_at_offset 75 clr 52
  blend_for_4_pixels_at_offset 75 clr 56
  blend_for_4_pixels_at_offset 75 clr 60
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_64_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12
  blend_for_4_pixels_at_offset 75 clr 16
  blend_for_4_pixels_at_offset 75 clr 20
  blend_for_4_pixels_at_offset 75 clr 24
  blend_for_4_pixels_at_offset 75 clr 28
  blend_for_4_pixels_at_offset 75 clr 32
  blend_for_4_pixels_at_offset 75 clr 36
  blend_for_4_pixels_at_offset 75 clr 40
  blend_for_4_pixels_at_offset 75 clr 44
  blend_for_4_pixels_at_offset 75 clr 48
  blend_for_4_pixels_at_offset 75 clr 52
  blend_for_4_pixels_at_offset 75 clr 56
  blend_for_4_pixels_at_offset 75 clr 60
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_32_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12
  blend_for_4_pixels_at_offset 75 clr 16
  blend_for_4_pixels_at_offset 75 clr 20
  blend_for_4_pixels_at_offset 75 clr 24
  blend_for_4_pixels_at_offset 75 clr 28
  
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_32_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12
  blend_for_4_pixels_at_offset 75 clr 16
  blend_for_4_pixels_at_offset 75 clr 20
  blend_for_4_pixels_at_offset 75 clr 24
  blend_for_4_pixels_at_offset 75 clr 28
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_16_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_16_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  blend_for_4_pixels_at_offset 75 clr 8
  blend_for_4_pixels_at_offset 75 clr 12
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_8_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_8_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  blend_for_4_pixels_at_offset 75 clr 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_1_pixel_at_offset_0:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_1_pixel_at_offset_0_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_1_pixel_at_offset_1:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_1_pixel_at_offset_1_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_1_pixel_at_offset_2:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_1_pixel_at_offset_2_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_1_pixel_at_offset_3:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_1_pixel_at_offset_3_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_2_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_2_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_2_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_2_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_2_pixels_at_offset_2:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_2_pixels_at_offset_2_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_3_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_3_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_3_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 75 clr
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_3_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 75 clr
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_4_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_color_blend_75_for_4_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 clr 0
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_256_pixels_in_loop:
  enter_blend_fcn
begin_src_blend_25_loop:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12
  blend_for_4_pixels_at_offset 25 src 16
  blend_for_4_pixels_at_offset 25 src 20
  blend_for_4_pixels_at_offset 25 src 24
  blend_for_4_pixels_at_offset 25 src 28
  blend_for_4_pixels_at_offset 25 src 32
  blend_for_4_pixels_at_offset 25 src 36
  blend_for_4_pixels_at_offset 25 src 40
  blend_for_4_pixels_at_offset 25 src 44
  blend_for_4_pixels_at_offset 25 src 48
  blend_for_4_pixels_at_offset 25 src 52
  blend_for_4_pixels_at_offset 25 src 56
  blend_for_4_pixels_at_offset 25 src 60
  blend_for_4_pixels_at_offset 25 src 64
  blend_for_4_pixels_at_offset 25 src 68
  blend_for_4_pixels_at_offset 25 src 72
  blend_for_4_pixels_at_offset 25 src 76
  blend_for_4_pixels_at_offset 25 src 80
  blend_for_4_pixels_at_offset 25 src 84
  blend_for_4_pixels_at_offset 25 src 88
  blend_for_4_pixels_at_offset 25 src 92
  blend_for_4_pixels_at_offset 25 src 96
  blend_for_4_pixels_at_offset 25 src 100
  blend_for_4_pixels_at_offset 25 src 104
  blend_for_4_pixels_at_offset 25 src 108
  blend_for_4_pixels_at_offset 25 src 112
  blend_for_4_pixels_at_offset 25 src 116
  blend_for_4_pixels_at_offset 25 src 120
  blend_for_4_pixels_at_offset 25 src 124

  blend_for_4_pixels_at_offset 25 src 128
  blend_for_4_pixels_at_offset 25 src 132
  blend_for_4_pixels_at_offset 25 src 136
  blend_for_4_pixels_at_offset 25 src 140
  blend_for_4_pixels_at_offset 25 src 144
  blend_for_4_pixels_at_offset 25 src 148
  blend_for_4_pixels_at_offset 25 src 152
  blend_for_4_pixels_at_offset 25 src 156
  blend_for_4_pixels_at_offset 25 src 160
  blend_for_4_pixels_at_offset 25 src 164
  blend_for_4_pixels_at_offset 25 src 168
  blend_for_4_pixels_at_offset 25 src 172
  blend_for_4_pixels_at_offset 25 src 176
  blend_for_4_pixels_at_offset 25 src 180
  blend_for_4_pixels_at_offset 25 src 184
  blend_for_4_pixels_at_offset 25 src 188
  blend_for_4_pixels_at_offset 25 src 192
  blend_for_4_pixels_at_offset 25 src 196
  blend_for_4_pixels_at_offset 25 src 200
  blend_for_4_pixels_at_offset 25 src 204
  blend_for_4_pixels_at_offset 25 src 208
  blend_for_4_pixels_at_offset 25 src 212
  blend_for_4_pixels_at_offset 25 src 216
  blend_for_4_pixels_at_offset 25 src 220
  blend_for_4_pixels_at_offset 25 src 224
  blend_for_4_pixels_at_offset 25 src 228
  blend_for_4_pixels_at_offset 25 src 232
  blend_for_4_pixels_at_offset 25 src 236
  blend_for_4_pixels_at_offset 25 src 240
  blend_for_4_pixels_at_offset 25 src 244
  blend_for_4_pixels_at_offset 25 src 248
  blend_for_4_pixels_at_offset 25 src 252

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_src_blend_25_loop
  j      begin_src_blend_25_loop
end_src_blend_25_loop:
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_128_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12
  blend_for_4_pixels_at_offset 25 src 16
  blend_for_4_pixels_at_offset 25 src 20
  blend_for_4_pixels_at_offset 25 src 24
  blend_for_4_pixels_at_offset 25 src 28
  blend_for_4_pixels_at_offset 25 src 32
  blend_for_4_pixels_at_offset 25 src 36
  blend_for_4_pixels_at_offset 25 src 40
  blend_for_4_pixels_at_offset 25 src 44
  blend_for_4_pixels_at_offset 25 src 48
  blend_for_4_pixels_at_offset 25 src 52
  blend_for_4_pixels_at_offset 25 src 56
  blend_for_4_pixels_at_offset 25 src 60
  blend_for_4_pixels_at_offset 25 src 64
  blend_for_4_pixels_at_offset 25 src 68
  blend_for_4_pixels_at_offset 25 src 72
  blend_for_4_pixels_at_offset 25 src 76
  blend_for_4_pixels_at_offset 25 src 80
  blend_for_4_pixels_at_offset 25 src 84
  blend_for_4_pixels_at_offset 25 src 88
  blend_for_4_pixels_at_offset 25 src 92
  blend_for_4_pixels_at_offset 25 src 96
  blend_for_4_pixels_at_offset 25 src 100
  blend_for_4_pixels_at_offset 25 src 104
  blend_for_4_pixels_at_offset 25 src 108
  blend_for_4_pixels_at_offset 25 src 112
  blend_for_4_pixels_at_offset 25 src 116
  blend_for_4_pixels_at_offset 25 src 120
  blend_for_4_pixels_at_offset 25 src 124
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_128_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12
  blend_for_4_pixels_at_offset 25 src 16
  blend_for_4_pixels_at_offset 25 src 20
  blend_for_4_pixels_at_offset 25 src 24
  blend_for_4_pixels_at_offset 25 src 28
  blend_for_4_pixels_at_offset 25 src 32
  blend_for_4_pixels_at_offset 25 src 36
  blend_for_4_pixels_at_offset 25 src 40
  blend_for_4_pixels_at_offset 25 src 44
  blend_for_4_pixels_at_offset 25 src 48
  blend_for_4_pixels_at_offset 25 src 52
  blend_for_4_pixels_at_offset 25 src 56
  blend_for_4_pixels_at_offset 25 src 60
  blend_for_4_pixels_at_offset 25 src 64
  blend_for_4_pixels_at_offset 25 src 68
  blend_for_4_pixels_at_offset 25 src 72
  blend_for_4_pixels_at_offset 25 src 76
  blend_for_4_pixels_at_offset 25 src 80
  blend_for_4_pixels_at_offset 25 src 84
  blend_for_4_pixels_at_offset 25 src 88
  blend_for_4_pixels_at_offset 25 src 92
  blend_for_4_pixels_at_offset 25 src 96
  blend_for_4_pixels_at_offset 25 src 100
  blend_for_4_pixels_at_offset 25 src 104
  blend_for_4_pixels_at_offset 25 src 108
  blend_for_4_pixels_at_offset 25 src 112
  blend_for_4_pixels_at_offset 25 src 116
  blend_for_4_pixels_at_offset 25 src 120
  blend_for_4_pixels_at_offset 25 src 124
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_64_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12
  blend_for_4_pixels_at_offset 25 src 16
  blend_for_4_pixels_at_offset 25 src 20
  blend_for_4_pixels_at_offset 25 src 24
  blend_for_4_pixels_at_offset 25 src 28
  blend_for_4_pixels_at_offset 25 src 32
  blend_for_4_pixels_at_offset 25 src 36
  blend_for_4_pixels_at_offset 25 src 40
  blend_for_4_pixels_at_offset 25 src 44
  blend_for_4_pixels_at_offset 25 src 48
  blend_for_4_pixels_at_offset 25 src 52
  blend_for_4_pixels_at_offset 25 src 56
  blend_for_4_pixels_at_offset 25 src 60
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_64_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12
  blend_for_4_pixels_at_offset 25 src 16
  blend_for_4_pixels_at_offset 25 src 20
  blend_for_4_pixels_at_offset 25 src 24
  blend_for_4_pixels_at_offset 25 src 28
  blend_for_4_pixels_at_offset 25 src 32
  blend_for_4_pixels_at_offset 25 src 36
  blend_for_4_pixels_at_offset 25 src 40
  blend_for_4_pixels_at_offset 25 src 44
  blend_for_4_pixels_at_offset 25 src 48
  blend_for_4_pixels_at_offset 25 src 52
  blend_for_4_pixels_at_offset 25 src 56
  blend_for_4_pixels_at_offset 25 src 60
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_32_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12
  blend_for_4_pixels_at_offset 25 src 16
  blend_for_4_pixels_at_offset 25 src 20
  blend_for_4_pixels_at_offset 25 src 24
  blend_for_4_pixels_at_offset 25 src 28
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_32_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12
  blend_for_4_pixels_at_offset 25 src 16
  blend_for_4_pixels_at_offset 25 src 20
  blend_for_4_pixels_at_offset 25 src 24
  blend_for_4_pixels_at_offset 25 src 28
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_16_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_16_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  blend_for_4_pixels_at_offset 25 src 8
  blend_for_4_pixels_at_offset 25 src 12
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_8_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_8_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  blend_for_4_pixels_at_offset 25 src 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_1_pixel_at_offset_0:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_1_pixel_at_offset_0_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_1_pixel_at_offset_1:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_1_pixel_at_offset_1_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_1_pixel_at_offset_2:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_1_pixel_at_offset_2_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_1_pixel_at_offset_3:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_1_pixel_at_offset_3_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_2_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_2_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_2_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_2_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_2_pixels_at_offset_2:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_2_pixels_at_offset_2_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_3_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_3_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_3_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 25 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_3_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 25 src
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_4_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_25_for_4_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 25 src 0
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_256_pixels_in_loop:
  enter_blend_fcn
begin_src_blend_50_loop:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12
  blend_for_4_pixels_at_offset 50 src 16
  blend_for_4_pixels_at_offset 50 src 20
  blend_for_4_pixels_at_offset 50 src 24
  blend_for_4_pixels_at_offset 50 src 28
  blend_for_4_pixels_at_offset 50 src 32
  blend_for_4_pixels_at_offset 50 src 36
  blend_for_4_pixels_at_offset 50 src 40
  blend_for_4_pixels_at_offset 50 src 44
  blend_for_4_pixels_at_offset 50 src 48
  blend_for_4_pixels_at_offset 50 src 52
  blend_for_4_pixels_at_offset 50 src 56
  blend_for_4_pixels_at_offset 50 src 60
  blend_for_4_pixels_at_offset 50 src 64
  blend_for_4_pixels_at_offset 50 src 68
  blend_for_4_pixels_at_offset 50 src 72
  blend_for_4_pixels_at_offset 50 src 76
  blend_for_4_pixels_at_offset 50 src 80
  blend_for_4_pixels_at_offset 50 src 84
  blend_for_4_pixels_at_offset 50 src 88
  blend_for_4_pixels_at_offset 50 src 92
  blend_for_4_pixels_at_offset 50 src 96
  blend_for_4_pixels_at_offset 50 src 100
  blend_for_4_pixels_at_offset 50 src 104
  blend_for_4_pixels_at_offset 50 src 108
  blend_for_4_pixels_at_offset 50 src 112
  blend_for_4_pixels_at_offset 50 src 116
  blend_for_4_pixels_at_offset 50 src 120
  blend_for_4_pixels_at_offset 50 src 124

  blend_for_4_pixels_at_offset 50 src 128
  blend_for_4_pixels_at_offset 50 src 132
  blend_for_4_pixels_at_offset 50 src 136
  blend_for_4_pixels_at_offset 50 src 140
  blend_for_4_pixels_at_offset 50 src 144
  blend_for_4_pixels_at_offset 50 src 148
  blend_for_4_pixels_at_offset 50 src 152
  blend_for_4_pixels_at_offset 50 src 156
  blend_for_4_pixels_at_offset 50 src 160
  blend_for_4_pixels_at_offset 50 src 164
  blend_for_4_pixels_at_offset 50 src 168
  blend_for_4_pixels_at_offset 50 src 172
  blend_for_4_pixels_at_offset 50 src 176
  blend_for_4_pixels_at_offset 50 src 180
  blend_for_4_pixels_at_offset 50 src 184
  blend_for_4_pixels_at_offset 50 src 188
  blend_for_4_pixels_at_offset 50 src 192
  blend_for_4_pixels_at_offset 50 src 196
  blend_for_4_pixels_at_offset 50 src 200
  blend_for_4_pixels_at_offset 50 src 204
  blend_for_4_pixels_at_offset 50 src 208
  blend_for_4_pixels_at_offset 50 src 212
  blend_for_4_pixels_at_offset 50 src 216
  blend_for_4_pixels_at_offset 50 src 220
  blend_for_4_pixels_at_offset 50 src 224
  blend_for_4_pixels_at_offset 50 src 228
  blend_for_4_pixels_at_offset 50 src 232
  blend_for_4_pixels_at_offset 50 src 236
  blend_for_4_pixels_at_offset 50 src 240
  blend_for_4_pixels_at_offset 50 src 244
  blend_for_4_pixels_at_offset 50 src 248
  blend_for_4_pixels_at_offset 50 src 252

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_src_blend_50_loop
  j      begin_src_blend_50_loop
end_src_blend_50_loop:
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_128_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12
  blend_for_4_pixels_at_offset 50 src 16
  blend_for_4_pixels_at_offset 50 src 20
  blend_for_4_pixels_at_offset 50 src 24
  blend_for_4_pixels_at_offset 50 src 28
  blend_for_4_pixels_at_offset 50 src 32
  blend_for_4_pixels_at_offset 50 src 36
  blend_for_4_pixels_at_offset 50 src 40
  blend_for_4_pixels_at_offset 50 src 44
  blend_for_4_pixels_at_offset 50 src 48
  blend_for_4_pixels_at_offset 50 src 52
  blend_for_4_pixels_at_offset 50 src 56
  blend_for_4_pixels_at_offset 50 src 60
  blend_for_4_pixels_at_offset 50 src 64
  blend_for_4_pixels_at_offset 50 src 68
  blend_for_4_pixels_at_offset 50 src 72
  blend_for_4_pixels_at_offset 50 src 76
  blend_for_4_pixels_at_offset 50 src 80
  blend_for_4_pixels_at_offset 50 src 84
  blend_for_4_pixels_at_offset 50 src 88
  blend_for_4_pixels_at_offset 50 src 92
  blend_for_4_pixels_at_offset 50 src 96
  blend_for_4_pixels_at_offset 50 src 100
  blend_for_4_pixels_at_offset 50 src 104
  blend_for_4_pixels_at_offset 50 src 108
  blend_for_4_pixels_at_offset 50 src 112
  blend_for_4_pixels_at_offset 50 src 116
  blend_for_4_pixels_at_offset 50 src 120
  blend_for_4_pixels_at_offset 50 src 124
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_128_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12
  blend_for_4_pixels_at_offset 50 src 16
  blend_for_4_pixels_at_offset 50 src 20
  blend_for_4_pixels_at_offset 50 src 24
  blend_for_4_pixels_at_offset 50 src 28
  blend_for_4_pixels_at_offset 50 src 32
  blend_for_4_pixels_at_offset 50 src 36
  blend_for_4_pixels_at_offset 50 src 40
  blend_for_4_pixels_at_offset 50 src 44
  blend_for_4_pixels_at_offset 50 src 48
  blend_for_4_pixels_at_offset 50 src 52
  blend_for_4_pixels_at_offset 50 src 56
  blend_for_4_pixels_at_offset 50 src 60
  blend_for_4_pixels_at_offset 50 src 64
  blend_for_4_pixels_at_offset 50 src 68
  blend_for_4_pixels_at_offset 50 src 72
  blend_for_4_pixels_at_offset 50 src 76
  blend_for_4_pixels_at_offset 50 src 80
  blend_for_4_pixels_at_offset 50 src 84
  blend_for_4_pixels_at_offset 50 src 88
  blend_for_4_pixels_at_offset 50 src 92
  blend_for_4_pixels_at_offset 50 src 96
  blend_for_4_pixels_at_offset 50 src 100
  blend_for_4_pixels_at_offset 50 src 104
  blend_for_4_pixels_at_offset 50 src 108
  blend_for_4_pixels_at_offset 50 src 112
  blend_for_4_pixels_at_offset 50 src 116
  blend_for_4_pixels_at_offset 50 src 120
  blend_for_4_pixels_at_offset 50 src 124
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_64_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12
  blend_for_4_pixels_at_offset 50 src 16
  blend_for_4_pixels_at_offset 50 src 20
  blend_for_4_pixels_at_offset 50 src 24
  blend_for_4_pixels_at_offset 50 src 28
  blend_for_4_pixels_at_offset 50 src 32
  blend_for_4_pixels_at_offset 50 src 36
  blend_for_4_pixels_at_offset 50 src 40
  blend_for_4_pixels_at_offset 50 src 44
  blend_for_4_pixels_at_offset 50 src 48
  blend_for_4_pixels_at_offset 50 src 52
  blend_for_4_pixels_at_offset 50 src 56
  blend_for_4_pixels_at_offset 50 src 60
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_64_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12
  blend_for_4_pixels_at_offset 50 src 16
  blend_for_4_pixels_at_offset 50 src 20
  blend_for_4_pixels_at_offset 50 src 24
  blend_for_4_pixels_at_offset 50 src 28
  blend_for_4_pixels_at_offset 50 src 32
  blend_for_4_pixels_at_offset 50 src 36
  blend_for_4_pixels_at_offset 50 src 40
  blend_for_4_pixels_at_offset 50 src 44
  blend_for_4_pixels_at_offset 50 src 48
  blend_for_4_pixels_at_offset 50 src 52
  blend_for_4_pixels_at_offset 50 src 56
  blend_for_4_pixels_at_offset 50 src 60
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_32_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12
  blend_for_4_pixels_at_offset 50 src 16
  blend_for_4_pixels_at_offset 50 src 20
  blend_for_4_pixels_at_offset 50 src 24
  blend_for_4_pixels_at_offset 50 src 28
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_32_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12
  blend_for_4_pixels_at_offset 50 src 16
  blend_for_4_pixels_at_offset 50 src 20
  blend_for_4_pixels_at_offset 50 src 24
  blend_for_4_pixels_at_offset 50 src 28
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_16_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_16_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  blend_for_4_pixels_at_offset 50 src 8
  blend_for_4_pixels_at_offset 50 src 12
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_8_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_8_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  blend_for_4_pixels_at_offset 50 src 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_1_pixel_at_offset_0:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_1_pixel_at_offset_0_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_1_pixel_at_offset_1:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_1_pixel_at_offset_1_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_1_pixel_at_offset_2:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_1_pixel_at_offset_2_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_1_pixel_at_offset_3:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_1_pixel_at_offset_3_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_2_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_2_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_2_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_2_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_2_pixels_at_offset_2:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_2_pixels_at_offset_2_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_3_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_3_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_3_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 50 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_3_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 50 src
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_4_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_50_for_4_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 50 src 0
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_256_pixels_in_loop:
  enter_blend_fcn
begin_src_blend_75_loop:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12
  blend_for_4_pixels_at_offset 75 src 16
  blend_for_4_pixels_at_offset 75 src 20
  blend_for_4_pixels_at_offset 75 src 24
  blend_for_4_pixels_at_offset 75 src 28
  blend_for_4_pixels_at_offset 75 src 32
  blend_for_4_pixels_at_offset 75 src 36
  blend_for_4_pixels_at_offset 75 src 40
  blend_for_4_pixels_at_offset 75 src 44
  blend_for_4_pixels_at_offset 75 src 48
  blend_for_4_pixels_at_offset 75 src 52
  blend_for_4_pixels_at_offset 75 src 56
  blend_for_4_pixels_at_offset 75 src 60
  blend_for_4_pixels_at_offset 75 src 64
  blend_for_4_pixels_at_offset 75 src 68
  blend_for_4_pixels_at_offset 75 src 72
  blend_for_4_pixels_at_offset 75 src 76
  blend_for_4_pixels_at_offset 75 src 80
  blend_for_4_pixels_at_offset 75 src 84
  blend_for_4_pixels_at_offset 75 src 88
  blend_for_4_pixels_at_offset 75 src 92
  blend_for_4_pixels_at_offset 75 src 96
  blend_for_4_pixels_at_offset 75 src 100
  blend_for_4_pixels_at_offset 75 src 104
  blend_for_4_pixels_at_offset 75 src 108
  blend_for_4_pixels_at_offset 75 src 112
  blend_for_4_pixels_at_offset 75 src 116
  blend_for_4_pixels_at_offset 75 src 120
  blend_for_4_pixels_at_offset 75 src 124

  blend_for_4_pixels_at_offset 75 src 128
  blend_for_4_pixels_at_offset 75 src 132
  blend_for_4_pixels_at_offset 75 src 136
  blend_for_4_pixels_at_offset 75 src 140
  blend_for_4_pixels_at_offset 75 src 144
  blend_for_4_pixels_at_offset 75 src 148
  blend_for_4_pixels_at_offset 75 src 152
  blend_for_4_pixels_at_offset 75 src 156
  blend_for_4_pixels_at_offset 75 src 160
  blend_for_4_pixels_at_offset 75 src 164
  blend_for_4_pixels_at_offset 75 src 168
  blend_for_4_pixels_at_offset 75 src 172
  blend_for_4_pixels_at_offset 75 src 176
  blend_for_4_pixels_at_offset 75 src 180
  blend_for_4_pixels_at_offset 75 src 184
  blend_for_4_pixels_at_offset 75 src 188
  blend_for_4_pixels_at_offset 75 src 192
  blend_for_4_pixels_at_offset 75 src 196
  blend_for_4_pixels_at_offset 75 src 200
  blend_for_4_pixels_at_offset 75 src 204
  blend_for_4_pixels_at_offset 75 src 208
  blend_for_4_pixels_at_offset 75 src 212
  blend_for_4_pixels_at_offset 75 src 216
  blend_for_4_pixels_at_offset 75 src 220
  blend_for_4_pixels_at_offset 75 src 224
  blend_for_4_pixels_at_offset 75 src 228
  blend_for_4_pixels_at_offset 75 src 232
  blend_for_4_pixels_at_offset 75 src 236
  blend_for_4_pixels_at_offset 75 src 240
  blend_for_4_pixels_at_offset 75 src 244
  blend_for_4_pixels_at_offset 75 src 248
  blend_for_4_pixels_at_offset 75 src 252

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  beqz   REG_LOOP_INDEX, end_src_blend_75_loop
  j      begin_src_blend_75_loop
end_src_blend_75_loop:
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_128_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12
  blend_for_4_pixels_at_offset 75 src 16
  blend_for_4_pixels_at_offset 75 src 20
  blend_for_4_pixels_at_offset 75 src 24
  blend_for_4_pixels_at_offset 75 src 28
  blend_for_4_pixels_at_offset 75 src 32
  blend_for_4_pixels_at_offset 75 src 36
  blend_for_4_pixels_at_offset 75 src 40
  blend_for_4_pixels_at_offset 75 src 44
  blend_for_4_pixels_at_offset 75 src 48
  blend_for_4_pixels_at_offset 75 src 52
  blend_for_4_pixels_at_offset 75 src 56
  blend_for_4_pixels_at_offset 75 src 60
  blend_for_4_pixels_at_offset 75 src 64
  blend_for_4_pixels_at_offset 75 src 68
  blend_for_4_pixels_at_offset 75 src 72
  blend_for_4_pixels_at_offset 75 src 76
  blend_for_4_pixels_at_offset 75 src 80
  blend_for_4_pixels_at_offset 75 src 84
  blend_for_4_pixels_at_offset 75 src 88
  blend_for_4_pixels_at_offset 75 src 92
  blend_for_4_pixels_at_offset 75 src 96
  blend_for_4_pixels_at_offset 75 src 100
  blend_for_4_pixels_at_offset 75 src 104
  blend_for_4_pixels_at_offset 75 src 108
  blend_for_4_pixels_at_offset 75 src 112
  blend_for_4_pixels_at_offset 75 src 116
  blend_for_4_pixels_at_offset 75 src 120
  blend_for_4_pixels_at_offset 75 src 124
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_128_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12
  blend_for_4_pixels_at_offset 75 src 16
  blend_for_4_pixels_at_offset 75 src 20
  blend_for_4_pixels_at_offset 75 src 24
  blend_for_4_pixels_at_offset 75 src 28
  blend_for_4_pixels_at_offset 75 src 32
  blend_for_4_pixels_at_offset 75 src 36
  blend_for_4_pixels_at_offset 75 src 40
  blend_for_4_pixels_at_offset 75 src 44
  blend_for_4_pixels_at_offset 75 src 48
  blend_for_4_pixels_at_offset 75 src 52
  blend_for_4_pixels_at_offset 75 src 56
  blend_for_4_pixels_at_offset 75 src 60
  blend_for_4_pixels_at_offset 75 src 64
  blend_for_4_pixels_at_offset 75 src 68
  blend_for_4_pixels_at_offset 75 src 72
  blend_for_4_pixels_at_offset 75 src 76
  blend_for_4_pixels_at_offset 75 src 80
  blend_for_4_pixels_at_offset 75 src 84
  blend_for_4_pixels_at_offset 75 src 88
  blend_for_4_pixels_at_offset 75 src 92
  blend_for_4_pixels_at_offset 75 src 96
  blend_for_4_pixels_at_offset 75 src 100
  blend_for_4_pixels_at_offset 75 src 104
  blend_for_4_pixels_at_offset 75 src 108
  blend_for_4_pixels_at_offset 75 src 112
  blend_for_4_pixels_at_offset 75 src 116
  blend_for_4_pixels_at_offset 75 src 120
  blend_for_4_pixels_at_offset 75 src 124
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_64_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12
  blend_for_4_pixels_at_offset 75 src 16
  blend_for_4_pixels_at_offset 75 src 20
  blend_for_4_pixels_at_offset 75 src 24
  blend_for_4_pixels_at_offset 75 src 28
  blend_for_4_pixels_at_offset 75 src 32
  blend_for_4_pixels_at_offset 75 src 36
  blend_for_4_pixels_at_offset 75 src 40
  blend_for_4_pixels_at_offset 75 src 44
  blend_for_4_pixels_at_offset 75 src 48
  blend_for_4_pixels_at_offset 75 src 52
  blend_for_4_pixels_at_offset 75 src 56
  blend_for_4_pixels_at_offset 75 src 60
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_64_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12
  blend_for_4_pixels_at_offset 75 src 16
  blend_for_4_pixels_at_offset 75 src 20
  blend_for_4_pixels_at_offset 75 src 24
  blend_for_4_pixels_at_offset 75 src 28
  blend_for_4_pixels_at_offset 75 src 32
  blend_for_4_pixels_at_offset 75 src 36
  blend_for_4_pixels_at_offset 75 src 40
  blend_for_4_pixels_at_offset 75 src 44
  blend_for_4_pixels_at_offset 75 src 48
  blend_for_4_pixels_at_offset 75 src 52
  blend_for_4_pixels_at_offset 75 src 56
  blend_for_4_pixels_at_offset 75 src 60
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_32_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12
  blend_for_4_pixels_at_offset 75 src 16
  blend_for_4_pixels_at_offset 75 src 20
  blend_for_4_pixels_at_offset 75 src 24
  blend_for_4_pixels_at_offset 75 src 28
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_32_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12
  blend_for_4_pixels_at_offset 75 src 16
  blend_for_4_pixels_at_offset 75 src 20
  blend_for_4_pixels_at_offset 75 src 24
  blend_for_4_pixels_at_offset 75 src 28
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_16_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_16_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  blend_for_4_pixels_at_offset 75 src 8
  blend_for_4_pixels_at_offset 75 src 12
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_8_pixels:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_8_pixels_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  blend_for_4_pixels_at_offset 75 src 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_1_pixel_at_offset_0:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_1_pixel_at_offset_0_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_0 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_1_pixel_at_offset_1:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_1_pixel_at_offset_1_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_1 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_1_pixel_at_offset_2:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_1_pixel_at_offset_2_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_2 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_1_pixel_at_offset_3:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_1_pixel_at_offset_3_last:
  enter_blend_fcn
  blend_for_1_pixel_at_offset_3 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_2_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_2_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_0 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_2_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_2_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_1 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_2_pixels_at_offset_2:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_2_pixels_at_offset_2_last:
  enter_blend_fcn
  blend_for_2_pixels_at_offset_2 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_3_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_3_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_0 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_3_pixels_at_offset_1:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 75 src
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_3_pixels_at_offset_1_last:
  enter_blend_fcn
  blend_for_3_pixels_at_offset_1 75 src
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_4_pixels_at_offset_0:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  leave_blend_fcn

  .align 4
fcn_src_blend_75_for_4_pixels_at_offset_0_last:
  enter_blend_fcn
  blend_for_4_pixels_at_offset 75 src 0
  leave_blend_fcn

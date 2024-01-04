// di_code.cpp - Supports creating ESP32 functions dynamically.
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

#include "di_code.h"
#include "di_constants.h"
#include "di_primitive_const.h"
#include "../agon.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
#include <vector>

#define EXTRA_CODE_SIZE 8

#define OUTER_RET_ADDR_IN_STACK   (4)
#define INNER_RET_ADDR_IN_STACK   (8)

#define FIX_OFFSET(off)    ((off)^2)

#define MASK_ISOLATE_BR    0x33333333 // mask to isolate blue & red, removing green
#define MASK_ISOLATE_G     0x0C0C0C0C // mask to isolate green, removing red & blue

extern uint32_t fcn_draw_256_pixels_in_loop;
extern uint32_t fcn_draw_128_pixels;
extern uint32_t fcn_draw_128_pixels_last;
extern uint32_t fcn_draw_64_pixels;
extern uint32_t fcn_draw_64_pixels_last;
extern uint32_t fcn_draw_32_pixels;
extern uint32_t fcn_draw_32_pixels_last;
extern uint32_t fcn_draw_16_pixels;
extern uint32_t fcn_draw_16_pixels_last;
extern uint32_t fcn_draw_8_pixels;
extern uint32_t fcn_draw_8_pixels_last;
extern uint32_t fcn_get_blend_25_for_4_pixels;
extern uint32_t fcn_get_blend_50_for_4_pixels;
extern uint32_t fcn_get_blend_75_for_4_pixels;
extern uint32_t fcn_dummy;

extern uint32_t fcn_skip_draw_256_pixels_in_loop;
extern uint32_t fcn_skip_draw_128_pixels;
extern uint32_t fcn_skip_draw_64_pixels;
extern uint32_t fcn_skip_draw_32_pixels;
extern uint32_t fcn_skip_draw_16_pixels;
extern uint32_t fcn_skip_draw_8_pixels;

extern uint32_t fcn_copy_256_pixels_in_loop;
extern uint32_t fcn_copy_128_pixels;
extern uint32_t fcn_copy_128_pixels_last;
extern uint32_t fcn_copy_64_pixels;
extern uint32_t fcn_copy_64_pixels_last;
extern uint32_t fcn_copy_32_pixels;
extern uint32_t fcn_copy_32_pixels_last;
extern uint32_t fcn_copy_16_pixels;
extern uint32_t fcn_copy_16_pixels_last;
extern uint32_t fcn_copy_8_pixels;
extern uint32_t fcn_copy_8_pixels_last;

extern uint32_t fcn_skip_copy_256_pixels_in_loop;
extern uint32_t fcn_skip_copy_128_pixels;
extern uint32_t fcn_skip_copy_64_pixels;
extern uint32_t fcn_skip_copy_32_pixels;
extern uint32_t fcn_skip_copy_16_pixels;
extern uint32_t fcn_skip_copy_8_pixels;

extern uint32_t fcn_color_blend_25_for_256_pixels_in_loop;
extern uint32_t fcn_color_blend_25_for_128_pixels;
extern uint32_t fcn_color_blend_25_for_128_pixels_last;
extern uint32_t fcn_color_blend_25_for_64_pixels;
extern uint32_t fcn_color_blend_25_for_64_pixels_last;
extern uint32_t fcn_color_blend_25_for_32_pixels;
extern uint32_t fcn_color_blend_25_for_32_pixels_last;
extern uint32_t fcn_color_blend_25_for_16_pixels;
extern uint32_t fcn_color_blend_25_for_16_pixels_last;
extern uint32_t fcn_color_blend_25_for_8_pixels;
extern uint32_t fcn_color_blend_25_for_8_pixels_last;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_0;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_1;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_2;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_3;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_0;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_1;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_2;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_color_blend_25_for_3_pixels_at_offset_0;
extern uint32_t fcn_color_blend_25_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_25_for_3_pixels_at_offset_1;
extern uint32_t fcn_color_blend_25_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_25_for_4_pixels_at_offset_0;
extern uint32_t fcn_color_blend_25_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_color_blend_50_for_256_pixels_in_loop;
extern uint32_t fcn_color_blend_50_for_128_pixels;
extern uint32_t fcn_color_blend_50_for_128_pixels_last;
extern uint32_t fcn_color_blend_50_for_64_pixels;
extern uint32_t fcn_color_blend_50_for_64_pixels_last;
extern uint32_t fcn_color_blend_50_for_32_pixels;
extern uint32_t fcn_color_blend_50_for_32_pixels_last;
extern uint32_t fcn_color_blend_50_for_16_pixels;
extern uint32_t fcn_color_blend_50_for_16_pixels_last;
extern uint32_t fcn_color_blend_50_for_8_pixels;
extern uint32_t fcn_color_blend_50_for_8_pixels_last;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_0;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_1;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_2;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_3;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_0;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_1;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_2;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_color_blend_50_for_3_pixels_at_offset_0;
extern uint32_t fcn_color_blend_50_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_50_for_3_pixels_at_offset_1;
extern uint32_t fcn_color_blend_50_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_50_for_4_pixels_at_offset_0;
extern uint32_t fcn_color_blend_50_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_color_blend_75_for_256_pixels_in_loop;
extern uint32_t fcn_color_blend_75_for_128_pixels;
extern uint32_t fcn_color_blend_75_for_128_pixels_last;
extern uint32_t fcn_color_blend_75_for_64_pixels;
extern uint32_t fcn_color_blend_75_for_64_pixels_last;
extern uint32_t fcn_color_blend_75_for_32_pixels;
extern uint32_t fcn_color_blend_75_for_32_pixels_last;
extern uint32_t fcn_color_blend_75_for_16_pixels;
extern uint32_t fcn_color_blend_75_for_16_pixels_last;
extern uint32_t fcn_color_blend_75_for_8_pixels;
extern uint32_t fcn_color_blend_75_for_8_pixels_last;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_0;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_1;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_2;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_3;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_0;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_1;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_2;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_color_blend_75_for_3_pixels_at_offset_0;
extern uint32_t fcn_color_blend_75_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_75_for_3_pixels_at_offset_1;
extern uint32_t fcn_color_blend_75_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_75_for_4_pixels_at_offset_0;
extern uint32_t fcn_color_blend_75_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_src_blend_25_for_256_pixels_in_loop;
extern uint32_t fcn_src_blend_25_for_128_pixels;
extern uint32_t fcn_src_blend_25_for_128_pixels_last;
extern uint32_t fcn_src_blend_25_for_64_pixels;
extern uint32_t fcn_src_blend_25_for_64_pixels_last;
extern uint32_t fcn_src_blend_25_for_32_pixels;
extern uint32_t fcn_src_blend_25_for_32_pixels_last;
extern uint32_t fcn_src_blend_25_for_16_pixels;
extern uint32_t fcn_src_blend_25_for_16_pixels_last;
extern uint32_t fcn_src_blend_25_for_8_pixels;
extern uint32_t fcn_src_blend_25_for_8_pixels_last;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_0;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_1;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_2;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_3;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_0;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_1;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_2;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_src_blend_25_for_3_pixels_at_offset_0;
extern uint32_t fcn_src_blend_25_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_25_for_3_pixels_at_offset_1;
extern uint32_t fcn_src_blend_25_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_25_for_4_pixels_at_offset_0;
extern uint32_t fcn_src_blend_25_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_src_blend_50_for_256_pixels_in_loop;
extern uint32_t fcn_src_blend_50_for_128_pixels;
extern uint32_t fcn_src_blend_50_for_128_pixels_last;
extern uint32_t fcn_src_blend_50_for_64_pixels;
extern uint32_t fcn_src_blend_50_for_64_pixels_last;
extern uint32_t fcn_src_blend_50_for_32_pixels;
extern uint32_t fcn_src_blend_50_for_32_pixels_last;
extern uint32_t fcn_src_blend_50_for_16_pixels;
extern uint32_t fcn_src_blend_50_for_16_pixels_last;
extern uint32_t fcn_src_blend_50_for_8_pixels;
extern uint32_t fcn_src_blend_50_for_8_pixels_last;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_0;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_1;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_2;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_3;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_0;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_1;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_2;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_src_blend_50_for_3_pixels_at_offset_0;
extern uint32_t fcn_src_blend_50_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_50_for_3_pixels_at_offset_1;
extern uint32_t fcn_src_blend_50_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_50_for_4_pixels_at_offset_0;
extern uint32_t fcn_src_blend_50_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_src_blend_75_for_256_pixels_in_loop;
extern uint32_t fcn_src_blend_75_for_128_pixels;
extern uint32_t fcn_src_blend_75_for_128_pixels_last;
extern uint32_t fcn_src_blend_75_for_64_pixels;
extern uint32_t fcn_src_blend_75_for_64_pixels_last;
extern uint32_t fcn_src_blend_75_for_32_pixels;
extern uint32_t fcn_src_blend_75_for_32_pixels_last;
extern uint32_t fcn_src_blend_75_for_16_pixels;
extern uint32_t fcn_src_blend_75_for_16_pixels_last;
extern uint32_t fcn_src_blend_75_for_8_pixels;
extern uint32_t fcn_src_blend_75_for_8_pixels_last;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_0;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_1;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_2;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_3;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_0;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_1;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_2;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_src_blend_75_for_3_pixels_at_offset_0;
extern uint32_t fcn_src_blend_75_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_75_for_3_pixels_at_offset_1;
extern uint32_t fcn_src_blend_75_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_75_for_4_pixels_at_offset_0;
extern uint32_t fcn_src_blend_75_for_4_pixels_at_offset_0_last;

EspFunction::EspFunction() {
    init_members();
}

EspFunction::~EspFunction() {
    if (m_code) {
        heap_caps_free(m_code);
    }
}

void EspFunction::init_members() {
    m_alloc_size = 0;
    m_code_size = 0;
    m_code_index = 0;
    m_code = 0;
}

void EspFunction::draw_line(EspFixups& fixups, uint32_t x_offset,
                            uint32_t skip, uint32_t draw_width,
                            const DiLineSections* sections,
                            uint16_t flags, uint8_t opaqueness, bool outer_fcn) {
    auto at_jump = (outer_fcn ? enter_outer_function() : enter_inner_function());
    auto at_data = begin_data();

    uint32_t at_isolate_br = 0;
    uint32_t at_isolate_g = 0;
    if (opaqueness != 100) {
        at_isolate_br = d32(MASK_ISOLATE_BR); // mask to isolate blue & red, removing green
        at_isolate_g = d32(MASK_ISOLATE_G); // mask to isolate green, removing red & blue
    }

    begin_code(at_jump);
    set_reg_dst_pixel_ptr_for_draw(flags);

    if (opaqueness != 100) {
        l32r_from(REG_ISOLATE_BR, at_isolate_br);
        l32r_from(REG_ISOLATE_G, at_isolate_g);
    }

    l32i(REG_PIXEL_COLOR, REG_THIS_PTR, FLD_color);
    s32i(REG_RETURN_ADDR, REG_STACK_PTR, (outer_fcn ? OUTER_RET_ADDR_IN_STACK : INNER_RET_ADDR_IN_STACK));

    if (opaqueness != 100) {
        mov(REG_SAVE_COLOR, REG_PIXEL_COLOR);
    }

    draw_line_loop(fixups, x_offset, skip, draw_width, sections, flags, opaqueness);

    l32i(REG_RETURN_ADDR, REG_STACK_PTR, (outer_fcn ? OUTER_RET_ADDR_IN_STACK : INNER_RET_ADDR_IN_STACK));

    if (outer_fcn) retw(); else ret();
}

void EspFunction::adjust_dst_pixel_ptr(uint32_t draw_x, uint32_t x) {
    auto start_x = draw_x & 0xFFFFFFFC;
    auto end_x = x & 0xFFFFFFFC;
    while (end_x > start_x) {
        uint32_t diff = end_x - start_x;
        if (diff < 4) {
            break;
        }
        if (diff >= 120) {
            addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120);
            start_x += 120;
        } else if (diff >= 64) {
            addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64);
            start_x += 64;
        } else if (diff >= 32) {
            addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32);
            start_x += 32;
        } else if (diff >= 16) {
            addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16);
            start_x += 16;
        } else if (diff >= 8) {
            addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8);
            start_x += 8;
        } else {
            addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
            start_x += 4;
        }
    }
}

void EspFunction::draw_line_loop(EspFixups& fixups, uint32_t x_offset,
    uint32_t skip, uint32_t draw_width,
    const DiLineSections* sections, uint16_t flags, uint8_t opaqueness) {

    auto num_sections = (uint32_t)sections->m_pieces.size();
    auto x = x_offset;

    for (uint16_t si = 0; (si < num_sections) && draw_width;) {
        if (skip && (skip >= draw_width)) {
            break;
        }

        auto next_x = sections->m_pieces[si].m_x;
        uint16_t gap = 0;
        if (next_x > x) {
            gap = next_x - x;
            if (gap >= draw_width) {
                break;
            }
            if (skip) {
                if (gap < skip) {
                    skip -= gap;
                } else {
                    skip = 0;
                }
            }
            cover_width(fixups, x_offset, gap, 0, false, true);
            x += gap;
            draw_width -= gap;
        }

        auto width = sections->m_pieces[si].m_width;
        if (x > next_x) {
            width -= x - next_x;
        }
        width = MIN(width, draw_width);
        if (skip >= width) {
            auto skip_now = MIN(skip, width);
            cover_width(fixups, x_offset, skip_now, 0, false, (si + 1 < num_sections));
            skip -= skip_now;            
        } else {
            auto width_now = width;
            if (skip) {
                cover_width(fixups, x_offset, skip, 0, false, true);
                width_now -= skip;
                skip = 0;
            }
            cover_width(fixups, x_offset, width_now, opaqueness, false, (si + 1 < num_sections));            
        }
        x += width;
        draw_width -= width;
        si++;
    }
}

void EspFunction::cover_width(EspFixups& fixups, uint32_t& x_offset, u_int32_t width, uint8_t opaqueness, bool copy, bool more) {
    uint32_t p_fcn = 0;
    while (width) {
        auto offset = x_offset & 3;
        uint32_t sub = 1;
        switch (offset) {
            case 0:
                if (width >= 4) {
                    if (width >= 256) {
                        // Need at least 64 full words
                        auto times = width / 256;
                        p_fcn = cover_256(fixups, times, opaqueness, copy);
                        sub = times * 256;
                    } else if (width >= 128) {
                        // Need at least 32 full words
                        p_fcn = cover_128(fixups, width, opaqueness, copy, more);
                        sub = 128;
                    } else if (width >= 64) {
                        // Need at least 16 full words
                        p_fcn = cover_64(fixups, width, opaqueness, copy, more);
                        sub = 64;
                    } else if (width >= 32) {
                        // Need at least 8 full words
                        p_fcn = cover_32(fixups, width, opaqueness, copy, more);
                        sub = 32;
                    } else if (width >= 16) {
                        // Need at least 4 full words
                        p_fcn = cover_16(fixups, width, opaqueness, copy, more);
                        sub = 16;
                    } else if (width >= 8) {
                        // Need at least 2 full words
                        p_fcn = cover_8(fixups, width, opaqueness, copy, more);
                        sub = 8;
                    } else {
                        // Need at least 1 full word
                        p_fcn = cover_4(fixups, width, opaqueness, copy, more);
                        sub = 4;
                    }
                } else if (width == 3) {
                    p_fcn = cover_3_at_0(fixups, width, opaqueness, copy);
                    //if (more) {
                    //    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    //}
                    sub = 3;
                } else if (width == 2) {
                    p_fcn = cover_2_at_0(fixups, width, opaqueness, copy);
                    //if (more) {
                    //    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    //}
                    sub = 2;
                } else { // width == 1
                    p_fcn = cover_1_at_0(fixups, width, opaqueness, copy);
                    //if (more) {
                    //    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    //}
                }
                break;

            case 1:
                if (width >= 3) {
                    p_fcn = cover_3_at_1(fixups, width, opaqueness, copy, more);
                    sub = 3;                
                } else if (width == 2) {
                    p_fcn = cover_2_at_1(fixups, width, opaqueness, copy);
                    //if (more) {
                    //    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    //}
                    sub = 2;
                } else { // width == 1
                    p_fcn = cover_1_at_1(fixups, width, opaqueness, copy);
                    //if (more) {
                    //    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    //}
                }
                break;

            case 2:
                if (width >= 2) {
                    p_fcn = cover_2_at_2(fixups, width, opaqueness, copy, more);
                    sub = 2;
                } else { // width == 1
                    p_fcn = cover_1_at_2(fixups, width, opaqueness, copy);
                    //if (more) {
                    //    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    //}
                }
                break;
            
            case 3:
                p_fcn = cover_1_at_3(fixups, width, opaqueness, copy, more);
                break;
        }
        width -= sub;
        x_offset += sub;
        if (p_fcn) {
            fixups.push_back(EspFixup { get_code_index(), p_fcn });
            call0(0);
            p_fcn = 0;
        }
    }
}

void EspFunction::copy_line(EspFixups& fixups, uint32_t x_offset,
        uint32_t skip, uint32_t width,
        uint16_t flags, uint8_t transparent_color,
        uint32_t* src_pixels, bool outer_fcn) {
    auto at_jump = (outer_fcn ? enter_outer_function() : enter_inner_function());
    auto at_data = begin_data();

    uint32_t at_isolate_br = 0;
    uint32_t at_isolate_g = 0;
    if (flags & PRIM_FLAGS_BLENDED) {
        at_isolate_br = d32(MASK_ISOLATE_BR); // mask to isolate blue & red, removing green
        at_isolate_g = d32(MASK_ISOLATE_G); // mask to isolate green, removing red & blue
    }

    begin_code(at_jump);
    set_reg_dst_pixel_ptr_for_copy(flags);

    if (flags & PRIM_FLAGS_BLENDED) {
        l32r_from(REG_ISOLATE_BR, at_isolate_br);
        l32r_from(REG_ISOLATE_G, at_isolate_g);
    }

    s32i(REG_RETURN_ADDR, REG_STACK_PTR, (outer_fcn ? OUTER_RET_ADDR_IN_STACK : INNER_RET_ADDR_IN_STACK));

    copy_line_loop(fixups, x_offset, skip, width, flags, transparent_color, src_pixels);

    l32i(REG_RETURN_ADDR, REG_STACK_PTR, (outer_fcn ? OUTER_RET_ADDR_IN_STACK : INNER_RET_ADDR_IN_STACK));

    if (outer_fcn) retw(); else ret();
}

void EspFunction::copy_line_loop(EspFixups& fixups, uint32_t x_offset,
        uint32_t skip, uint32_t draw_width,
        uint16_t flags, uint8_t transparent_color, uint32_t* src_pixels) {

    auto x = x_offset;

    uint8_t* p_src_bytes = (uint8_t*) src_pixels;

    while (draw_width) {

        uint8_t opaqueness = 100;
        uint32_t width;
        if (skip) {
            width = MIN(skip, draw_width);
            opaqueness = 0;
            skip = 0;
        } else if (!(flags & (PRIM_FLAGS_MASKED|PRIM_FLAGS_BLENDED))) {
            // Transfer all pixels at 100% opaqueness.
            width = draw_width;            
        } else {
            // Determine the width of adjacent, similarly transparent (or opaque) pixels in the line.
            // The colors do not have to be equal.
            width = 1;
            uint32_t index = FIX_OFFSET(x_offset);
            uint8_t src_color = p_src_bytes[index];
            uint8_t first_alpha = ((flags & PRIM_FLAGS_MASKED) && (src_color == transparent_color)) ? 0xFF : (src_color & 0xC0);
            for (uint32_t i = 1; i < draw_width; i++) {
                index = FIX_OFFSET(x_offset + i);
                src_color = p_src_bytes[index];
                uint8_t next_alpha = ((flags && PRIM_FLAGS_MASKED) && (src_color == transparent_color)) ? 0xFF : (src_color & 0xC0);
                if (next_alpha == first_alpha) {
                    width++;
                } else {
                    break;
                }
            }

            // This tests using inverted alpha masks.
            switch (first_alpha) {
                case PIXEL_ALPHA_INV_25_MASK: opaqueness = 25; break;
                case PIXEL_ALPHA_INV_50_MASK: opaqueness = 50; break;
                case PIXEL_ALPHA_INV_75_MASK: opaqueness = 75; break;
                case PIXEL_ALPHA_INV_100_MASK: opaqueness = 100; break;
                default: opaqueness = 0; break;
            }
        }
        draw_width -= width;
        // Use the series of pixels, rather than the rest of the line, if necessary.
        cover_width(fixups, x_offset, width, opaqueness, true, (draw_width > 0));
    }
}

void EspFunction::do_fixups(EspFixups& fixups) {
    uint32_t save_pc = get_code_index();
    for (auto fixup = fixups.begin();
        fixup != fixups.end();
        ++fixup) {
        set_code_index(fixup->code_index);
        call_inner_fcn(fixup->fcn_address);
    }
    set_code_index(save_pc);
}

void EspFunction::enter_and_leave_outer_function() {
    entry(REG_STACK_PTR, 32);
    retw();
}

uint32_t EspFunction::enter_outer_function() {
    entry(REG_STACK_PTR, 32);
    auto at_jump = get_code_index();
    j(0);
    return at_jump;
}

uint32_t EspFunction::enter_inner_function() {
    auto at_jump = get_code_index();
    j(0);
    return at_jump;
}

uint32_t EspFunction::begin_data() {
    align32();
    return get_code_index();
}

uint32_t EspFunction::init_jump_table(uint32_t num_items) {
    /* 00 */ entry(sp, 32);
    /* 03 */ l32i(REG_ABS_Y, REG_THIS_PTR, FLD_abs_y);
    /* 06 */ s32i(REG_RETURN_ADDR, REG_STACK_PTR, OUTER_RET_ADDR_IN_STACK);
    /* 09 */ call0(0); // 8 + 0*4 + 4
    /* 12 */ sub(REG_LINE_INDEX, REG_LINE_INDEX, REG_ABS_Y);
    /* 15 */ slli(REG_JUMP_ADDRESS, REG_LINE_INDEX, 2);
    /* 18 */ addi(REG_JUMP_ADDRESS, REG_JUMP_ADDRESS, 24);
    /* 21 */ add(REG_JUMP_ADDRESS, REG_JUMP_ADDRESS, REG_RETURN_ADDR);
    /* 24 */ l32i(REG_PIXEL_COLOR, REG_THIS_PTR, FLD_color);
    /* 27 */ callx0(REG_JUMP_ADDRESS);
    /* 30 */ l32i(REG_RETURN_ADDR, REG_STACK_PTR, OUTER_RET_ADDR_IN_STACK);
    /* 33 */ retw();
    /* 36 */ auto at_jump_table = get_code_index();
    for (uint32_t i = 0; i < num_items; i++) {
        /* 36+i*4 */ ret(); // will be changed to j(?) later
        /* 39+i*4 */ align32();
    }
    return at_jump_table;
}

void EspFunction::begin_code(uint32_t at_jump) {
    align32();
    j_to_here(at_jump);
}

void EspFunction::set_reg_dst_pixel_ptr_for_draw(uint16_t flags) {
    if (!(flags & PRIM_FLAGS_X)) {
        l32i(REG_DST_PIXEL_PTR, REG_THIS_PTR, FLD_draw_x);
    }
    srli(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 2);
    slli(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 2);
    add(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, REG_LINE_PTR);
}

void EspFunction::set_reg_dst_pixel_ptr_for_copy(uint16_t flags) {
    if (!(flags & PRIM_FLAGS_X_SRC)) {
        l32i(REG_DST_PIXEL_PTR, REG_THIS_PTR, FLD_draw_x);
    }
    srli(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 2);
    slli(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 2);
    add(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, REG_LINE_PTR);
}

void EspFunction::call_inner_fcn(uint32_t real_address) {
    uint32_t offset = (real_address - 4 - (get_real_address(get_code_index() & 0xFFFFFFFC))) & 0xFFFFF;
    call0(offset);
}

void EspFunction::store(uint8_t instr_byte) {
    auto i = m_code_index >> 2;
    switch (m_code_index & 3) {
        case 0:
            m_code[i] = (m_code[i] & 0xFFFFFF00) | (uint32_t)instr_byte;
            break;
        case 1:
            m_code[i] = (m_code[i] & 0xFFFF00FF) | ((uint32_t)instr_byte) << 8;
            break;
        case 2:
            m_code[i] = (m_code[i] & 0xFF00FFFF) | ((uint32_t)instr_byte) << 16;
            break;
        case 3:
            m_code[i] = (m_code[i] & 0x00FFFFFF) | ((uint32_t)instr_byte) << 24;
            break;
    }

    if (++m_code_index > m_code_size) {
        m_code_size = m_code_index;
    }
}

void EspFunction::align16() {
    if (m_code_index & 1) {
        d8(0);
    }
}

void EspFunction::align32() {
    align16();
    if (m_code_index & 2) {
        d16(0);
    }
}

void EspFunction::j_to_here(uint32_t from) {
    uint32_t save_pc = get_code_index();
    set_code_index(from);
    j(save_pc - from - 4);
    set_code_index(save_pc);
}

void EspFunction::bgez_to_here(reg_t src, s_off_t from) {
    auto save_pc = get_code_index();
    set_code_index(from);
    beqz(src, save_pc - from - 4);
    set_code_index(save_pc);
}


void EspFunction::l32r_from(reg_t reg, uint32_t from) {
    l32r(reg, from - ((get_code_index() + 3) & 0xFFFFFFFC));
}

void EspFunction::loop_to_here(reg_t reg, uint32_t from) {
    auto save_pc = get_code_index();
    set_code_index(from);
    loop(reg, save_pc - from - 4);
    set_code_index(save_pc);
}

uint16_t EspFunction::dup8_to_16(uint8_t value) {
    return (((uint16_t)value) << 8) | ((uint16_t)value);
}

uint32_t EspFunction::dup8_to_32(uint8_t value) {
    return (dup8_to_16(value) << 16) | dup8_to_16(value);
}

uint32_t EspFunction::dup16_to_32(uint16_t value) {
    return (((uint32_t)value) << 16) | ((uint32_t)value);
}

void EspFunction::allocate(uint32_t size) {
    if (m_alloc_size) {
        if (m_alloc_size - m_code_index < size) {
            size_t new_size = (size_t)(m_alloc_size + size + EXTRA_CODE_SIZE + 3) &0xFFFFFFFC;
            void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_EXEC);
            memcpy(p, m_code, (m_code_size + 3) &0xFFFFFFFC);
            heap_caps_free(m_code);
            m_alloc_size = (uint32_t)new_size;
            m_code = (uint32_t*)p;
        }
    } else {
        size_t new_size = (size_t)(size + EXTRA_CODE_SIZE + 3) &0xFFFFFFFC;
        void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_EXEC);
        m_alloc_size = (uint32_t)new_size;
        m_code = (uint32_t*)p;
    }
}

uint32_t EspFunction::write8(const char* mnemonic, instr_t data) {
    allocate(1);
    auto at_data = get_code_index();
    store((uint8_t)(data & 0xFF));
    return at_data;
}

uint32_t EspFunction::write16(const char* mnemonic, instr_t data) {
    allocate(2);
    auto at_data = get_code_index();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    return at_data;
}

uint32_t EspFunction::write24(const char* mnemonic, instr_t data) {
    allocate(3);
    auto at_data = get_code_index();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    store((uint8_t)((data >> 16) & 0xFF));
    return at_data;
}

uint32_t EspFunction::write32(const char* mnemonic, instr_t data) {
    allocate(4);
    auto at_data = get_code_index();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    store((uint8_t)((data >> 16) & 0xFF));
    store((uint8_t)((data >> 24) & 0xFF));
    return at_data;
}

instr_t EspFunction::isieo(uint32_t instr, reg_t src, int32_t imm, u_off_t offset) {
    if (imm == -1) imm = 0;
    else if (imm == 10) imm = 9;
    else if (imm == 12) imm = 10;
    else if (imm == 16) imm = 11;
    else if (imm == 32) imm = 12;
    else if (imm == 64) imm = 13;
    else if (imm == 128) imm = 14;
    else if (imm == 256) imm = 15;
    return instr | (offset << 16) | (imm << 12) | (src << 8);
}

uint32_t EspFunction::cover_256(EspFixups& fixups, uint32_t times, uint8_t opaqueness, bool copy) {
    movi(REG_LOOP_INDEX, times);

    if (copy) {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_src_blend_25_for_256_pixels_in_loop; break;
            case 50: return (uint32_t) &fcn_src_blend_50_for_256_pixels_in_loop; break;
            case 75: return  (uint32_t) &fcn_src_blend_75_for_256_pixels_in_loop; break;
            case 100: return (uint32_t) &fcn_copy_256_pixels_in_loop; break;
            default: return (uint32_t) &fcn_skip_copy_256_pixels_in_loop; break;
        }
    } else {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_color_blend_25_for_256_pixels_in_loop; break;
            case 50: return (uint32_t) &fcn_color_blend_50_for_256_pixels_in_loop; break;
            case 75: return  (uint32_t) &fcn_color_blend_75_for_256_pixels_in_loop; break;
            case 100: return (uint32_t) &fcn_draw_256_pixels_in_loop; break;
            default: return (uint32_t) &fcn_skip_draw_256_pixels_in_loop; break;
        }
    }
}

uint32_t EspFunction::cover_128(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 128 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_128_pixels; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_128_pixels; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_128_pixels; break;
                case 100: return (uint32_t) &fcn_copy_128_pixels; break;
                default: return (uint32_t) &fcn_skip_copy_128_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_128_pixels_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_128_pixels_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_128_pixels_last; break;
                case 100: return (uint32_t) &fcn_copy_128_pixels_last; break;
            }
        }
    } else {
        if (width > 128 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_128_pixels; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_128_pixels; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_128_pixels; break;
                case 100: return (uint32_t) &fcn_draw_128_pixels; break;
                default: return (uint32_t) &fcn_skip_draw_128_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_128_pixels_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_128_pixels_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_128_pixels_last; break;
                case 100: return (uint32_t) &fcn_draw_128_pixels_last; break;
            }
        }
    }
    return 0;
}

uint32_t EspFunction::cover_64(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 64 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_64_pixels; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_64_pixels; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_64_pixels; break;
                case 100: return (uint32_t) &fcn_copy_64_pixels; break;
                default: return (uint32_t) &fcn_skip_copy_64_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_64_pixels_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_64_pixels_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_64_pixels_last; break;
                case 100: return (uint32_t) &fcn_copy_64_pixels_last; break;
            }
        }
    } else {
        if (width > 64 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_64_pixels; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_64_pixels; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_64_pixels; break;
                case 100: return (uint32_t) &fcn_draw_64_pixels; break;
                default: return (uint32_t) &fcn_skip_draw_64_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_64_pixels_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_64_pixels_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_64_pixels_last; break;
                case 100: return (uint32_t) &fcn_draw_64_pixels_last; break;
            }
        }
    }
    return 0;
}

uint32_t EspFunction::cover_32(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 32 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_32_pixels; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_32_pixels; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_32_pixels; break;
                case 100: return (uint32_t) &fcn_copy_32_pixels; break;
                default: return (uint32_t) &fcn_skip_copy_32_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_32_pixels_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_32_pixels_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_32_pixels_last; break;
                case 100: return (uint32_t) &fcn_copy_32_pixels_last; break;
            }
        }
    } else {
        if (width > 32 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_32_pixels; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_32_pixels; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_32_pixels; break;
                case 100: return (uint32_t) &fcn_draw_32_pixels; break;
                default: return (uint32_t) &fcn_skip_draw_32_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_32_pixels_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_32_pixels_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_32_pixels_last; break;
                case 100: return (uint32_t) &fcn_draw_32_pixels_last; break;
            }
        }
    }
    return 0;
}

uint32_t EspFunction::cover_16(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 16 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_16_pixels; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_16_pixels; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_16_pixels; break;
                case 100: return (uint32_t) &fcn_copy_16_pixels; break;
                default: return (uint32_t) &fcn_skip_copy_16_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_16_pixels_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_16_pixels_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_16_pixels_last; break;
                case 100: return (uint32_t) &fcn_copy_16_pixels_last; break;
            }
        }
    } else {
        if (width > 16 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_16_pixels; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_16_pixels; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_16_pixels; break;
                case 100: return (uint32_t) &fcn_draw_16_pixels; break;
                default: return (uint32_t) &fcn_skip_draw_16_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_16_pixels_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_16_pixels_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_16_pixels_last; break;
                case 100: return (uint32_t) &fcn_draw_16_pixels_last; break;
            }
        }
    }
    return 0;
}

uint32_t EspFunction::cover_8(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 8 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_8_pixels; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_8_pixels; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_8_pixels; break;
                case 100: return (uint32_t) &fcn_copy_8_pixels; break;
                default: return (uint32_t) &fcn_skip_copy_8_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_8_pixels_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_8_pixels_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_8_pixels_last; break;
                case 100: return (uint32_t) &fcn_copy_8_pixels_last; break;
            }
        }
    } else {
        if (width > 8 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_8_pixels; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_8_pixels; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_8_pixels; break;
                case 100: return (uint32_t) &fcn_draw_8_pixels; break;
                default: return (uint32_t) &fcn_skip_draw_8_pixels; break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_8_pixels_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_8_pixels_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_8_pixels_last; break;
                case 100: return (uint32_t) &fcn_draw_8_pixels_last; break;
            }
        }
    }
    return 0;
}

uint32_t EspFunction::cover_4(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 4 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_4_pixels_at_offset_0; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_4_pixels_at_offset_0; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_4_pixels_at_offset_0; break;
                case 100:
                    l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);
                    s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
                    addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_4_pixels_at_offset_0_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_4_pixels_at_offset_0_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_4_pixels_at_offset_0_last; break;
                case 100:
                    l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);
                    s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
                    break;
            }
        }
    } else {
        if (width > 4 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_4_pixels_at_offset_0; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_4_pixels_at_offset_0; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_4_pixels_at_offset_0; break;
                case 100:
                    s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
                default: addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4); break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_4_pixels_at_offset_0_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_4_pixels_at_offset_0_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_4_pixels_at_offset_0_last; break;
                case 100:
                    s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
                    break;
            }
        }
    }
    return 0;
}

uint32_t EspFunction::cover_3_at_0(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy) {
    if (copy) {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_src_blend_25_for_3_pixels_at_offset_0_last; break;
            case 50: return (uint32_t) &fcn_src_blend_50_for_3_pixels_at_offset_0_last; break;
            case 75: return (uint32_t) &fcn_src_blend_75_for_3_pixels_at_offset_0_last; break;
            case 100:
                l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0));
                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                break;
        }
    } else {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_color_blend_25_for_3_pixels_at_offset_0_last; break;
            case 50: return (uint32_t) &fcn_color_blend_50_for_3_pixels_at_offset_0_last; break;
            case 75: return (uint32_t) &fcn_color_blend_75_for_3_pixels_at_offset_0_last; break;
            case 100:
                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                break;
        }
    }
    return 0;
}

uint32_t EspFunction::cover_2_at_0(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy) {
    if (copy) {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_src_blend_25_for_2_pixels_at_offset_0_last; break;
            case 50: return (uint32_t) &fcn_src_blend_50_for_2_pixels_at_offset_0_last; break;
            case 75: return (uint32_t) &fcn_src_blend_75_for_2_pixels_at_offset_0_last; break;
            case 100:
                l16ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0));
                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                break;
        }
    } else {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_color_blend_25_for_2_pixels_at_offset_0_last; break;
            case 50: return (uint32_t) &fcn_color_blend_50_for_2_pixels_at_offset_0_last; break;
            case 75: return (uint32_t) &fcn_color_blend_75_for_2_pixels_at_offset_0_last; break;
            case 100:
                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                break;
        }
    }
    return 0;
}

uint32_t EspFunction::cover_1_at_0(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy) {
    if (copy) {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_0_last; break;
            case 50: return (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_0_last; break;
            case 75: return (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_0_last; break;
            case 100:
                l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0));
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                break;
        }
    } else {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_0_last; break;
            case 50: return (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_0_last; break;
            case 75: return (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_0_last; break;
            case 100:
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                break;
        }
    }
    return 0;
}

uint32_t EspFunction::cover_3_at_1(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 3 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_3_pixels_at_offset_1; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_3_pixels_at_offset_1; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_3_pixels_at_offset_1; break;
                case 100:
                    l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);    
                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                    addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_3_pixels_at_offset_1_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_3_pixels_at_offset_1_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_3_pixels_at_offset_1_last; break;
                case 100:
                    l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);    
                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                    break;
            }
        }
    } else {
        if (width > 3) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_3_pixels_at_offset_1; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_3_pixels_at_offset_1; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_3_pixels_at_offset_1; break;
                case 100:
                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
                default: addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4); break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_3_pixels_at_offset_1_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_3_pixels_at_offset_1_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_3_pixels_at_offset_1_last; break;
                case 100:
                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                    break;
            }
            if (more) {
                addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
            }
        }
    }
    return 0;
}

uint32_t EspFunction::cover_2_at_1(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy) {
    if (copy) {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_src_blend_25_for_2_pixels_at_offset_1_last; break;
            case 50: return (uint32_t) &fcn_src_blend_50_for_2_pixels_at_offset_1_last; break;
            case 75: return (uint32_t) &fcn_src_blend_75_for_2_pixels_at_offset_1_last; break;
            case 100:
                l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);    
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                break;
        }
    } else {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_color_blend_25_for_2_pixels_at_offset_1_last; break;
            case 50: return (uint32_t) &fcn_color_blend_50_for_2_pixels_at_offset_1_last; break;
            case 75: return (uint32_t) &fcn_color_blend_75_for_2_pixels_at_offset_1_last; break;
            case 100:
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                break;
        }
    }
    return 0;
}

uint32_t EspFunction::cover_1_at_1(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy) {
    if (copy) {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_1_last; break;
            case 50: return (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_1_last; break;
            case 75: return (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_1_last; break;
            case 100:
                l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(1));    
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
                break;
        }
    } else {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_1_last; break;
            case 50: return (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_1_last; break;
            case 75: return (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_1_last; break;
            case 100:
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
                break;
        }
    }
    return 0;
}

uint32_t EspFunction::cover_2_at_2(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 2 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_2_pixels_at_offset_2; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_2_pixels_at_offset_2; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_2_pixels_at_offset_2; break;
                case 100:
                    l16ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2));
                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                    addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_2_pixels_at_offset_2_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_2_pixels_at_offset_2_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_2_pixels_at_offset_2_last; break;
                case 100:
                    l16ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2));
                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                    addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
            }
        }
    } else {
        if (width > 2) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_2_pixels_at_offset_2; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_2_pixels_at_offset_2; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_2_pixels_at_offset_2; break;
                case 100:
                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
                default: addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4); break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_2_pixels_at_offset_2_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_2_pixels_at_offset_2_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_2_pixels_at_offset_2_last; break;
                case 100:
                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                    break;
            }
            if (more) {
                addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
            }
        }
    }
    return 0;
}

uint32_t EspFunction::cover_1_at_2(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy) {
    if (copy) {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_2_last; break;
            case 50: return (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_2_last; break;
            case 75: return (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_2_last; break;
            case 100:
                l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2));
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                break;
        }
    } else {
        switch (opaqueness) {
            case 25: return (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_2_last; break;
            case 50: return (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_2_last; break;
            case 75: return (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_2_last; break;
            case 100:
                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                break;
        }
    }
    return 0;
}

uint32_t EspFunction::cover_1_at_3(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more) {
    if (copy) {
        if (width > 1 || more) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_3; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_3; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_3; break;
                case 100:
                    l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(3));
                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
                    addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_3_last; break;
                case 50: return (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_3_last; break;
                case 75: return (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_3_last; break;
                case 100:
                    l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(3));
                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
                    break;
            }
        }
    } else {
        if (width > 1) {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_3; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_3; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_3; break;
                case 100:
                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                    break;
                default: addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4); break;
            }
        } else {
            switch (opaqueness) {
                case 25: return (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_3_last; break;
                case 50: return (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_3_last; break;
                case 75: return (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_3_last; break;
                case 100:
                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
                    break;
            }
            if (more) {
                addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
            }
        }
    }
    return 0;
}

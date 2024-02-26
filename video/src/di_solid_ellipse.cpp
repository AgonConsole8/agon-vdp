// di_solid_ellipse.cpp - Function definitions for drawing solid ellipses
//
// A solid ellipse is filled with a single color.
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

#include "di_solid_ellipse.h"
#include "di_timing.h"

DiSolidEllipse::DiSolidEllipse(uint16_t flags) : DiPrimitive(flags) {
}

void DiSolidEllipse::init_params(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
  m_rel_x = x;
  m_rel_y = y;
  m_width = width;
  m_height = height;
  color &= 0x3F; // remove any alpha bits
  m_color = PIXEL_COLOR_X4(color) | otf_video_params.m_syncs_off_x4;
}

void IRAM_ATTR DiSolidEllipse::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
}

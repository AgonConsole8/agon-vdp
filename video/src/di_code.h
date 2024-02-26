// di_code.h - Supports creating ESP32 functions dynamically.
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

#include <stdint.h>
#include <vector>
#include "di_line_pieces.h"

typedef enum {
    a0 = 0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15,
    ra = 0, sp = 1
} reg_t;

typedef uint32_t instr_t;   // instruction
typedef uint32_t u_off_t;     // unsigned offset
typedef int32_t s_off_t;   // signed offset

extern "C" {
typedef void (*CallEspFcn)(void* p_this, volatile uint32_t* p_scan_line, uint32_t line_index);
typedef void (*CallEspA5Fcn)(void* p_this, volatile uint32_t* p_scan_line, uint32_t line_index,
                                uint32_t x);
typedef void (*CallEspA5A6Fcn)(void* p_this, volatile uint32_t* p_scan_line, uint32_t line_index,
                                uint32_t a5_value, uint32_t a6_value);
};

typedef union {
    uint32_t        m_address;
    CallEspFcn      m_simple;
    CallEspA5Fcn    m_a5;
    CallEspA5A6Fcn  m_a5a6;

    void clear() { m_address = 0; }
} EspFcnPtr;

typedef struct {
    uint32_t code_index;
    uint32_t fcn_address;
} EspFixup;

typedef std::vector<EspFixup> EspFixups;
typedef std::vector<EspFcnPtr> EspFcnPtrs;

class EspFunction {
    public:
    EspFunction();
    ~EspFunction();

    // Pixel-level operations:

    void draw_line(EspFixups& fixups, uint32_t x_offset,
                        uint32_t skip, uint32_t draw_width,
                        const DiLineSections* sections, uint16_t flags,
                        uint8_t opaqueness, bool outer_fcn);

    void draw_line_loop(EspFixups& fixups, uint32_t x_offset,
                        uint32_t skip, uint32_t draw_width,
                        const DiLineSections* sections, uint16_t flags, uint8_t opaqueness);

    void copy_line(EspFixups& fixups, uint32_t x_offset,
        uint32_t skip, uint32_t width,
        uint16_t flags, uint8_t transparent_color,
        uint32_t* src_pixels, bool outer_fcn);

    void copy_line_loop(EspFixups& fixups, uint32_t x_offset,
        uint32_t skip, uint32_t draw_width,
        uint16_t flags, uint8_t transparent_color,
        uint32_t* src_pixels);

    // Common operations in functions:

    void do_fixups(EspFixups& fixups);
    void enter_and_leave_outer_function();
    uint32_t enter_outer_function();
    uint32_t enter_inner_function();
    uint32_t begin_data();
    uint32_t init_jump_table(uint32_t num_items);
    void begin_code(uint32_t at_jump);
    void set_reg_dst_pixel_ptr_for_draw(uint16_t flags);
    void set_reg_dst_pixel_ptr_for_copy(uint16_t flags);

    // Utility operations:

    inline void clear() { m_code_index = 0; m_code_size = 0; }
    inline uint32_t get_code_index() { return m_code_index; }
    inline void set_code_index(uint32_t code_index) { m_code_index = code_index; }
    inline uint32_t get_code_size() { return m_code_size; }
    inline uint32_t get_code(uint32_t address) { return m_code[address >> 2]; }
    inline uint32_t get_code_start() { return (uint32_t)(void*) m_code; }
    inline uint32_t get_real_address() { return ((uint32_t)m_code) + m_code_index; }
    inline uint32_t get_real_address(uint32_t code_index) { return ((uint32_t)m_code) + code_index; }
    void align16();
    void align32();
    void j_to_here(uint32_t from);
    void l32r_from(reg_t reg, uint32_t from);
    void loop_to_here(reg_t reg, uint32_t from);
    uint16_t dup8_to_16(uint8_t value);
    uint32_t dup8_to_32(uint8_t value);
    uint32_t dup16_to_32(uint16_t value);

    // Assembler-level instructions:

    void add(reg_t dst, reg_t src1, reg_t src2) { write24("add", issd(0x800000, src1, src2, dst)); }
    void addi(reg_t dst, reg_t src, s_off_t offset) { write24("addi", idsi(0x00C002, dst, src, offset)); }
    void bbc(reg_t src, reg_t dst, s_off_t offset) { write24("bbc", isdo(0x005007, src, dst, offset)); }
    void bbci(reg_t src, uint32_t imm, s_off_t offset) { write24("bbci", isio(0x006007, src, imm, offset)); }
    void bbs(reg_t src, reg_t dst, s_off_t offset) { write24("bbs", isdo(0x00D007, src, dst, offset)); }
    void bbsi(reg_t src, uint32_t imm, s_off_t offset) { write24("bbsi", isio(0x007007, src, imm, offset)); }
    void beq(reg_t src, reg_t dst, s_off_t offset) { write24("beq", isdo(0x001007, src, dst, offset)); }
    void beqi(reg_t src, uint32_t imm, s_off_t offset) { write24("beqi", isieo(0x000026, src, imm, offset)); }
    void beqz(reg_t src, s_off_t offset) { write24("beqz", iso(0x000016, src, offset)); }
    void bne(reg_t src, reg_t dst, s_off_t offset) { write24("bne", isdo(0x009007, src, dst, offset)); }
    void bnei(reg_t src, uint32_t imm, s_off_t offset) { write24("bnei", isieo(0x000066, src, imm, offset)); }
    void bnez(reg_t src, s_off_t offset) { write24("bnez", iso(0x000056, src, offset)); }
    void bge(reg_t src, reg_t dst, s_off_t offset) { write24("bge", isdo(0x00A007, src, dst, offset)); }
    void bgei(reg_t src, uint32_t imm, s_off_t offset) { write24("bgei", isieo(0x0000E6, src, imm, offset)); }
    void bgeu(reg_t src, reg_t dst, s_off_t offset) { write24("bgeu", isdo(0x00B007, src, dst, offset)); }
    void bgeui(reg_t src, uint32_t imm, s_off_t offset) { write24("bgeui", isieo(0x0000F6, src, imm, offset)); }
    void bgez(reg_t src, s_off_t offset) { write24("bgez", iso(0x0000D6, src, offset)); }
    void bgez_to_here(reg_t src, s_off_t from);
    void blt(reg_t src, reg_t dst, s_off_t offset) { write24("blt", isdo(0x002007, src, dst, offset)); }
    void blti(reg_t src, uint32_t imm, s_off_t offset) { write24("blti", isieo(0x0000A6, src, imm, offset)); }
    void bltu(reg_t src, reg_t dst, s_off_t offset) { write24("bltu", isdo(0x003007, src, dst, offset)); }
    void bltui(reg_t src, uint32_t imm, s_off_t offset) { write24("bltui", isieo(0x0000B6, src, imm, offset)); }
    void bltz(reg_t src, s_off_t offset) { write24("bltz", iso(0x000096, src, offset)); }
    void call0(s_off_t offset) { write24("call0", isco(0x000005, offset)); }
    void callx0(reg_t src) { write24("callx0", iscxo(0x0000C0, src)); }
    uint32_t d8(uint32_t value) { return write8("d8", value); }
    uint32_t d16(uint32_t value) { return write16("d16", value); }
    uint32_t d24(uint32_t value) { return write24("d24", value); }
    uint32_t d32(uint32_t value) { return write32("d32", value); }
    void entry(reg_t src, u_off_t offset) { write24("entry", iso(0x000036, src, (offset >> 3))); }
    void j(s_off_t offset) { write24("j", io(0x000006, offset)); }
    void jx(reg_t src) { write24("jx", iscxo(0x0000A0, src)); }
    void l16si(reg_t dst, reg_t src, u_off_t offset) { write24("l16si", idso16(0x009002, dst, src, offset)); }
    void l16ui(reg_t dst, reg_t src, u_off_t offset) { write24("l16ui", idso16(0x001002, dst, src, offset)); }
    void l32i(reg_t dst, reg_t src, u_off_t offset) { write24("l32i", idso32(0x002002, dst, src, offset)); }
    void l32r(reg_t dst, s_off_t offset) { write24("l32r", ido(0x000001, dst, offset)); }
    void l8ui(reg_t dst, reg_t src, u_off_t offset) { write24("l8ui", idso8(0x000002, dst, src, offset)); }
    void loop(reg_t src, u_off_t offset) { write24("loop", iso8(0x008076, src, offset)); }
    void mov(reg_t dst, reg_t src) { write24("mov", ids(0x200000, dst, src)); }
    void movi(reg_t dst, uint32_t value) { write24("movi", iv(0x00A002, dst, value)); }
    void ret() { write24("ret", 0x000080); }
    void retw() { write24("retw", 0x000090); }
    void s16i(reg_t dst, reg_t src, u_off_t offset) { write24("s16i", idso16(0x005002, dst, src, offset)); }
    void s32i(reg_t dst, reg_t src, u_off_t offset) { write24("s32i", idso32(0x006002, dst, src, offset)); }
    void s8i(reg_t dst, reg_t src, u_off_t offset) { write24("s8i", idso8(0x004002, dst, src, offset)); }
    void slli(reg_t dst, reg_t src, uint8_t bits) { write24("slli", idsb(0x010000, dst, src, bits)); }
    void srli(reg_t dst, reg_t src, uint8_t bits) { write24("srli", idsrb(0x410000, dst, src, bits)); }
    void sub(reg_t dst, reg_t src1, reg_t src2) { write24("sub", issd(0xC00000, src1, src2, dst)); }

    // a0 = return address
    // a1 = stack ptr
    // a2 = p_this
    // a3 = p_scan_line
    // a4 = line_index
    inline void call(void* p_this, volatile uint32_t* p_scan_line, uint32_t line_index) {
        (*((CallEspFcn)m_code))(p_this, p_scan_line, line_index);
    }

    // a0 = return address
    // a1 = stack ptr
    // a2 = p_this
    // a3 = p_scan_line
    // a4 = line_index
    // a5 = draw_x
    inline void call_x(void* p_this, volatile uint32_t* p_scan_line, uint32_t line_index,
                        uint32_t draw_x) {
        (*((CallEspA5Fcn)m_code))(p_this, p_scan_line, line_index, draw_x);
    }

    // a0 = return address
    // a1 = stack ptr
    // a2 = p_this
    // a3 = p_scan_line
    // a4 = line_index
    // a5 = a5_value
    // a6 = a6_value
    inline void call_a5_a6(void* p_this, volatile uint32_t* p_scan_line, uint32_t line_index,
                        uint32_t a5_value, uint32_t a6_value) {
        (*((CallEspA5A6Fcn)m_code))(p_this, p_scan_line, line_index, a5_value, a6_value);
    }

    protected:
    uint32_t    m_alloc_size;
    uint32_t    m_code_size;
    uint32_t    m_code_index;
    uint32_t*   m_code;

    void init_members();
    void allocate(uint32_t size);
    void store(uint8_t instr_byte);
    uint32_t write8(const char* mnemonic, instr_t data);
    uint32_t write16(const char* mnemonic, instr_t data);
    uint32_t write24(const char* mnemonic, instr_t data);
    uint32_t write32(const char* mnemonic, instr_t data);
    void call_inner_fcn(uint32_t real_address);
    void adjust_dst_pixel_ptr(uint32_t draw_x, uint32_t x);

    inline instr_t issd(uint32_t instr, reg_t src1, reg_t src2, reg_t dst) {
        return instr | (dst << 12) | (src1 << 8) | (src2 << 4); }

    inline instr_t ids(uint32_t instr, reg_t dst, reg_t src) {
        return instr | (dst << 12) | (src << 8) | (src << 4); }

    inline instr_t idso16(uint32_t instr, reg_t dst, reg_t src, u_off_t offset) {
        return instr | ((offset >> 1) << 16) | (dst << 4) | (src << 8); }

    inline instr_t idso32(uint32_t instr, reg_t dst, reg_t src, u_off_t offset) {
        return instr | ((offset >> 2) << 16) | (dst << 4) | (src << 8); }

    inline instr_t idso8(uint32_t instr, reg_t dst, reg_t src, u_off_t offset) {
        return instr | (offset << 16) | (dst << 4) | (src << 8); }

    inline instr_t isdo(uint32_t instr, reg_t src, reg_t dst, s_off_t offset) {
        return instr | (offset << 16) | (dst << 4) | (src << 8); }

    inline instr_t idsi(uint32_t instr, reg_t dst, reg_t src, s_off_t imm) {
        return instr | (((int32_t)imm) << 16) | (dst << 4) | (src << 8); }

    inline instr_t idsb(uint32_t instr, reg_t dst, reg_t src, uint8_t bits) {
        bits = 32 - bits;
        return instr | ((bits >> 4) << 20) | (dst << 12) | (src << 8) | ((bits & 0xF) << 4); }

    inline instr_t idsrb(uint32_t instr, reg_t dst, reg_t src, uint8_t bits) {
        return instr | (dst << 12) | (src << 4) | (bits << 8); }

    inline instr_t isio(uint32_t instr, reg_t src, uint32_t imm, s_off_t offset) {
        return instr | (offset << 16) | ((imm & 0xF) << 4) | ((imm & 0x10) << 8) | (src << 8); }

    instr_t isieo(uint32_t instr, reg_t src, int32_t imm, u_off_t offset);

    inline instr_t iso(uint32_t instr, reg_t src, s_off_t offset) {
        return instr | (offset << 12) | (src << 8); }

    inline instr_t isco(uint32_t instr, u_off_t offset) {
        return instr | ((offset >> 2) << 6); }

    inline instr_t iscxo(uint32_t instr, reg_t src) {
        return instr | (src << 8); }

    inline instr_t iso8(uint32_t instr, reg_t src, u_off_t offset) {
        return instr | (offset << 16) | (src << 8); }

    inline instr_t ido(uint32_t instr, reg_t dst, u_off_t offset) {
        return instr | ((offset >> 2) << 8) | (dst << 4); }

    inline instr_t io(uint32_t instr, u_off_t offset) {
        return instr | (offset << 6); }

    inline instr_t iv(uint32_t instr, reg_t dst, uint32_t value) {
        return instr | ((value & 0xFF) << 16) | (dst << 4) | (value & 0xF00); }

    void cover_width(EspFixups& fixups, uint32_t& x_offset, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_256(EspFixups& fixups, uint32_t times, uint8_t opaqueness, bool copy);
    uint32_t cover_128(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_64(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_32(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_16(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_8(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_4(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_3_at_0(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy);
    uint32_t cover_2_at_0(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy);
    uint32_t cover_1_at_0(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy);
    uint32_t cover_3_at_1(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_2_at_1(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy);
    uint32_t cover_1_at_1(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy);
    uint32_t cover_2_at_2(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
    uint32_t cover_1_at_2(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy);
    uint32_t cover_1_at_3(EspFixups& fixups, uint32_t width, uint8_t opaqueness, bool copy, bool more);
};

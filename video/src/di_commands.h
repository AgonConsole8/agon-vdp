// di_commands.h - Data structures of VDU serial commands.
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

#pragma pack(push, 1)

#define _anglex int16_t  m_anglex;
#define _angley int16_t  m_angley;
#define _anglez int16_t  m_anglez;
#define _blue   uint8_t  m_blue;
#define _bgcolor uint8_t m_bgcolor;
#define _bmid   uint16_t m_bmid;
#define _bottom uint16_t m_bottom;
#define _bottom8 uint8_t m_bottom;
#define _char   uint8_t  m_char;
#define _color  uint8_t  m_color;
#define _colors uint8_t  m_colors[1];
#define _column uint16_t m_column;
#define _column8 uint8_t m_column;
#define _columns uint16_t m_columns;
#define _coords int16_t  m_coords[1];
#define _distx  int16_t  m_distx;
#define _disty  int16_t  m_disty;
#define _distz  int16_t  m_distz;
#define _firstchar uint8_t m_firstchar;
#define _fgcolor uint8_t m_fgcolor;
#define _flags  uint16_t m_flags;
#define _green  uint8_t m_green;
#define _h      uint16_t m_h;
#define _i0     uint16_t m_i0;
#define _id     uint16_t m_id;
#define _ignore uint8_t  m_ignore;
#define _ix     int16_t  m_ix;
#define _iy     int16_t  m_iy;
#define _lastchar uint8_t m_lastchar;
#define _left   uint16_t m_left;
#define _left8  uint8_t m_left;
#define _logical  uint8_t m_logical;
#define _mid    uint16_t m_mid;
#define _mode   uint8_t m_mode;
#define _n      uint16_t m_n;
#define _oid    uint16_t m_oid;
#define _physical uint8_t m_physical;
#define _pid    uint16_t m_pid;
#define _psram  uint8_t m_psram;
#define _r1     uint8_t m_r1;
#define _r2     uint8_t m_r2;
#define _r3     uint8_t m_r3;
#define _r4     uint8_t m_r4;
#define _r5     uint8_t m_r5;
#define _r6     uint8_t m_r6;
#define _r7     uint8_t m_r7;
#define _r8     uint8_t m_r8;
#define _red    uint8_t m_red;
#define _row    uint16_t m_row;
#define _row8   uint8_t m_row;
#define _rows   uint16_t m_rows;
#define _right  uint16_t m_right;
#define _right8 uint8_t m_right;
#define _s      uint16_t m_s;
#define _scalex uint16_t m_scalex;
#define _scaley uint16_t m_scaley;
#define _scalez uint16_t m_scalez;
#define _sx0    int16_t  m_sx0;
#define _sx1    int16_t  m_sx1;
#define _sy0    int16_t  m_sy0;
#define _sy1    int16_t  m_sy1;
#define _th     uint16_t m_th;
#define _top    uint16_t m_top;
#define _top8   uint8_t m_top;
#define _tw     uint16_t m_tw;
#define _u0     uint16_t m_u0;
#define _v0     uint16_t m_v0;
#define _w      uint16_t m_w;
#define _x      int16_t  m_x;
#define _x0     int16_t  m_x0;
#define _x1     int16_t  m_x1;
#define _x2     int16_t  m_x2;
#define _x3     int16_t  m_x3;
#define _x4     int16_t  m_x4;
#define _y      int16_t  m_y;
#define _y0     int16_t  m_y0;
#define _y1     int16_t  m_y1;
#define _y2     int16_t  m_y2;
#define _y3     int16_t  m_y3;
#define _y4     int16_t  m_y4;
#define _z0     int16_t  m_z0;

#define ARGS(...)   __VA_ARGS__

#define OTFCMD(cmd, params, name) \
typedef struct { \
    uint8_t     m_category; \
    uint8_t     m_subcategory; \
    uint8_t     m_command; \
    ARGS params \
} OtfCmd_##cmd##name;

OTFCMD(0,(_id _flags),_Set_flags_for_primitive)
OTFCMD(1,(_id _x _y),_Set_primitive_position)
OTFCMD(2,(_id _ix _iy),_Adjust_primitive_position)
OTFCMD(3,(_id),_Delete_primitive)
OTFCMD(4,(_id),_Generate_code_for_primitive)
OTFCMD(10,(_id _pid _flags _x _y _color),_Create_primitive_Point)
OTFCMD(20,(_id _pid _flags _x1 _y1 _x2 _y2 _color),_Create_primitive_Line)
OTFCMD(30,(_id _pid _flags _color _x1 _y1 _x2 _y2 _x3 _y3),_Create_primitive_Triangle_Outline)
OTFCMD(31,(_id _pid _flags _color _x1 _y1 _x2 _y2 _x3 _y3),_Create_primitive_Solid_Triangle)
OTFCMD(32,(_id _pid _flags _n _color _coords),_Create_primitive_Triangle_List_Outline)
OTFCMD(33,(_id _pid _flags _n _color _coords),_Create_primitive_Solid_Triangle_List)
OTFCMD(34,(_id _pid _flags _n _color _sx0 _sy0 _sx1 _sy1 _coords),_Create_primitive_Triangle_Fan_Outline)
OTFCMD(35,(_id _pid _flags _n _color _sx0 _sy0 _sx1 _sy1 _coords),_Create_primitive_Solid_Triangle_Fan)
OTFCMD(36,(_id _pid _flags _n _color _sx0 _sy0 _sx1 _sy1 _coords),_Create_primitive_Triangle_Strip_Outline)
OTFCMD(37,(_id _pid _flags _n _color _sx0 _sy0 _sx1 _sy1 _coords),_Create_primitive_Solid_Triangle_Strip)
OTFCMD(40,(_id _pid _flags _x _y _w _h _color),_Create_primitive_Rectangle_Outline)
OTFCMD(41,(_id _pid _flags _x _y _w _h _color),_Create_primitive_Solid_Rectangle)
OTFCMD(50,(_id _pid _flags _x _y _w _h _color),_Create_primitive_Ellipse_Outline)
OTFCMD(51,(_id _pid _flags _x _y _w _h _color),_Create_primitive_Solid_Ellipse)
OTFCMD(60,(_id _pid _flags _color _x1 _y1 _x2 _y2 _x3 _y3 _x4 _y4),_Create_primitive_Quad_Outline)
OTFCMD(61,(_id _pid _flags _color _x1 _y1 _x2 _y2 _x3 _y3 _x4 _y4),_Create_primitive_Solid_Quad)
OTFCMD(62,(_id _pid _flags _n _color _coords),_Create_primitive_Quad_List_Outline)
OTFCMD(63,(_id _pid _flags _n _color _coords),_Create_primitive_Solid_Quad_List)
OTFCMD(64,(_id _pid _flags _n _color _sx0 _sy0 _sx1 _sy1 _coords),_Create_primitive_Quad_Strip_Outline)
OTFCMD(65,(_id _pid _flags _n _color _sx0 _sy0 _sx1 _sy1 _coords),_Create_primitive_Solid_Quad_Strip)
OTFCMD(80,(_id _pid _flags _columns _rows _tw _th _w _h),_Create_primitive_Tile_Array)
OTFCMD(81,(_id _bmid),_Create_Solid_Bitmap_for_Tile_Array)
OTFCMD(82,(_id _bmid _color),_Create_Masked_Bitmap_for_Tile_Array)
OTFCMD(83,(_id _bmid _color),_Create_Transparent_Bitmap_for_Tile_Array)
OTFCMD(84,(_id _column _row _bmid),_Set_bitmap_ID_for_tile_in_Tile_Array)
OTFCMD(85,(_id _bmid _x _y _color),_Set_solid_bitmap_pixel_in_Tile_Array)
OTFCMD(86,(_id _bmid _x _y _color),_Set_masked_bitmap_pixel_in_Tile_Array)
OTFCMD(87,(_id _bmid _x _y _color),_Set_transparent_bitmap_pixel_in_Tile_Array)
OTFCMD(88,(_id _bmid _x _y _n _colors),_Set_solid_bitmap_pixels_in_Tile_Array)
OTFCMD(89,(_id _bmid _x _y _n _colors),_Set_masked_bitmap_pixels_in_Tile_Array)
OTFCMD(90,(_id _bmid _x _y _n _colors),_Set_transparent_bitmap_pixels_in_Tile_Array)
OTFCMD(100,(_id _pid _flags _columns _rows _tw _th _w _h),_Create_primitive_Tile_Map)
OTFCMD(101,(_id _bmid),_Create_Solid_Bitmap_for_Tile_Map)
OTFCMD(102,(_id _bmid _color),_Create_Masked_Bitmap_for_Tile_Map)
OTFCMD(103,(_id _bmid _color),_Create_Transparent_Bitmap_for_Tile_Map)
OTFCMD(104,(_id _column _row _bmid),_Set_bitmap_ID_for_tile_in_Tile_Map)
OTFCMD(105,(_id _bmid _x _y _color),_Set_solid_bitmap_pixel_in_Tile_Map)
OTFCMD(106,(_id _bmid _x _y _color),_Set_masked_bitmap_pixel_in_Tile_Map)
OTFCMD(107,(_id _bmid _x _y _color),_Set_transparent_bitmap_pixel_in_Tile_Map)
OTFCMD(108,(_id _bmid _x _y _n _colors),_Set_solid_bitmap_pixels_in_Tile_Map)
OTFCMD(109,(_id _bmid _x _y _n _colors),_Set_masked_bitmap_pixels_in_Tile_Map)
OTFCMD(110,(_id _bmid _x _y _n _colors),_Set_transparent_bitmap_pixels_in_Tile_Map)
OTFCMD(120,(_id _pid _flags _w _h _psram),_Create_primitive_Solid_Bitmap)
OTFCMD(121,(_id _pid _flags _w _h _psram _color),_Create_primitive_Masked_Bitmap)
OTFCMD(122,(_id _pid _flags _w _h _psram _color),_Create_primitive_Transparent_Bitmap)
OTFCMD(123,(_id _x _y _s _h),_Set_position_and_slice_solid_bitmap)
OTFCMD(124,(_id _x _y _s _h),_Set_position_and_slice_masked_bitmap)
OTFCMD(125,(_id _x _y _s _h),_Set_position_and_slice_transparent_bitmap)
OTFCMD(126,(_id _x _y _s _h),_Adjust_position_and_slice_solid_bitmap)
OTFCMD(127,(_id _x _y _s _h),_Adjust_position_and_slice_masked_bitmap)
OTFCMD(128,(_id _x _y _s _h),_Adjust_position_and_slice_transparent_bitmap)
OTFCMD(129,(_id _x _y _color),_Set_solid_bitmap_pixel)
OTFCMD(130,(_id _x _y _color),_Set_masked_bitmap_pixel)
OTFCMD(131,(_id _x _y _color),_Set_transparent_bitmap_pixel)
OTFCMD(132,(_id _x _y _n _colors),_Set_solid_bitmap_pixels)
OTFCMD(133,(_id _x _y _n _colors),_Set_masked_bitmap_pixels)
OTFCMD(134,(_id _x _y _n _colors),_Set_transparent_bitmap_pixels)
OTFCMD(135,(_id _pid _flags _bmid),_Create_primitive_Reference_Solid_Bitmap)
OTFCMD(136,(_id _pid _flags _bmid),_Create_primitive_Reference_Masked_Bitmap)
OTFCMD(137,(_id _pid _flags _bmid),_Create_primitive_Reference_Transparent_Bitmap)
OTFCMD(140,(_id _pid _flags _x _y _w _h),_Create_primitive_Group)
OTFCMD(150,(_id _pid _flags _x _y _columns _rows _fgcolor _bgcolor),_Create_primitive_Text_Area)
OTFCMD(151,(_id),_Select_Active_Text_Area)
OTFCMD(152,(_id _char _fgcolor _bgcolor),_Define_Text_Area_Character)
OTFCMD(153,(_id _firstchar _lastchar _fgcolor _bgcolor),_Define_Text_Area_Character_Range)
OTFCMD(200,(_id _pid _flags _x _y _w _h _psram),_Create_primitive_Solid_Render)
OTFCMD(201,(_id _pid _flags _x _y _w _h _psram _color),_Create_primitive_Masked_Render)
OTFCMD(202,(_id _pid _flags _x _y _w _h _psram _color),_Create_primitive_Transparent_Render)
OTFCMD(203,(_id _mid _n _x0 _y0 _z0),_Define_Mesh_Vertices)
OTFCMD(204,(_id _mid _n _i0),_Set_Mesh_Vertex_Indices)
OTFCMD(205,(_id _mid _n _u0 _v0),_Define_Texture_Coordinates)
OTFCMD(206,(_id _mid _n _i0),_Set_Texture_Coordinate_Indices)
OTFCMD(207,(_id _oid _mid _bmid),_Create_Object)
OTFCMD(208,(_id _oid _scalex),_Set_Object_X_Scale_Factor)
OTFCMD(209,(_id _oid _scaley),_Set_Object_Y_Scale_Factor)
OTFCMD(210,(_id _oid _scalez),_Set_Object_Z_Scale_Factor)
OTFCMD(211,(_id _oid _scalex _scaley _scalez),_Set_Object_XYZ_Scale_Factors)
OTFCMD(212,(_id _oid _anglex),_Set_Object_X_Rotation_Angle)
OTFCMD(213,(_id _oid _angley),_Set_Object_Y_Rotation_Angle)
OTFCMD(214,(_id _oid _anglez),_Set_Object_Z_Rotation_Angle)
OTFCMD(215,(_id _oid _anglex _angley _anglez),_Set_Object_XYZ_Rotation_Angles)
OTFCMD(216,(_id _oid _distx),_Set_Object_X_Translation_Distance)
OTFCMD(217,(_id _oid _disty),_Set_Object_Y_Translation_Distance)
OTFCMD(218,(_id _oid _distz),_Set_Object_Z_Translation_Distance)
OTFCMD(219,(_id _oid _distx _disty _distz),_Set_Object_XYZ_Translation_Distances)
OTFCMD(220,(_id),_Render_To_Bitmap)

#define VDUCMD(cmd, params, name) \
typedef struct { \
    uint8_t     m_category; \
    ARGS params \
} VduCmd_##cmd##name;

VDUCMD(0,(_ignore),_Ignore_data)
VDUCMD(1,(_char),_Print_character)
VDUCMD(2,(),_Enable_print_mode)
VDUCMD(3,(),_Disable_print_mode)
VDUCMD(4,(),_Print_at_text_cursor)
VDUCMD(5,(),_Print_at_graphics_cursor)
VDUCMD(6,(),_Enable_output_to_screen)
VDUCMD(7,(),_Beep)
VDUCMD(8,(),_Move_text_cursor_left)
VDUCMD(9,(),_Move_text_cursor_right)
VDUCMD(10,(),_Move_text_cursor_down)
VDUCMD(11,(),_Move_text_cursor_up)
VDUCMD(12,(),_Clear_text_viewport)
VDUCMD(13,(),_Move_text_cursor_boln)
VDUCMD(14,(),_Enable_auto_page_mode)
VDUCMD(15,(),_Disable_auto_page_mode)
VDUCMD(16,(),_Clear_graphics_viewport)
VDUCMD(17,(_color),_Set_text_color)
VDUCMD(18,(_mode _color),_Set_graphics_mode_color)
VDUCMD(19,(_logical _physical _red _green _blue),_Set_palette_color)
VDUCMD(20,(),_Reset_colors)
VDUCMD(21,(),_Disable_output_to_screen)
VDUCMD(22,(_mode),_Set_video_mode)
VDUCMD(23,(_char _r1 _r2 _r3 _r4 _r5 _r6 _r7 _r8),_Manipulate_text)
VDUCMD(24,(_left _bottom _right _top),_Define_graphics_viewport)
VDUCMD(25,(_mode _x _y),_Plot_graphics)
VDUCMD(26,(),_Reset_viewports)
VDUCMD(27,(_char),_Display_character)
VDUCMD(28,(_left8 _bottom8 _right8 _top8),_Define_text_viewport)
VDUCMD(29,(_x _y),_Set_graphics_origin)
VDUCMD(30,(),_Move_text_cursor_home)
VDUCMD(31,(_row8 _column8),_Set_text_tab_position)
VDUCMD(127,(),_Backspace)

typedef union {
    OtfCmd_0_Set_flags_for_primitive m_0_Set_flags_for_primitive;
    OtfCmd_1_Set_primitive_position m_1_Set_primitive_position;
    OtfCmd_2_Adjust_primitive_position m_2_Adjust_primitive_position;
    OtfCmd_3_Delete_primitive m_3_Delete_primitive;
    OtfCmd_4_Generate_code_for_primitive m_4_Generate_code_for_primitive;
    OtfCmd_10_Create_primitive_Point m_10_Create_primitive_Point;
    OtfCmd_20_Create_primitive_Line m_20_Create_primitive_Line;
    OtfCmd_30_Create_primitive_Triangle_Outline m_30_Create_primitive_Triangle_Outline;
    OtfCmd_31_Create_primitive_Solid_Triangle m_31_Create_primitive_Solid_Triangle;
    OtfCmd_32_Create_primitive_Triangle_List_Outline m_32_Create_primitive_Triangle_List_Outline;
    OtfCmd_33_Create_primitive_Solid_Triangle_List m_33_Create_primitive_Solid_Triangle_List;
    OtfCmd_34_Create_primitive_Triangle_Fan_Outline m_34_Create_primitive_Triangle_Fan_Outline;
    OtfCmd_35_Create_primitive_Solid_Triangle_Fan m_35_Create_primitive_Solid_Triangle_Fan;
    OtfCmd_36_Create_primitive_Triangle_Strip_Outline m_36_Create_primitive_Triangle_Strip_Outline;
    OtfCmd_37_Create_primitive_Solid_Triangle_Strip m_37_Create_primitive_Solid_Triangle_Strip;
    OtfCmd_40_Create_primitive_Rectangle_Outline m_40_Create_primitive_Rectangle_Outline;
    OtfCmd_41_Create_primitive_Solid_Rectangle m_41_Create_primitive_Solid_Rectangle;
    OtfCmd_50_Create_primitive_Ellipse_Outline m_50_Create_primitive_Ellipse_Outline;
    OtfCmd_51_Create_primitive_Solid_Ellipse m_51_Create_primitive_Solid_Ellipse;
    OtfCmd_60_Create_primitive_Quad_Outline m_60_Create_primitive_Quad_Outline;
    OtfCmd_61_Create_primitive_Solid_Quad m_61_Create_primitive_Solid_Quad;
    OtfCmd_62_Create_primitive_Quad_List_Outline m_62_Create_primitive_Quad_List_Outline;
    OtfCmd_63_Create_primitive_Solid_Quad_List m_63_Create_primitive_Solid_Quad_List;
    OtfCmd_64_Create_primitive_Quad_Strip_Outline m_64_Create_primitive_Quad_Strip_Outline;
    OtfCmd_65_Create_primitive_Solid_Quad_Strip m_65_Create_primitive_Solid_Quad_Strip;
    OtfCmd_80_Create_primitive_Tile_Array m_80_Create_primitive_Tile_Array;
    OtfCmd_81_Create_Solid_Bitmap_for_Tile_Array m_81_Create_Solid_Bitmap_for_Tile_Array;
    OtfCmd_82_Create_Masked_Bitmap_for_Tile_Array m_82_Create_Masked_Bitmap_for_Tile_Array;
    OtfCmd_83_Create_Transparent_Bitmap_for_Tile_Array m_83_Create_Transparent_Bitmap_for_Tile_Array;
    OtfCmd_84_Set_bitmap_ID_for_tile_in_Tile_Array m_84_Set_bitmap_ID_for_tile_in_Tile_Array;
    OtfCmd_85_Set_solid_bitmap_pixel_in_Tile_Array m_85_Set_solid_bitmap_pixel_in_Tile_Array;
    OtfCmd_86_Set_masked_bitmap_pixel_in_Tile_Array m_86_Set_masked_bitmap_pixel_in_Tile_Array;
    OtfCmd_87_Set_transparent_bitmap_pixel_in_Tile_Array m_87_Set_transparent_bitmap_pixel_in_Tile_Array;
    OtfCmd_88_Set_solid_bitmap_pixels_in_Tile_Array m_88_Set_solid_bitmap_pixels_in_Tile_Array;
    OtfCmd_89_Set_masked_bitmap_pixels_in_Tile_Array m_89_Set_masked_bitmap_pixels_in_Tile_Array;
    OtfCmd_90_Set_transparent_bitmap_pixels_in_Tile_Array m_90_Set_transparent_bitmap_pixels_in_Tile_Array;
    OtfCmd_100_Create_primitive_Tile_Map m_100_Create_primitive_Tile_Map;
    OtfCmd_101_Create_Solid_Bitmap_for_Tile_Map m_101_Create_Solid_Bitmap_for_Tile_Map;
    OtfCmd_102_Create_Masked_Bitmap_for_Tile_Map m_102_Create_Masked_Bitmap_for_Tile_Map;
    OtfCmd_103_Create_Transparent_Bitmap_for_Tile_Map m_103_Create_Transparent_Bitmap_for_Tile_Map;
    OtfCmd_104_Set_bitmap_ID_for_tile_in_Tile_Map m_104_Set_bitmap_ID_for_tile_in_Tile_Map;
    OtfCmd_105_Set_solid_bitmap_pixel_in_Tile_Map m_105_Set_solid_bitmap_pixel_in_Tile_Map;
    OtfCmd_106_Set_masked_bitmap_pixel_in_Tile_Map m_106_Set_masked_bitmap_pixel_in_Tile_Map;
    OtfCmd_107_Set_transparent_bitmap_pixel_in_Tile_Map m_107_Set_transparent_bitmap_pixel_in_Tile_Map;
    OtfCmd_108_Set_solid_bitmap_pixels_in_Tile_Map m_108_Set_solid_bitmap_pixels_in_Tile_Map;
    OtfCmd_109_Set_masked_bitmap_pixels_in_Tile_Map m_109_Set_masked_bitmap_pixels_in_Tile_Map;
    OtfCmd_110_Set_transparent_bitmap_pixels_in_Tile_Map m_110_Set_transparent_bitmap_pixels_in_Tile_Map;
    OtfCmd_120_Create_primitive_Solid_Bitmap m_120_Create_primitive_Solid_Bitmap;
    OtfCmd_121_Create_primitive_Masked_Bitmap m_121_Create_primitive_Masked_Bitmap;
    OtfCmd_122_Create_primitive_Transparent_Bitmap m_122_Create_primitive_Transparent_Bitmap;
    OtfCmd_123_Set_position_and_slice_solid_bitmap m_123_Set_position_and_slice_solid_bitmap;
    OtfCmd_124_Set_position_and_slice_masked_bitmap m_124_Set_position_and_slice_masked_bitmap;
    OtfCmd_125_Set_position_and_slice_transparent_bitmap m_125_Set_position_and_slice_transparent_bitmap;
    OtfCmd_126_Adjust_position_and_slice_solid_bitmap m_126_Adjust_position_and_slice_solid_bitmap;
    OtfCmd_127_Adjust_position_and_slice_masked_bitmap m_127_Adjust_position_and_slice_masked_bitmap;
    OtfCmd_128_Adjust_position_and_slice_transparent_bitmap m_128_Adjust_position_and_slice_transparent_bitmap;
    OtfCmd_129_Set_solid_bitmap_pixel m_129_Set_solid_bitmap_pixel;
    OtfCmd_130_Set_masked_bitmap_pixel m_130_Set_masked_bitmap_pixel;
    OtfCmd_131_Set_transparent_bitmap_pixel m_131_Set_transparent_bitmap_pixel;
    OtfCmd_132_Set_solid_bitmap_pixels m_132_Set_solid_bitmap_pixels;
    OtfCmd_133_Set_masked_bitmap_pixels m_133_Set_masked_bitmap_pixels;
    OtfCmd_134_Set_transparent_bitmap_pixels m_134_Set_transparent_bitmap_pixels;
    OtfCmd_135_Create_primitive_Reference_Solid_Bitmap m_135_Create_primitive_Reference_Solid_Bitmap;
    OtfCmd_136_Create_primitive_Reference_Masked_Bitmap m_136_Create_primitive_Reference_Masked_Bitmap;
    OtfCmd_137_Create_primitive_Reference_Transparent_Bitmap m_137_Create_primitive_Reference_Transparent_Bitmap;
    OtfCmd_140_Create_primitive_Group m_140_Create_primitive_Group;
    OtfCmd_150_Create_primitive_Text_Area m_150_Create_primitive_Text_Area;
    OtfCmd_151_Select_Active_Text_Area m_151_Select_Active_Text_Area;
    OtfCmd_152_Define_Text_Area_Character m_152_Define_Text_Area_Character;
    OtfCmd_153_Define_Text_Area_Character_Range m_153_Define_Text_Area_Character_Range;
    OtfCmd_200_Create_primitive_Solid_Render m_200_Create_primitive_Solid_Render;
    OtfCmd_201_Create_primitive_Masked_Render m_201_Create_primitive_Masked_Render;
    OtfCmd_202_Create_primitive_Transparent_Render m_202_Create_primitive_Transparent_Render;
    OtfCmd_203_Define_Mesh_Vertices m_203_Define_Mesh_Vertices;
    OtfCmd_204_Set_Mesh_Vertex_Indices m_204_Set_Mesh_Vertex_Indices;
    OtfCmd_205_Define_Texture_Coordinates m_205_Define_Texture_Coordinates;
    OtfCmd_206_Set_Texture_Coordinate_Indices m_206_Set_Texture_Coordinate_Indices;
    OtfCmd_207_Create_Object m_207_Create_Object;
    OtfCmd_208_Set_Object_X_Scale_Factor m_208_Set_Object_X_Scale_Factor;
    OtfCmd_209_Set_Object_Y_Scale_Factor m_209_Set_Object_Y_Scale_Factor;
    OtfCmd_210_Set_Object_Z_Scale_Factor m_210_Set_Object_Z_Scale_Factor;
    OtfCmd_211_Set_Object_XYZ_Scale_Factors m_211_Set_Object_XYZ_Scale_Factors;
    OtfCmd_212_Set_Object_X_Rotation_Angle m_212_Set_Object_X_Rotation_Angle;
    OtfCmd_213_Set_Object_Y_Rotation_Angle m_213_Set_Object_Y_Rotation_Angle;
    OtfCmd_214_Set_Object_Z_Rotation_Angle m_214_Set_Object_Z_Rotation_Angle;
    OtfCmd_215_Set_Object_XYZ_Rotation_Angles m_215_Set_Object_XYZ_Rotation_Angles;
    OtfCmd_216_Set_Object_X_Translation_Distance m_216_Set_Object_X_Translation_Distance;
    OtfCmd_217_Set_Object_Y_Translation_Distance m_217_Set_Object_Y_Translation_Distance;
    OtfCmd_218_Set_Object_Z_Translation_Distance m_218_Set_Object_Z_Translation_Distance;
    OtfCmd_219_Set_Object_XYZ_Translation_Distances m_219_Set_Object_XYZ_Translation_Distances;
    OtfCmd_220_Render_To_Bitmap m_220_Render_To_Bitmap;
} OtfCmdUnion;

typedef union {
    VduCmd_0_Ignore_data m_0_Ignore_data;
    VduCmd_1_Print_character m_1_Print_character;
    VduCmd_2_Enable_print_mode m_2_Enable_print_mode;
    VduCmd_3_Disable_print_mode m_3_Disable_print_mode;
    VduCmd_4_Print_at_text_cursor m_4_Print_at_text_cursor;
    VduCmd_5_Print_at_graphics_cursor m_5_Print_at_graphics_cursor;
    VduCmd_6_Enable_output_to_screen m_6_Enable_output_to_screen;
    VduCmd_7_Beep m_7_Beep;
    VduCmd_8_Move_text_cursor_left m_8_Move_text_cursor_left;
    VduCmd_9_Move_text_cursor_right m_9_Move_text_cursor_right;
    VduCmd_10_Move_text_cursor_down m_10_Move_text_cursor_down;
    VduCmd_11_Move_text_cursor_up m_11_Move_text_cursor_up;
    VduCmd_12_Clear_text_viewport m_12_Clear_text_viewport;
    VduCmd_13_Move_text_cursor_boln m_13_Move_text_cursor_boln;
    VduCmd_14_Enable_auto_page_mode m_14_Enable_auto_page_mode;
    VduCmd_15_Disable_auto_page_mode m_15_Disable_auto_page_mode;
    VduCmd_16_Clear_graphics_viewport m_16_Clear_graphics_viewport;
    VduCmd_17_Set_text_color m_17_Set_text_color;
    VduCmd_18_Set_graphics_mode_color m_18_Set_graphics_mode_color;
    VduCmd_19_Set_palette_color m_19_Set_palette_color;
    VduCmd_20_Reset_colors m_20_Reset_colors;
    VduCmd_21_Disable_output_to_screen m_21_Disable_output_to_screen;
    VduCmd_22_Set_video_mode m_22_Set_video_mode;
    VduCmd_23_Manipulate_text m_23_Manipulate_text;
    VduCmd_24_Define_graphics_viewport m_24_Define_graphics_viewport;
    VduCmd_25_Plot_graphics m_25_Plot_graphics;
    VduCmd_26_Reset_viewports m_26_Reset_viewports;
    VduCmd_27_Display_character m_27_Display_character;
    VduCmd_28_Define_text_viewport m_28_Define_text_viewport;
    VduCmd_29_Set_graphics_origin m_29_Set_graphics_origin;
    VduCmd_30_Move_text_cursor_home m_30_Move_text_cursor_home;
    VduCmd_31_Set_text_tab_position m_31_Set_text_tab_position;
    VduCmd_127_Backspace m_127_Backspace;
} VduCmdUnion;

#pragma pack(pop)

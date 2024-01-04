// di_manager.h - Function declarations for managing drawing-instruction primitives
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

#include <vector>
#include <map>
#include "rom/lldesc.h"
#include "di_video_buffer.h"
#include "di_text_area.h"
#include "di_tile_map.h"
#include "di_render.h"
#include "di_solid_rectangle.h"
#include "di_commands.h"

typedef void (*DiVoidCallback)();

#define INCOMING_DATA_BUFFER_SIZE  2048
#define INCOMING_COMMAND_SIZE      24

class DiManager {
  public:
  // Construct a drawing-instruction manager, which handles multiple drawing primitives.
  DiManager();

  // Destroy the manager and its child primitives.
  ~DiManager();

  // Create the root primitive.
  void create_root();

  // Create various types of drawing primitives.
  DiPrimitive* create_point(OtfCmd_10_Create_primitive_Point* cmd);
  DiPrimitive* create_line(OtfCmd_20_Create_primitive_Line* cmd);
  DiPrimitive* create_rectangle_outline(OtfCmd_40_Create_primitive_Rectangle_Outline* cmd);
  DiSolidRectangle* create_solid_rectangle(OtfCmd_41_Create_primitive_Solid_Rectangle* cmd);
  DiPrimitive* create_ellipse(OtfCmd_50_Create_primitive_Ellipse_Outline* cmd);
  DiPrimitive* create_solid_ellipse(OtfCmd_51_Create_primitive_Solid_Ellipse* cmd);
  DiPrimitive* create_triangle_outline(OtfCmd_30_Create_primitive_Triangle_Outline* cmd);
  DiPrimitive* create_solid_triangle(OtfCmd_31_Create_primitive_Solid_Triangle* cmd);
  DiPrimitive* create_triangle_list_outline(OtfCmd_32_Create_primitive_Triangle_List_Outline* cmd);
  DiPrimitive* create_solid_triangle_list(OtfCmd_33_Create_primitive_Solid_Triangle_List* cmd);
  DiPrimitive* create_triangle_fan_outline(OtfCmd_34_Create_primitive_Triangle_Fan_Outline* cmd);
  DiPrimitive* create_solid_triangle_fan(OtfCmd_35_Create_primitive_Solid_Triangle_Fan* cmd);
  DiPrimitive* create_triangle_strip_outline(OtfCmd_36_Create_primitive_Triangle_Strip_Outline* cmd);
  DiPrimitive* create_solid_triangle_strip(OtfCmd_37_Create_primitive_Solid_Triangle_Strip* cmd);
  DiPrimitive* create_quad_outline(OtfCmd_60_Create_primitive_Quad_Outline* cmd);
  DiPrimitive* create_solid_quad(OtfCmd_61_Create_primitive_Solid_Quad* cmd);
  DiPrimitive* create_quad_list_outline(OtfCmd_62_Create_primitive_Quad_List_Outline* cmd);
  DiPrimitive* create_solid_quad_list(OtfCmd_63_Create_primitive_Solid_Quad_List* cmd);
  DiPrimitive* create_quad_strip_outline(OtfCmd_64_Create_primitive_Quad_Strip_Outline* cmd);
  DiPrimitive* create_solid_quad_strip(OtfCmd_65_Create_primitive_Solid_Quad_Strip* cmd);
  DiTileMap* create_tile_map(OtfCmd_100_Create_primitive_Tile_Map* cmd);
  DiTileArray* create_tile_array(OtfCmd_80_Create_primitive_Tile_Array* cmd);
  DiTextArea* create_text_area(OtfCmd_150_Create_primitive_Text_Area* cmd, const uint8_t* font);
  void select_active_text_area(OtfCmd_151_Select_Active_Text_Area* cmd);
  DiBitmap* create_solid_bitmap(OtfCmd_120_Create_primitive_Solid_Bitmap* cmd);
  DiBitmap* create_masked_bitmap(OtfCmd_121_Create_primitive_Masked_Bitmap* cmd);
  DiBitmap* create_transparent_bitmap(OtfCmd_122_Create_primitive_Transparent_Bitmap* cmd);
  DiBitmap* create_reference_solid_bitmap(OtfCmd_135_Create_primitive_Reference_Solid_Bitmap* cmd);
  DiBitmap* create_reference_masked_bitmap(OtfCmd_136_Create_primitive_Reference_Masked_Bitmap* cmd);
  DiBitmap* create_reference_transparent_bitmap(OtfCmd_137_Create_primitive_Reference_Transparent_Bitmap* cmd);
  DiBitmap* create_solid_bitmap_for_tile_array(OtfCmd_81_Create_Solid_Bitmap_for_Tile_Array* cmd);
  DiBitmap* create_masked_bitmap_for_tile_array(OtfCmd_82_Create_Masked_Bitmap_for_Tile_Array* cmd);
  DiBitmap* create_transparent_bitmap_for_tile_array(OtfCmd_83_Create_Transparent_Bitmap_for_Tile_Array* cmd);
  DiBitmap* create_solid_bitmap_for_tile_map(OtfCmd_101_Create_Solid_Bitmap_for_Tile_Map *cmd);
  DiBitmap* create_masked_bitmap_for_tile_map(OtfCmd_102_Create_Masked_Bitmap_for_Tile_Map* cmd);
  DiBitmap* create_transparent_bitmap_for_tile_map(OtfCmd_103_Create_Transparent_Bitmap_for_Tile_Map* cmd);
  DiRender* create_solid_render(OtfCmd_200_Create_primitive_Solid_Render* cmd);
  DiRender* create_masked_render(OtfCmd_201_Create_primitive_Masked_Render* cmd);
  DiRender* create_transparent_render(OtfCmd_202_Create_primitive_Transparent_Render* cmd);
  DiPrimitive* create_primitive_group(OtfCmd_140_Create_primitive_Group* cmd);

  // Set the flags for an existing primitive.
  void set_primitive_flags(uint16_t id, uint16_t flags);

  // Move an existing primitive to an absolute position.
  void set_primitive_position(uint16_t id, int32_t x, int32_t y);

  // Move an existing primitive to a relative position.
  void adjust_primitive_position(uint16_t id, int32_t x, int32_t y);

  // Delete an existing primitive.
  void delete_primitive(uint16_t id);

  // Generate code for an existing primitive.
  void generate_code_for_primitive(uint16_t id);

  // Move an existing bitmap to an absolute position and slice it.
  void slice_solid_bitmap_absolute(OtfCmd_123_Set_position_and_slice_solid_bitmap* cmd);
  void slice_masked_bitmap_absolute(OtfCmd_124_Set_position_and_slice_masked_bitmap* cmd);
  void slice_transparent_bitmap_absolute(OtfCmd_125_Set_position_and_slice_transparent_bitmap* cmd);

  // Move an existing bitmap to a relative position and slice it.
  void slice_solid_bitmap_relative(OtfCmd_126_Adjust_position_and_slice_solid_bitmap* cmd);
  void slice_masked_bitmap_relative(OtfCmd_127_Adjust_position_and_slice_masked_bitmap* cmd);
  void slice_transparent_bitmap_relative(OtfCmd_128_Adjust_position_and_slice_transparent_bitmap* cmd);

  // Set a pixel within an existing bitmap.
  void set_solid_bitmap_pixel(OtfCmd_129_Set_solid_bitmap_pixel* cmd, int16_t nth);
  void set_masked_bitmap_pixel(OtfCmd_130_Set_masked_bitmap_pixel* cmd, int16_t nth);
  void set_transparent_bitmap_pixel(OtfCmd_131_Set_transparent_bitmap_pixel* cmd, int16_t nth);
  void set_solid_bitmap_pixel_for_tile_array(OtfCmd_85_Set_solid_bitmap_pixel_in_Tile_Array* cmd, int16_t nth);
  void set_masked_bitmap_pixel_for_tile_array(OtfCmd_86_Set_masked_bitmap_pixel_in_Tile_Array* cmd, int16_t nth);
  void set_transparent_bitmap_pixel_for_tile_array(OtfCmd_87_Set_transparent_bitmap_pixel_in_Tile_Array* cmd, int16_t nth);
  void set_solid_bitmap_pixel_for_tile_map(OtfCmd_105_Set_solid_bitmap_pixel_in_Tile_Map* cmd, int16_t nth);
  void set_masked_bitmap_pixel_for_tile_map(OtfCmd_106_Set_masked_bitmap_pixel_in_Tile_Map* cmd, int16_t nth);
  void set_transparent_bitmap_pixel_for_tile_map(OtfCmd_107_Set_transparent_bitmap_pixel_in_Tile_Map* cmd, int16_t nth);

  // Set bitmap ID for tile in tile array.
  void set_tile_array_bitmap_id(OtfCmd_84_Set_bitmap_ID_for_tile_in_Tile_Array* cmd);

  // Set bitmap ID for tile in tile map.
  void set_tile_map_bitmap_id(OtfCmd_104_Set_bitmap_ID_for_tile_in_Tile_Map* cmd);

  // Setup a callback for when the visible frame pixels have been sent to DMA,
  // and the vertical blanking time begins.
  void set_on_vertical_blank_cb(DiVoidCallback callback_fcn);

  // Setup and run the main loop to do continuous drawing.
  // For the demo, the loop never ends.
  void IRAM_ATTR run();

  // Store an incoming character for use later.
  void store_character(uint8_t character);

  // Store an incoming character string for use later.
  // The string is null-terminated.
  void store_string(const uint8_t* string);
  inline void store_string(const char* string) { store_string((const uint8_t*) string); }

  // Validate a primitive ID.
  inline bool validate_id(int16_t id) { return (id >= 0) && (id < MAX_NUM_PRIMITIVES); }

  // Get a safe primitive pointer.
  inline DiPrimitive* get_safe_primitive(int16_t id) { return validate_id(id) ? m_primitives[id] : NULL; }

  protected:
  // Structures used to support DMA for video.
  volatile lldesc_t *         m_dma_descriptor; // [DMA_TOTAL_DESCR]
  DiVideoScanLine *           m_video_lines; // [NUM_ACTIVE_BUFFERS]
  DiVideoScanLine *           m_front_porch;
  DiVideoScanLine *           m_vertical_sync;
  DiVideoScanLine *           m_back_porch;
  DiVoidCallback              m_on_vertical_blank_cb;
  uint32_t                    m_next_buffer_write;
  uint32_t                    m_next_buffer_read;
  uint32_t                    m_num_buffer_chars;
  uint32_t                    m_command_data_index;
  DiTextArea*                 m_text_area;
  DiPrimitive*                m_cursor;
  uint8_t                     m_flash_count;
  uint8_t                     m_incoming_data[INCOMING_DATA_BUFFER_SIZE];
  std::vector<uint8_t>        m_incoming_command;
  DiPrimitive *               m_primitives[MAX_NUM_PRIMITIVES]; // Indexes of array are primitive IDs
  std::vector<DiPrimitive*>*  m_groups; // Vertical scan groups (for optimizing paint calls)

  // Setup the DMA stuff.
  void initialize();

  // Run the main loop.
  void IRAM_ATTR loop();

  // Clear the primitive data, etc.
  void clear();

  // Add a primitive to the manager.
  void add_primitive(DiPrimitive* prim, DiPrimitive* parent);

  // Delete a primitive from the manager.
  void remove_primitive(DiPrimitive* prim);

  // Recompute the geometry and paint list membership for a primitive.
  void recompute_primitive(DiPrimitive* prim, uint16_t old_flags,
                            int32_t old_min_group, int32_t old_max_group);
  // Finish creating a primitive.
  DiPrimitive* finish_create(uint16_t id, DiPrimitive* prim, DiPrimitive* parent_prim);

  // Draw all primitives that belong to the active scan line group.
  void IRAM_ATTR draw_primitives(volatile uint32_t* p_scan_line, uint32_t line_index);

  // Setup a single DMA descriptor.
  void init_dma_descriptor(DiVideoScanLine* vscan, uint32_t scan_index, uint32_t descr_index);

  // Process all stored characters.
  void process_stored_characters();

  // Process an incoming character, which could be printable data or part of some
  // VDU command. If the character is printable, it will be written to the text_area
  // display. If the character is non-printable, or part of a VDU command, it will
  // be treated accordingly. This function returns true if the character was fully
  // processed, and false otherwise.
  bool process_character(uint8_t character);

  // Process an incoming string, which could be printable data and/or part of some
  // VDU command(s). This function calls process_character(), for each character
  // in the given string. The string is null-terminated.
  void process_string(const uint8_t* string);

  uint8_t get_param_8(uint32_t index);
  int16_t get_param_16(uint32_t index);
  bool handle_udg_sys_cmd();
  bool handle_otf_cmd();
  bool define_graphics_viewport();
  bool define_text_viewport();
  void move_cursor_tab(VduCmd_31_Set_text_tab_position* cmd);
  void clear_screen();
  void move_cursor_left();
  void move_cursor_right();
  void move_cursor_down();
  void move_cursor_up();
  void move_cursor_home();
  void move_cursor_boln();
  void do_backspace();
  uint8_t read_character(int16_t x, int16_t y);
  void write_character(uint8_t character);
  void report(uint8_t character);
  static uint8_t to_hex(uint8_t value);
  uint8_t peek_into_buffer();
  uint8_t read_from_buffer();
  void skip_from_buffer();
  void send_cursor_position();
  void send_screen_char(int16_t x, int16_t y);
  void send_screen_pixel(int16_t x, int16_t y);
  void send_mode_information();
  void send_general_poll(uint8_t b);
};
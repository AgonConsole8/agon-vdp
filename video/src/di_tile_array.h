// di_tile_array.h - Function declarations for drawing tile arrays
//
// A tile array is a set of rectangular tiles, where each tile is a bitmap of
// the same size (width and height). Tiles are arranged in a rectangular
// grid, where the entire portion of the grid that fits within the visible
// area of the screen may be displayed at any given moment. In other words
// multiple tiles show at the same time.
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
#include "di_bitmap.h"
#include <map>

typedef uint32_t DiRowColumn;

#ifndef _DiTileBitmapID
#define _DiTileBitmapID
typedef uint32_t DiTileBitmapID;
typedef std::map<DiTileBitmapID, DiBitmap*> DiTileIdToBitmapMap;
#endif

class DiTileArray: public DiPrimitive {
  public:
  // Construct a tile array.
  DiTileArray(uint32_t screen_width, uint32_t screen_height,
            uint32_t columns, uint32_t rows,
            uint32_t tile_width, uint32_t tile_height, uint16_t flags);

  // Destroy a tile array.
  virtual ~DiTileArray();

  // Reassemble the custom instructions needed to draw the primitive.
  virtual void generate_instructions();

  // Create the array of pixels for the tile bitmap.
  DiBitmap* create_bitmap(DiTileBitmapID bm_id);

  // Save the pixel value of a particular pixel in a specific tile bitmap. A tile bitmap
  // may appear many times on the screen, based on the use of the bitmap ID.
  void set_pixel(DiTileBitmapID bm_id, int32_t x, int32_t y, uint8_t color);

  // Set the bitmap ID to use to draw a tile at a specific row and column.
  void set_tile(int16_t column, int16_t row, DiTileBitmapID bm_id);

  // Unset the bitmap ID at a specific row and column, to remove the tile.
  void unset_tile(int16_t column, int16_t row);

  // Set the bitmap ID to use to fill a rectangle of tiles.
  void set_tiles(int16_t column, int16_t row, DiTileBitmapID bm_id, int16_t columns, int16_t rows);

  // Unset the bitmap IDs to remove a rectangle of tiles.
  void unset_tiles(int16_t column, int16_t row, int16_t columns, int16_t rows);

  // Get the bitmap ID presently at the given row and column.
  DiTileBitmapID get_tile(int16_t column, int16_t row);

  // Get the relative coordinates of a specific tile position.
  void get_rel_tile_coordinates(int16_t column, int16_t row,
            int16_t& x, int16_t& y, int16_t& x_extent, int16_t& y_extent);

  // Get the absolute coordinates of a specific tile position.
  void get_abs_tile_coordinates(int16_t column, int16_t row,
            int16_t& x, int16_t& y, int16_t& x_extent, int16_t& y_extent);

  virtual void IRAM_ATTR paint(volatile uint32_t* p_scan_line, uint32_t line_index);

  protected:
  uint32_t  m_columns;              // number of columns (cells in each row)
  uint32_t  m_rows;                 // number of rows (cells in each column)
  uint32_t  m_bytes_per_line;       // number of 1-pixel bytes in each bitmap line
  uint32_t  m_bytes_per_position;   // number of 1-pixel bytes in each bitmap position
  uint32_t  m_visible_columns;      // number of columns that fit on the screen
  uint32_t  m_visible_rows;         // number of rows that fit on the screen
  uint32_t  m_tile_width;           // width of 1 tile in pixels
  uint32_t  m_tile_height;          // height of 1 tile in pixels
  uint8_t   m_transparent_color;    // value indicating not to draw the pixel
  DiTileIdToBitmapMap m_id_to_bitmap_map; // caches bitmaps based on bitmap ID
  uint32_t** m_tile_pixels;         // 2D array of addresses of tile bitmap pixels
};

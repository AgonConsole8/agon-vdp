#ifndef AGON_LAYERS_H
#define AGON_LAYERS_H

// Title:			Agon Tile Engine
// Author:			Julian Regel
// Created:			07/09/2024
// Last Updated:	07/09/2024

// This code included in VDP by adding:
// #include "vdu_layers.h"
// in vdu_sys.h and is called by VDUStreamProcessor::vdu_sys_video()

#define VDP_LAYERS							0xC2		// VDU 23,0,194
#define VDP_LAYER_TILEBANK_INIT				0x00		// VDU 23,0,194,0
#define VDP_LAYER_TILEBANK_LOAD				0x01		// VDU 23,0,194,1
#define VDP_LAYER_TILEBANK_LOAD_BUFFER		0x02		// VDU 23,0,194,2	[Future]
#define VDP_LAYER_TILEBANK_DRAW				0x06		// VDU 23,0,194,6
#define VDP_LAYER_TILEBANK_FREE				0x07		// VDU 23,0,194,7
#define VDP_LAYER_TILEPALETTE_INIT			0x08		// VDU 23,0,194,8	[Future]
#define VDP_LAYER_TILEPALETTE_SET			0x09		// VDU 23,0,194,9	[Future]
#define VDP_LAYER_TILEPALETTE_SET_MULTIPLE	0x0A		// VDU 23,0,194,10	[Future]
#define VDP_LAYER_TILEPALETTE_ACTIVATE		0x0E		// VDU 23,0,194,14 	[Future]
#define VDP_LAYER_TILEPALETTE_FREE			0x0F		// VDU 23,0,194,15 	[Future]
#define VDP_LAYER_TILEMAP_INIT				0x10		// VDU 23,0,194,16
#define VDP_LAYER_TILEMAP_SET_TILE			0x11		// VDU 23,0,194,17
#define VDP_LAYER_TILEMAP_SET_MULTIPLE		0x12		// VDU 23,0,194,18	[Future]
#define VDP_LAYER_TILEMAP_FREE				0x17		// VDU 23,0,194,23
#define VDP_LAYER_TILELAYER_INIT			0x18		// VDU 23,0,194,24
#define VDP_LAYER_TILELAYER_SET_PROPERTY	0x19		// VDU 23,0,194,25
#define VDP_LAYER_TILELAYER_SET_SCROLL		0x1A		// VDU 23,0,194,26
#define VDP_LAYER_TILELAYER_SET_TABLE		0x1B		// VDU 23,0,194,27	[Future]
#define VDP_LAYER_TILELAYER_SET_DRAW_PARAM	0x1D		// VDU 23,0,194,29 	[Future]
#define VDP_LAYER_TILELAYER_DRAW			0x1E		// VDU 23,0,194,30
#define VDP_LAYER_TILELAYER_FREE			0x1F		// VDU 23,0,194,31


void VDUStreamProcessor::vdu_sys_layers(void) {

	auto cmd = readByte_t();

	switch (cmd) {

		// Tile Bank Functions

		case VDP_LAYER_TILEBANK_INIT: {

			// VDU 23,0,194,0,<tileBankNum>,<tileBankBitDepth>,<reservedParameter1>,<reservedParameter2>

			uint8_t tileBankNum = readByte_t();			// 0 [Future: 0-3]
			uint8_t tileBankBitDepth = readByte_t();	// 0 = 64 colours, [Future: 1 = 16 colours, 2 = 4 colours, 3 = 2 colours]
			uint8_t reservedParameter1 = readByte_t();	// Ignored in release 1.0. Should be set to 0.
			uint8_t reservedParameter2 = readByte_t();	// Ignored in release 1.0. Should be set to 0.
			
			vdu_sys_layers_tilebank_init(tileBankNum, tileBankBitDepth);

		} break;

		case VDP_LAYER_TILEBANK_LOAD: {

			// VDU 23,0,194,1,<tileBankNum>,<tileNumber>,<pixel0>,...<pixel63>

			uint8_t tileBankNum = readByte_t();			// 0 [Future: 0-3]
			uint8_t tileId = readByte_t();				// 0-255

			vdu_sys_layers_tilebank_load(tileBankNum, tileId);

		} break;

		case VDP_LAYER_TILEBANK_LOAD_BUFFER: {

		} break;

		case VDP_LAYER_TILEBANK_DRAW: {

			// VDU 23,0,194,6,<tileBankNum>,<tileId>,<palette>,<xpos>,<ypos>,<xoffset>,<yoffset>,<attribute>

			uint8_t tileBankNum = readByte_t();
			uint8_t tileId = readByte_t();
			uint8_t palette = readByte_t();
			uint8_t xPos = readByte_t();
			uint8_t yPos = readByte_t();
			uint8_t xOffset = readByte_t();
			uint8_t yOffset = readByte_t();
			uint8_t tileAttribute = readByte_t();

			vdu_sys_layers_tilebank_draw(tileBankNum, tileId, palette, xPos, yPos, xOffset, yOffset, tileAttribute);

		} break;

		case VDP_LAYER_TILEBANK_FREE: {

			uint8_t tileBankNum = readByte_t();

			vdu_sys_layers_tilebank_free(tileBankNum);

		} break;

		// Tile Map Functions

		case VDP_LAYER_TILEMAP_INIT: {

			// VDU 23,0,194,16,<tilelayernumber>,<tilemapsize>,<reservedParameter0>,<reservedParameter1>
			// Size is one of:
			//		0 = 32x32, 1=32x64, 2=32x128, 3=64x32, 4=64x64, 5=64x128, 6=128x32, 7=128x64, 8=128x128

			uint8_t tileLayerNum = readByte_t();
			uint8_t tileMapSize = readByte_t();
			uint8_t reservedParameter1 = readByte_t();	// Ignored in release 1.0. Should be set to 0.
			uint8_t reservedParameter2 = readByte_t();	// Ignored in release 1.0. Should be set to 0.

			vdu_sys_layers_tilemap_init(tileLayerNum, tileMapSize);


		} break;

		case VDP_LAYER_TILEMAP_SET_TILE: {

			// VDU 23,0,194,17,<tilelayernumber>,<xpos>,<ypos>,<tileid>,<tileattribute>

			uint8_t tileLayerNum = readByte_t();
			uint8_t xPos = readByte_t();
			uint8_t yPos = readByte_t();
			uint8_t tileId = readByte_t();
			uint8_t tileAttribute = readByte_t();

			vdu_sys_layers_tilemap_set(tileLayerNum, xPos, yPos, tileId, tileAttribute);

		} break;		

		case VDP_LAYER_TILEMAP_SET_MULTIPLE: {

		} break;

		case VDP_LAYER_TILEMAP_FREE: {

			// VDU 23,0,194,23,<tileMapNum>

			uint8_t tileMapNum = readByte_t();

			vdu_sys_layers_tilemap_free(tileMapNum);

		} break;

		// Tile Layer Functions

		case VDP_LAYER_TILELAYER_INIT: {
	
			// VDU 23,0,194,24,<tileLayerNum>,<tileLayerSize>,<tileSize>,<reservedParameter1>
			//
			// tileLayerSize is one of:
			// 0 = 80x60 (640x480 modes), 1 = 80x30 (640x240 modes), 2 = 40x30 (320x240 modes), 3 = 40x25 (320x200 modes)

			uint8_t tileLayerNum = readByte_t();
			uint8_t tileLayerSize = readByte_t();
			uint8_t tileSize = readByte_t();
			uint8_t reservedParameter1 = readByte_t();	// Ignored in release 1.0. Should be set to 0.
		

			vdu_sys_layers_tilelayer_init(tileLayerNum, tileLayerSize, tileSize);
	
		} break;

		case VDP_LAYER_TILELAYER_SET_PROPERTY: {

		} break;

		case VDP_LAYER_TILELAYER_SET_SCROLL: {

			// VDU 23,0,194,26,<tileLayerNum>,<xpos>,<ypos>,<xoffset>,<yoffset>

			uint8_t tileLayerNum = readByte_t();
			uint8_t xPos = readByte_t();
			uint8_t yPos = readByte_t();
			uint8_t xOffset = readByte_t();
			uint8_t yOffset = readByte_t();

			vdu_sys_layers_tilelayer_set_scroll(tileLayerNum, xPos, yPos, xOffset, yOffset);

		} break;

		case VDP_LAYER_TILELAYER_SET_TABLE: {

		} break;

		case VDP_LAYER_TILELAYER_DRAW: {

			// VDU 23,0,194,30,<tileLayerNum>
			// Parameters are currently undefined but would facilitate drawing priority

			uint8_t tileLayerNum = readByte_t();

			vdu_sys_layers_tilelayer_draw(tileLayerNum);

		} break;

		case VDP_LAYER_TILELAYER_FREE: {
			// Not required? We don't dynamically allocate memory in VDP_LAYER_TILELAYER_INIT
			// so there is nothing to free?
		} break;
	}
}

void VDUStreamProcessor::vdu_sys_layers_tilebank_init(uint8_t tileBankNum, uint8_t tileBankBitDepth) {

	// Initial release to only support 8bpp tiles
	if (tileBankBitDepth != 0) return;

	// Initial release to only support 8x8 tiles
	uint8_t tileBankTileWidth = 8;
	uint8_t tileBankTileHeight = 8;

	// Create a buffer of width, height and colour depth for 256 tiles
	int tileBankBufferSize = tileBankTileWidth * tileBankTileHeight * 256;

	switch (tileBankNum) {		// Which tile bank is being initiated?

		case 0: {				// Tile Bank 0
			
			// Check if already exists
			
			if (tileBank0Data !=NULL) {

				// If already exists, then free and reallocate
				vdu_sys_layers_tilebank_free(tileBankNum);
			}
			
			// Now allocate the new memory
			tileBank0Data = heap_caps_malloc(tileBankBufferSize,MALLOC_CAP_SPIRAM);
						
			// Only continue if the reinit was successful
			
			if (tileBank0Data != NULL) {

				// Cast the void pointer to an integer
				tileBank0Ptr = (uint8_t *)tileBank0Data;

				// Set every byte in the tile bank to 0
				for (auto i=0; i <tileBankBufferSize; i++)
					*(tileBank0Ptr + i) = 0; 
			}
			else {
				discardBytes(tileBankBufferSize);
			}
		} break;

		default: {
			debug_log("vdu_sys_layers_tilebank_init: Invalid tilebank %d specified.\r\n",tileBankNum);
			return;
		}
	}
}

void VDUStreamProcessor::vdu_sys_layers_tilebank_load(uint8_t tileBankNum, uint8_t tileId) {

	// Note: In the initial release of the Tile Engine, the tileBankNum is ignored as there is only
	// a single tile bank.

	uint8_t sourcePixel = 0;
	int destPixel = 0;

	// Only do something if the tilebank exists

	if (tileBank0Data != NULL) {
	
		// Initial implementation is hardcoded to 8x8 tiles and 64 colours (i.e., 64 pixels * 1 byte per pixel)

		for (auto n=0; n<64; n++) {
			sourcePixel = readByte_t();
			destPixel = (tileId * 64) + n;
			tileBank0Ptr[destPixel]= sourcePixel;
		}
	} else {
		debug_log("vdu_sys_layers_tilebank_load: tileBank0Data is not defined.\r\n");
	}
}

void VDUStreamProcessor::vdu_sys_layers_tilebank_draw(uint8_t tileBankNum, uint8_t tileId, uint8_t palette, uint8_t xPos, uint8_t yPos, uint8_t xOffset, uint8_t yOffset, uint8_t tileAttribute) {

	// Initial release only supports tileBank 0

	if (tileBankNum != 0) {
		debug_log("vdu_sys_layers_tilebank_draw: Invalid tileBankNum %d specified.\r\n",tileBankNum);
		return;
	}
	
	// tileId 0 is special so cannot be drawn
	if (tileId == 0) return;

	int xPix, yPix;

	if (tileBank0Data != NULL) {

		// Check attribute bits 0 and 1 to get tile draw direction and call appropriate draw function

		uint8_t tileFlip = tileAttribute & 0x03;

		switch (tileFlip) {
			case 0x00:		// Normal drawing
				writeTileToBuffer(tileId, 0, 0, currentTileDataBuffer, 1);
				break;
			case 0x01:		// Flip X
				writeTileToBufferFlipX(tileId, 0, 0, currentTileDataBuffer, 1);
				break;
			case 0x02:		// Flip Y
				writeTileToBufferFlipY(tileId, 0, 0, currentTileDataBuffer, 1);
				break;
			case 0x03:		// Flip X and Y
				writeTileToBufferFlipXY(tileId, 0, 0, currentTileDataBuffer, 1);
				break;
		}

		xPix = (xPos * 8) - xOffset;
		yPix = (yPos * 8) - yOffset;

		currentTile = Bitmap(8, 8, currentTileDataBuffer, PixelFormat::RGBA2222);

		// Draw Tile

		canvas->drawBitmap(xPix,yPix,&currentTile);

		waitPlotCompletion();

	} else {
		debug_log("vdu_sys_layers_tilebank_draw: tileBank0Data not initialised.\r\n");
	}
}

void VDUStreamProcessor::vdu_sys_layers_tilebank_free(uint8_t tileBankNum) {

	switch (tileBankNum) {
		case 0: {
			if (tileBank0Data != NULL) {
				debug_log("vdu_sys_layers_tilebank_free: Freeing tileBank0Data.\r\n");
				heap_caps_free(tileBank0Data);
				tileBank0Data = NULL;
			}
		} break;
		
		default: {
			debug_log("vdu_sys_layers_tilebank_free: Invalid tileBankNum %d specified.\r\n",tileBankNum);
		}
	}
}

void VDUStreamProcessor::vdu_sys_layers_tilemap_init(uint8_t tileLayerNum, uint8_t tileMapSize) {

	// Check if tileMap0 already exists and free if it is.

	switch (tileLayerNum) {

		case 0: {		// If tilemap/layer number is 0
			
			// Check if already exists
	
			if (tileMap0 !=NULL) {
				// If already exists, then free and reinitialise

				vdu_sys_layers_tilemap_free(tileLayerNum); 
			}
			
		} break;

		default: {
			debug_log("vdu_sys_layers_tilemap_init: Invalid tileLayerNum %d specified.\r\n",tileLayerNum);
			return;
		}
	}

	//	The following tile map sizes are supported:
	//	0=32x32, 1=32x64, 2=32x128, 3=64x32, 4=64x64, 5=64x128, 6=128x32, 7=128x64, 8=128x128

	switch (tileMapSize) {

		case 0: {		// 32x32 tilemap
			
			tileMap0Properties.width = 32;
			tileMap0Properties.height = 32;

		} break;

		case 1: {		// 32x64 tilemap
			
			tileMap0Properties.width = 32;
			tileMap0Properties.height = 64;

		} break;

		case 2: {		// 32x128 tilemap
			
			tileMap0Properties.width = 32;
			tileMap0Properties.height = 128;

		} break;

		case 3: {		// 64x32 tilemap
			
			tileMap0Properties.width = 64;
			tileMap0Properties.height = 32;

		} break;

		case 4: {		// 64x64 tilemap
			
			tileMap0Properties.width = 64;
			tileMap0Properties.height = 64;

		} break;

		case 5: {		// 64x128 tilemap
			
			tileMap0Properties.width = 64;
			tileMap0Properties.height = 128;

		} break;

		case 6: {		// 128x32 tilemap
			
			tileMap0Properties.width = 128;
			tileMap0Properties.height = 32;

		} break;

		case 7: {		// 128x64 tilemap
			
			tileMap0Properties.width = 128;
			tileMap0Properties.height = 64;

		} break;

		case 8: {		// 128x128 tilemap
			
			tileMap0Properties.width = 128;
			tileMap0Properties.height = 128;

		} break;

		default: {
			debug_log("vdu_sys_layers_tilemap_init: Invalid tileMapSize %d specified.\r\n",tileMapSize);
			return;
		}
	}

	uint8_t tileMapWidth = tileMap0Properties.width;
	uint8_t tileMapHeight = tileMap0Properties.height;

	int tileMapBufferSize = tileMapWidth * sizeof(struct Tile*);

	// Initial release to only support a single tile layer
	if (tileLayerNum != 0) {
		debug_log("vdu_sys_layers_tilemap_init: Invalid tileLayerNum %d specified.\r\n",tileLayerNum);
		return;
	}

	switch (tileLayerNum) {

		case 0: {		// If tile map number is 0
						
			// Allocate memory for each column in the tile map
			tileMap0 = (struct Tile**)heap_caps_malloc(tileMapBufferSize,MALLOC_CAP_SPIRAM);

			// Allocate memory for each row in the tile map
			for (auto i=0; i<tileMapWidth; i++) {
				tileMap0[i] = (struct Tile*)heap_caps_malloc(tileMapHeight * sizeof(struct Tile),MALLOC_CAP_SPIRAM);
			}
						
			// Only continue if the init was successful
			
			if (tileMap0 != NULL) {

				// Set every byte in the tile map to 0

				for (auto i=0; i<tileMapWidth; i++) {
					for (auto j=0; j<tileMapHeight; j++) {
						tileMap0[i][j].id = 0;
						tileMap0[i][j].attribute = 0 ;
					}
				}
			}
			else {
				debug_log("vdu_sys_layers_tilemap_init: Error when attempting to allocate memory for tilemap.\r\n");
				discardBytes(tileMapBufferSize);
			}
		} break;
	}
}

void VDUStreamProcessor::vdu_sys_layers_tilemap_set(uint8_t tileLayerNum, uint8_t xPos, uint8_t yPos, uint8_t tileId, uint8_t tileAttribute) {
	
	switch (tileLayerNum) {
		case 0: {
			if (tileMap0 != NULL) {
				// Skip if passed x and y are greater than the size of the tilemap
				if (xPos >= tileMap0Properties.width || yPos >= tileMap0Properties.height) return;

				if (tileMap0 != NULL) {
					tileMap0[xPos][yPos].id = tileId;
					tileMap0[xPos][yPos].attribute = tileAttribute;
				}
			}
		} break;

		default: {
			debug_log("vdu_sys_layers_tilemap_set: Invalid tileLayerNum %d specified.\r\n",tileLayerNum);
			return;
		}
	}
}

void VDUStreamProcessor::vdu_sys_layers_tilemap_free(uint8_t tileLayerNum) {

	switch (tileLayerNum) {
		case 0: {

			uint8_t tileMapWidth = tileMap0Properties.width;

			if (tileMap0 != NULL) {

				debug_log("vdu_sys_layers_tilemap_free: Freeing tileMap0.\r\n");

				for (auto i=0; i<tileMapWidth; i++) {
					heap_caps_free(tileMap0[i]);
				}
				heap_caps_free(tileMap0);

				tileMap0 = NULL;
			}
		} break;

		default: {
			debug_log("vdu_sys_layers_tilemap_free: Invalid tileLayerNum %d specified.\r\n",tileLayerNum);
			return;
		}
	}		
}

void VDUStreamProcessor::vdu_sys_layers_tilelayer_init(uint8_t tileLayerNum, uint8_t tileLayerSize, uint8_t tileSize) {
	
	uint8_t tileLayerHeight;
	uint8_t tileLayerWidth;

	switch (tileLayerSize) {

		case 0: {		// 80x60 layer
			
			tileLayerHeight = 60;
			tileLayerWidth = 80;

		} break;

		case 1: {		// 80x30 layer
			
			tileLayerHeight = 30;
			tileLayerWidth = 80;

		} break;

		case 2: {		// 40x30 layer
			
			tileLayerHeight = 30;
			tileLayerWidth = 40;

		} break;

		case 3: {		// 40x25 layer
			
			tileLayerHeight = 25;
			tileLayerWidth = 40;

		} break;
		
		default: {
			debug_log("vdu_sys_layers_tilelayer_init: Invalid tileLayerSize %d specified.\r\n",tileLayerSize);
			return;
		}
	}

	int rowBufferWidth = tileLayerWidth * 8;						// Number of bytes (pixels) wide

	int rowBufferHeight = 8;										// Number of scanlines in the buffer

	int rowDataBufferSize = rowBufferWidth * rowBufferHeight;		// Size of the buffer in bytes

	// Create the row bitmap

	currentRow = Bitmap(rowBufferWidth, rowBufferHeight, currentRowDataBuffer, PixelFormat::RGBA2222);

	// Zero out the active parts of the buffer

	for (auto i=0; i<rowDataBufferSize; i++) {
		currentRowDataBuffer[i] = 0;
	}

	switch (tileLayerNum) {

		case 0: {		// Tile Layer 0
			
			tileLayer0.height = tileLayerHeight;
			tileLayer0.width = tileLayerWidth;
			tileLayer0.sourceXPos = 0;
			tileLayer0.sourceYPos = 0;
			tileLayer0.xOffset = 0;
			tileLayer0.yOffset = 0;
			tileLayer0.attribute = 0;

			tileLayer0init = 1;		// Set as initialised

		} break;

		default: {
			debug_log("vdu_sys_layers_tilelayer_init: Invalid tileLayerNum %d specified.\r\n",tileLayerNum);
			return;
		}

	}
}

void VDUStreamProcessor::vdu_sys_layers_tilelayer_set_scroll(uint8_t tileLayerNum, uint8_t xPos, uint8_t yPos, uint8_t xOffset, uint8_t yOffset) {

	switch (tileLayerNum) {
		
		case 0: {

			if (tileLayer0init != 0) {		// Only continue if the tile layer is initialised
				if (tileMap0 != NULL) {		// Only continue if the tile map is initialised

					uint8_t tileMapWidth = tileMap0Properties.width;
					uint8_t tileMapHeight = tileMap0Properties.height;

					if (xPos >= tileMapWidth) { xPos = 0; }
					if (yPos >= tileMapHeight) { yPos = 0; }

					if (xOffset > 7) { xOffset = 0; }
					if (yOffset > 7) { yOffset = 0;	}

					tileLayer0.sourceXPos = xPos;
					tileLayer0.sourceYPos = yPos;
					tileLayer0.xOffset = xOffset;
					tileLayer0.yOffset = yOffset;
				} else {
					debug_log("vdu_sys_layers_tilelayer_set_scroll: tileMap0 is not initialised.\r\n");
					return;
				}
			} else {
				debug_log("vdu_sys_layers_tilelayer_set_scroll: tileLayer is not initialised.\r\n");
				return;
			}

		} break;

		default: {
			debug_log("vdu_sys_layers_tilelayer_set_scroll: Invalid tileLayerNum %d specified.\r\n",tileLayerNum);
			return;
		}
	}
}

void VDUStreamProcessor::vdu_sys_layers_tilelayer_draw(uint8_t tileLayerNum) {

	// Timing debug statements

	// auto startTime = xTaskGetTickCountFromISR();
  	// auto endTime = xTaskGetTickCountFromISR();
	// auto sumTime = xTaskGetTickCountFromISR();
	// sumTime = 0;

	int xPix = 0;		// X position in pixels is now always 0 as the offset is written directly to the tileRowBuffer
	int yPix = 0;

	uint8_t tileId;
	uint8_t tileAttribute;
	uint8_t tileLayerHeight;
	uint8_t tileLayerWidth;
	uint8_t sourceXPos;
	uint8_t sourceYPos;
	uint8_t xOffset;
	uint8_t yOffset;
	uint8_t tileMapWidth;
	uint8_t tileMapHeight;
	uint8_t tileRowOffset = 0;

	switch (tileLayerNum) {

		case 0: {

			if (tileLayer0init != 0) {
				if (tileMap0 != NULL) {

					tileLayerHeight = tileLayer0.height;
					tileLayerWidth = tileLayer0.width;

					sourceXPos = tileLayer0.sourceXPos;
					sourceYPos = tileLayer0.sourceYPos;

					xOffset = tileLayer0.xOffset;
					yOffset = tileLayer0.yOffset;

					tileMapWidth = tileMap0Properties.width;
					tileMapHeight = tileMap0Properties.height;
				} else {
					debug_log("vdu_sys_layers_tilelayer_draw: tileMap0 is not initialised.\r\n");
					return;
				}
			} else {
				debug_log ("vdu_sys_layers_tilelayer_draw: tileLayer0 is not initialised.\r\n");
				return;
			}
		} break;

		default: {
			debug_log("Invalid tileLayerNum: %d\r\n",tileLayerNum);
			return;
		}
	}

	int rowBufferWidth = tileLayerWidth * 8;
	int rowBufferHeight = 8;
	int rowDataBufferSize = rowBufferWidth * rowBufferHeight;

	// Perform validation checks

	if (sourceXPos >= tileMapWidth) { sourceXPos = 0; }
	if (sourceYPos >= tileMapHeight) { sourceYPos = 0; }

	// Do not continue if tileBank is not initialised.
	if (tileBank0Data == NULL) { 
		debug_log("vdu_sys_layers_tilelayer_draw: tileBank0Data is not initialised.\r\n");
		return;
	}

	// Process rows for each frame

	for (auto y=0; y<=tileLayerHeight; y++) {				

		// Clear Row Buffer

		for (auto i=0; i<rowDataBufferSize; i++) {
			currentRowDataBuffer[i] = 0;
		}

		// Process tile map for current row

		for (auto x=0; x<=tileLayerWidth; x++) {
																
			// read the Tile Map

			tileId = tileMap0[sourceXPos][sourceYPos].id;

			tileAttribute = tileMap0[sourceXPos][sourceYPos].attribute;

			tileRowOffset = x;	

			if (tileId == 0) {		// Tile 0 is special...

				// What you do here depends on the tile attribute in "special" mode:

				switch (tileAttribute) {

					case 0: {		// Tile is transparent and not drawn

					} break;
				}
			} else {				// Tile is normal and should be processed accordingly

				// Check attribute to get tile draw direction and call appropriate draw function

				uint8_t tileFlip = tileAttribute & 0x03;

				switch (tileFlip) {
					case 0x00:		// Normal drawing
						writeTileToBuffer(tileId, tileRowOffset, xOffset, currentRowDataBuffer, tileLayerWidth);
						break;
					case 0x01:		// Flip X
						writeTileToBufferFlipX(tileId, tileRowOffset, xOffset, currentRowDataBuffer, tileLayerWidth);
						break;
					case 0x02:		// Flip Y
						writeTileToBufferFlipY(tileId, tileRowOffset, xOffset, currentRowDataBuffer, tileLayerWidth);
						break;
					case 0x03:		// Flip X and Y
						writeTileToBufferFlipXY(tileId, tileRowOffset, xOffset, currentRowDataBuffer, tileLayerWidth);
						break;
					default:
						debug_log("Invalid tileAttribute value: %d\r\n",tileFlip);
				}
			}

			// If we're at the edge of the tile map, reset to the beginning.

			sourceXPos++;
			if (sourceXPos == tileMapWidth) { 
				sourceXPos = 0; 
			}
		}

		// At the end of the row, reset sourceXPos back (else it will keep incrementing)
		sourceXPos = tileLayer0.sourceXPos;
		
		// Set Y position to display on screen
		yPix = (y * 8) - tileLayer0.yOffset;		// Y position in pixels 
	
		// Draw Row

		// startTime = xTaskGetTickCountFromISR();

		currentRow = Bitmap(rowBufferWidth, rowBufferHeight, currentRowDataBuffer, PixelFormat::RGBA2222);

		canvas->drawBitmap(xPix,yPix,&currentRow);		

		waitPlotCompletion();

		// endTime = xTaskGetTickCountFromISR();
		// auto elapsedTime = endTime - startTime;
		// sumTime = sumTime + elapsedTime;

		sourceYPos++;
		if (sourceYPos == tileMapHeight) {
			sourceYPos = 0; 
		}
	}

	// debug_log("vdu_sys_layers_tilelayer_draw: Sum time is %d\r\n",sumTime);
}

// Tile drawing functions

void VDUStreamProcessor::writeTileToBuffer(uint8_t tileId, uint8_t tileCount, uint8_t xOffset, uint8_t tileBuffer[], uint8_t tileLayerWidth) {

	int destStartPos;
	int destRowStart;
	int destPixel;
	int bufferWidth = tileLayerWidth * 8;

	// Read 64 bytes from the data bank and write to the tile buffer

	int sourcePixel = tileId * 64;

	// Handle the first column on screen which may only be partially drawn depending on the xOffset value
	// This is also called by vdu_sys_layers_tilebank_draw() when drawing a single tile.

	if (tileCount == 0) {

		destStartPos = 0;

		sourcePixel = sourcePixel + xOffset;				// Need to start reading the source data at the xOffset

		for (auto y=0; y<8; y++) {

			destRowStart = destStartPos + (bufferWidth * y);

			for (auto x=xOffset; x<8; x++) {				// Start counting at xOffset through to end of the tile line

				destPixel = destRowStart + x - xOffset;

				tileBuffer[destPixel] = tileBank0Ptr[sourcePixel];

				sourcePixel++;
			}

			sourcePixel = sourcePixel + xOffset;
		}
	}

	// General drawing code for all columns except the first and last columns

	if (tileCount > 0 && tileCount < tileLayerWidth) {

		destStartPos = (tileCount * 8) - xOffset;

		for (auto y=0; y<8; y++) {

			destRowStart = destStartPos + (bufferWidth * y);

			for (auto x=0; x<8; x++) {

				destPixel = destRowStart + x;

				tileBuffer[destPixel] = tileBank0Ptr[sourcePixel];

				sourcePixel++;
			}
		}
	}

	// Handle the final column on screen which may only be partially drawn depending on the xOffset value

	if (tileCount == tileLayerWidth) {

		destStartPos = (tileCount * 8) - xOffset;

		for (auto y=0; y<8; y++) {

			destRowStart = destStartPos + (bufferWidth * y);

			for (auto x=0; x<xOffset; x++) {				// Start counting at the start of the tile through to the xOffset

				destPixel = destRowStart + x;

				tileBuffer[destPixel] = tileBank0Ptr[sourcePixel];

				sourcePixel++;
			}

			sourcePixel = sourcePixel + (8 - xOffset);		// As we are not drawing all pixels in the tile, skip ahead to the next line
		}
	}
}

void VDUStreamProcessor::writeTileToBufferFlipX(uint8_t tileId, uint8_t tileCount, uint8_t xOffset, uint8_t tileBuffer[], uint8_t tileLayerWidth) {

	int destStartPos;
	int destRowStart;
	int destPixel;
	int bufferWidth = tileLayerWidth * 8;

	int sourceTile = tileId * 64;		// The start of the specified tile Id
	int sourcePixel = 0;

	// Handle the first column on screen which may only be partially drawn depending on the xOffset value.
	// This is also called by vdu_sys_layers_tilebank_draw() when drawing a single tile.

	if (tileCount == 0) {

		destStartPos = 0;

		for (auto y=0; y<8; y++) {

			destRowStart = destStartPos + (bufferWidth * y);

			destPixel = destRowStart;

			for (auto x=(7-xOffset); x>=0; x--) {		
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}

			destPixel = destPixel + xOffset;
		}
	}

	// General drawing code for all columns except the first and last columns

	if (tileCount > 0 && tileCount < tileLayerWidth) {

		destStartPos = (tileCount * 8) - xOffset;

		for (auto y=0; y<8; y++) {

			destRowStart = destStartPos + (bufferWidth * y);

			destPixel = destRowStart;

			for (auto x=7; x>=0; x--) {		
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}
		}
	}

	// Handle the final column on screen which may only be partially drawn depending on the xOffset value

	if (tileCount == tileLayerWidth) {

		destStartPos = (tileCount * 8) - xOffset;

		for (auto y=0; y<8; y++) {

			destRowStart = destStartPos + (bufferWidth * y);

			destPixel = destRowStart;

			for (auto x=7; x>(7-xOffset); x--) {		
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}

			destPixel = destPixel + xOffset;
		}	
	}		
}

void VDUStreamProcessor::writeTileToBufferFlipY(uint8_t tileId, uint8_t tileCount, uint8_t xOffset, uint8_t tileBuffer[], uint8_t tileLayerWidth) {

	int destStartPos;		// Position in buffer the current tile needs to be written
	int destRowStart;
	int destPixel;
	int bufferWidth = tileLayerWidth * 8;
	
	int sourceTile = tileId * 64;		// The start of the specified tile Id
	int sourcePixel = 0;
	int destRowCount = 0;

	// Handle the first column on screen which may only be partially drawn depending on the xOffset value.
	// This is also called by vdu_sys_layers_tilebank_draw() when drawing a single tile.

	if (tileCount == 0) {

		destStartPos = 0;

		for (auto y=7; y>=0; y--) {

			destRowStart = destStartPos + (bufferWidth * destRowCount);

			destPixel = destRowStart;

			for (auto x=xOffset; x<8; x++) {
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}

			destRowCount++;
			destPixel = destPixel + xOffset;
		}

	}

	// General drawing code for all columns except the first and last columns

	if (tileCount > 0 && tileCount < tileLayerWidth) {

		destStartPos = (tileCount * 8) - xOffset;

		for (auto y=7; y>=0; y--) {

			destRowStart = destStartPos + (bufferWidth * destRowCount);

			destPixel = destRowStart;

			for (auto x=0; x<8; x++) {
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}

			destRowCount++;
		}	

	}

	// Handle the final column on screen which may only be partially drawn depending on the xOffset value

	if (tileCount == tileLayerWidth) {

		destStartPos = (tileCount * 8) - xOffset;

		for (auto y=7; y>=0; y--) {

			destRowStart = destStartPos + (bufferWidth * destRowCount);

			destPixel = destRowStart;

			for (auto x=0; x<xOffset; x++) {
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}

			destRowCount++;
			destPixel = destPixel + xOffset;
		}	
	}
}

void VDUStreamProcessor::writeTileToBufferFlipXY(uint8_t tileId, uint8_t tileCount, uint8_t xOffset, uint8_t tileBuffer[], uint8_t tileLayerWidth) {

	int destStartPos; 		// Position in buffer the current tile needs to be written
	int destRowStart;
	int destPixel;
	int bufferWidth = tileLayerWidth * 8;
	
	int sourceTile = tileId * 64;		// The start of the specified tile Id
	int sourcePixel = 0;
	int destRowCount = 0;

	// Handle the first column on screen which may only be partially drawn depending on the xOffset value.
	// This is also called by vdu_sys_layers_tilebank_draw() when drawing a single tile.

	if (tileCount == 0) {

		destStartPos = 0;

		for (auto y=7; y>=0; y--) {

			destRowStart = destStartPos + (bufferWidth * destRowCount);

			destPixel = destRowStart;

			for (auto x=(7-xOffset); x>=0; x--) {
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}

			destRowCount++;
			destPixel = destPixel + xOffset;
		}	
	}

	// General drawing code for all columns except the first and last columns

	if (tileCount > 0 && tileCount < tileLayerWidth) {

		destStartPos = (tileCount * 8) - xOffset;

		for (auto y=7; y>=0; y--) {

			destRowStart = destStartPos + (bufferWidth * destRowCount);

			destPixel = destRowStart;

			for (auto x=7; x>=0; x--) {
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}

			destRowCount++;
		}	
	}

	// Handle the final column on screen which may only be partially drawn depending on the xOffset value

	if (tileCount == tileLayerWidth) {

		destStartPos = (tileCount * 8) - xOffset;

		for (auto y=7; y>=0; y--) {

			destRowStart = destStartPos + (bufferWidth * destRowCount);

			destPixel = destRowStart;

			for (auto x=7; x>(7-xOffset); x--) {
				sourcePixel = sourceTile + (y * 8) + x;
				tileBuffer[destPixel]=tileBank0Ptr[sourcePixel];
				destPixel++;
			}

			destRowCount++;
			destPixel = destPixel + xOffset;
		}

	}
}

#endif // AGON_LAYERS_H
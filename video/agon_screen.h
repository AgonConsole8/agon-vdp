#ifndef AGON_SCREEN_H
#define AGON_SCREEN_H

#include <memory>
#include <fabgl.h>

#include "agon_palette.h"						// Colour lookup table

std::unique_ptr<fabgl::Canvas>	canvas;			// The canvas class
std::unique_ptr<fabgl::VGABaseController>	_VGAController;		// Pointer to the current VGA controller class (one of the above)

bool			legacyModes = false;			// Default legacy modes being false
uint8_t			_VGAColourDepth = -1;			// Number of colours per pixel (2, 4, 8, 16 or 64)
uint8_t			palette[64];					// Storage for the palette
uint16_t		canvasW;						// Canvas width
uint16_t		canvasH;						// Canvas height
uint8_t			videoMode;						// Current video mode

extern void debug_log(const char * format, ...);		// Debug log function

void setLegacyModes(bool legacy) {
	legacyModes = legacy;
}

// Get a new VGA controller
// Parameters:
// - colours: Number of colours per pixel (2, 4, 8, 16 or 64)
// Returns:
// - A singleton instance of a VGAController class
//
std::unique_ptr<fabgl::VGABaseController> getVGAController(uint8_t colours) {
	switch (colours) {
		case  2: return std::move(std::unique_ptr<fabgl::VGA2Controller>(new fabgl::VGA2Controller()));
		case  4: return std::move(std::unique_ptr<fabgl::VGA4Controller>(new fabgl::VGA4Controller()));
		case  8: return std::move(std::unique_ptr<fabgl::VGA8Controller>(new fabgl::VGA8Controller()));
		case 16: return std::move(std::unique_ptr<fabgl::VGA16Controller>(new fabgl::VGA16Controller()));
		case 64: return std::move(std::unique_ptr<fabgl::VGAController>(new fabgl::VGAController()));
	}
	return nullptr;
}

// Update the internal FabGL LUT
//
void updateRGB2PaletteLUT() {
	// Use instance, as call not present on VGABaseController
	switch (_VGAColourDepth) {
		case 2: fabgl::VGA2Controller::instance()->updateRGB2PaletteLUT(); break;
		case 4: fabgl::VGA4Controller::instance()->updateRGB2PaletteLUT(); break;
		case 8: fabgl::VGA8Controller::instance()->updateRGB2PaletteLUT(); break;
		case 16: fabgl::VGA16Controller::instance()->updateRGB2PaletteLUT(); break;
	}
}

// Get current colour depth
//
inline uint8_t getVGAColourDepth() {
	return _VGAColourDepth;
}

// Set a palette item
// Parameters:
// - l: The logical colour to change
// - c: The new colour
// 
void setPaletteItem(uint8_t l, RGB888 c) {
	auto depth = getVGAColourDepth();
	if (l < depth) {
		// Use instance, as call not present on VGABaseController
		switch (depth) {
			case 2: fabgl::VGA2Controller::instance()->setPaletteItem(l, c); break;
			case 4: fabgl::VGA4Controller::instance()->setPaletteItem(l, c); break;
			case 8: fabgl::VGA8Controller::instance()->setPaletteItem(l, c); break;
			case 16: fabgl::VGA16Controller::instance()->setPaletteItem(l, c); break;
		}
	}
}

// Get the palette index for a given RGB888 colour
//
uint8_t getPaletteIndex(RGB888 colour) {
	for (uint8_t i = 0; i < getVGAColourDepth(); i++) {
		if (colourLookup[palette[i]] == colour) {
			return i;
		}
	}
	return 0;
}

// Set logical palette
// Parameters:
// - l: The logical colour to change
// - p: The physical colour to change
// - r: The red component
// - g: The green component
// - b: The blue component
//
int8_t setLogicalPalette(uint8_t l, uint8_t p, uint8_t r, uint8_t g, uint8_t b) {
	RGB888 col;				// The colour to set

	if (p == 255) {					// If p = 255, then use the RGB values
		col = RGB888(r, g, b);
	} else if (p < 64) {			// If p < 64, then look the value up in the colour lookup table
		col = colourLookup[p];
	} else {
		debug_log("vdu_palette: p=%d not supported\n\r", p);
		return -1;
	}

	debug_log("vdu_palette: %d,%d,%d,%d,%d\n\r", l, p, r, g, b);
	if (getVGAColourDepth() < 64) {		// If it is a paletted video mode
		setPaletteItem(l, col);
	} else {
		// adjust our palette array for new colour
		// palette is an index into the colourLookup table, and our index is in 00RRGGBB format
		uint8_t index = (col.R >> 6) << 4 | (col.G >> 6) << 2 | (col.B >> 6);
		auto lookedup = colourLookup[index];
		debug_log("vdu_palette: col.R %02X, col.G %02X, col.B %02X, index %d (%02X), lookup %02X, %02X, %02X\n\r", col.R, col.G, col.B, index, index, lookedup.R, lookedup.G, lookedup.B);
		palette[l] = index;
		return index;
	}
	return -1;
}

// Reset the palette and set the foreground and background drawing colours
// Parameters:
// - colour: Array of indexes into colourLookup table
// - sizeOfArray: Size of passed colours array
//
void resetPalette(const uint8_t colours[]) {
	for (uint8_t i = 0; i < 64; i++) {
		uint8_t c = colours[i % getVGAColourDepth()];
		palette[i] = c;
		setPaletteItem(i, colourLookup[c]);
	}
	updateRGB2PaletteLUT();
}

// Update our VGA controller based on number of colours
// returns true on success, false if the number of colours was invalid
bool updateVGAController(uint8_t colours) {
	if (colours == _VGAColourDepth) {
		return true;
	}

	auto controller = getVGAController(colours);	// Get a new controller
	if (!controller) {
		return false;
	}

	_VGAColourDepth = colours;
	if (_VGAController) {						// If there is an existing controller running then
		_VGAController->end();					// end it
		_VGAController.reset();					// Delete it
	}
	_VGAController = std::move(controller);		// Switch to the new controller
	_VGAController->begin();					// And spin it up
	_VGAController->enableBackgroundPrimitiveExecution(true);
	_VGAController->enableBackgroundPrimitiveTimeout(false);

	return true;
}

// Change video resolution
// Parameters:
// - colours: Number of colours per pixel (2, 4, 8, 16 or 64)
// - modeLine: A modeline string (see the FabGL documentation for more details)
// Returns:
// - 0: Successful
// - 1: Invalid # of colours
// - 2: Not enough memory for mode
//
int8_t change_resolution(uint8_t colours, const char * modeLine, bool doubleBuffered = false) {
	if (!updateVGAController(colours)) {			// If we can't update the controller then
		return 1;									// Return the error
	}

	canvas.reset();									// Delete the canvas

	if (modeLine) {									// If modeLine is not a null pointer then
		_VGAController->setResolution(modeLine, -1, -1, doubleBuffered);	// Set the resolution
	} else {
		debug_log("change_resolution: modeLine is null\n\r");
	}

	canvas.reset(new fabgl::Canvas(_VGAController.get()));		// Create the new canvas
	debug_log("after change of canvas...\n\r");
	debug_log("  free internal: %d\n\r  free 8bit: %d\n\r  free 32bit: %d\n\r",
		heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
		heap_caps_get_free_size(MALLOC_CAP_8BIT),
		heap_caps_get_free_size(MALLOC_CAP_32BIT)
	);
	//
	// Check whether the selected mode has enough memory for the vertical resolution
	//
	if (_VGAController->getScreenHeight() != _VGAController->getViewPortHeight()) {
		return 2;
	}
	return 0;										// Return with no errors
}

// Ask our screen controller if we're double buffered
//
inline bool isDoubleBuffered() {
	return _VGAController->isDoubleBuffered();
}

// Wait for plot completion
//
inline void waitPlotCompletion(bool waitForVSync = false) {
	canvas->waitCompletion(waitForVSync);
}

// Swap to other buffer if we're in a double-buffered mode
// Always waits for VSYNC
//
void switchBuffer() {
	if (isDoubleBuffered()) {
		canvas->swapBuffers();
	} else {
		waitPlotCompletion(true);
	}
}

void setMouseCursorPos(uint16_t x, uint16_t y) {
	_VGAController->setMouseCursorPos(x, y);
}

#endif // AGON_SCREEN_H

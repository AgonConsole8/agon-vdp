#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <algorithm>
#include <vector>

#include <fabgl.h>

#include "agon.h"
#include "agon_ps2.h"
#include "agon_screen.h"
#include "agon_fonts.h"							// The Acorn BBC Micro Font
#include "agon_palette.h"						// Colour lookup table
#include "agon_ttxt.h"
#include "cursor.h"
#include "sprites.h"
#include "viewport.h"

fabgl::PaintOptions			gpofg;				// Graphics paint options foreground
fabgl::PaintOptions			gpobg;				// Graphics paint options background
fabgl::PaintOptions			tpo;				// Text paint options
fabgl::PaintOptions			cpo;				// Cursor paint options

const fabgl::FontInfo		* font;				// Current active font

Point			p1, p2, p3;						// Coordinate store for plot
Point			rp1;							// Relative coordinates store for plot
Point			up1;							// Unscaled coordinates store for plot
RGB888			gfg, gbg;						// Graphics foreground and background colour
RGB888			tfg, tbg;						// Text foreground and background colour
uint8_t			gfgc, gbgc, tfgc, tbgc;			// Logical colour values for graphics and text
uint8_t			cursorVStart;					// Cursor vertical start offset
uint8_t			cursorVEnd;						// Cursor vertical end
uint8_t			cursorHStart;					// Cursor horizontal start offset
uint8_t			cursorHEnd;						// Cursor horizontal end
uint8_t			videoMode;						// Current video mode
bool			legacyModes = false;			// Default legacy modes being false
bool			rectangularPixels = false;		// Pixels are square by default
uint8_t			palette[64];					// Storage for the palette
std::vector<Point>	pathPoints;					// Storage for path points

extern bool ttxtMode;							// Teletext mode
extern agon_ttxt ttxt_instance;					// Teletext instance

// Change the currently selected font
//
void changeFont(const fabgl::FontInfo * f) {
	if (!ttxtMode) {
		// adjust our cursor position so baseline matches new font
		cursorRelativeMove(0, font->ascent - f->ascent);
		debug_log("changeFont - y adjustment is %d\n\r", font->ascent - f->ascent);
		font = f;
		canvas->selectFont(f);
	}
}

// Copy the AGON font data from Flash to RAM
//
void copy_font() {
	memcpy(fabgl::FONT_AGON_DATA, fabgl::FONT_AGON_BITMAP, sizeof(fabgl::FONT_AGON_BITMAP));
}

// Redefine a character in the font
//
void redefineCharacter(uint8_t c, uint8_t * data) {
	if (font == &fabgl::FONT_AGON) {
		memcpy(&fabgl::FONT_AGON_DATA[c * 8], data, 8);
	} else {
		debug_log("redefineCharacter: alternate font redefinition not supported with this API\n\r");
	}
}

bool cmpChar(uint8_t * c1, uint8_t *c2, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		if (*c1++ != *c2++) {
			return false;
		}
	}
	return true;
}

// Try and match a character at given pixel position
//
char getScreenChar(uint16_t px, uint16_t py) {
	RGB888	pixel;
	uint8_t	charRow;
	uint8_t	charData[font->height];
	uint8_t R = tbg.R;
	uint8_t G = tbg.G;
	uint8_t B = tbg.B;

	// TODO charRow and charData here are tied to font dimensions
	// so charRow will need to adjust depending on font width

	// Do some bounds checking first
	//
	if (px >= canvasW - font->width || py >= canvasH - font->height) {
		return 0;
	}
	if (ttxtMode) {
		return ttxt_instance.get_screen_char(px,py);
	} else {
		// Now scan the screen and get the 8 byte pixel representation in charData
		//
		for (uint8_t y = 0; y < font->height; y++) {
			charRow = 0;
			for (uint8_t x = 0; x < font->width; x++) {
				pixel = canvas->getPixel(px + x, py + y);
				if (!(pixel.R == R && pixel.G == G && pixel.B == B)) {
					charRow |= (0x80 >> x);
				}
			}
			charData[y] = charRow;
		}
		//
		// Finally try and match with the character set array
		// starts at space character (32) and goes beyond the normal ASCII range
		// Character checked is ANDed with 0xFF, so we check 32-255 then 0-31
		// thus allowing characters 0-31 to be checked after conventional characters
		// as by default those characters are the same as space
		//
		for (auto i = 32; i <= (255 + 31); i++) {
			uint8_t c = i & 0xFF;
			// TODO charData here is tied to font dimensions
			// - we should have a function to get a pointer to the font character data
			if (cmpChar(charData, &fabgl::FONT_AGON_DATA[c * 8], 8)) {
				return c;
			}
		}
	}
	return 0;
}

// Get pixel value at screen coordinates
//
RGB888 getPixel(uint16_t x, uint16_t y) {
	Point p = translateCanvas(scale(x, y));
	if (p.X >= 0 && p.Y >= 0 && p.X < canvasW && p.Y < canvasH) {
		return canvas->getPixel(p.X, p.Y);
	}
	return RGB888(0,0,0);
}

// Horizontal scan until we find a pixel not non-equalto given colour
// returns x coordinate for the last pixel before the match
uint16_t scanH(int16_t x, int16_t y, RGB888 colour, int8_t direction = 1) {
	uint16_t w = direction > 0 ? canvas->getWidth() - 1 : 0;
	if (x < 0 || x >= canvas->getWidth()) return x;

	while (x != w) {
		RGB888 pixel = canvas->getPixel(x, y);
		if (pixel == colour) {
			x += direction;
		} else {
			return x - direction;
		}
	}

	return w;
}

// Horizontal scan until we find a pixel matching the given colour
// returns x coordinate for the last pixel before the match
uint16_t scanHToMatch(int16_t x, int16_t y, RGB888 colour, int8_t direction = 1) {
	uint16_t w = direction > 0 ? canvas->getWidth() - 1 : 0;
	if (x < 0 || x >= canvas->getWidth()) return x;

	while (x != w) {
		RGB888 pixel = canvas->getPixel(x, y);
		if (pixel == colour) {
			return x - direction;
		}
		x += direction;
	}

	return w;
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
void setPalette(uint8_t l, uint8_t p, uint8_t r, uint8_t g, uint8_t b) {
	RGB888 col;				// The colour to set

	if (p == 255) {					// If p = 255, then use the RGB values
		col = RGB888(r, g, b);
	} else if (p < 64) {			// If p < 64, then look the value up in the colour lookup table
		col = colourLookup[p];
	} else {
		debug_log("vdu_palette: p=%d not supported\n\r", p);
		return;
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
		if (l == tfgc) {
			tfg = lookedup;
		}
		if (l == tbgc) {
			tbg = lookedup;
		}
		if (l == gfgc) {
			gfg = lookedup;
		}
		if (l == gbgc) {
			gbg = lookedup;
		}
	}
}

// Reset the palette and set the foreground and background drawing colours
// Parameters:
// - colour: Array of indexes into colourLookup table
// - sizeOfArray: Size of passed colours array
//
void resetPalette(const uint8_t colours[]) {
	if (ttxtMode) return;
	for (uint8_t i = 0; i < 64; i++) {
		uint8_t c = colours[i % getVGAColourDepth()];
		palette[i] = c;
		setPaletteItem(i, colourLookup[c]);
	}
	updateRGB2PaletteLUT();
}

// Get the paint options for a given GCOL mode
//
fabgl::PaintOptions getPaintOptions(fabgl::PaintMode mode, fabgl::PaintOptions priorPaintOptions) {
	fabgl::PaintOptions p = priorPaintOptions;
	p.mode = mode;
	return p;
}

// Restore palette to default for current mode
//
void restorePalette() {
	auto depth = getVGAColourDepth();
	gbgc = tbgc = 0;
	gfgc = tfgc = 15 % depth;
	switch (depth) {
		case  2: resetPalette(defaultPalette02); break;
		case  4: resetPalette(defaultPalette04); break;
		case  8: resetPalette(defaultPalette08); break;
		case 16: resetPalette(defaultPalette10); break;
		case 64: resetPalette(defaultPalette40); break;
	}
	gfg = colourLookup[0x3F];
	gbg = colourLookup[0x00];
	tfg = colourLookup[0x3F];
	tbg = colourLookup[0x00];
	tpo = getPaintOptions(fabgl::PaintMode::Set, tpo);
	cpo = getPaintOptions(fabgl::PaintMode::XOR, tpo);
	gpofg = getPaintOptions(fabgl::PaintMode::Set, gpofg);
	gpobg = getPaintOptions(fabgl::PaintMode::Set, gpobg);
}

// Set text colour (handles COLOUR / VDU 17)
//
void setTextColour(uint8_t colour) {
	if (ttxtMode) return;

	uint8_t col = colour % getVGAColourDepth();
	uint8_t c = palette[col];

	if (colour < 64) {
		tfg = colourLookup[c];
		tfgc = col;
		debug_log("vdu_colour: tfg %d = %02X : %02X,%02X,%02X\n\r", colour, c, tfg.R, tfg.G, tfg.B);
	}
	else if (colour >= 128 && colour < 192) {
		tbg = colourLookup[c];
		tbgc = col;
		debug_log("vdu_colour: tbg %d = %02X : %02X,%02X,%02X\n\r", colour, c, tbg.R, tbg.G, tbg.B);
	}
	else {
		debug_log("vdu_colour: invalid colour %d\n\r", colour);
	}
}

// Set graphics colour (handles GCOL / VDU 18)
//
void setGraphicsColour(uint8_t mode, uint8_t colour) {
	if (ttxtMode) return;

	uint8_t col = colour % getVGAColourDepth();
	uint8_t c = palette[col];

	if (mode <= 7) {
		if (colour < 64) {
			gfg = colourLookup[c];
			gfgc = col;
			debug_log("vdu_gcol: mode %d, gfg %d = %02X : %02X,%02X,%02X\n\r", mode, colour, c, gfg.R, gfg.G, gfg.B);
		}
		else if (colour >= 128 && colour < 192) {
			gbg = colourLookup[c];
			gbgc = col;
			debug_log("vdu_gcol: mode %d, gbg %d = %02X : %02X,%02X,%02X\n\r", mode, colour, c, gbg.R, gbg.G, gbg.B);
		}
		else {
			debug_log("vdu_gcol: invalid colour %d\n\r", colour);
		}
		if (colour < 128) {
			gpofg = getPaintOptions((fabgl::PaintMode)mode, gpofg);
		} else {
			gpobg = getPaintOptions((fabgl::PaintMode)mode, gpobg);
		}
	}
	else {
		debug_log("vdu_gcol: invalid mode %d\n\r", mode);
	}
}

// Clear a viewport
//
void clearViewport(Rect * viewport) {
	if (ttxtMode) {
		ttxt_instance.cls();
	} else {
		if (canvas) {
			canvas->fillRectangle(*viewport);
		}
	}
}

//// Graphics drawing routines

// Push point to list
//
void pushPoint(Point p) {
	rp1 = Point(p.X - p1.X, p.Y - p1.Y);
	p3.X = p2.X;
	p3.Y = p2.Y;
	p2.X = p1.X;
	p2.Y = p1.Y;
	p1.X = p.X;
	p1.Y = p.Y;
}
void pushPoint(uint16_t x, uint16_t y) {
	up1 = Point(x, y);
	pushPoint(translateCanvas(scale(x, y)));
}
void pushPointRelative(int16_t x, int16_t y) {
	pushPoint(up1.X + x, up1.Y + y);
}

// get graphics cursor
//
Point * getGraphicsCursor() {
	return &p1;
}

// Set up canvas for drawing graphics
//
void setGraphicsOptions(uint8_t mode) {
	auto colourMode = mode & 0x03;
	canvas->setClippingRect(graphicsViewport);
	switch (colourMode) {
		case 0: break;	// move command
		case 1: {
			// use fg colour
			canvas->setPenColor(gfg);
			canvas->setPaintOptions(gpofg);
		} break;
		case 2: {
			// logical inverse colour - override paint options
			auto options = getPaintOptions(fabgl::PaintMode::Invert, gpofg);
			canvas->setPaintOptions(options);
			return;
		} break;
		case 3: {
			// use bg colour
			canvas->setPenColor(gbg);
			canvas->setPaintOptions(gpobg);
		} break;
	}
}

// Set up canvas for drawing filled graphics
//
void setGraphicsFill(uint8_t mode) {
	auto colourMode = mode & 0x03;
	switch (colourMode) {
		case 0: break;	// move command
		case 1: {
			// use fg colour
			canvas->setBrushColor(gfg);
		} break;
		case 2: break;	// logical inverse colour (not suported)
		case 3: {
			// use bg colour
			canvas->setBrushColor(gbg);
		} break;
	}
}

// Move to
//
void moveTo() {
	canvas->moveTo(p1.X, p1.Y);
}

// Line plot
//
void plotLine(bool omitFirstPoint = false, bool omitLastPoint = false, bool usePattern = false, bool resetPattern = true) {
	if (!textCursorActive()) {
		// if we're in graphics mode, we need to move the cursor to the last point
		canvas->moveTo(p2.X, p2.Y);
	}

	auto lineOptions = fabgl::LineOptions();
	lineOptions.omitFirst = omitFirstPoint;
	lineOptions.omitLast = omitLastPoint;
	lineOptions.usePattern = usePattern;
	if (resetPattern) {
		canvas->setLinePatternOffset(0);
	}
	canvas->setLineOptions(lineOptions);

	canvas->lineTo(p1.X, p1.Y);
}

// Fill horizontal line
//
void fillHorizontalLine(bool scanLeft, bool match, RGB888 matchColor) {
	canvas->waitCompletion(false);
	int16_t y = p1.Y;
	int16_t x1 = scanLeft ? (match ? scanHToMatch(p1.X, y, matchColor, -1) : scanH(p1.X, y, matchColor, -1)) : p1.X;
	int16_t x2 = match ? scanHToMatch(p1.X, y, matchColor, 1) : scanH(p1.X, y, matchColor, 1);
	debug_log("fillHorizontalLine: (%d, %d) transformed to (%d,%d) -> (%d,%d)\n\r", p1.X, p1.Y, x1, y, x2, y);

	if (x1 == x2 || x1 > x2) {
		// Coordinate needs to be tweaked to match Acorn's behaviour
		auto p = toCurrentCoordinates(scanLeft ? x2 + 1 : x2, y);
		pushPoint(p.X, up1.Y);
		// nothing to draw
		return;
	}
	canvas->moveTo(x1, y);
	canvas->lineTo(x2, y);

	auto p = toCurrentCoordinates(x2, y);
	pushPoint(p.X, up1.Y);
}

// Point point
//
void plotPoint() {
	canvas->setPixel(p1.X, p1.Y);
}

// Triangle plot
//
void plotTriangle() {
	Point p[3] = {
		p3,
		p2,
		p1,
	};
	// if (gpo.mode == fabgl::PaintMode::Set) {
	// 	canvas->drawPath(p, 3);
	// }
	canvas->fillPath(p, 3);
}

// Path plot
//
void plotPath(uint8_t mode, uint8_t lastMode) {
	debug_log("plotPath: mode %d, lastMode %d, pathPoints.size() %d\n\r", mode, lastMode, pathPoints.size());
	// if the mode indicates a "move", then this is a "commit" command
	// so we should draw the path and clear the pathPoints array
	if ((mode & 0x03) == 0) {
		if (pathPoints.size() < 3) {
			// we need at least three points to draw a path
			debug_log("plotPath: not enough points to draw a path - clearing\n\r");
			pathPoints.clear();
			return;
		}
		debug_log("plotPath: drawing path\n\r");
		// iterate over our pathPoints and output in debug statement
		for (auto p : pathPoints) {
			debug_log("plotPath: (%d,%d)\n\r", p.X, p.Y);
		}
		debug_log("plotPath: setting graphics fill with lastMode %d\n\r", lastMode);
		// i'm not entirely sure yet whether this is needed
		setGraphicsOptions(lastMode);
		setGraphicsFill(lastMode);
		canvas->fillPath(pathPoints.data(), pathPoints.size());
		pathPoints.clear();
		return;
	}

	// if we have an empty pathPoints list, then push two points
	if (pathPoints.size() == 0) {
		pathPoints.push_back(p3);
		pathPoints.push_back(p2);
	}
	// push latest point
	pathPoints.push_back(p1);
}

// Rectangle plot
//
void plotRectangle() {
	canvas->fillRectangle(p2.X, p2.Y, p1.X, p1.Y);
}

// Parallelogram plot
//
void plotParallelogram() {
	Point p[4] = {
		p3,
		p2,
		p1,
		Point(p1.X + (p3.X - p2.X), p1.Y + (p3.Y - p2.Y)),
	};
	// if (gpo.mode == fabgl::PaintMode::Set) {
	// 	canvas->drawPath(p, 4);
	// }
	canvas->fillPath(p, 4);
}

// Circle plot
//
void plotCircle(bool filled = false) {
	auto size = 2 * sqrt(rp1.X * rp1.X + (rp1.Y * rp1.Y * (rectangularPixels ? 4 : 1)));
	if (filled) {
		canvas->fillEllipse(p2.X, p2.Y, size, rectangularPixels ? size / 2 : size);
	} else {
		canvas->drawEllipse(p2.X, p2.Y, size, rectangularPixels ? size / 2 : size);
	}
}

// Arc plot
void plotArc() {
	debug_log("plotArc: (%d,%d) -> (%d,%d), (%d,%d)\n\r", p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
	canvas->drawArc(p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
}

// Segment plot
void plotSegment() {
	debug_log("plotSegment: (%d,%d) -> (%d,%d), (%d,%d)\n\r", p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
	canvas->fillSegment(p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
}

// Sector plot
void plotSector() {
	debug_log("plotSector: (%d,%d) -> (%d,%d), (%d,%d)\n\r", p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
	canvas->fillSector(p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
}

// Copy or move a rectangle
//
void plotCopyMove(uint8_t mode) {
	uint16_t width = abs(p3.X - p2.X);
	uint16_t height = abs(p3.Y - p2.Y);
	uint16_t sourceX = p3.X < p2.X ? p3.X : p2.X;
	uint16_t sourceY = p3.Y < p2.Y ? p3.Y : p2.Y;
	uint16_t destX = p1.X;
	uint16_t destY = p1.Y - height;

	debug_log("plotCopyMove: mode %d, (%d,%d) -> (%d,%d), width: %d, height: %d\n\r", mode, sourceX, sourceY, destX, destY, width, height);
	canvas->copyRect(sourceX, sourceY, destX, destY, width + 1, height + 1);
	if (mode == 1 || mode == 5) {
		// move rectangle needs to clear source rectangle
		// being careful not to clear the destination rectangle
		canvas->setBrushColor(gbg);
		canvas->setPaintOptions(getPaintOptions(fabgl::PaintMode::Set, gpobg));
		Rect sourceRect = Rect(sourceX, sourceY, sourceX + width, sourceY + height);
		debug_log("plotCopyMove: source rectangle (%d,%d) -> (%d,%d)\n\r", sourceRect.X1, sourceRect.Y1, sourceRect.X2, sourceRect.Y2);
		Rect destRect = Rect(destX, destY, destX + width, destY + height);
		debug_log("plotCopyMove: destination rectangle (%d,%d) -> (%d,%d)\n\r", destRect.X1, destRect.Y1, destRect.X2, destRect.Y2);
		if (sourceRect.intersects(destRect)) {
			// we can use clipping rects to block out parts of the screen
			// the areas above, below, left, and right of the destination rectangle
			// and then draw rectangles over our source rectangle
			auto intersection = sourceRect.intersection(destRect);
			debug_log("intersection: (%d,%d) -> (%d,%d)\n\r", intersection.X1, intersection.Y1, intersection.X2, intersection.Y2);

			if (intersection.X1 > sourceRect.X1) {
				// fill in source area to the left of destination
				debug_log("clearing left of destination\n\r");
				auto clearClip = Rect(sourceRect.X1, sourceRect.Y1, intersection.X1 - 1, sourceRect.Y2);
				debug_log("clearClip: (%d,%d) -> (%d,%d)\n\r", clearClip.X1, clearClip.Y1, clearClip.X2, clearClip.Y2);
				canvas->setClippingRect(clearClip);
				canvas->fillRectangle(sourceRect);
			}
			if (intersection.X2 < sourceRect.X2) {
				// fill in source area to the right of destination
				debug_log("clearing right of destination\n\r");
				auto clearClip = Rect(intersection.X2 + 1, sourceRect.Y1, sourceRect.X2, sourceRect.Y2);
				debug_log("clearClip: (%d,%d) -> (%d,%d)\n\r", clearClip.X1, clearClip.Y1, clearClip.X2, clearClip.Y2);
				canvas->setClippingRect(clearClip);
				canvas->fillRectangle(sourceRect);
			}
			if (intersection.Y1 > sourceRect.Y1) {
				// fill in source area above destination
				debug_log("clearing above destination\n\r");
				auto clearClip = Rect(sourceRect.X1, sourceRect.Y1, sourceRect.X2, intersection.Y1 - 1);
				debug_log("clearClip: (%d,%d) -> (%d,%d)\n\r", clearClip.X1, clearClip.Y1, clearClip.X2, clearClip.Y2);
				canvas->setClippingRect(clearClip);
				canvas->fillRectangle(sourceRect);
			}
			if (intersection.Y2 < sourceRect.Y2) {
				// fill in source area below destination
				debug_log("clearing below destination\n\r");
				auto clearClip = Rect(sourceRect.X1, intersection.Y2 + 1, sourceRect.X2, sourceRect.Y2);
				debug_log("clearClip: (%d,%d) -> (%d,%d)\n\r", clearClip.X1, clearClip.Y1, clearClip.X2, clearClip.Y2);
				canvas->setClippingRect(clearClip);
				canvas->fillRectangle(sourceRect);
			}
		} else {
			canvas->fillRectangle(sourceRect);
		}
	}
}

// Plot bitmap
//
void plotBitmap(uint8_t mode) {
	if ((mode & 0x03) == 0x03) {
		// get a copy of gpobg, without changing it's paint mode
		auto paintOptions = getPaintOptions(gpobg.mode, gpobg);
		// swapFGBG on bitmap plots indicates to plot using pen color instead of bitmap
		paintOptions.swapFGBG = true;
		canvas->setPaintOptions(paintOptions);
	}
	drawBitmap(p1.X, p1.Y, true);
}

// Character plot
//
void plotCharacter(char c) {
	if (ttxtMode) {
		ttxt_instance.draw_char(activeCursor->X, activeCursor->Y, c);
	} else {
		if (cursorBehaviour.scrollProtect) {
			cursorAutoNewline();
		}
		bool isTextCursor = textCursorActive();
		auto bitmap = getBitmapFromChar(c);
		if (isTextCursor) {
			canvas->setClippingRect(defaultViewport);
			canvas->setPenColor(tfg);
			canvas->setBrushColor(tbg);
			canvas->setPaintOptions(tpo);
		} else {
			canvas->setClippingRect(graphicsViewport);
			canvas->setPenColor(gfg);
			canvas->setPaintOptions(gpofg);
		}
		if (bitmap) {
			canvas->drawBitmap(activeCursor->X, activeCursor->Y + font->height - bitmap->height, bitmap.get());
		} else {
			canvas->drawChar(activeCursor->X, activeCursor->Y, c);
		}
	}
	if (!cursorBehaviour.xHold) {
		cursorRight(cursorBehaviour.scrollProtect);
	}
}

// Backspace plot
//
void plotBackspace() {
	cursorLeft();
	if (ttxtMode) {
		ttxt_instance.draw_char(activeCursor->X, activeCursor->Y, ' ');
	} else {
		canvas->setBrushColor(textCursorActive() ? tbg : gbg);
		canvas->fillRectangle(activeCursor->X, activeCursor->Y, activeCursor->X + font->width - 1, activeCursor->Y + font->height - 1);
	}
}

// Set character overwrite mode (background fill)
//
inline void setCharacterOverwrite(bool overwrite) {
	canvas->setGlyphOptions(GlyphOptions().FillBackground(overwrite));
}

// Set a clipping rectangle
//
void setClippingRect(Rect rect) {
	canvas->setClippingRect(rect);
}

// Draw cursor
//
void drawCursor(Point p) {
	if (textCursorActive()) {
		if (cursorHStart < font->width && cursorHStart <= cursorHEnd && cursorVStart < font->height && cursorVStart <= cursorVEnd) {
			canvas->setPaintOptions(cpo);
			canvas->setBrushColor(tbg);
			canvas->fillRectangle(p.X + cursorHStart, p.Y + cursorVStart, p.X + std::min(((int)cursorHEnd), font->width - 1), p.Y + std::min(((int)cursorVEnd), font->height - 1));
			canvas->setBrushColor(tfg);
			canvas->fillRectangle(p.X + cursorHStart, p.Y + cursorVStart, p.X + std::min(((int)cursorHEnd), font->width - 1), p.Y + std::min(((int)cursorVEnd), font->height - 1));
			canvas->setPaintOptions(tpo);
		}
	}
}

// Clear the screen
//
void cls(bool resetViewports) {
	if (resetViewports) {
		if (ttxtMode) {
			ttxt_instance.set_window(0,24,39,0);
		}
		viewportReset();
	}
	if (canvas) {
		canvas->setPenColor(tfg);
		canvas->setBrushColor(tbg);
		canvas->setPaintOptions(tpo);
		clearViewport(getViewport(VIEWPORT_TEXT));
	}
	if (hasActiveSprites()) {
		activateSprites(0);
		clearViewport(getViewport(VIEWPORT_TEXT));
	}
	cursorHome();
	setPagedMode();
}

// Clear the graphics area
//
void clg() {
	if (canvas) {
		canvas->setPenColor(gfg);
		canvas->setBrushColor(gbg);
		canvas->setPaintOptions(gpobg);
		clearViewport(getViewport(VIEWPORT_GRAPHICS));
	}
	pushPoint(0, 0);		// Reset graphics origin (as per BBC Micro CLG)
}

// Do the mode change
// Parameters:
// - mode: The video mode
// Returns:
// -  0: Successful
// -  1: Invalid # of colours
// -  2: Not enough memory for mode
// - -1: Invalid mode
//
int8_t change_mode(uint8_t mode) {
	int8_t errVal = -1;

	cls(true);
	ttxtMode = false;
	switch (mode) {
		case 0:
			if (legacyModes == true) {
				errVal = change_resolution(2, SVGA_1024x768_60Hz);
			} else {
				errVal = change_resolution(16, VGA_640x480_60Hz);	// VDP 1.03 Mode 3, VGA Mode 12h
			}
			break;
		case 1:
			if (legacyModes == true) {
				errVal = change_resolution(16, VGA_512x384_60Hz);
			} else {
				errVal = change_resolution(4, VGA_640x480_60Hz);
			}
			break;
		case 2:
			if (legacyModes == true) {
				errVal = change_resolution(64, VGA_320x200_75Hz);
			} else {
				errVal = change_resolution(2, VGA_640x480_60Hz);
			}
			break;
		case 3:
			if (legacyModes == true) {
				errVal = change_resolution(16, VGA_640x480_60Hz);
			} else {
				errVal = change_resolution(64, VGA_640x240_60Hz);
			}
			break;
		case 4:
			errVal = change_resolution(16, VGA_640x240_60Hz);
			break;
		case 5:
			errVal = change_resolution(4, VGA_640x240_60Hz);
			break;
		case 6:
			errVal = change_resolution(2, VGA_640x240_60Hz);
			break;
		case 7:
			errVal = change_resolution(16, VGA_640x480_60Hz);
			if (errVal == 0) {
				errVal = ttxt_instance.init();
				if (errVal == 0) ttxtMode = true;
			}
			break;
		case 8:
			errVal = change_resolution(64, QVGA_320x240_60Hz);		// VGA "Mode X"
			break;
		case 9:
			errVal = change_resolution(16, QVGA_320x240_60Hz);
			break;
		case 10:
			errVal = change_resolution(4, QVGA_320x240_60Hz);
			break;
		case 11:
			errVal = change_resolution(2, QVGA_320x240_60Hz);
			break;
		case 12:
			errVal = change_resolution(64, VGA_320x200_70Hz);		// VGA Mode 13h
			break;
		case 13:
			errVal = change_resolution(16, VGA_320x200_70Hz);
			break;
		case 14:
			errVal = change_resolution(4, VGA_320x200_70Hz);
			break;
		case 15:
			errVal = change_resolution(2, VGA_320x200_70Hz);
			break;
		case 16:
			errVal = change_resolution(4, SVGA_800x600_60Hz);
			break;
		case 17:
			errVal = change_resolution(2, SVGA_800x600_60Hz);
			break;
		case 18:
			errVal = change_resolution(2, SVGA_1024x768_60Hz);		// VDP 1.03 Mode 0
			break;
		case 129:
			errVal = change_resolution(4, VGA_640x480_60Hz, true);
			break;
		case 130:
			errVal = change_resolution(2, VGA_640x480_60Hz, true);
			break;
		case 132:
			errVal = change_resolution(16, VGA_640x240_60Hz, true);
			break;
		case 133:
			errVal = change_resolution(4, VGA_640x240_60Hz, true);
			break;
		case 134:
			errVal = change_resolution(2, VGA_640x240_60Hz, true);
			break;
		case 136:
			errVal = change_resolution(64, QVGA_320x240_60Hz, true);		// VGA "Mode X"
			break;
		case 137:
			errVal = change_resolution(16, QVGA_320x240_60Hz, true);
			break;
		case 138:
			errVal = change_resolution(4, QVGA_320x240_60Hz, true);
			break;
		case 139:
			errVal = change_resolution(2, QVGA_320x240_60Hz, true);
			break;
		case 140:
			errVal = change_resolution(64, VGA_320x200_70Hz, true);		// VGA Mode 13h
			break;
		case 141:
			errVal = change_resolution(16, VGA_320x200_70Hz, true);
			break;
		case 142:
			errVal = change_resolution(4, VGA_320x200_70Hz, true);
			break;
		case 143:
			errVal = change_resolution(2, VGA_320x200_70Hz, true);
			break;
	}
	if (errVal != 0) {
		return errVal;
	}
	restorePalette();
	if (!ttxtMode) {
		canvas->selectFont(&fabgl::FONT_AGON);
	}
	setCharacterOverwrite(true);
	canvas->setPenWidth(1);
	setCanvasWH(canvas->getWidth(), canvas->getHeight());
	// simple heuristic to determine rectangular pixels
	rectangularPixels = ((float)canvasW / (float)canvasH) > 2;
	font = canvas->getFontInfo();
	viewportReset();
	setOrigin(0,0);
	pushPoint(0,0);
	pushPoint(0,0);
	pushPoint(0,0);
	moveTo();
	resetCursor();
	if (isDoubleBuffered()) {
		switchBuffer();
		cls(false);
	}
	resetMousePositioner(canvasW, canvasH, _VGAController.get());
	setMouseCursor();
	debug_log("do_modeChange: canvas(%d,%d), scale(%f,%f), mode %d, videoMode %d\n\r", canvasW, canvasH, logicalScaleX, logicalScaleY, mode, videoMode);
	return 0;
}

// Change the video mode
// If there is an error, restore the last mode
// Parameters:
// - mode: The video mode
//
void set_mode(uint8_t mode) {
	auto errVal = change_mode(mode);
	if (errVal != 0) {
		debug_log("set_mode: error %d\n\r", errVal);
		errVal = change_mode(videoMode);
		if (errVal != 0) {
			debug_log("set_mode: error %d on restoring mode\n\r", errVal);
			videoMode = 1;
			change_mode(1);
		}
		return;
	}
	videoMode = mode;
}

void setLegacyModes(bool legacy) {
	legacyModes = legacy;
}

void scrollRegion(Rect * region, uint8_t direction, int16_t movement) {
	canvas->setScrollingRegion(region->X1, region->Y1, region->X2, region->Y2);
	canvas->setPenColor(tbg);
	canvas->setBrushColor(tbg);
	canvas->setPaintOptions(tpo);
	if (ttxtMode) {
		if (direction & 3 == 3) {
			ttxt_instance.scroll();
		}
	} else {
		auto moveX = 0;
		auto moveY = 0;
		switch (direction) {
			case 0:		// Right
				moveX = 1;
				break;
			case 1:		// Left
				moveX = -1;
				break;
			case 2:		// Down
				moveY = 1;
				break;
			case 3:		// Up
				moveY = -1;
				break;
			case 4:		// positive X
				if (cursorBehaviour.flipXY) {
					moveY = cursorBehaviour.invertVertical ? -1 : 1;
				} else {
					moveX = cursorBehaviour.invertHorizontal ? -1 : 1;
				}
				break;
			case 5:		// negative X
				if (cursorBehaviour.flipXY) {
					moveY = cursorBehaviour.invertVertical ? 1 : -1;
				} else {
					moveX = cursorBehaviour.invertHorizontal ? 1 : -1;
				}
				break;
			case 6:		// positive Y
				if (cursorBehaviour.flipXY) {
					moveX = cursorBehaviour.invertHorizontal ? -1 : 1;
				} else {
					moveY = cursorBehaviour.invertVertical ? -1 : 1;
				}
				break;
			case 7:		// negative Y
				if (cursorBehaviour.flipXY) {
					moveX = cursorBehaviour.invertHorizontal ? 1 : -1;
				} else {
					moveY = cursorBehaviour.invertVertical ? 1 : -1;
				}
				break;
		}
		if (moveX != 0 || moveY != 0) {
			if (movement == 0) {
				if (moveX != 0) {
					movement = font->width;
				} else {
					movement = font->height;
				}
			}
			canvas->scroll(movement * moveX, movement * moveY);
		}
	}
}

void setLineThickness(uint8_t thickness) {
	canvas->setPenWidth(thickness);
}

void setDottedLinePattern(uint8_t pattern[8]) {
	auto linePattern = fabgl::LinePattern();
	linePattern.setPattern(pattern);
	canvas->setLinePattern(linePattern);
}

void setDottedLinePatternLength(uint8_t length) {
	if (length == 0) {
		// reset the line pattern
		auto linePattern = fabgl::LinePattern();
		canvas->setLinePattern(linePattern);
		canvas->setLinePatternLength(8);
		return;
	}
	canvas->setLinePatternLength(length);
}

#endif

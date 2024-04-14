#ifndef CONTEXT_GRAPHICS_H
#define CONTEXT_GRAPHICS_H

#include <algorithm>
#include <vector>

#include <fabgl.h>

#include "agon.h"
#include "agon_ps2.h"
#include "agon_screen.h"
#include "agon_fonts.h"							// The Acorn BBC Micro Font
#include "agon_palette.h"						// Colour lookup table
#include "agon_ttxt.h"
#include "sprites.h"

// Definitions for the functions we're implementing here
#include "context.h"

extern bool ttxtMode;							// Teletext mode
extern agon_ttxt ttxt_instance;					// Teletext instance

// Plot command handler
//
bool IRAM_ATTR Context::plot(int16_t x, int16_t y, uint8_t command) {
	auto mode = command & 0x07;
	auto operation = command & 0xF8;
	bool pending = false;

	if (mode < 4) {
		pushPointRelative(x, y);
	} else {
		pushPoint(x, y);
	}

	debug_log("vdu_plot: operation: %X, mode %d, lastPlotCommand %X, (%d,%d) -> (%d,%d)\n\r", operation, mode, lastPlotCommand, x, y, p1.X, p1.Y);

	if (((lastPlotCommand & 0xF8) == 0xD8) && ((lastPlotCommand & 0xFB) != (command & 0xFB))) {
		debug_log("vdu_plot: last plot was a path, but different command detected\n\r");
		// We're not doing a path any more - so commit it
		plotPath(0, lastPlotCommand & 0x03);
	}

	setGraphicsOptions(mode);

	// if (mode != 0 && mode != 4) {
	if (mode & 0x03) {
		switch (operation) {
			case 0x00:	// line
				plotLine(false, false, false, false);
				break;
			case 0x08:	// line, omitting last point
				plotLine(false, true, false, false);
				break;
			case 0x10:	// dot-dash line
				plotLine(false, false, true, false);
				break;
			case 0x18:	// dot-dash line, omitting last point
				plotLine(false, true, true, false);
				break;
			case 0x30:	// dot-dash line, omitting first, pattern continued
				plotLine(true, false, true, false);
				break;
			case 0x38:	// dot-dash line, omitting both, pattern continued
				plotLine(true, true, true, false);
				break;
			case 0x20:	// solid line, first point omitted
				plotLine(true, false, false, false);
				break;
			case 0x28:	// solid line, first and last points omitted
				plotLine(true, true, false, false);
				break;
			case 0x40:	// point
				plotPoint();
				break;
			case 0x48:	// line fill left/right to non-bg
				fillHorizontalLine(true, false, gbg);
				break;
			case 0x50:	// triangle fill
				setGraphicsFill(mode);
				plotTriangle();
				break;
			case 0x58:	// line fill right to bg
				fillHorizontalLine(false, true, gbg);
				break;
			case 0x60:	// rectangle fill
				setGraphicsFill(mode);
				plotRectangle();
				break;
			case 0x68:	// line fill left/left to fg
				fillHorizontalLine(true, true, gfg);
				break;
			case 0x70:	// parallelogram fill
				setGraphicsFill(mode);
				plotParallelogram();
				break;
			case 0x78:	// line fill right to non-fg
				fillHorizontalLine(false, false, gfg);
				break;
			case 0x80:	// flood to non-bg
			case 0x88:	// flood to fg
				debug_log("plot flood fill not implemented\n\r");
				break;
			case 0x90:	// circle outline
				plotCircle(false);
				break;
			case 0x98:	// circle fill
				setGraphicsFill(mode);
				plotCircle(true);
				break;
			case 0xA0:	// circular arc
				plotArc();
				break;
			case 0xA8:	// circular segment
				setGraphicsFill(mode);
				plotSegment();
				break;
			case 0xB0:	// circular sector
				setGraphicsFill(mode);
				plotSector();
				break;
			case 0xB8:	// copy/move
				plotCopyMove(mode);
				break;
			case 0xC0:	// ellipse outline
			case 0xC8:	// ellipse fill
				// fab-gl's ellipse isn't compatible with BBC BASIC
				debug_log("plot ellipse not implemented\n\r");
				break;
			case 0xD8:	// plot path (unassigned on Acorn and other BBC BASIC versions)
				plotPath(mode, lastPlotCommand & 0x03);
				// vdu_plotPath(mode);
				pending = true;
				break;
			case 0xD0:	// unassigned ("Font printing" (do not use) in RISC OS)
			case 0xE0:	// unassigned
				debug_log("plot operation unassigned\n\r");
				break;
			case 0xE8:	// Bitmap plot
				plotBitmap(mode);
				break;
			case 0xF0:	// unassigned
			case 0xF8:	// Swap rectangle (BBC Basic for Windows extension)
				// only actually supports "foreground" codes &F9 and &FD
				debug_log("plot swap rectangle not implemented\n\r");
				break;
		}
	}
	lastPlotCommand = command;
	moveTo();
	return pending;
}

void Context::plotPending(int16_t peeked) {
	// Currently pending plot commands can only be flagged for path drawing
	// In future we may need to check the lastPlotCommand here
	if (peeked == -1 || peeked != 25) {
		plotPath(0, lastPlotCommand & 0x03);
	}
}

// Plot path handler
//
// void Context::vdu_plotPath(uint8_t mode) {
// 	auto lastMode = lastPlotCommand & 0x03;
// 	// call the graphics handler for plotPath
// 	plotPath(mode, lastMode);

// 	// work out if our next VDU command byte is another PLOT command
// 	// if it isn't, then we should commit the path
// 	auto peeked = peekByte_t(FAST_COMMS_TIMEOUT);
// 	if (peeked == -1 || peeked != 25) {
// 		// timed out, or not a plot command, so commit the path
// 		debug_log("vdu_plotPath: committing path on timeout or no subsequent plot (peeked = %d)\n\r", peeked);
// 		plotPath(0, mode);
// 	}
// 	// otherwise just continue
// }


// Change the currently selected font
//
void Context::changeFont(const fabgl::FontInfo * f, uint8_t flags) {
	if (!ttxtMode) {
		if (f->flags & FONTINFOFLAGS_VARWIDTH) {
			debug_log("changeFont: variable width fonts not supported - yet\n\r");
			return;
		}
		// adjust our cursor position so baseline matches new font
		// TODO this movement will need to bear in mind cursor behaviour
		// as if our cursor is moving right to left we'll also need to adjust our x position
		// if we're moving bottom to top we'll need to adjust our y position, based on height rather than ascent
		// and may not want to adjust our y position if we're moving top to bottom
		if (flags & FONT_SELECTFLAG_ADJUSTBASE) {
			int8_t x = 0;
			int8_t y = 0;
			if (cursorBehaviour.flipXY) {
				// cursor is moving vertically, so we need to adjust y by font height
				if (cursorBehaviour.invertHorizontal) {
					y = font->height - f->height;
				}
				if (cursorBehaviour.invertVertical) {
					// cursor x movement is right to left, so we need to adjust x by font width
					x = font->width - f->width;
				}
			} else {
				// normal x and y movement
				// so we always need to adjust our y position
				y = font->ascent - f->ascent;
				if (cursorBehaviour.invertHorizontal) {
					// cursor is moving right to left, so we need to adjust x by font width
					x = font->width - f->width;
				}
			}
			cursorRelativeMove(x, y);
			debug_log("changeFont - relative adjustment is %d, %d\n\r", x, y);
		}
		font = f;
		if (textCursorActive()) {
			textFont = f;
		} else {
			graphicsFont = f;
		}
		canvas->selectFont(f);
	}
}

bool Context::usingSystemFont() {
	return font == &FONT_AGON;
}

bool Context::cmpChar(uint8_t * c1, uint8_t *c2, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		if (*c1++ != *c2++) {
			return false;
		}
	}
	return true;
}

// Try and match a character at given text position
//
char Context::getScreenChar(uint8_t x, uint8_t y) {
	Point p = { x * (font->width == 0 ? 8 : font->width), y * font->height };
	
	return getScreenChar(p);
}

// Try and match a character at given pixel position
//
char Context::getScreenCharAt(uint16_t px, uint16_t py) {
	return getScreenChar(translateCanvas(scale(px, py)));
}

char Context::getScreenChar(Point p) {
	if (font->flags & FONTINFOFLAGS_VARWIDTH) {
		debug_log("getScreenChar: variable width fonts not supported\n\r");
		return 0;
	}

	// Do some bounds checking first
	//
	if (p.X >= canvasW - font->width || p.Y >= canvasH - font->height) {
		return 0;
	}
	if (ttxtMode) {
		return ttxt_instance.get_screen_char(p.X, p.Y);
	} else {
		waitPlotCompletion();
		uint8_t charWidthBytes = (font->width + 7) / 8;
		uint8_t charSize = charWidthBytes * font->height;
		uint8_t	charData[charSize];
		uint8_t R = tbg.R;
		uint8_t G = tbg.G;
		uint8_t B = tbg.B;

		// Now scan the screen and get the 8 byte pixel representation in charData
		//
		for (uint8_t y = 0; y < font->height; y++) {
			uint8_t readByte = 0;
			for (uint8_t x = 0; x < font->width; x++) {
				if ((x % 8) == 0) {
					readByte = 0;
				}
				RGB888 pixel = canvas->getPixel(p.X + x, p.Y + y);
				if (!(pixel.R == R && pixel.G == G && pixel.B == B)) {
					readByte |= (0x80 >> (x % 8));
				}
				if ((x % 8) == 7) {
					charData[(y * charWidthBytes) + (x / 8)] = readByte;
				}
			}
			charData[((y + 1) * charWidthBytes) - 1] = readByte;
		}

		// Finally try and match with the character set array
		// starts at space character (32) and goes beyond the normal ASCII range
		// Character checked is ANDed with 0xFF, so we check 32-255 then 0-31
		// thus allowing characters 0-31 to be checked after conventional characters
		// as by default those characters are the same as space
		//
		for (auto i = 32; i <= (255 + 31); i++) {
			uint8_t c = i & 0xFF;
			if (cmpChar(charData, getCharPtr(font, c), charSize)) {
				debug_log("getScreenChar: matched character %d\n\r", c);
				return c;
			}
		}
	}
	return 0;
}

// Get pixel value at screen coordinates
//
RGB888 Context::getPixel(uint16_t x, uint16_t y) {
	Point p = translateCanvas(scale(x, y));
	if (p.X >= 0 && p.Y >= 0 && p.X < canvasW && p.Y < canvasH) {
		return canvas->getPixel(p.X, p.Y);
	}
	return RGB888(0,0,0);
}

// Horizontal scan until we find a pixel not non-equalto given colour
// returns x coordinate for the last pixel before the match
uint16_t Context::scanH(int16_t x, int16_t y, RGB888 colour, int8_t direction = 1) {
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
uint16_t Context::scanHToMatch(int16_t x, int16_t y, RGB888 colour, int8_t direction = 1) {
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

// Update selected colours based on palette change in 64 colour modes
//
void Context::updateColours(uint8_t l, uint8_t index) {
	auto lookedup = colourLookup[index];
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

// Get currently set colour values
//
bool Context::getColour(uint8_t colour, RGB888 * pixel) {
	switch (colour) {
		case 128:	// text foreground
			*pixel = tfg;
			break;
		case 129:	// text background
			*pixel = tbg;
			break;
		case 130:	// graphics foreground
			*pixel = gfg;
			break;
		case 131:	// graphics background
			*pixel = gbg;
			break;
		default:
			return false;
	}
	return true;
}

// Get the paint options for a given GCOL mode
//
fabgl::PaintOptions Context::getPaintOptions(fabgl::PaintMode mode, fabgl::PaintOptions priorPaintOptions) {
	fabgl::PaintOptions p = priorPaintOptions;
	p.mode = mode;
	return p;
}

// Restore palette to default for current mode
//
void Context::restorePalette() {
	auto depth = getVGAColourDepth();
	gbgc = tbgc = 0;
	gfgc = tfgc = 15 % depth;
	if (!ttxtMode) {
		switch (depth) {
			case  2: resetPalette(defaultPalette02); break;
			case  4: resetPalette(defaultPalette04); break;
			case  8: resetPalette(defaultPalette08); break;
			case 16: resetPalette(defaultPalette10); break;
			case 64: resetPalette(defaultPalette40); break;
		}
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
void Context::setTextColour(uint8_t colour) {
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
void Context::setGraphicsColour(uint8_t mode, uint8_t colour) {
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
void Context::clearViewport(ViewportType type) {
	if (ttxtMode) {
		ttxt_instance.cls();
	} else {
		if (canvas) {
			auto viewport = getViewport(type);
			canvas->fillRectangle(*viewport);
		}
	}
}

//// Graphics drawing routines

// Push point to list
//
void Context::pushPoint(Point p) {
	rp1 = Point(p.X - p1.X, p.Y - p1.Y);
	p3.X = p2.X;
	p3.Y = p2.Y;
	p2.X = p1.X;
	p2.Y = p1.Y;
	p1.X = p.X;
	p1.Y = p.Y;
}
void Context::pushPoint(uint16_t x, uint16_t y) {
	up1 = Point(x, y);
	pushPoint(translateCanvas(scale(x, y)));
}
void Context::pushPointRelative(int16_t x, int16_t y) {
	pushPoint(up1.X + x, up1.Y + y);
}

// Get rect from last two points
//
Rect Context::getGraphicsRect() {
	return Rect(std::min(p1.X, p2.X), std::min(p1.Y, p2.Y), std::max(p1.X, p2.X), std::max(p1.Y, p2.Y));
}

// Set up canvas for drawing graphics
//
void Context::setGraphicsOptions(uint8_t mode) {
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
void Context::setGraphicsFill(uint8_t mode) {
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
void Context::moveTo() {
	canvas->moveTo(p1.X, p1.Y);
}

// Line plot
//
void Context::plotLine(bool omitFirstPoint, bool omitLastPoint, bool usePattern, bool resetPattern) {
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
void Context::fillHorizontalLine(bool scanLeft, bool match, RGB888 matchColor) {
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
void Context::plotPoint() {
	canvas->setPixel(p1.X, p1.Y);
}

// Triangle plot
//
void Context::plotTriangle() {
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
void Context::plotPath(uint8_t mode, uint8_t lastMode) {
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
void Context::plotRectangle() {
	canvas->fillRectangle(p2.X, p2.Y, p1.X, p1.Y);
}

// Parallelogram plot
//
void Context::plotParallelogram() {
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
void Context::plotCircle(bool filled) {
	auto size = 2 * sqrt(rp1.X * rp1.X + (rp1.Y * rp1.Y * (rectangularPixels ? 4 : 1)));
	if (filled) {
		canvas->fillEllipse(p2.X, p2.Y, size, rectangularPixels ? size / 2 : size);
	} else {
		canvas->drawEllipse(p2.X, p2.Y, size, rectangularPixels ? size / 2 : size);
	}
}

// Arc plot
void Context::plotArc() {
	debug_log("plotArc: (%d,%d) -> (%d,%d), (%d,%d)\n\r", p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
	canvas->drawArc(p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
}

// Segment plot
void Context::plotSegment() {
	debug_log("plotSegment: (%d,%d) -> (%d,%d), (%d,%d)\n\r", p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
	canvas->fillSegment(p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
}

// Sector plot
void Context::plotSector() {
	debug_log("plotSector: (%d,%d) -> (%d,%d), (%d,%d)\n\r", p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
	canvas->fillSector(p3.X, p3.Y, p2.X, p2.Y, p1.X, p1.Y);
}

// Copy or move a rectangle
//
void Context::plotCopyMove(uint8_t mode) {
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
void Context::plotBitmap(uint8_t mode) {
	if ((mode & 0x03) == 0x03) {
		// get a copy of gpobg, without changing it's paint mode
		auto paintOptions = getPaintOptions(gpobg.mode, gpobg);
		// swapFGBG on bitmap plots indicates to plot using pen color instead of bitmap
		paintOptions.swapFGBG = true;
		canvas->setPaintOptions(paintOptions);
	}
	drawBitmap(p1.X, p1.Y, true, false);
}

// draw bitmap
//
void Context::drawBitmap(uint16_t x, uint16_t y, bool compensateHeight, bool forceSet) {
	auto bitmap = getBitmap();
	if (bitmap) {
		if (forceSet) {
			auto options = getPaintOptions(fabgl::PaintMode::Set, gpofg);
			canvas->setPaintOptions(options);
		}
		canvas->drawBitmap(x, (compensateHeight && logicalCoords) ? (y + 1 - bitmap->height) : y, bitmap.get());
	} else {
		debug_log("drawBitmap: bitmap %d not found\n\r", currentBitmap);
	}
}

// Character plot
//
void Context::plotCharacter(char c) {
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
void Context::plotBackspace() {
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
inline void Context::setCharacterOverwrite(bool overwrite) {
	canvas->setGlyphOptions(GlyphOptions().FillBackground(overwrite));
}

// Set a clipping rectangle
//
void Context::setClippingRect(Rect rect) {
	canvas->setClippingRect(rect);
}

// Draw cursor
//
void Context::drawCursor(Point p) {
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
void Context::cls(bool resetViewports) {
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
		clearViewport(ViewportType::TextViewport);
	}
	if (hasActiveSprites()) {
		activateSprites(0);
		clearViewport(ViewportType::TextViewport);
	}
	cursorHome();
	setPagedMode(pagedMode);
}

// Clear the graphics area
//
void Context::clg() {
	if (canvas) {
		canvas->setPenColor(gfg);
		canvas->setBrushColor(gbg);
		canvas->setPaintOptions(gpobg);
		clearViewport(ViewportType::GraphicsViewport);
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
int8_t Context::change_mode(uint8_t mode) {
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
		canvas->selectFont(&FONT_AGON);
	}
	setCharacterOverwrite(true);
	canvas->setPenWidth(1);

	canvasW = canvas->getWidth();
	canvasH = canvas->getHeight();
	logicalScaleX = LOGICAL_SCRW / (double)canvasW;
	logicalScaleY = LOGICAL_SCRH / (double)canvasH;

	// simple heuristic to determine rectangular pixels
	rectangularPixels = ((float)canvasW / (float)canvasH) > 2;
	font = canvas->getFontInfo();
	textFont = font;
	graphicsFont = font;
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
void Context::set_mode(uint8_t mode) {
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

void Context::scrollRegion(ViewportType viewport, uint8_t direction, int16_t movement) {
	scrollRegion(getViewport(viewport), direction, movement);
}

void Context::scrollRegion(Rect * region, uint8_t direction, int16_t movement) {
	auto moveX = 0;
	auto moveY = 0;
	canvas->setScrollingRegion(region->X1, region->Y1, region->X2, region->Y2);
	canvas->setPenColor(tbg);
	canvas->setBrushColor(tbg);
	canvas->setPaintOptions(tpo);
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
		if (ttxtMode) {
			ttxt_instance.scroll(moveX, moveY);
		} else {
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

void Context::setLineThickness(uint8_t thickness) {
	canvas->setPenWidth(thickness);
}

void Context::setDottedLinePattern(uint8_t pattern[8]) {
	auto linePattern = fabgl::LinePattern();
	linePattern.setPattern(pattern);
	canvas->setLinePattern(linePattern);
}

void Context::setDottedLinePatternLength(uint8_t length) {
	if (length == 0) {
		// reset the line pattern
		auto linePattern = fabgl::LinePattern();
		canvas->setLinePattern(linePattern);
		canvas->setLinePatternLength(8);
		return;
	}
	canvas->setLinePatternLength(length);
}

#endif // CONTEXT_GRAPHICS_H

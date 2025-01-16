#ifndef CONTEXT_FONTS_H
#define CONTEXT_FONTS_H

#include <algorithm>

#include <fabgl.h>

#include "agon.h"
#include "agon_fonts.h"
#include "context.h"

// Private font management functions

// Get pointer to our currently selected font
//
const fabgl::FontInfo * Context::getFont() {
	return font == nullptr ? canvas->getFontInfo() : font.get();
}

void Context::changeFont(std::shared_ptr<fabgl::FontInfo> newFont, std::shared_ptr<BufferStream> fontData, uint8_t flags) {
	if (ttxtMode) {
		debug_log("changeFont: teletext mode does not support font changes\n\r");
		return;
	}
	auto newFontPtr = newFont == nullptr ? &FONT_AGON : newFont.get();
	auto oldFontPtr = getFont();

	if (newFontPtr->flags & FONTINFOFLAGS_VARWIDTH) {
		debug_log("changeFont: variable width fonts not supported - yet\n\r");
		return;
	}
	// adjust our cursor position, according to flags
	if (flags & FONT_SELECTFLAG_ADJUSTBASE) {
		int8_t x = 0;
		int8_t y = 0;
		if (cursorBehaviour.flipXY) {
			// cursor is moving vertically, so we need to adjust y by font height
			if (cursorBehaviour.invertHorizontal) {
				y = oldFontPtr->height - newFontPtr->height;
			}
			if (cursorBehaviour.invertVertical) {
				// cursor x movement is right to left, so we need to adjust x by font width
				x = oldFontPtr->width - newFontPtr->width;
			}
		} else {
			// normal x and y movement
			// so we always need to adjust our y position
			y = oldFontPtr->ascent - newFontPtr->ascent;
			if (cursorBehaviour.invertHorizontal) {
				// cursor is moving right to left, so we need to adjust x by font width
				x = oldFontPtr->width - newFontPtr->width;
			}
		}
		cursorRelativeMove(x, y);
		debug_log("changeFont - relative adjustment is %d, %d\n\r", x, y);
	}

	canvas->selectFont(newFontPtr);
	font = newFont;
	if (textCursorActive()) {
		textFont = newFont;
		textFontData = fontData;
	} else {
		graphicsFont = newFont;
		graphicsFontData = fontData;
	}
}

bool Context::cmpChar(uint8_t * c1, uint8_t *c2, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		if (*c1++ != *c2++) {
			return false;
		}
	}
	return true;
}

char Context::getScreenChar(Point p) {
	auto fontPtr = getFont();
	if (fontPtr->flags & FONTINFOFLAGS_VARWIDTH) {
		debug_log("getScreenChar: variable width fonts not supported\n\r");
		return 0;
	}
	uint8_t fontWidth = fontPtr->width;
	uint8_t fontHeight = fontPtr->height;

	// Do some bounds checking first
	//
	if (p.X >= canvasW - fontWidth || p.Y >= canvasH - fontHeight) {
		return 0;
	}
	if (ttxtMode) {
		return ttxt_instance.get_screen_char(p.X, p.Y);
	} else {
		waitPlotCompletion();
		uint8_t charWidthBytes = (fontWidth + 7) / 8;
		uint8_t charSize = charWidthBytes * fontHeight;
		uint8_t	charData[charSize];
		uint8_t R = tbg.R;
		uint8_t G = tbg.G;
		uint8_t B = tbg.B;

		// Now scan the screen and get the 8 byte pixel representation in charData
		//
		for (uint8_t y = 0; y < fontHeight; y++) {
			uint8_t readByte = 0;
			for (uint8_t x = 0; x < fontWidth; x++) {
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

// Set character overwrite mode (background fill)
//
inline void Context::setCharacterOverwrite(bool overwrite) {
	canvas->setGlyphOptions(GlyphOptions().FillBackground(overwrite));
}


// Public font management functions

// Change the currently selected font
//
void Context::changeFont(uint16_t newFontId, uint8_t flags) {
	if (ttxtMode) {
		debug_log("changeFont: teletext mode does not support font changes\n\r");
		return;
	}

	bool isSystemFont = newFontId == 65535;
	if (!isSystemFont && fonts.find(newFontId) == fonts.end()) {
		debug_log("changeFont: font %d not found\n\r", newFontId);
		return;
	}

	auto newFont = isSystemFont ? nullptr : fonts[newFontId];
	auto fontData = isSystemFont ? nullptr : buffers[newFontId][0];
	changeFont(newFont, fontData, flags);
}

void Context::resetFonts() {
	if (!ttxtMode) {
		canvas->selectFont(&FONT_AGON);
	}
	font = nullptr;
	textFont = nullptr;
	graphicsFont = nullptr;
	textFontData = nullptr;
	graphicsFontData = nullptr;
	setCharacterOverwrite(true);
}

bool Context::usingSystemFont() {
	return font == nullptr;
}

// Try and match a character at given text position
//
char Context::getScreenChar(uint8_t x, uint8_t y) {
	auto font = getFont();
	Point p = { x * (font->width == 0 ? 8 : font->width), y * font->height };
	
	return getScreenChar(p);
}

// Try and match a character at given pixel position
//
char Context::getScreenCharAt(uint16_t px, uint16_t py) {
	return getScreenChar(toScreenCoordinates(px, py));
}

void Context::mapCharToBitmap(uint8_t c, uint16_t bitmapId) {
	auto bitmap = getBitmap(bitmapId);
	if (bitmap) {
		charToBitmap[c] = bitmapId;
	} else {
		debug_log("mapCharToBitmap: bitmap %d not found\n\r", bitmapId);
		charToBitmap[c] = 65535;
	}
}

void Context::unmapBitmapFromChars(uint16_t bitmapId) {
	// remove this bitmap from the charToBitmap mapping
	for (auto it = charToBitmap.begin(); it != charToBitmap.end(); it++) {
		if (*it == bitmapId) {
			*it = 65535;
		}
	}
}

void Context::resetCharToBitmap() {
	std::fill(charToBitmap.begin(), charToBitmap.end(), 65535);
}

#endif // CONTEXT_FONTS_H

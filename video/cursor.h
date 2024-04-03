#ifndef CURSOR_H
#define CURSOR_H

#include <fabgl.h>

#include "agon_ps2.h"
#include "graphics.h"
#include "viewport.h"
// TODO remove this, somehow
#include "vdp_protocol.h"

extern const fabgl::FontInfo * font;
extern uint8_t	cursorVStart;
extern uint8_t	cursorVEnd;
extern uint8_t	cursorHStart;
extern uint8_t	cursorHEnd;

extern void drawCursor(Point p);
extern void scrollRegion(Rect * r, uint8_t direction, int16_t amount);

typedef union {
	uint8_t value = 0;
	struct {
		uint8_t scrollProtect : 1;
		uint8_t invertHorizontal : 1;
		uint8_t invertVertical : 1;
		uint8_t flipXY : 1;
		uint8_t yWrap : 1;
		uint8_t xHold : 1;
		uint8_t grNoSpecialActions : 1;
		uint8_t _undefined : 1;
	};
} CursorBehaviour;

Point			textCursor;						// Text cursor
Point *			activeCursor = &textCursor;		// Pointer to the active text cursor (textCursor or p1)
bool			cursorEnabled = true;			// Cursor visibility
bool			cursorFlashing = true;			// Cursor is flashing
uint16_t		cursorFlashRate = CURSOR_PHASE;	// Cursor flash rate
CursorBehaviour cursorBehaviour;				// New cursor behavior
bool 			pagedMode = false;				// Is output paged or not? Set by VDU 14 and 15
uint8_t			pagedModeCount = 0;				// Scroll counter for paged mode

void cursorDown(bool moveOnly);
void cursorUp(bool moveOnly);

// Render a cursor at the current screen position
//
void do_cursor() {
	if (cursorEnabled) {
		drawCursor(textCursor);
	}
}

inline Point * getTextCursor() {
	return &textCursor;
}

inline bool textCursorActive() {
	return activeCursor == &textCursor;
}

inline void setActiveCursor(Point * cursor) {
	activeCursor = cursor;
}

inline void setCursorBehaviour(uint8_t setting, uint8_t mask = 0xFF) {
	cursorBehaviour.value = (cursorBehaviour.value & mask) ^ setting;
}

// Adjustments to ensure cursor position sits at the nearest character boundary
int getXAdjustment() {
	return activeViewport->width() % font->width;
}

int getYAdjustment() {
	return activeViewport->height() % font->height;
}

Point getNormalisedCursorPosition(Point * cursor = activeCursor) {
	Point p;
	if (cursorBehaviour.flipXY) {
		// our normalised Y needs to take values from X and vice versa
		if (cursorBehaviour.invertHorizontal) {
			p.Y = activeViewport->X2 - cursor->X;
		} else {
			p.Y = cursor->X - activeViewport->X1;
		}
		if (cursorBehaviour.invertVertical) {
			p.X = activeViewport->Y2 - cursor->Y;
		} else {
			p.X = cursor->Y - activeViewport->Y1;
		}
	} else {
		if (cursorBehaviour.invertHorizontal) {
			p.X = activeViewport->X2 - cursor->X;
		} else {
			p.X = cursor->X - activeViewport->X1;
		}
		if (cursorBehaviour.invertVertical) {
			p.Y = activeViewport->Y2 - cursor->Y;
		} else {
			p.Y = cursor->Y - activeViewport->Y1;
		}
	}
	return p;
}

int getNormalisedViewportWidth() {
	if (cursorBehaviour.flipXY) {
		return activeViewport->height() - getYAdjustment();
	}
	return activeViewport->width() - getXAdjustment();
}

int getNormalisedViewportHeight() {
	if (cursorBehaviour.flipXY) {
		auto height = activeViewport->width() - getXAdjustment();
		if (!cursorBehaviour.invertHorizontal) {
			height -= font->width - 1;
		}
		return height;
	}
	auto height = activeViewport->height() - getYAdjustment();
	if (!cursorBehaviour.invertVertical) {
		height -= font->height - 1;
	}
	return height;
}

bool cursorIsOffRight() {
	return getNormalisedCursorPosition().X >= getNormalisedViewportWidth();
}

bool cursorIsOffLeft() {
	return getNormalisedCursorPosition().X < 0;
}

bool cursorIsOffTop() {
	return getNormalisedCursorPosition().Y < 0;
}

bool cursorIsOffBottom() {
	return getNormalisedCursorPosition().Y >= getNormalisedViewportHeight();
}

// Move the active cursor to the leftmost position in the viewport
//
void cursorCR(Rect * viewport = activeViewport) {
	if (cursorBehaviour.flipXY) {
		activeCursor->Y = cursorBehaviour.invertVertical ? (viewport->Y2 + 1 - font->height - getYAdjustment()) : viewport->Y1;
	} else {
		activeCursor->X = cursorBehaviour.invertHorizontal ? (viewport->X2 + 1 - font->width - getXAdjustment()) : viewport->X1;
	}
}

void cursorEndRow(Rect * viewport = activeViewport) {
	if (cursorBehaviour.flipXY) {
		activeCursor->Y = cursorBehaviour.invertVertical ? viewport->Y1 : (viewport->Y2 + 1 - font->height - getYAdjustment());
	} else {
		activeCursor->X = cursorBehaviour.invertHorizontal ? viewport->X1 : (viewport->X2 + 1 - font->width - getXAdjustment());
	}
}

void cursorTop(Rect * viewport = activeViewport) {
	if (cursorBehaviour.flipXY) {
		activeCursor->X = cursorBehaviour.invertHorizontal ? (viewport->X2 + 1 - font->width - getXAdjustment()) : viewport->X1;
	} else {
		activeCursor->Y = cursorBehaviour.invertVertical ? (viewport->Y2 + 1 - font->height - getYAdjustment()) : viewport->Y1;
	}
}

void cursorEndCol(Rect * viewport = activeViewport) {
	if (cursorBehaviour.flipXY) {
		activeCursor->X = cursorBehaviour.invertHorizontal ? viewport->X1 : (viewport->X2 + 1 - font->width - getXAdjustment());
	} else {
		activeCursor->Y = cursorBehaviour.invertVertical ? viewport->Y1 : (viewport->Y2 + 1 - font->height - getYAdjustment());
	}
}

// Check if the cursor is off the edge of the viewport and take appropriate action
// returns true if the cursor wrapped, false if no action was taken or the screen scrolled
bool cursorScrollOrWrap() {
	bool offLeft = cursorIsOffLeft();
	bool offRight = cursorIsOffRight();
	bool offTop = cursorIsOffTop();
	bool offBottom = cursorIsOffBottom();
	if (!offLeft && !offRight && !offTop && !offBottom) {
		// cursor within current viewport, so do nothing
		return false;
	}

	if (textCursorActive() && !cursorBehaviour.yWrap) {
		// text cursor, scrolling for our Y direction is enabled
		if (offTop) {
			// scroll screen down by 1 line
			scrollRegion(activeViewport, 6, 0);
			// move cursor down until it's within the viewport
			do {
				cursorDown(true);
			} while (cursorIsOffTop());
			return false;
		}
		if (offBottom) {
			// scroll screen up by 1 line
			scrollRegion(activeViewport, 7, 0);
			// move cursor up until it's within the viewport
			do {
				cursorUp(true);
			} while (cursorIsOffBottom());
			return false;
		}
	}

	// if we get here we have a graphics cursor, or text cursor with wrap enabled
	if (!textCursorActive() && cursorBehaviour.grNoSpecialActions) {
		return false;
	}

	// if we've reached here, we're wrapping, so move cursor to the opposite edge
	if (offLeft) {
		cursorEndRow();
	}
	if (offRight) {
		cursorCR();
	}
	if (offTop) {
		cursorEndCol();
	}
	if (offBottom) {
		cursorTop();
	}
	return true;
}

// Move the active cursor down a line
//
void cursorDown(bool moveOnly = false) {
	if (cursorBehaviour.flipXY) {
		activeCursor->X += (cursorBehaviour.invertHorizontal ? -font->width : font->width);
	} else {
		activeCursor->Y += (cursorBehaviour.invertVertical ? -font->height : font->height);
	}
	if (moveOnly) {
		return;
	}
	//
	// handle paging if we need to
	//
	if (textCursorActive() && pagedMode) {
		pagedModeCount++;
		if (pagedModeCount >= (
				cursorBehaviour.flipXY ? (activeViewport->width()) / font->width
					: (activeViewport->height()) / font->height
			)
		) {
			pagedModeCount = 0;
			uint8_t ascii;
			uint8_t vk;
			uint8_t down;
			if (!wait_shiftkey(&ascii, &vk, &down)) {
				// ESC pressed
				uint8_t packet[] = {
					ascii,
					0,
					vk,
					down,
				};
				// TODO replace this, somehow
				send_packet(PACKET_KEYCODE, sizeof packet, packet);
			}
		}
	}
	//
	// Check if scroll required
	//
	cursorScrollOrWrap();
}

// Move the active cursor up a line
//
void cursorUp(bool moveOnly = false) {
	if (cursorBehaviour.flipXY) {
		activeCursor->X += (cursorBehaviour.invertHorizontal ? font->width : -font->width);
	} else {
		activeCursor->Y += (cursorBehaviour.invertVertical ? font->height : -font->height);
	}
	if (moveOnly) {
		return;
	}
	cursorScrollOrWrap();
}

// Move the active cursor back one character
//
void cursorLeft() {
	if (cursorBehaviour.flipXY) {
		activeCursor->Y += (cursorBehaviour.invertVertical ? font->height : -font->height);
	} else {
		activeCursor->X += (cursorBehaviour.invertHorizontal ? font->width : -font->width);
	}
	if (cursorScrollOrWrap()) {
		// wrapped, so move cursor up a line
		cursorUp();
	}
}

void cursorAutoNewline() {
	if (cursorIsOffRight() && (textCursorActive() || !cursorBehaviour.grNoSpecialActions)) {
		cursorCR();
		cursorDown();
	}
}

// Advance the active cursor right one character
//
void cursorRight(bool scrollProtect = false) {
	// deal with any pending newline that we may have
	cursorAutoNewline();

	if (cursorBehaviour.flipXY) {
		activeCursor->Y += (cursorBehaviour.invertVertical ? -font->height : font->height);
	} else {
		activeCursor->X += (cursorBehaviour.invertHorizontal ? -font->width : font->width);
	}
	if (!scrollProtect) {
		cursorAutoNewline();
	}
}

// Move the active cursor to the top-left position in the viewport
//
void cursorHome(Point * cursor = activeCursor, Rect * viewport = activeViewport) {
	cursorCR(viewport);
	cursorTop(viewport);
}

// TAB(x,y)
//
void cursorTab(uint8_t x, uint8_t y) {
	int xPos, yPos;
	if (cursorBehaviour.flipXY) {
		if (cursorBehaviour.invertHorizontal) {
			xPos = activeViewport->X2 - ((y + 1) * font->width) - getXAdjustment();
		} else {
			xPos = activeViewport->X1 + (y * font->width);
		}
		if (cursorBehaviour.invertVertical) {
			yPos = activeViewport->Y2 - ((x + 1) * font->height) - getYAdjustment();
		} else {
			yPos = activeViewport->Y1 + (x * font->height);
		}
	} else {
		if (cursorBehaviour.invertHorizontal) {
			xPos = activeViewport->X2 - ((x + 1) * font->width) - getXAdjustment();
		} else {
			xPos = activeViewport->X1 + (x * font->width);
		}
		if (cursorBehaviour.invertVertical) {
			yPos = activeViewport->Y2 - ((y + 1) * font->height) - getYAdjustment();
		} else {
			yPos = activeViewport->Y1 + (y * font->height);
		}
	}
	if (activeViewport->X1 <= xPos
		&& xPos < activeViewport->X2 - getXAdjustment()
		&& activeViewport->Y1 <= yPos
		&& yPos < activeViewport->Y2 - getYAdjustment()
	) {
		activeCursor->X = xPos;
		activeCursor->Y = yPos;
	}
}

void setPagedMode(bool mode = pagedMode) {
	pagedMode = mode;
	pagedModeCount = 0;
}

// Reset basic cursor control
// used when changing screen modes
//
void resetCursor() {
	setActiveCursor(getTextCursor());
	// visual cursor appearance reset
	cursorEnabled = true;
	cursorFlashing = true;
	cursorFlashRate = CURSOR_PHASE;
	cursorVStart = 0;
	cursorVEnd = font->height - 1;
	cursorHStart = 0;
	cursorHEnd = font->width - 1;
	// cursor behaviour however is _not_ reset here
	cursorHome();
	setPagedMode(false);
}

inline void enableCursor(bool enable = true) {
	cursorEnabled = enable;
}

void ensureCursorInViewport(Rect viewport) {
	if (textCursor.X < viewport.X1
		|| textCursor.X > viewport.X2 - getXAdjustment()
		|| textCursor.Y < viewport.Y1
		|| textCursor.Y > viewport.Y2 - getYAdjustment()
	) {
		cursorHome(&textCursor, &viewport);
	}
}

#endif	// CURSOR_H

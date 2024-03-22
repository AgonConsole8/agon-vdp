#ifndef CURSOR_H
#define CURSOR_H

#include <fabgl.h>

#include "agon_ps2.h"
#include "graphics.h"
#include "viewport.h"
// TODO remove this, somehow
#include "vdp_protocol.h"

extern uint8_t	fontW;
extern uint8_t	fontH;
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

// Move the active cursor to the leftmost position in the viewport
//
void cursorCR() {
	if (cursorBehaviour.flipXY) {
		activeCursor->Y = cursorBehaviour.invertVertical ? (activeViewport->Y2 + 1 - fontH) : activeViewport->Y1;
	} else {
		activeCursor->X = cursorBehaviour.invertHorizontal ? (activeViewport->X2 + 1 - fontW) : activeViewport->X1;
	}
}

bool cursorIsOffRight() {
	if (cursorBehaviour.flipXY) {
		if (cursorBehaviour.invertHorizontal) {
			return activeCursor->Y < activeViewport->Y1;
		}
		return activeCursor->Y >= activeViewport->Y2;
	}
	if (cursorBehaviour.invertHorizontal) {
		return activeCursor->X < activeViewport->X1;
	}
	return activeCursor->X >= activeViewport->X2;
}

bool cursorScrollOrWrap() {
	bool outsideY = activeCursor->Y < activeViewport->Y1 || activeCursor->Y >= activeViewport->Y2;
	bool outsideX = activeCursor->X < activeViewport->X1 || activeCursor->X >= activeViewport->X2;
	if (!outsideX && !outsideY) {
		// cursor within current viewport, so do nothing
		return false;
	}

	if (textCursorActive() && !cursorBehaviour.yWrap) {
		// text cursor, scrolling for our Y direction is enabled
		if (cursorBehaviour.flipXY && outsideX || !cursorBehaviour.flipXY && outsideY) {
			// we are outside of our cursor vertical direction
			// scroll in appropriate direction by 1 character (movement amount 0)
			// our direction will be a 6 or 7, depending on whether we're outside the "top" or "bottom"
			if (outsideX) {
				if (activeCursor->X < activeViewport->X1) {
					scrollRegion(activeViewport, cursorBehaviour.invertVertical ? 7 : 6, 0);
					activeCursor->X = activeViewport->X1;
				} else {
					scrollRegion(activeViewport, cursorBehaviour.invertVertical ? 6 : 7, 0);
					activeCursor->X = activeViewport->X2 + 1 - fontW;
				}
			} else {
				if (activeCursor->Y < activeViewport->Y1) {
					scrollRegion(activeViewport, cursorBehaviour.invertHorizontal ? 7 : 6, 0);
					activeCursor->Y = activeViewport->Y1;
				} else {
					scrollRegion(activeViewport, cursorBehaviour.invertHorizontal ? 6 : 7, 0);
					activeCursor->Y = activeViewport->Y2 + 1 - fontH;
				}
			}
			return false;
		}
	}

	// if we get here we have a graphics cursor, or text cursor with wrap enabled

	if (!textCursorActive() && cursorBehaviour.grNoSpecialActions) {
		return false;
	}

	// here we just wrap everything
	// TODO these don't work properly if our viewport isn't a multiple of fontW or fontH
	// NB this will only be a potential problem if we're using a graphics cursor with a restricted viewport
	// or potentially if we're using an oddly sized font
	if (activeCursor->X < activeViewport->X1) {
		activeCursor->X = activeViewport->X2 + 1 - fontW;
	}
	if (activeCursor->X >= activeViewport->X2) {
		activeCursor->X = activeViewport->X1;
	}
	if (activeCursor->Y < activeViewport->Y1) {
		activeCursor->Y = activeViewport->Y2 + 1 - fontH;
	}
	if (activeCursor->Y >= activeViewport->Y2) {
		activeCursor->Y = activeViewport->Y1;
	}

	// alignment to a multiple of fontW can go _something_ like this
	// but needs to bear in mind that our axis might be inverted
	// int16_t xOffset = (activeCursor->X - activeViewport->X1) % fontW;
	// activeCursor->X = activeCursor->X + xOffset;

	return true;
}

// Move the active cursor down a line
//
void cursorDown() {
	if (cursorBehaviour.flipXY) {
		activeCursor->X += (cursorBehaviour.invertHorizontal ? -fontW : fontW);
	} else {
		activeCursor->Y += (cursorBehaviour.invertVertical ? -fontH : fontH);
	}
	//
	// handle paging if we need to
	//
	if (textCursorActive() && pagedMode) {
		pagedModeCount++;
		if (pagedModeCount >= (
				cursorBehaviour.flipXY ? (activeViewport->X2 - activeViewport->X1 + 1) / fontW
					: (activeViewport->Y2 - activeViewport->Y1 + 1) / fontH
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
void cursorUp() {
	if (cursorBehaviour.flipXY) {
		activeCursor->X += (cursorBehaviour.invertHorizontal ? fontW : -fontW);
	} else {
		activeCursor->Y += (cursorBehaviour.invertVertical ? fontH : -fontH);
	}
	cursorScrollOrWrap();
}

// Move the active cursor back one character
//
void cursorLeft() {
	if (cursorBehaviour.flipXY) {
		activeCursor->Y += (cursorBehaviour.invertVertical ? fontH : -fontH);
	} else {
		activeCursor->X += (cursorBehaviour.invertHorizontal ? fontW : -fontW);
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
void cursorRight(bool scrollProtect) {
	// deal with any pending newline that we may have
	cursorAutoNewline();

	if (cursorBehaviour.flipXY) {
		activeCursor->Y += (cursorBehaviour.invertVertical ? -fontH : fontH);
	} else {
		activeCursor->X += (cursorBehaviour.invertHorizontal ? -fontW : fontW);
	}
	if (!scrollProtect) {
		cursorAutoNewline();
	}
}

// Move the active cursor to the top-left position in the viewport
//
void cursorHome(Point * cursor, Rect * viewport) {
	if (cursorBehaviour.flipXY) {
		cursor->X = cursorBehaviour.invertHorizontal ? (viewport->X2 + 1 - fontW) : viewport->X1;
		cursor->Y = cursorBehaviour.invertVertical ? (viewport->Y2 + 1 - fontH) : viewport->Y1;
	} else {
		cursor->X = cursorBehaviour.invertHorizontal ? (viewport->X2 + 1 - fontW) : viewport->X1;
		cursor->Y = cursorBehaviour.invertVertical ? (viewport->Y2 + 1 - fontH) : viewport->Y1;
	}
}

// TAB(x,y)
//
void cursorTab(uint8_t x, uint8_t y) {
	int xPos, yPos;
	if (cursorBehaviour.flipXY) {
		if (cursorBehaviour.invertHorizontal) {
			xPos = activeViewport->X2 - ((y + 1) * fontW);
		} else {
			xPos = activeViewport->X1 + (y * fontW);
		}
		if (cursorBehaviour.invertVertical) {
			yPos = activeViewport->Y2 - ((x + 1) * fontH);
		} else {
			yPos = activeViewport->Y1 + (x * fontH);
		}
	} else {
		if (cursorBehaviour.invertHorizontal) {
			xPos = activeViewport->X2 - ((x + 1) * fontW);
		} else {
			xPos = activeViewport->X1 + (x * fontW);
		}
		if (cursorBehaviour.invertVertical) {
			yPos = activeViewport->Y2 - ((y + 1) * fontH);
		} else {
			yPos = activeViewport->Y1 + (y * fontH);
		}
	}
	if (activeViewport->X1 <= xPos && xPos < activeViewport->X2 && activeViewport->Y1 <= yPos && yPos < activeViewport->Y2) {
		activeCursor->X = xPos;
		activeCursor->Y = yPos;
	}
}

void setPagedMode(bool mode = pagedMode) {
	pagedMode = mode;
	pagedModeCount = 0;
}

// Reset basic cursor control
//
void resetCursor() {
	setActiveCursor(getTextCursor());
	cursorEnabled = true;
	cursorBehaviour.value = 0;
	setPagedMode(false);
}

inline void enableCursor(bool enable = true) {
	cursorEnabled = enable;
}

void ensureCursorInViewport(Rect viewport) {
	if (textCursor.X < viewport.X1
		|| textCursor.X > viewport.X2
		|| textCursor.Y < viewport.Y1
		|| textCursor.Y > viewport.Y2
	) {
		cursorHome(&textCursor, &viewport);
	}
}

#endif	// CURSOR_H

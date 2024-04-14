#ifndef VDU_H
#define VDU_H

#include <HardwareSerial.h>

#include "agon.h"
#include "vdu_audio.h"
#include "vdu_sys.h"

extern bool consoleMode;
extern bool printerOn;
extern HardwareSerial DBGSerial;

// Handle VDU commands
//
void VDUStreamProcessor::vdu(uint8_t c) {
	// We want to send raw chars back to the debugger
	// this allows binary (faster) data transfer in ZDI mode
	// to inspect memory and register values
	//
	if (consoleMode) {
		DBGSerial.write(c);
	}

	if (printerOn) {
		switch(c) {
			case 0x03:
				printerOn = false;
				break;
			// case 0x09:	// translate "cursor right" to a space, as terminals treat character 9 as tab
			// 	vdu_print(32);
			// 	break;
			case 8 ... 13:
			case 0x20 ... 0xFF: {
				vdu_print(c);
			}
		}
	}

	if (!commandsEnabled) {
		switch (c) {
			case 0x01: {
				// capture character and send to "printer" if enabled
				auto b = readByte_t();	if (b == -1) return;
				vdu_print(b);
			}	break;
			case 0x06:	// resume VDU command system
				commandsEnabled = true;
				break;
		}
		return;
	}

	switch(c) {
		case 0x01: {
			// capture character and send to "printer" if enabled
			auto b = readByte_t();	if (b == -1) return;
			vdu_print(b);
		}	break;
		case 0x02:
			printerOn = true;
			break;
		case 0x04:
			// enable text cursor
			context->setActiveCursor(CursorType::TextCursor);
			sendModeInformation();
			break;
		case 0x05:
			// enable graphics cursor
			context->setActiveCursor(CursorType::GraphicsCursor);
			sendModeInformation();
			break;
		case 0x06:
			// Resume VDU system
			break;
		case 0x07:	// Bell
			playNote(0, 100, 750, 125);
			break;
		case 0x08:	// Cursor Left
			if (!context->textCursorActive() && peekByte_t(20) == 0x20) {
				// left followed by a space is almost certainly a backspace
				// but MOS doesn't send backspaces to delete characters on line edits
				context->plotBackspace();
			} else {
				context->cursorLeft();
			}
			break;
		case 0x09:	// Cursor Right
			context->cursorRight();
			break;
		case 0x0A:	// Cursor Down
			context->cursorDown();
			break;
		case 0x0B:	// Cursor Up
			context->cursorUp();
			break;
		case 0x0C:	// CLS
			context->cls(false);
			break;
		case 0x0D:	// CR
			context->cursorCR();
			break;
		case 0x0E:	// Paged mode ON
			context->setPagedMode(true);
			break;
		case 0x0F:	// Paged mode OFF
			context->setPagedMode(false);
			break;
		case 0x10:	// CLG
			context->clg();
			break;
		case 0x11:	// COLOUR
			vdu_colour();
			break;
		case 0x12:	// GCOL
			vdu_gcol();
			break;
		case 0x13:	// Define Logical Colour
			vdu_palette();
			break;
		case 0x14: { // Reset colours
			restorePalette();
			// TODO consider if this should iterate over all stored contexts
			// and if not, how to handle the fact that the palette has changed
			context->resetPaintingInfo();
		}	break;
		case 0x15:
			commandsEnabled = false;
			break;
		case 0x16:	// Mode
			vdu_mode();
			break;
		case 0x17:	// VDU 23
			vdu_sys();
			break;
		case 0x18:	// Define a graphics viewport
			vdu_graphicsViewport();
			sendModeInformation();
			break;
		case 0x19:	// PLOT
			vdu_plot();
			break;
		case 0x1A:	// Reset text and graphics viewports
			vdu_resetViewports();
			sendModeInformation();
			break;
		case 0x1B: { // VDU 27
			auto b = readByte_t();	if (b == -1) return;
			context->plotCharacter(b);
			vdu_print(c);
		}	break;
		case 0x1C:	// Define a text viewport
			vdu_textViewport();
			sendModeInformation();
			break;
		case 0x1D:	// VDU_29
			vdu_origin();
			break;
		case 0x1E:	// Move cursor to top left of the viewport
			context->cursorHome();
			break;
		case 0x1F:	// TAB(X,Y)
			vdu_cursorTab();
			break;
		case 0x20 ... 0x7E:
		case 0x80 ... 0xFF:
			context->plotCharacter(c);
			break;
		case 0x7F:	// Backspace
			context->plotBackspace();
			break;
	}
}

// VDU "print" command
// will output to "printer", if enabled
//
void VDUStreamProcessor::vdu_print(uint8_t c) {
	if (printerOn && !consoleMode) {
		// if consoleMode is enabled we're echoing everything back anyway
		DBGSerial.write(c);
	}
}

// VDU 17 Handle COLOUR
//
void VDUStreamProcessor::vdu_colour() {
	auto colour = readByte_t();

	context->setTextColour(colour);
}

// VDU 18 Handle GCOL
//
void VDUStreamProcessor::vdu_gcol() {
	auto mode = readByte_t();
	auto colour = readByte_t();

	context->setGraphicsColour(mode, colour);
}

// VDU 19 Handle palette
//
void VDUStreamProcessor::vdu_palette() {
	auto l = readByte_t(); if (l == -1) return; // Logical colour
	auto p = readByte_t(); if (p == -1) return; // Physical colour
	auto r = readByte_t(); if (r == -1) return; // The red component
	auto g = readByte_t(); if (g == -1) return; // The green component
	auto b = readByte_t(); if (b == -1) return; // The blue component

	auto index = setLogicalPalette(l, p, r, g, b);

	if (index != -1) {
		// palette item in a 64 colour mode changed, so we may need to update our active colours
		// TODO iterate over all stored contexts and update the palette
		context->updateColours(l, index);
	}
}

// VDU 22 Handle MODE
//
void VDUStreamProcessor::vdu_mode() {
	auto mode = readByte_t();
	debug_log("vdu_mode: %d\n\r", mode);
	if (mode >= 0) {
		// TODO reset to primary context, and clear out all others
		// clear screen, resetting current viewports
		context->cls(true);
		ttxtMode = false;
		auto errVal = changeMode(mode);
		if (errVal != 0) {
			debug_log("vdu_mode: Error %d changing to mode %d\n\r", errVal, mode);
			errVal = changeMode(videoMode);
			if (errVal != 0) {
				debug_log("vdu_mode: Error %d changing back to mode %d\n\r", errVal, videoMode);
				videoMode = 1;
				changeMode(1);
			}
		}
		context->reset();
		if (isDoubleBuffered()) {
			switchBuffer();
			context->cls(false);
		}
		// reset mouse
		setMouseCursor();
		resetMousePositioner(canvasW, canvasH, _VGAController.get());
		// update MOS with new info
		sendModeInformation();
		if (mouseEnabled) {
			sendMouseData();
		}
	}
}

// VDU 24 Graphics viewport
// Example: VDU 24,640;256;1152;896;
//
void VDUStreamProcessor::vdu_graphicsViewport() {
	auto x1 = readWord_t();			// Left
	auto y2 = readWord_t();			// Bottom
	auto x2 = readWord_t();			// Right
	auto y1 = readWord_t();			// Top

	if (context->setGraphicsViewport(x1, y1, x2, y2)) {
		debug_log("vdu_graphicsViewport: OK %d,%d,%d,%d\n\r", x1, y1, x2, y2);
	} else {
		debug_log("vdu_graphicsViewport: Invalid Viewport %d,%d,%d,%d\n\r", x1, y1, x2, y2);
	}
}

// VDU 25 Handle PLOT
//
void IRAM_ATTR VDUStreamProcessor::vdu_plot() {
	auto command = readByte_t(); if (command == -1) return;
	auto x = readWord_t(); if (x == -1) return;
	auto y = readWord_t(); if (y == -1) return;

	if (ttxtMode) return;

	if (context->plot((int16_t) x, (int16_t) y, command)) {
		// we have a pending plot command
		context->plotPending(peekByte_t(FAST_COMMS_TIMEOUT));
	}
}

// VDU 26 Reset graphics and text viewports
//
void VDUStreamProcessor::vdu_resetViewports() {
	if (ttxtMode) {
		ttxt_instance.set_window(0,24,39,0);
	}
	context->viewportReset();
	// reset cursors too (according to BBC BASIC manual)
	context->cursorHome();
	context->pushPoint(0, 0);
	debug_log("vdu_resetViewport\n\r");
}

// VDU 28: text viewport
// Example: VDU 28,20,23,34,4
//
void VDUStreamProcessor::vdu_textViewport() {
	auto cx1 = readByte_t();			// Left
	auto cy2 = readByte_t();			// Bottom
	auto cx2 = readByte_t();			// Right
	auto cy1 = readByte_t();			// Top

	if (ttxtMode) {
		if (cx2 > 39) cx2 = 39;
		if (cy2 > 24) cy2 = 24;
		if (cx2 >= cx1 && cy2 >= cy1)
		ttxt_instance.set_window(cx1,cy2,cx2,cy1);
	} else if (context->setTextViewport(cx1, cy1, cx2, cy2)) {
		debug_log("vdu_textViewport: OK %d,%d,%d,%d\n\r", cx1, cy1, cx2, cy2);
	} else {
		debug_log("vdu_textViewport: Invalid Viewport %d,%d,%d,%d\n\r", cx1, cy1, cx2, cy2);
	}
}

// Handle VDU 29
//
void VDUStreamProcessor::vdu_origin() {
	auto x = readWord_t();
	if (x >= 0) {
		auto y = readWord_t();
		if (y >= 0) {
			context->setOrigin(x, y);
			debug_log("vdu_origin: %d,%d\n\r", x, y);
		}
	}
}

// VDU 30 TAB(x,y)
//
void VDUStreamProcessor::vdu_cursorTab() {
	auto x = readByte_t();
	if (x >= 0) {
		auto y = readByte_t();
		if (y >= 0) {
			context->cursorTab(x, y);
		}
	}
}

#endif // VDU_H

#ifndef CONTEXT_H
#define CONTEXT_H

// Text and graphics system context management
// This includes all cursor, viewport, and graphics contextual data
//

#include <memory>
#include <vector>

#include <fabgl.h>

#include "agon.h"
#include "sprites.h"

// Support structures

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

enum class CursorType : uint8_t {
	Text,
	Graphics,
};

enum class ViewportType : uint8_t {
	Text = 0,		// Text viewport
	Default,		// Default (whole screen) viewport
	Graphics,		// Graphics viewport
	Active,			// Active viewport
};


// Context object
//
class Context {
	private:
		// Font tracking
		// "activating" a context will need to set the font to the current font
		std::shared_ptr<fabgl::FontInfo>	font;				// Current active font
		std::shared_ptr<fabgl::FontInfo>	textFont;			// Current active font for text cursor
		std::shared_ptr<fabgl::FontInfo>	graphicsFont;		// Current active font for graphics cursor
		std::shared_ptr<BufferStream>		textFontData;
		std::shared_ptr<BufferStream>		graphicsFontData;

		// Cursor management data
		bool			cursorEnabled = true;			// Cursor visibility
		bool			cursorFlashing = true;			// Cursor is flashing
		uint16_t		cursorFlashRate = pdMS_TO_TICKS(CURSOR_PHASE);	// Cursor flash rate
		CursorBehaviour cursorBehaviour;				// Cursor behavior byte
		Point			textCursor;						// Text cursor
		Point *			activeCursor = &textCursor;		// Pointer to the active text cursor (textCursor or p1)
		bool			cursorShowing = false;			// Cursor is currently showing on screen
		bool			cursorTemporarilyHidden = false;	// Cursor is temporarily hidden for command processing
		TickType_t		cursorTime;						// Time of last cursor flash event

		// Cursor rendering
		uint8_t			cursorVStart;					// Cursor vertical start offset
		uint8_t			cursorVEnd;						// Cursor vertical end
		uint8_t			cursorHStart;					// Cursor horizontal start offset
		uint8_t			cursorHEnd;						// Cursor horizontal end

		// Paged mode tracking
		bool 			pagedMode = false;				// Is output paged or not? Set by VDU 14 and 15
		uint8_t			pagedModeCount = 0;				// Scroll counter for paged mode

		// Viewport management data
		Rect *			activeViewport;					// Pointer to the active text viewport (textViewport or graphicsViewport)
		Rect			defaultViewport;				// Default viewport
		Rect			textViewport;					// Text viewport
		Rect			graphicsViewport;				// Graphics viewport

		// Graphics management data
		fabgl::PaintOptions			gpofg;				// Graphics paint options foreground
		fabgl::PaintOptions			gpobg;				// Graphics paint options background
		fabgl::PaintOptions			tpo;				// Text paint options
		fabgl::PaintOptions			cpo;				// Cursor paint options
		RGB888			gfg, gbg;						// Graphics foreground and background colour
		RGB888			tfg, tbg;						// Text foreground and background colour
		uint8_t			gfgc, gbgc, tfgc, tbgc;			// Logical colour values for graphics and text
		uint8_t			lineThickness = 1;				// Line thickness
		uint16_t		currentBitmap = BUFFERED_BITMAP_BASEID;	// Current bitmap ID
		uint16_t		bitmapTransform = -1;			// Bitmap transform buffer ID
		fabgl::LinePattern	linePattern = fabgl::LinePattern();				// Dotted line pattern
		uint8_t			linePatternLength = 8;			// Dotted line pattern length
		std::vector<uint16_t>	charToBitmap = std::vector<uint16_t>(256, 65535);	// character to bitmap mapping
		bool			plottingText = false;			// Are we currently plotting text?
		bool			logicalCoords = true;			// Use BBC BASIC logical coordinates

		Point			origin;							// Screen origin
		Point			uOrigin;						// Screen origin, un-scaled
		Point			p1, p2, p3;						// Coordinate store for plot (p1 = the graphics cursor)
		Point			rp1;							// Relative coordinates store for plot
		Point			up1;							// Unscaled coordinates store for plot
		std::vector<Point>	pathPoints;					// Storage for path points
		uint8_t			lastPlotCommand = 0;			// Tracking of last plot command to allow continuing plots

		// Cursor management functions
		int getXAdjustment();
		int getYAdjustment();
		int getNormalisedViewportWidth();
		int getNormalisedViewportHeight();
		Point getNormalisedCursorPosition();
		Point getNormalisedCursorPosition(Point * cursor);

		bool cursorIsOffRight();
		bool cursorIsOffLeft();
		bool cursorIsOffTop();
		bool cursorIsOffBottom();

		void cursorEndRow();
		void cursorEndRow(Point * cursor, Rect * viewport);
		void cursorTop();
		void cursorTop(Point * cursor, Rect * viewport);
		void cursorEndCol();
		void cursorEndCol(Point * cursor, Rect * viewport);

		bool cursorScrollOrWrap();
		void cursorAutoNewline();
		void ensureCursorInViewport(Rect viewport);

		// Viewport management functions
		Rect * getViewport(ViewportType type);
		bool setTextViewport(Rect rect);
		Point invScale(Point p);

		// Font management functions
		const fabgl::FontInfo * getFont();
		void changeFont(std::shared_ptr<fabgl::FontInfo> newFont, std::shared_ptr<BufferStream> fontData, uint8_t flags);
		bool cmpChar(uint8_t * c1, uint8_t *c2, uint8_t len);
		char getScreenChar(Point p);
		inline void setCharacterOverwrite(bool overwrite);		// TODO integrate into setActiveCursor?
		inline std::shared_ptr<Bitmap> getBitmapFromChar(uint8_t c) {
			return getBitmap(charToBitmap[c]);
		}

		// Graphics functions
		fabgl::PaintOptions getPaintOptions(fabgl::PaintMode mode, fabgl::PaintOptions priorPaintOptions);
		void setGraphicsOptions(uint8_t mode);
		void setGraphicsFill(uint8_t mode);
		inline void setClippingRect(Rect rect);

		void pushPoint(Point p);
		void pushPointRelative(int16_t x, int16_t y);

		void moveTo();
		void plotLine(bool omitFirstPoint, bool omitLastPoint, bool usePattern, bool resetPattern);
		void plotPoint();
		void fillHorizontalLine(bool scanLeft, bool match, RGB888 matchColor);
		void plotTriangle();
		void plotRectangle();
		void plotParallelogram();
		void plotCircle(bool filled);
		void plotArc();
		void plotSegment();
		void plotSector();
		void plotCopyMove(uint8_t mode);
		void plotPath(uint8_t mode, uint8_t lastMode);
		void plotBitmap(uint8_t mode);

		void clearViewport(ViewportType viewport);
		void scrollRegion(Rect * region, uint8_t direction, int16_t movement);

		uint16_t scanH(int16_t x, int16_t y, RGB888 colour, int8_t direction);
		uint16_t scanHToMatch(int16_t x, int16_t y, RGB888 colour, int8_t direction);

	public:

		// Constructor
		Context() {
			cursorTime = xTaskGetTickCountFromISR();
			reset();
		};

		// Copy constructor
		Context(const Context &c);

		bool readVariable(uint16_t var, uint16_t * value);
		void setVariable(uint16_t var, uint16_t value);

		// Cursor management functions
		void hideCursor();
		void showCursor();
		void doCursorFlash();
		inline bool textCursorActive();
		inline void setActiveCursor(CursorType type);
		inline void setCursorBehaviour(uint8_t setting, uint8_t mask);
		inline void enableCursor(uint8_t enable);
		void setCursorAppearance(uint8_t appearance);
		void setCursorVStart(uint8_t start);
		void setCursorVEnd(uint8_t end);
		void setCursorHStart(uint8_t start);
		void setCursorHEnd(uint8_t end);
		void setPagedMode(bool mode);
		void resetTextCursor();

		void cursorUp();
		void cursorUp(bool moveOnly);
		void cursorDown();
		void cursorDown(bool moveOnly);
		void cursorLeft();
		void cursorRight();
		void cursorRight(bool scrollProtect);
		void cursorCR();
		void cursorCR(Point * cursor, Rect * viewport);
		void cursorHome();
		void cursorHome(Point * cursor, Rect * viewport);
		void cursorTab(uint8_t x, uint8_t y);
		void cursorRelativeMove(int8_t x, int8_t y);
		void getCursorTextPosition(uint8_t * x, uint8_t * y);

		// Viewport management functions
		void viewportReset();
		void setActiveViewport(ViewportType type);
		bool setGraphicsViewport(Point p1, Point p2);
		bool setGraphicsViewport(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
		bool setGraphicsViewport();
		bool setTextViewport(uint8_t cx1, uint8_t cy1, uint8_t cx2, uint8_t cy2);
		bool setTextViewport();
		uint8_t getNormalisedViewportCharWidth();
		uint8_t getNormalisedViewportCharHeight();
		void setOrigin(int x, int y);
		void setOrigin();
		void shiftOrigin();
		void setLogicalCoords(bool b);
		Point scale(int16_t X, int16_t Y);
		Point toCurrentCoordinates(int16_t X, int16_t Y);
		Point toScreenCoordinates(int16_t X, int16_t Y);

		// Font management functions
		void changeFont(uint16_t newFontId, uint8_t flags);
		void resetFonts();
		bool usingSystemFont();
		char getScreenChar(uint8_t x, uint8_t y);
		char getScreenCharAt(uint16_t px, uint16_t py);
		void mapCharToBitmap(uint8_t c, uint16_t bitmapId);
		void unmapBitmapFromChars(uint16_t bitmapId);
		void resetCharToBitmap();
		
		// Graphics functions
		inline void setCurrentBitmap(uint16_t b) {
			currentBitmap = b;
		}
		inline uint16_t getCurrentBitmapId() {
			return currentBitmap;
		}

		void setLineThickness(uint8_t thickness);
		void setDottedLinePattern(uint8_t pattern[8]);
		void setDottedLinePatternLength(uint8_t length);

		void setTextColour(uint8_t colour);
		void setGraphicsColour(uint8_t mode, uint8_t colour);
		void updateColours(uint8_t l, uint8_t p);
		bool getColour(uint8_t colour, RGB888 * pixel);
		RGB888 getPixel(uint16_t x, uint16_t y);

		void pushPoint(uint16_t x, uint16_t y);
		Rect getGraphicsRect();							// Used by sprites system to capture screen area

		bool plot(int16_t x, int16_t y, uint8_t command);
		void plotPending(int16_t peeked);

		void plotString(const std::string & s);
		void plotBackspace();
		void drawBitmap(uint16_t x, uint16_t y, bool compensateHeight, bool forceSet);
		void drawCursor(Point p);

		void setAffineTransform(uint8_t flags, uint16_t bufferId);

		void cls();
		void clg();
		void scrollRegion(ViewportType viewport, uint8_t direction, int16_t movement);

		void resetGraphicsPainting();
		void resetGraphicsOptions();
		void resetGraphicsPositioning();
		void resetTextPainting();

		void reset();
		void activate();
};


 Context::Context(const Context &c) {
	// Copy all the data

	// Font tracking
	font = c.font;
	textFont = c.textFont;
	graphicsFont = c.graphicsFont;
	textFontData = c.textFontData;
	graphicsFontData = c.graphicsFontData;

	// Text cursor management data
	cursorEnabled = c.cursorEnabled;
	cursorFlashing = c.cursorFlashing;
	cursorFlashRate = c.cursorFlashRate;
	cursorBehaviour = c.cursorBehaviour;
	textCursor = c.textCursor;
	cursorVStart = c.cursorVStart;
	cursorVEnd = c.cursorVEnd;
	cursorHStart = c.cursorHStart;
	cursorHEnd = c.cursorHEnd;
	// Data related to cursor rendering
	cursorShowing = c.cursorShowing;
	cursorTemporarilyHidden = c.cursorTemporarilyHidden;
	cursorTime = c.cursorTime;

	pagedMode = c.pagedMode;
	pagedModeCount = c.pagedModeCount;

	// Viewport management data
	defaultViewport = c.defaultViewport;
	textViewport = c.textViewport;
	graphicsViewport = c.graphicsViewport;

	// Graphics painting options
	gpofg = c.gpofg;
	gpobg = c.gpobg;
	gfg = c.gfg;
	gbg = c.gbg;
	gfgc = c.gfgc;
	gbgc = c.gbgc;
	lineThickness = c.lineThickness;
	currentBitmap = c.currentBitmap;
	linePattern.setPattern(c.linePattern.pattern);
	linePatternLength = c.linePatternLength;

	// Graphics positioning data
	logicalCoords = c.logicalCoords;
	origin = c.origin;
	uOrigin = c.uOrigin;
	p1 = c.p1;
	p2 = c.p2;
	p3 = c.p3;
	rp1 = c.rp1;
	up1 = c.up1;
	// pathPoints and lastPlotCommand are currently completely transient, so don't need to be copied

	// Text painting options
	tfg = c.tfg;
	tbg = c.tbg;
	tfgc = c.tfgc;
	tbgc = c.tbgc;
	tpo = c.tpo;
	cpo = c.cpo;
	charToBitmap = c.charToBitmap;

	if (c.activeCursor == &c.textCursor) {
		activeCursor = &textCursor;
	} else {
		activeCursor = &p1;
	}

	if (c.activeViewport == &c.graphicsViewport) {
		activeViewport = &graphicsViewport;
	} else {
		activeViewport = &textViewport;
	}
}

bool Context::readVariable(uint16_t var, uint16_t * value) {
	switch (var) {
		// Mode variables
		// 0 is "mode flags" - omitting for now
		case 1:		// Text columns - 1 (characters)
			if (value) {
				*value = (canvasW / getFont()->width) - 1;
			}
			break;
		case 2:		// Text rows - 1
			if (value) {
				*value = (canvasH / getFont()->height) - 1;
			}
			break;
		case 3:		// Max logical colour
			if (value) {
				*value = getVGAColourDepth() - 1;
			}
			break;
		// Variables 4 and 5 are X and Y Eigen factor - omitting for now
		// Omitting variables 6-10 as they aren't really relevant without direct screen memory access
		case 11:	// Screen width in pixels - 1
			if (value) {
				*value = canvasW - 1;
			}
			break;
		case 12:	// Screen height in pixels - 1
			if (value) {
				*value = canvasH - 1;
			}
			break;
		case 13:	// Number of screen banks
			if (value) {
				*value =  isDoubleBuffered() ? 2 : 1;
			}
			break;

		// Variables 14-127 are currently undefined

		// VDU Variables

		// - text and graphics windows
		case 0x80:	// Graphics window, LH column, pixel coordinates
			if (value) {
				*value = getViewport(ViewportType::Graphics)->X1;
			}
			break;
		case 0x81:	// Graphics window, Bottom row, pixel coordinates
			if (value) {
				*value = getViewport(ViewportType::Graphics)->Y2;
			}
			break;
		case 0x82:	// Graphics window, RH column, pixel coordinates
			if (value) {
				*value = getViewport(ViewportType::Graphics)->X2;
			}
			break;
		case 0x83:	// Graphics window, Top row, pixel coordinates
			if (value) {
				*value = getViewport(ViewportType::Graphics)->Y1;
			}
			break;
		case 0x84:	// Text window, LH column, character coordinates
			if (value) {
				*value = getViewport(ViewportType::Text)->X1 / getFont()->width;
			}
			break;
		case 0x85:	// Text window, Bottom row, character coordinates
			if (value) {
				*value = getViewport(ViewportType::Text)->Y2 / getFont()->height;
			}
			break;
		case 0x86:	// Text window, RH column, character coordinates
			if (value) {
				*value = getViewport(ViewportType::Text)->X2 / getFont()->width;
			}
			break;
		case 0x87:	// Text window, Top row, character coordinates
			if (value) {
				*value = getViewport(ViewportType::Text)->Y1 / getFont()->height;
			}
			break;

		// Graphics origin
		// NB these are OS coordinates, as per Acorn's implementation
		// we may wish to add a parallel set of variables for screen coordinates
		case 0x88:	// Graphics origin, X, OS coordinates
			if (value) {
				*value = uOrigin.X;
			}
			break;
		case 0x89:	// Graphics origin, Y, OS coordinates
			if (value) {
				*value = uOrigin.Y;
			}
			break;

		// Graphics cursor data
		case 0x8A:	// Graphics cursor, X, OS coordinates
			if (value) {
				*value = up1.X;
			}
			break;
		case 0x8B:	// Graphics cursor, Y, OS coordinates
			if (value) {
				*value = up1.Y;
			}
			break;
		case 0x8C:	// Oldest Graphics cursor, X, screen coordinates
			if (value) {
				*value = p3.X;
			}
			break;
		case 0x8D:	// Oldest Graphics cursor, Y, screen coordinates
			if (value) {
				*value = p3.Y;
			}
			break;
		case 0x8E:	// Previous Graphics cursor, X, screen coordinates
			if (value) {
				*value = p2.X;
			}
			break;
		case 0x8F:	// Previous Graphics cursor, Y, screen coordinates
			if (value) {
				*value = p2.Y;
			}
			break;
		case 0x90:	// Graphics cursor, X, screen coordinates
		case 0x92:	// New point, X, screen coordinates
			if (value) {
				*value = p1.X;
			}
			break;
		case 0x91:	// Graphics cursor, Y, screen coordinates
		case 0x93:	// New point, X, screen coordinates
			if (value) {
				*value = p1.Y;
			}
			break;
		// Acorn has variables 0x92 and 0x93 as "new point"
		// but it is not clear how they would differ from the current graphics cursor
		// so we are treating them as the same

		// Variables 0x94-0x96 are not relevant on Agon, as there is no direct screen memory access

		// GCOL actions and selected colours
		case 0x97:	// GCOL action for foreground colour
			if (value) {
				*value = (uint8_t)gpofg.mode;
			}
			break;
		case 0x98:	// GCOL action for background colour
			if (value) {
				*value = (uint8_t)gpobg.mode;
			}
			break;
		case 0x99:	// Graphics foreground (logical) colour
			if (value) {
				*value = gfgc;
			}
			break;
		case 0x9A:	// Graphics background (logical) colour
			if (value) {
				*value = gbgc;
			}
			break;
		case 0x9B:	// Text foreground (logical) colour
			if (value) {
				*value = tfgc;
			}
			break;
		case 0x9C:	// Text background (logical) colour
			if (value) {
				*value = tbgc;
			}
			break;
		// Variables &9D-&A0 are not relevant on Agon, as they are "tint" values which we can't support

		case 0xA1:	// Max mode number (not double-buffered)
			// NB this is hard-coded
			if (value) {
				*value = 23;
			}
			break;

		// Font size info
		// NB Agon currently doesn't support changing font spacing
		case 0xA2:	// X font size, graphics cursor
			if (value) {
				*value = graphicsFont ? graphicsFont->width : 8;
			}
			break;
		case 0xA3:	// Y font size, graphics cursor
			if (value) {
				*value = graphicsFont ? graphicsFont->height : 8;
			}
			break;
		case 0xA4:	// X font spacing, graphics cursor
			if (value) {
				*value = graphicsFont ? graphicsFont->width : 8;
			}
			break;
		case 0xA5:	// Y font spacing, graphics cursor
			if (value) {
				*value = graphicsFont ? graphicsFont->height : 8;
			}
			break;
		// &A6 omitted as it is not relevant on Agon ("address of horizontal line-draw routine")
		case 0xA7:	// X font size, text cursor
			if (value) {
				*value = textFont ? textFont->width : 8;
			}
			break;
		case 0xA8:	// Y font size, text cursor
			if (value) {
				*value = textFont ? textFont->height : 8;
			}
			break;
		case 0xA9:	// X font spacing, text cursor
			if (value) {
				*value = textFont ? textFont->width : 8;
			}
			break;
		case 0xAA:	// Y font spacing, text cursor
			if (value) {
				*value = textFont ? textFont->height : 8;
			}
			break;
		// NB we have more font info available so may add more variables to expose some of it
		// also note that if/when we support variable width fonts, the width here will be zero

		// RISC OS also defines a few other variables beyond these which are not relevant on Agon

		case 0x100:		// width of text window in chars
			if (value) {
				// We will assume that 
				*value = getNormalisedViewportCharWidth();
			}
			break;
		case 0x101:		// height of text window in chars
			if (value) {
				// Acorn seems to reduce this value by 1 on RISC OS
				*value = getNormalisedViewportCharHeight() - 1;
			}
			break;


		// we have other areas of context state that could be exposed as variables
		// such as cursor size info, cursor drawing status, line thickness, line pattern, etc.
		// plus we wish to expose the current context ID
		// Exposing the context ID needs the "default context" changes to be merged

		default:
			debug_log("readVariable: variable %d not found\n\r", var);
			return false;
	}
	
	return true;
}

void Context::setVariable(uint16_t var, uint16_t value) {
	switch (var) {
		// Mode variables (0-13)
		// All mode variables are read-only

		// Variables 14-127 are currently undefined

		// VDU Variables

		// - text and graphics windows 0x80-0x87
		// Currently read-only, as changing them here could break things
		// since we need to ensure that x,y pairs are correctly ordered

		// - text and graphics windows
		case 0x80: {	// Graphics window, LH column, pixel coordinates
			uint16_t y1, x2, y2;
			readVariable(0x81, &y1);
			readVariable(0x82, &x2);
			readVariable(0x83, &y2);
			setGraphicsViewport(Point(value, y1), Point(x2, y2));
		}	break;
		case 0x81: {	// Graphics window, Bottom row, pixel coordinates
			uint16_t x1, x2, y2;
			readVariable(0x80, &x1);
			readVariable(0x82, &x2);
			readVariable(0x83, &y2);
			setGraphicsViewport(Point(x1, value), Point(x2, y2));
		}	break;
		case 0x82: {	// Graphics window, RH column, pixel coordinates
			uint16_t x1, y1, y2;
			readVariable(0x80, &x1);
			readVariable(0x81, &y1);
			readVariable(0x83, &y2);
			setGraphicsViewport(Point(x1, y1), Point(value, y2));
		}	break;
		case 0x83: {	// Graphics window, Top row, pixel coordinates
			uint16_t x1, y1, x2;
			readVariable(0x80, &x1);
			readVariable(0x81, &y1);
			readVariable(0x82, &x2);
			setGraphicsViewport(Point(x1, y1), Point(x2, value));
		}	break;
		case 0x84: {	// Text window, LH column, character coordinates
			uint16_t y1, x2, y2;
			readVariable(0x85, &y1);
			readVariable(0x86, &x2);
			readVariable(0x87, &y2);
			setTextViewport(value, y2, x2, y1);
		}	break;
		case 0x85: {	// Text window, Bottom row, character coordinates
			uint16_t x1, y2, x2;
			readVariable(0x84, &x1);
			readVariable(0x86, &x2);
			readVariable(0x87, &y2);
			setTextViewport(x1, y2, x2, value);
		}	break;
		case 0x86: {	// Text window, RH column, character coordinates
			uint16_t x1, y1, y2;
			readVariable(0x84, &x1);
			readVariable(0x85, &y1);
			readVariable(0x87, &y2);
			setTextViewport(x1, y2, value, y1);
		}	break;
		case 0x87: {	// Text window, Top row, character coordinates
			uint16_t x1, y1, x2;
			readVariable(0x84, &x1);
			readVariable(0x85, &y1);
			readVariable(0x86, &x2);
			setTextViewport(x1, value, x2, y1);
		}	break;

		// Graphics origin (0x88-0x89)
		case 0x88:	// Graphics origin, X, OS coordinates
			setOrigin(value, uOrigin.Y);
			break;
		case 0x89:	// Graphics origin, Y, OS coordinates
			setOrigin(uOrigin.X, value);
			break;

		// Graphics cursor data
		case 0x8A:	// Graphics cursor, X, OS coordinates
			p1.X = value;
			break;
		case 0x8B:	// Graphics cursor, Y, OS coordinates
			p1.Y = value;
			break;
		case 0x8C:	// Oldest Graphics cursor, X, screen coordinates
			p3.X = value;
			break;
		case 0x8D:	// Oldest Graphics cursor, Y, screen coordinates
			p3.Y = value;
			break;
		case 0x8E:	// Previous Graphics cursor, X, screen coordinates
			p2.X = value;
			break;
		case 0x8F:	// Previous Graphics cursor, Y, screen coordinates
			p2.Y = value;
			break;
		case 0x90:	// Graphics cursor, X, screen coordinates
		case 0x92:	// New point, X, screen coordinates
			p1.X = value;
			break;
		case 0x91:	// Graphics cursor, Y, screen coordinates
		case 0x93:	// New point, X, screen coordinates
			p1.Y = value;
			break;

		// Variables 0x94-0x96 are not relevant on Agon, as there is no direct screen memory access

		// GCOL actions and selected colours
		case 0x97:	// GCOL action for foreground colour
			if (value <= 7) {
				gpofg = getPaintOptions((fabgl::PaintMode)value, gpofg);
			}
			break;
		case 0x98:	// GCOL action for background colour
			if (value <= 7) {
				gpobg = getPaintOptions((fabgl::PaintMode)value, gpobg);
			}
			break;
		case 0x99:	// Graphics foreground (logical) colour
			setGraphicsColour((uint8_t)gpofg.mode, value & 63);
			break;
		case 0x9A:	// Graphics background (logical) colour
			setGraphicsColour((uint8_t)gpobg.mode, (value & 63) + 128);
			break;
		case 0x9B:	// Text foreground (logical) colour
			setTextColour(value & 63);
			break;
		case 0x9C:	// Text background (logical) colour
			setTextColour((value & 63) + 128);
			break;
		// Variables &9D-&A0 are not relevant on Agon, as they are "tint" values which we can't support

		// Max mode number (0xA1) and font info (0xA2-0xAA) are read-only
	}
}


#include "context/cursor.h"
#include "context/fonts.h"
#include "context/graphics.h"
#include "context/viewport.h"

#endif // CONTEXT_H

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

enum CursorType : uint8_t {
	TextCursor,
	GraphicsCursor,
};

enum ViewportType : uint8_t {
	TextViewport = 0,		// Text viewport
	DefaultViewport,		// Default (whole screen) viewport
	GraphicsViewport,		// Graphics viewport
	ActiveViewport,			// Active viewport
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
		fabgl::LinePattern	linePattern = fabgl::LinePattern();				// Dotted line pattern
		uint8_t			linePatternLength = 8;			// Dotted line pattern length
		std::vector<uint16_t>	charToBitmap = std::vector<uint16_t>(255, 65535);	// character to bitmap mapping

		bool			logicalCoords = true;			// Use BBC BASIC logical coordinates

		Point			origin;							// Screen origin
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
		Point scale(int16_t X, int16_t Y);
		Point invScale(Point p);

		// Font management functions
		const fabgl::FontInfo * getFont();
		void changeFont(std::shared_ptr<fabgl::FontInfo> newFont, std::shared_ptr<BufferStream> fontData, uint8_t flags);
		bool cmpChar(uint8_t * c1, uint8_t *c2, uint8_t len);
		char getScreenChar(Point p);
		inline void setCharacterOverwrite(bool overwrite);		// TODO integrate into setActiveCursor?
		inline std::shared_ptr<Bitmap> getBitmapFromChar(char c) {
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
		Point toCurrentCoordinates(int16_t X, int16_t Y);
		Point toScreenCoordinates(int16_t X, int16_t Y);

		// Font management functions
		void changeFont(uint16_t newFontId, uint8_t flags);
		void resetFonts();
		bool usingSystemFont();
		char getScreenChar(uint8_t x, uint8_t y);
		char getScreenCharAt(uint16_t px, uint16_t py);
		void mapCharToBitmap(char c, uint16_t bitmapId);
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


#include "context/cursor.h"
#include "context/fonts.h"
#include "context/graphics.h"
#include "context/viewport.h"

#endif // CONTEXT_H
#ifndef CONTEXT_H
#define CONTEXT_H

// Text and graphics system context management
// This will include all cursor, viewport, and graphics contextual data
//
// Original sources:
// cursor.h
// graphics.h
// viewport.h

#include <fabgl.h>

#include "agon.h"

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
		// Cursor management data
		uint8_t			pagedModeCount = 0;				// Scroll counter for paged mode
		Point			textCursor;						// Text cursor
		Point *			activeCursor = &textCursor;		// Pointer to the active text cursor (textCursor or p1)

		// Viewport management data
		Point			origin;							// Screen origin
		bool			logicalCoords = true;			// Use BBC BASIC logical coordinates
		double			logicalScaleX;					// Scaling factor for logical coordinates
		double			logicalScaleY;
		Rect *			activeViewport;					// Pointer to the active text viewport (textViewport or graphicsViewport)
		Rect			defaultViewport;				// Default viewport
		Rect			textViewport;					// Text viewport
		Rect			graphicsViewport;				// Graphics viewport
		bool			useViewports = false;			// Viewports are enabled

		// Graphics management data
		fabgl::PaintOptions			gpofg;				// Graphics paint options foreground
		fabgl::PaintOptions			gpobg;				// Graphics paint options background
		fabgl::PaintOptions			tpo;				// Text paint options
		fabgl::PaintOptions			cpo;				// Cursor paint options

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
		std::vector<Point>	pathPoints;					// Storage for path points
		uint8_t			lastPlotCommand = 0;
		// Potentially move to agon_screen.h
		bool			rectangularPixels = false;		// Pixels are square by default
		// uint8_t			palette[64];					// Storage for the palette


		// Cursor management functions
		int getXAdjustment();
		int getYAdjustment();
		Point getNormalisedCursorPosition();
		Point getNormalisedCursorPosition(Point * cursor);
		int getNormalisedViewportWidth();
		int getNormalisedViewportHeight();
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
		void resetCursor();
		void ensureCursorInViewport(Rect viewport);

		// Viewport management functions
		Rect * getViewport(ViewportType type);
		bool setTextViewport(Rect rect);

		// Graphics functions
		bool cmpChar(uint8_t * c1, uint8_t *c2, uint8_t len);
		uint16_t scanH(int16_t x, int16_t y, RGB888 colour, int8_t direction);
		uint16_t scanHToMatch(int16_t x, int16_t y, RGB888 colour, int8_t direction);
		fabgl::PaintOptions getPaintOptions(fabgl::PaintMode mode, fabgl::PaintOptions priorPaintOptions);
		void pushPoint(Point p);
		void setClippingRect(Rect rect);

	public:

		// Font tracking - these will need to change to not be const
		// and "activating" a context will need to refresh the current active font in the context
		const fabgl::FontInfo		* font;				// Current active font
		const fabgl::FontInfo		* textFont;			// Current active font for text cursor
		const fabgl::FontInfo		* graphicsFont;		// Current active font for graphics cursor

		// Cursor management data
		bool			cursorEnabled = true;			// Cursor visibility
		bool			cursorFlashing = true;			// Cursor is flashing
		uint16_t		cursorFlashRate = CURSOR_PHASE;	// Cursor flash rate
		CursorBehaviour cursorBehaviour;				// New cursor behavior
		bool 			pagedMode = false;				// Is output paged or not? Set by VDU 14 and 15

		// Viewport management data

		// Graphics management data

		// Cursor management functions
		inline bool textCursorActive();
		inline void setActiveCursor(CursorType type);
		inline void setCursorBehaviour(uint8_t setting, uint8_t mask);
		void setCursorAppearance(uint8_t appearance);
		void setCursorVStart(uint8_t start);
		void setCursorVEnd(uint8_t end);
		void setCursorHStart(uint8_t start);
		void setCursorHEnd(uint8_t end);
		CursorBehaviour getCursorBehaviour();
		uint8_t getNormalisedViewportCharWidth();
		uint8_t getNormalisedViewportCharHeight();
		void do_cursor();       // TODO remove??
		void setPagedMode(bool mode);
		inline void enableCursor(uint8_t enable);
		// Cursor movement functions
		void cursorDown();
		void cursorDown(bool moveOnly);
		void cursorUp();
		void cursorUp(bool moveOnly);
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
		Point toCurrentCoordinates(int16_t X, int16_t Y);
		bool setGraphicsViewport(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
		bool setTextViewport(uint8_t cx1, uint8_t cy1, uint8_t cx2, uint8_t cy2);
		bool setTextViewportAt(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
		inline void setOrigin(int x, int y);
		inline void setLogicalCoords(bool b);
		Point scale(int16_t X, int16_t Y);
		Point scale(Point p);
		Point translateCanvas(int16_t X, int16_t Y);
		Point translateCanvas(Point p);

		// Graphics functions
		bool plot(int16_t x, int16_t y, uint8_t command);
		void plotPending(int16_t peeked);
		void changeFont(const fabgl::FontInfo * f, uint8_t flags);
		bool usingSystemFont();
		char getScreenChar(uint8_t x, uint8_t y);
		char getScreenCharAt(uint16_t px, uint16_t py);
		char getScreenChar(Point p);
		RGB888 getPixel(uint16_t x, uint16_t y);
		void drawBitmap(uint16_t x, uint16_t y, bool compensateHeight, bool forceSet);
		void restorePalette();							// TODO split in two, and move half to agon_screen.h
		void setTextColour(uint8_t colour);
		void setGraphicsColour(uint8_t mode, uint8_t colour);
		void clearViewport(ViewportType viewport);
		void pushPoint(uint16_t x, uint16_t y);
		void pushPointRelative(int16_t x, int16_t y);
		Rect getGraphicsRect();
		void setGraphicsOptions(uint8_t mode);
		void setGraphicsFill(uint8_t mode);
		void moveTo();
		void plotLine(bool omitFirstPoint, bool omitLastPoint, bool usePattern, bool resetPattern);
		void fillHorizontalLine(bool scanLeft, bool match, RGB888 matchColor);
		void plotPoint();
		void plotTriangle();
		void plotPath(uint8_t mode, uint8_t lastMode);
		void plotRectangle();
		void plotParallelogram();
		void plotCircle(bool filled);
		void plotArc();
		void plotSegment();
		void plotSector();
		void plotCopyMove(uint8_t mode);
		void plotBitmap(uint8_t mode);
		void plotCharacter(char c);
		void plotBackspace();
		inline void setCharacterOverwrite(bool overwrite);		// TODO integrate into setActiveCursor
		void drawCursor(Point p);
		void cls(bool resetViewports);
		void clg();
		void set_mode(uint8_t mode);
		void scrollRegion(Rect * region, uint8_t direction, int16_t movement);
		void scrollRegion(ViewportType viewport, uint8_t direction, int16_t movement);
		void setLineThickness(uint8_t thickness);
		void setDottedLinePattern(uint8_t pattern[8]);
		void setDottedLinePatternLength(uint8_t length);
		void updateColours(uint8_t l, uint8_t p);
		bool getColour(uint8_t colour, RGB888 * pixel);

		// Potentially move to agon_screen.h
		// void setPalette(uint8_t l, uint8_t p, uint8_t r, uint8_t g, uint8_t b);
		// void resetPalette(const uint8_t colours[]);
		int8_t change_mode(uint8_t mode);
};

#include "context/cursor.h"
#include "context/graphics.h"
#include "context/viewport.h"

#endif // CONTEXT_H
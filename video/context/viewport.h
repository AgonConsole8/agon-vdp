#ifndef CONTEXT_VIEWPORT_H
#define CONTEXT_VIEWPORT_H

#include <fabgl.h>

#include "agon.h"

// Definitions for the functions we're implementing here
#include "context.h"

// Private viewport functions

Rect * Context::getViewport(ViewportType type) {
	switch (type) {
		case ViewportType::TextViewport: return &textViewport;
		case ViewportType::DefaultViewport: return &defaultViewport;
		case ViewportType::GraphicsViewport: return &graphicsViewport;
		case ViewportType::ActiveViewport: return activeViewport;
		default: return &defaultViewport;
	}
}

bool Context::setTextViewport(Rect r) {
	if (r.X2 >= canvasW) r.X2 = canvasW - 1;
	if (r.Y2 >= canvasH) r.Y2 = canvasH - 1;

	if (r.X2 > r.X1 && r.Y2 > r.Y1) {
		textViewport = r;
		ensureCursorInViewport(textViewport);
		return true;
	}
	return false;
}

// Scale a point, as appropriate for coordinate system
//
Point Context::scale(int16_t X, int16_t Y) {
	if (logicalCoords) {
		return Point((double)X / logicalScaleX, (double)Y / logicalScaleY);
	}
	return Point(X, Y);
}


// Public viewport management functions

// Reset viewports to default
//
void Context::viewportReset() {
	defaultViewport = Rect(0, 0, canvasW - 1, canvasH - 1);
	textViewport =	Rect(0, 0, canvasW - 1, canvasH - 1);
	graphicsViewport = Rect(0, 0, canvasW - 1, canvasH - 1);
	activeViewport = &textViewport;
}

void Context::setActiveViewport(ViewportType type) {
	activeViewport = getViewport(type);
}

// Set graphics viewport
bool Context::setGraphicsViewport(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	auto p1 = toScreenCoordinates(x1, y1);
	auto p2 = toScreenCoordinates(x2, y2);

	if (p1.X >= 0 && p2.X < canvasW && p1.Y >= 0 && p2.Y < canvasH && p2.X >= p1.X && p2.Y >= p1.Y) {
		graphicsViewport = Rect(p1.X, p1.Y, p2.X, p2.Y);
		setClippingRect(graphicsViewport);
		return true;
	}
	return false;
}

// Set text viewport
// text coordinates
//
bool Context::setTextViewport(uint8_t cx1, uint8_t cy1, uint8_t cx2, uint8_t cy2) {
	auto font = getFont();
	auto x1 = cx1 * font->width;				// Left
	auto y2 = (cy2 + 1) * font->height - 1;		// Bottom
	auto x2 = (cx2 + 1) * font->width - 1;		// Right
	auto y1 = cy1 * font->height;				// Top

	return setTextViewport(Rect(x1, y1, x2, y2));
}

// With current graphics coordinates
bool Context::setTextViewportAt(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	Point p1 = toScreenCoordinates(x1, y1);
	Point p2 = toScreenCoordinates(x2, y2);

	return setTextViewport(Rect(std::min(p1.X, p2.X), std::min(p1.Y, p2.Y), std::max(p1.X, p2.X), std::max(p1.Y, p2.Y)));
}

// Return our viewport width in number of characters
uint8_t Context::getNormalisedViewportCharWidth() {
	if (cursorBehaviour.flipXY) {
		return activeViewport->height() / getFont()->height;
	}
	return activeViewport->width() / getFont()->width;
}

// Return our viewport height in number of characters
uint8_t Context::getNormalisedViewportCharHeight() {
	if (cursorBehaviour.flipXY) {
		return activeViewport->width() / getFont()->width;
	}
	return activeViewport->height() / getFont()->height;
}

inline void Context::setOrigin(int x, int y) {
	origin = scale(x, y);
}

inline void Context::setLogicalCoords(bool b) {
	logicalCoords = b;
}

// Convert to currently active coordinate system
//
Point Context::toCurrentCoordinates(int16_t X, int16_t Y) {
	// if we're using logical coordinates then we need to scale and invert the Y axis
	if (logicalCoords) {
		return Point((double)X * logicalScaleX, (double)((canvasH - 1) - Y) * logicalScaleY);
	}

	return Point(X, Y);
}

// Convert from currently active coordinate system to screen coordinates
//
inline Point Context::toScreenCoordinates(int16_t X, int16_t Y) {
	auto p = scale(X, Y);

	if (logicalCoords) {
		return Point(origin.X + p.X, (canvasH - 1) - (origin.Y + p.Y));
	}
	return Point(origin.X + p.X, origin.Y + p.Y);
}

#endif // CONTEXT_VIEWPORT_H
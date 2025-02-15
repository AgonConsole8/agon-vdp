#ifndef CONTEXT_VIEWPORT_H
#define CONTEXT_VIEWPORT_H

#include <fabgl.h>

#include "agon.h"

// Definitions for the functions we're implementing here
#include "context.h"

// Private viewport functions

Rect * Context::getViewport(ViewportType type) {
	switch (type) {
		case ViewportType::Text: return &textViewport;
		case ViewportType::Default: return &defaultViewport;
		case ViewportType::Graphics: return &graphicsViewport;
		case ViewportType::Active: return activeViewport;
		default: return &defaultViewport;
	}
}

bool Context::setTextViewport(Rect r) {
	if (r.X2 >= canvasW) r.X2 = canvasW - 1;
	if (r.Y2 >= canvasH) r.Y2 = canvasH - 1;
	plottingText = false;

	if (r.X2 > r.X1 && r.Y2 > r.Y1) {
		textViewport = r;
		ensureCursorInViewport(textViewport);
		return true;
	}
	return false;
}

Point Context::invScale(Point p) {
	if (logicalCoords) {
		return Point((double)p.X * logicalScaleX, -(double)p.Y * logicalScaleY);
	}
	return p;
}


// Public viewport management functions

// Reset viewports to default
//
void Context::viewportReset() {
	defaultViewport = Rect(0, 0, canvasW - 1, canvasH - 1);
	textViewport =	Rect(0, 0, canvasW - 1, canvasH - 1);
	graphicsViewport = Rect(0, 0, canvasW - 1, canvasH - 1);
	activeViewport = &textViewport;
	plottingText = false;
	setClippingRect(textViewport);
}

void Context::setActiveViewport(ViewportType type) {
	activeViewport = getViewport(type);
	plottingText = false;
}

// Set graphics viewport
bool Context::setGraphicsViewport(Point p1, Point p2) {
	plottingText = false;

	if (p1.X >= 0 && p2.X < canvasW && p1.Y >= 0 && p2.Y < canvasH && p2.X >= p1.X && p2.Y >= p1.Y) {
		graphicsViewport = Rect(p1.X, p1.Y, p2.X, p2.Y);
		return true;
	}
	return false;
}

bool Context::setGraphicsViewport(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	return setGraphicsViewport(Point(x1, y1), Point(x2, y2));
}

bool Context::setGraphicsViewport() {
	auto newViewport = getGraphicsRect();
	if (newViewport.width() == 0 || newViewport.height() == 0) {
		return false;
	}
	graphicsViewport = newViewport;
	plottingText = false;
	return true;
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

// With current graphics coordinates (from graphics cursor stack)
bool Context::setTextViewport() {
	return setTextViewport(getGraphicsRect());
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

void Context::setOrigin(int x, int y) {
	auto newOrigin = scale(x, y);

	if (logicalCoords) {
		newOrigin.Y = (canvasH - 1) + newOrigin.Y;
	}

	// shift up1 by the difference between the new and old origins, with scaling
	auto delta = invScale(newOrigin.sub(origin));
	up1.X = up1.X - delta.X;
	up1.Y = up1.Y - delta.Y;

	uOrigin = Point(x, y);
	origin = newOrigin;
}

void Context::setOrigin() {
	origin = p1;
	uOrigin = up1;
	up1 = Point(0, 0);
	debug_log("setOrigin: %d,%d\n\r", origin.X, origin.Y);
}

void Context::shiftOrigin() {
	Point originDelta = p1.sub(origin);

	textViewport = textViewport.translate(originDelta).intersection(defaultViewport);
	graphicsViewport = graphicsViewport.translate(originDelta).intersection(defaultViewport);

	origin = p1;
	uOrigin = up1;
	up1 = Point(0, 0);

	textCursor = textCursor.add(originDelta);
	ensureCursorInViewport(textViewport);
}

void Context::setLogicalCoords(bool b) {
	if (b != logicalCoords) {
		// invert our origin Y point, as we're changing coordinate systems
		origin.Y = canvasH - origin.Y - 1;
		logicalCoords = b;
		// change our unscaled point according to the new setting
		if (b) {
			// point was in screen coordinates, change to logical
			up1 = Point(up1.X * logicalScaleX, LOGICAL_SCRH - (up1.Y * logicalScaleY));
			uOrigin = Point(origin.X * logicalScaleX, LOGICAL_SCRH - (origin.Y * logicalScaleY));
		} else {
			// point was in logical coordinates, change to screen coordinates
			up1 = Point(up1.X / logicalScaleX, canvasH - (up1.Y / logicalScaleY));
			uOrigin = origin;
		}
	}
}

// Scale a point, as appropriate for coordinate system
//
Point Context::scale(int16_t X, int16_t Y) {
	if (logicalCoords) {
		return Point((double)X / logicalScaleX, -(double)Y / logicalScaleY);
	}
	return Point(X, Y);
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

	return Point(origin.X + p.X, origin.Y + p.Y);
}

#endif // CONTEXT_VIEWPORT_H

#ifndef SPRITES_H
#define SPRITES_H

#include <algorithm>
#include <limits>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <fabgl.h>

#include "agon.h"
#include "agon_ps2.h"
#include "agon_screen.h"

std::unordered_map<uint16_t, std::shared_ptr<Bitmap>> bitmaps;	// Storage for our bitmaps
uint8_t			numsprites = 0;					// Number of sprites on stage
uint8_t			current_sprite = 0;				// Current sprite number
Sprite			sprites[MAX_SPRITES];			// Sprite object storage

// track which sprites may be using a bitmap
std::unordered_map<uint16_t, std::vector<uint8_t>> bitmapUsers;

std::unordered_map<uint16_t, fabgl::Cursor> cursors;	// Storage for our cursors
uint16_t		mCursor = MOUSE_DEFAULT_CURSOR;	// Mouse cursor

// character to bitmap mapping
std::vector<uint16_t> charToBitmap(255, 65535);

std::shared_ptr<Bitmap> getBitmap(uint16_t id) {
	if (bitmaps.find(id) != bitmaps.end()) {
		return bitmaps[id];
	}
	return nullptr;
}

void mapCharToBitmap(char c, uint16_t bitmapId) {
	auto bitmap = getBitmap(bitmapId);
	if (bitmap) {
		charToBitmap[c] = bitmapId;
	} else {
		debug_log("mapCharToBitmap: bitmap %d not found\n\r", bitmapId);
		charToBitmap[c] = 65535;
	}
}

std::shared_ptr<Bitmap> getBitmapFromChar(char c) {
	auto bitmapId = charToBitmap[c];
	if (bitmapId == 65535) {
		return nullptr;
	}
	return getBitmap(bitmapId);
}

// TODO remove this??  it doesn't seem to be used
void clearCursor(uint16_t cursor) {
	if (cursors.find(cursor) != cursors.end()) {
		cursors.erase(cursor);
	}
}

bool makeCursor(uint16_t bitmapId, uint16_t hotX, uint16_t hotY) {
	auto bitmap = getBitmap(bitmapId);
	if (!bitmap) {
		debug_log("addCursor: bitmap %d not found\n\r", bitmapId);
		return false;
	}
	fabgl::Cursor c;
	c.bitmap = *bitmap;
	c.hotspotX = std::min(static_cast<uint16_t>(std::max(static_cast<int>(hotX), 0)), static_cast<uint16_t>(bitmap->width - 1));
	c.hotspotY = std::min(static_cast<uint16_t>(std::max(static_cast<int>(hotY), 0)), static_cast<uint16_t>(bitmap->height - 1));
	cursors[bitmapId] = c;
	return true;
}

bool setMouseCursor(uint16_t cursor = mCursor) {
	// if our mouse is enabled, then we'll set the cursor
	if (mouseEnabled) {
		// if this cursor exists then set it
		// first, check whether it's a built-in cursor
		auto minValue = static_cast<CursorName>(std::numeric_limits<std::underlying_type<CursorName>::type>::min());
		auto maxValue = static_cast<CursorName>(std::numeric_limits<std::underlying_type<CursorName>::type>::max());
		if (minValue <= cursor && cursor <= maxValue) {
			_VGAController->setMouseCursor(static_cast<CursorName>(cursor));
			mCursor = cursor;
			return true;
		} 
		// otherwise, check whether it's a custom cursor
		if (cursors.find(cursor) != cursors.end()) {
			_VGAController->setMouseCursor(&cursors[cursor]);
			mCursor = cursor;
			return true;
		}
	}
	// otherwise make sure we remove the cursor and keep track of the requested cursor number
	_VGAController->setMouseCursor(nullptr);
	if (cursor != 65535) {
		mCursor = cursor;
	}
	return false;
}

void resetBitmaps() {
	bitmaps.clear();
	// this will only be used after resetting sprites, so we can clear the bitmapUsers list
	bitmapUsers.clear();
	cursors.clear();
	if (!setMouseCursor()) {
		setMouseCursor(MOUSE_DEFAULT_CURSOR);
	}
	// reset charToBitmap to 65535s
	std::fill(charToBitmap.begin(), charToBitmap.end(), 65535);
}

Sprite * getSprite(uint8_t sprite = current_sprite) {
	return &sprites[sprite];
}

inline void setCurrentSprite(uint8_t s) {
	current_sprite = s;
}

inline uint8_t getCurrentSprite() {
	return current_sprite;
}

void clearSpriteFrames(uint8_t s = current_sprite) {
	auto sprite = getSprite(s);
	sprite->visible = false;
	sprite->setFrame(0);
	sprite->clearBitmaps();
	// find all bitmaps used by this sprite and remove it from the list
	for (auto bitmapUser : bitmapUsers) {
		auto users = bitmapUser.second;
		// remove all instances of this sprite from the users list
		auto it = std::find(users.begin(), users.end(), s);
		while (it != users.end()) {
			users.erase(it);
			it = std::find(users.begin(), users.end(), s);
		}
	}
}

void clearBitmap(uint16_t b) {
	if (bitmaps.find(b) == bitmaps.end()) {
		return;
	}
	bitmaps.erase(b);

	// remove this bitmap from the charToBitmap mapping
	for (auto it = charToBitmap.begin(); it != charToBitmap.end(); it++) {
		if (*it == b) {
			*it = 65535;
		}
	}

	// find all sprites that had used this bitmap and clear their frames
	if (bitmapUsers.find(b) != bitmapUsers.end()) {
		auto users = bitmapUsers[b];
		for (auto user : users) {
			debug_log("clearBitmap: sprite %d can no longer use bitmap %d, so clearing sprite frames\n\r", user, b);
			clearSpriteFrames(user);
		}
		bitmapUsers.erase(b);
	}
}

void addSpriteFrame(uint16_t bitmapId) {
	auto sprite = getSprite();
	auto bitmap = getBitmap(bitmapId);
	if (!bitmap) {
		debug_log("addSpriteFrame: bitmap %d not found\n\r", bitmapId);
		return;
	}
	bitmapUsers[bitmapId].push_back(current_sprite);
	sprite->addBitmap(bitmap.get());
}

void activateSprites(uint8_t n) {
	/*
	* Sprites 0-(numsprites-1) will be activated on-screen
	* Make sure all sprites have at least one frame attached to them
	*/
	numsprites = n;

	waitPlotCompletion();
	if (numsprites) {
		_VGAController->setSprites(sprites, numsprites);
	} else {
		_VGAController->removeSprites();
	}
}

inline bool hasActiveSprites() {
	return numsprites > 0;
}

void nextSpriteFrame() {
	auto sprite = getSprite();
	sprite->nextFrame();
}

void previousSpriteFrame() {
	auto sprite = getSprite();
	auto frame = sprite->currentFrame;
	sprite->setFrame(frame ? frame - 1 : sprite->framesCount - 1);
}

void setSpriteFrame(uint8_t n) {
	auto sprite = getSprite();
	if (n < sprite->framesCount) {
		sprite->setFrame(n);
	}
}

void showSprite() {
	auto sprite = getSprite();
	sprite->visible = 1;
}

void hideSprite(uint8_t s = current_sprite) {
	auto sprite = getSprite(s);
	sprite->visible = 0;
}

void moveSprite(int x, int y) {
	auto sprite = getSprite();
	sprite->moveTo(x, y);
}

void moveSpriteBy(int x, int y) {
	auto sprite = getSprite();
	sprite->moveBy(x, y);
}

void refreshSprites() {
	if (numsprites) {
		_VGAController->refreshSprites();
	}
}

void hideAllSprites() {
	if (numsprites == 0) {
		return;
	}
	for (auto n = 0; n < MAX_SPRITES; n++) {
		hideSprite(n);
	}
	refreshSprites();
}

void resetSprites() {
	waitPlotCompletion();
	hideAllSprites();
	for (auto n = 0; n < MAX_SPRITES; n++) {
		clearSpriteFrames(n);
	}
	activateSprites(0);
	setCurrentSprite(0);
	// replace all the sprite objects
	// for (auto n = 0; n < MAX_SPRITES; n++) {
	// 	sprites[n] = Sprite();
	// }
}

void setSpritePaintMode(uint8_t mode) {
	auto sprite = getSprite();
	if (mode <= 7) {
		sprite->paintOptions.mode = static_cast<fabgl::PaintMode>(mode);
	}
}

#endif // SPRITES_H

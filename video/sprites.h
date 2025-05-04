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
#include "types.h"

std::unordered_map<uint16_t, std::shared_ptr<Bitmap>,
	std::hash<uint16_t>, std::equal_to<uint16_t>,
	psram_allocator<std::pair<const uint16_t, std::shared_ptr<Bitmap>>>> bitmaps;	// Storage for our bitmaps
uint8_t			numsprites = 0;					// Number of sprites on stage
uint8_t			current_sprite = 0;				// Current sprite number
Sprite			sprites[MAX_SPRITES];			// Sprite object storage

// track which sprites may be using a bitmap
std::unordered_map<uint16_t, std::vector<uint8_t, psram_allocator<uint8_t>>> bitmapUsers;

std::unordered_map<uint16_t, fabgl::Cursor> mouseCursors;	// Storage for our mouse cursors
uint16_t		mCursor = MOUSE_DEFAULT_CURSOR;	// Mouse cursor

extern bool isFeatureFlagSet(uint16_t flag);

std::shared_ptr<Bitmap> getBitmap(uint16_t id) {
	if (bitmaps.find(id) != bitmaps.end()) {
		return bitmaps[id];
	}
	return nullptr;
}

bool makeMouseCursor(uint16_t bitmapId, uint16_t hotX, uint16_t hotY) {
	auto bitmap = getBitmap(bitmapId);
	if (!bitmap) {
		debug_log("addCursor: bitmap %d not found\n\r", bitmapId);
		return false;
	}
	fabgl::Cursor c;
	c.bitmap = *bitmap;
	c.hotspotX = std::min(static_cast<uint16_t>(std::max(static_cast<int>(hotX), 0)), static_cast<uint16_t>(bitmap->width - 1));
	c.hotspotY = std::min(static_cast<uint16_t>(std::max(static_cast<int>(hotY), 0)), static_cast<uint16_t>(bitmap->height - 1));
	mouseCursors[bitmapId] = c;
	return true;
}

// Sets the mouse cursor to the given ID
// Works whether mouse is enabled or not
// Cursor will be shown if it exists, otherwise it will be hidden
// Calling with 65535 to hide the cursor (but remember old cursor ID)
bool setMouseCursor(uint16_t cursor = mCursor) {
	bool result = false;
	auto minValue = static_cast<CursorName>(std::numeric_limits<std::underlying_type<CursorName>::type>::min());
	auto maxValue = static_cast<CursorName>(std::numeric_limits<std::underlying_type<CursorName>::type>::max());
	if (minValue <= cursor && cursor <= maxValue) {
		_VGAController->setMouseCursor(static_cast<CursorName>(cursor));
		result = true;
	} else if (mouseCursors.find(cursor) != mouseCursors.end()) {
		// otherwise, check whether it's a custom cursor
		_VGAController->setMouseCursor(&mouseCursors[cursor]);
		result = true;
	}
	if (!result) {
		// Cursor was not found, so we remove/hide it
		_VGAController->setMouseCursor(nullptr);
		cursor = 65535;
	}
	if (cursor != 65535) {
		mCursor = cursor;
	}
	return result;
}

void clearMouseCursor(uint16_t cursor) {
	if (mouseCursors.find(cursor) != mouseCursors.end()) {
		mouseCursors.erase(cursor);
		if (cursor == mCursor) {
			if (mouseEnabled) {
				// TODO this needs to actually detect if the cursor is visible, which it can't do right now
				setMouseCursor(MOUSE_DEFAULT_CURSOR);
			} else {
				mCursor = MOUSE_DEFAULT_CURSOR;
			}
		}
	}
}

void resetBitmaps() {
	bitmaps.clear();
	// this will only be used after resetting sprites, so we can clear the bitmapUsers list
	bitmapUsers.clear();
	mouseCursors.clear();
	if (mouseEnabled) {
		if (!setMouseCursor()) {
			setMouseCursor(MOUSE_DEFAULT_CURSOR);
		}
	} else {
		auto maxValue = static_cast<CursorName>(std::numeric_limits<std::underlying_type<CursorName>::type>::max());
		if (mCursor > maxValue) {
			mCursor = MOUSE_DEFAULT_CURSOR;
		}
	}
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
	if (bitmap->format == PixelFormat::Native || bitmap->format == PixelFormat::Undefined) {
		debug_log("addSpriteFrame: bitmap %d is in native or unknown format and cannot be used as a sprite frame\n\r", bitmapId);
		return;
	}
	bitmapUsers[bitmapId].push_back(current_sprite);
	sprite->addBitmap(bitmap.get());
	if (bitmap->format == PixelFormat::Mask) {
		sprite->hardware = 0;
	}
}

void replaceSpriteFrame(uint16_t bitmapId) {
	auto sprite = getSprite();
	auto bitmap = getBitmap(bitmapId);
	if (!bitmap) {
		debug_log("replaceSpriteFrame: bitmap %d not found\n\r", bitmapId);
		return;
	}
	if (bitmap->format == PixelFormat::Native || bitmap->format == PixelFormat::Undefined) {
		debug_log("replaceSpriteFrame: bitmap %d is in native or unknown format and cannot be used as a sprite frame\n\r", bitmapId);
		return;
	}

	// remove one instance of current_sprite from the bitmapUsers list for the shown frame
	// first step is to work out the bitmap ID of the current frame
	auto frameBitmap = sprite->frames[sprite->currentFrame];
	auto oldBitmapId = -1;
	for (auto bitmap : bitmaps) {
		if (bitmap.second.get() == frameBitmap) {
			oldBitmapId = bitmap.first;
			break;
		}
	}

	// if we found the old bitmap, remove one entry of the sprite from the bitmapUsers list
	if (oldBitmapId != -1) {
		auto users = bitmapUsers[oldBitmapId];
		for (auto user : users) {
			// remove only one copy of current_sprite from the users list, if it exists
			auto it = std::find(users.begin(), users.end(), current_sprite);
			if (it != users.end()) {
				users.erase(it);
				break;
			}
		}
	}

	sprite->frames[sprite->currentFrame] = bitmap.get();
	bitmapUsers[bitmapId].push_back(current_sprite);

	if (bitmap->format == PixelFormat::Mask) {
		sprite->hardware = 0;
	}
}

void activateSprites(uint8_t n) {
	/*
	* Sprites 0-(numsprites-1) will be activated on-screen
	* Make sure all sprites have at least one frame attached to them
	*/
	if (numsprites != n) {
		numsprites = n;

		waitPlotCompletion();
		if (numsprites) {
			_VGAController->setSprites(sprites, numsprites);
		} else {
			_VGAController->removeSprites();
		}
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

void setSpriteHardware() {
	auto sprite = getSprite();
	sprite->hardware = 1;
}

void setSpriteSoftware() {
	auto sprite = getSprite();
	sprite->hardware = 0;
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
	bool autoHardwareSprites = isFeatureFlagSet(TESTFLAG_HW_SPRITES) && isFeatureFlagSet(FEATUREFLAG_AUTO_HW_SPRITES);
	for (auto n = 0; n < MAX_SPRITES; n++) {
		auto sprite = getSprite(n);
		sprite->hardware = autoHardwareSprites ? 1 : 0;
		clearSpriteFrames(n);
	}
	activateSprites(0);
	setCurrentSprite(0);
}

void setSpritePaintMode(uint8_t mode) {
	auto sprite = getSprite();
	if (mode <= 7) {
		sprite->paintOptions.mode = static_cast<fabgl::PaintMode>(mode);
		if (mode > 0) {
			sprite->hardware = 0;
		}
	}
}

#endif // SPRITES_H

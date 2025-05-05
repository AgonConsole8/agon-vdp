#ifndef FEATURE_FLAGS_H
#define FEATURE_FLAGS_H

#include <memory>
#include <unordered_map>
#include <ESP32Time.h>

#include "agon.h"
#include "agon_ps2.h"
#include "vdu_stream_processor.h"
#include "vdp_protocol.h"

std::unordered_map<uint16_t, uint16_t> featureFlags;	// Feature/Test flags
extern VDUStreamProcessor *processor;
extern ESP32Time		rtc;

// Forward declare getVDPVariable
uint16_t getVDPVariable(uint16_t flag);

void setVDPVariable(uint16_t flag, uint16_t value) {
	if (flag >= VDPVAR_VDU_VARIABLES_START && flag <= VDPVAR_VDU_VARIABLES_END) {
		processor->getContext()->setVariable(flag & VDPVAR_VDU_VARIABLES_MASK, value);
		return;
	}
	if (flag >= VDPVAR_SYSTEM_BEGIN && flag <= VDPVAR_SYSTEM_END) {
		switch (flag) {
			case VDPVAR_RTC_YEAR:
				rtc.setTime(rtc.getSecond(), rtc.getMinute(), rtc.getHour(true), rtc.getDay(), rtc.getMonth(), value);
				return;
			case VDPVAR_RTC_MONTH:
				rtc.setTime(rtc.getSecond(), rtc.getMinute(), rtc.getHour(true), rtc.getDay(), value, rtc.getYear());
				return;
			case VDPVAR_RTC_DAY:
				rtc.setTime(rtc.getSecond(), rtc.getMinute(), rtc.getHour(true), value, rtc.getMonth(), rtc.getYear());
				return;
			case VDPVAR_RTC_HOUR:
				rtc.setTime(rtc.getSecond(), rtc.getMinute(), value, rtc.getDay(), rtc.getMonth(), rtc.getYear());
				return;
			case VDPVAR_RTC_MINUTE:
				rtc.setTime(rtc.getSecond(), value, rtc.getHour(true), rtc.getDay(), rtc.getMonth(), rtc.getYear());
				return;
			case VDPVAR_RTC_SECOND:
				rtc.setTime(value, rtc.getMinute(), rtc.getHour(true), rtc.getDay(), rtc.getMonth(), rtc.getYear());
				return;
			case VDPVAR_RTC_MILLIS:
			case VDPVAR_RTC_WEEKDAY:
			case VDPVAR_RTC_YEARDAY:
			case VDPVAR_FREEPSRAM_LOW:
			case VDPVAR_FREEPSRAM_HIGH:
			case VDPVAR_BUFFERS_USED:
				return;

			case VDPVAR_KEYBOARD_LAYOUT:
				setKeyboardLayout(value);
				return;
			case VDPVAR_KEYBOARD_CTRL_KEYS:
				controlKeys = value != 0;
				return;
			case VDPVAR_KEYBOARD_REP_DELAY:
				setKeyboardState(value, kbRepeatRate, 255);
				return;
			case VDPVAR_KEYBOARD_REP_RATE:
				setKeyboardState(kbRepeatDelay, value, 255);
				return;
			case VDPVAR_KEYBOARD_LED:
				getKeyboard()->setLEDs(value & 4, value & 2, value & 1);
				return;
			case VDPVAR_KEYBOARD_LED_NUM: {
				bool numLock;
				bool capsLock;
				bool scrollLock;
				auto kb = getKeyboard();
				kb->getLEDs(&numLock, &capsLock, &scrollLock);
				numLock = value & 1;
				kb->setLEDs(numLock, capsLock, scrollLock);
				return;
			}
			case VDPVAR_KEYBOARD_LED_CAPS: {
				bool numLock;
				bool capsLock;
				bool scrollLock;
				auto kb = getKeyboard();
				kb->getLEDs(&numLock, &capsLock, &scrollLock);
				capsLock = value & 1;
				kb->setLEDs(numLock, capsLock, scrollLock);
				return;
			}
			case VDPVAR_KEYBOARD_LED_SCROLL: {
				bool numLock;
				bool capsLock;
				bool scrollLock;
				auto kb = getKeyboard();
				kb->getLEDs(&numLock, &capsLock, &scrollLock);
				scrollLock = value & 1;
				kb->setLEDs(numLock, capsLock, scrollLock);
				return;
			}
				
			case VDPVAR_CONTEXT_ID:
				processor->selectContext(value);
				processor->sendModeInformation();
				return;

			case VDPVAR_MOUSE_CURSOR:	// Mouse cursor ID
				setMouseCursor(value);
				return;
			case VDPVAR_MOUSE_ENABLED:	// Mouse cursor enabled
				if (value) {
					enableMouse();
					processor->sendMouseData();
					processor->bufferCallCallbacks(CALLBACK_MOUSE);
				} else {
					disableMouse();
				}
				return;
			case VDPVAR_MOUSE_XPOS: {	// Mouse cursor X position (pixel coords)
				uint16_t mouseY = getVDPVariable(VDPVAR_MOUSE_YPOS);
				setMousePos(value, mouseY);
				setMouseCursorPos(value, mouseY);
				processor->sendMouseData();
				processor->bufferCallCallbacks(CALLBACK_MOUSE);
				return;
			}
			case VDPVAR_MOUSE_YPOS: {	// Mouse cursor Y position (pixel coords)
				uint16_t mouseX = getVDPVariable(VDPVAR_MOUSE_XPOS);
				setMousePos(mouseX, value);
				setMouseCursorPos(mouseX, value);
				processor->sendMouseData();
				processor->bufferCallCallbacks(CALLBACK_MOUSE);
				return;
			}
			case VDPVAR_MOUSE_BUTTONS:	// Mouse cursor button status
			case VDPVAR_MOUSE_WHEEL:	// Mouse wheel
				return;
			case VDPVAR_MOUSE_SAMPLERATE:	// Mouse sample rate
				setMouseSampleRate(value);
				return;
			case VDPVAR_MOUSE_RESOLUTION:	// Mouse resolution
				setMouseResolution(value);
				return;
			case VDPVAR_MOUSE_SCALING:	// Mouse scaling
				setMouseScaling(value);
				return;
			case VDPVAR_MOUSE_ACCELERATION:	// Mouse acceleration
				setMouseAcceleration(value);
				return;
			case VDPVAR_MOUSE_WHEELACC:	// Mouse wheel acceleration
				setMouseWheelAcceleration(value);
				return;
			case VDPVAR_MOUSE_VISIBLE:	// Mouse cursor visible (1) or hidden (0)
				if (value) {
					showMouseCursor();
				} else {
					hideMouseCursor();
				}
				return;
			// we have a range here (0x24C-0x24F) reserved for mouse area	
		}
	}
	switch (flag) {
		case VDPVAR_FULL_DUPLEX:
			setVDPProtocolDuplex(value != 0);
			debug_log("Full duplex mode requested\n\r");
			break;
		case TESTFLAG_ECHO:
			debug_log("Echo mode requested\n\r");
			processor->setEcho(value != 0);
			break;
		case TESTFLAG_VDPP_BUFFERSIZE:
			debug_log("Echo buffer size requested: %d\n\r", value);
			break;
	}

	featureFlags[flag] = value;
}

void clearVDPVariable(uint16_t flag) {
	auto flagIter = featureFlags.find(flag);

	switch (flag) {
		case VDPVAR_FULL_DUPLEX:
			setVDPProtocolDuplex(false);
			debug_log("Full duplex mode disabled\n\r");
			break;

		case TESTFLAG_ECHO:
			debug_log("Echo mode disabled\n\r");
			processor->setEcho(false);
			break;

		case VDPVAR_MOUSE_CURSOR:	// Mouse cursor ID
			setMouseCursor(MOUSE_DEFAULT_CURSOR);
			hideMouseCursor();
			return;
		case VDPVAR_MOUSE_ENABLED:
			disableMouse();
			return;
		case VDPVAR_MOUSE_VISIBLE:
			hideMouseCursor();
			return;
	}

	if (flagIter != featureFlags.end()) {
		featureFlags.erase(flagIter);
	}
}

bool isVDPVariableSet(uint16_t flag) {
	if (flag >= VDPVAR_VDU_VARIABLES_START && flag <= VDPVAR_VDU_VARIABLES_END) {
		return processor->getContext()->readVariable(flag & VDPVAR_VDU_VARIABLES_MASK, nullptr);
	}
	if (flag >= VDPVAR_SYSTEM_BEGIN && flag <= VDPVAR_SYSTEM_END) {
		switch (flag) {
			case VDPVAR_RTC_YEAR:
			case VDPVAR_RTC_MONTH:
			case VDPVAR_RTC_DAY:
			case VDPVAR_RTC_HOUR:
			case VDPVAR_RTC_MINUTE:
			case VDPVAR_RTC_SECOND:
			case VDPVAR_RTC_MILLIS:
			case VDPVAR_RTC_WEEKDAY:
			case VDPVAR_RTC_YEARDAY:
			case VDPVAR_FREEPSRAM_LOW:
			case VDPVAR_FREEPSRAM_HIGH:
			case VDPVAR_BUFFERS_USED:
			case VDPVAR_KEYBOARD_LAYOUT:
			case VDPVAR_KEYBOARD_CTRL_KEYS:
			case VDPVAR_KEYBOARD_REP_DELAY:
			case VDPVAR_KEYBOARD_REP_RATE:
			case VDPVAR_KEYBOARD_LED:
			case VDPVAR_KEYBOARD_LED_NUM:
			case VDPVAR_KEYBOARD_LED_CAPS:
			case VDPVAR_KEYBOARD_LED_SCROLL:
			case VDPVAR_CONTEXT_ID:
			case VDPVAR_MOUSE_CURSOR:
			case VDPVAR_MOUSE_ENABLED:
			case VDPVAR_MOUSE_XPOS:
			case VDPVAR_MOUSE_YPOS:
			case VDPVAR_MOUSE_BUTTONS:
			case VDPVAR_MOUSE_WHEEL:
			case VDPVAR_MOUSE_SAMPLERATE:
			case VDPVAR_MOUSE_RESOLUTION:
			case VDPVAR_MOUSE_SCALING:
			case VDPVAR_MOUSE_ACCELERATION:
			case VDPVAR_MOUSE_WHEELACC:
			case VDPVAR_MOUSE_VISIBLE:
			case VDPVAR_KEYEVENT_KEYCODE:
			case VDPVAR_KEYEVENT_VK:
			case VDPVAR_KEYEVENT_DOWN:
			case VDPVAR_KEYEVENT_MODIFIERS:
			case VDPVAR_KEYEVENT_CTRL:
			case VDPVAR_KEYEVENT_LALT:
			case VDPVAR_KEYEVENT_RALT:
			case VDPVAR_KEYEVENT_SHIFT:
			case VDPVAR_KEYEVENT_GUI:
			case VDPVAR_KEYEVENT_CAPSLOCK:
			case VDPVAR_KEYEVENT_NUMLOCK:
			case VDPVAR_KEYEVENT_SCROLLLOCK:
			case VDPVAR_KEYEVENT_SCANCODE1:
			case VDPVAR_KEYEVENT_SCANCODE2:
			case VDPVAR_KEYEVENT_SCANCODE3:
			case VDPVAR_KEYEVENT_SCANCODE4:
				return true;
		}
	}
	auto flagIter = featureFlags.find(flag);
	return flagIter != featureFlags.end();
}

uint16_t getVDPVariable(uint16_t flag) {
	if (flag >= VDPVAR_VDU_VARIABLES_START && flag <= VDPVAR_VDU_VARIABLES_END) {
		uint16_t value = 0;
		processor->getContext()->readVariable(flag & VDPVAR_VDU_VARIABLES_MASK, &value);
		return value;
	}
	auto flagIter = featureFlags.find(flag);
	if (flagIter != featureFlags.end()) {
		return featureFlags[flag];
	} else {
		switch (flag) {
			case VDPVAR_RTC_YEAR:
				return rtc.getYear();
			case VDPVAR_RTC_MONTH:
				return rtc.getMonth();		// 0 - 11
			case VDPVAR_RTC_DAY:
				return rtc.getDay();		// 1 - 31
			case VDPVAR_RTC_HOUR:
				return rtc.getHour(true);	// 0 - 23
			case VDPVAR_RTC_MINUTE:
				return rtc.getMinute();		// 0 - 59
			case VDPVAR_RTC_SECOND:
				return rtc.getSecond();		// 0 - 59
			case VDPVAR_RTC_MILLIS:
				return rtc.getMillis();		// 0 - 999
			case VDPVAR_RTC_WEEKDAY:
				return rtc.getDayofWeek();	// 0 - 6
			case VDPVAR_RTC_YEARDAY:
				return rtc.getDayofYear();	// 0 - 365

			case VDPVAR_FREEPSRAM_LOW:
				return heap_caps_get_free_size(MALLOC_CAP_SPIRAM) & 0xFFFF;
			case VDPVAR_FREEPSRAM_HIGH:
				return heap_caps_get_free_size(MALLOC_CAP_SPIRAM) >> 16;

			case VDPVAR_BUFFERS_USED:
				return buffers.size();

			case VDPVAR_KEYBOARD_LAYOUT:
				return kbRegion;
			case VDPVAR_KEYBOARD_CTRL_KEYS:
				return controlKeys ? 1 : 0;
			case VDPVAR_KEYBOARD_REP_DELAY:
				return kbRepeatDelay;
			case VDPVAR_KEYBOARD_REP_RATE:
				return kbRepeatRate;
			case VDPVAR_KEYBOARD_LED: {
				bool numLock;
				bool capsLock;
				bool scrollLock;
				getKeyboard()->getLEDs(&numLock, &capsLock, &scrollLock);
				return scrollLock | (capsLock << 1) | (numLock << 2);
			}
			case VDPVAR_KEYBOARD_LED_NUM: {
				bool numLock;
				bool capsLock;
				bool scrollLock;
				getKeyboard()->getLEDs(&numLock, &capsLock, &scrollLock);
				return numLock ? 1 : 0;
			}
			case VDPVAR_KEYBOARD_LED_CAPS: {
				bool numLock;
				bool capsLock;
				bool scrollLock;
				getKeyboard()->getLEDs(&numLock, &capsLock, &scrollLock);
				return capsLock ? 1 : 0;
			}
			case VDPVAR_KEYBOARD_LED_SCROLL: {
				bool numLock;
				bool capsLock;
				bool scrollLock;
				getKeyboard()->getLEDs(&numLock, &capsLock, &scrollLock);
				return scrollLock ? 1 : 0;
			}
				
			case VDPVAR_CONTEXT_ID:
				return processor->contextId;

			case VDPVAR_MOUSE_CURSOR:
				return mCursor;
			case VDPVAR_MOUSE_ENABLED:
				return mouseEnabled ? 1 : 0;
			case VDPVAR_MOUSE_XPOS: {
				auto mouse = getMouse();
				if (mouse) {
					auto mStatus = mouse->status();
					return mStatus.X;
				}
			}
			case VDPVAR_MOUSE_YPOS: {
				auto mouse = getMouse();
				if (mouse) {
					auto mStatus = mouse->status();
					return mStatus.Y;
				}
			}
			case VDPVAR_MOUSE_BUTTONS: {
				auto mouse = getMouse();
				if (mouse) {
					auto mStatus = mouse->status();
					return mStatus.buttons.left << 0 | mStatus.buttons.right << 1 | mStatus.buttons.middle << 2;
				}
			}
			case VDPVAR_MOUSE_WHEEL: {
				auto mouse = getMouse();
				if (mouse) {
					auto mStatus = mouse->status();
					return mStatus.wheelDelta;
				}
			}
			case VDPVAR_MOUSE_SAMPLERATE:
				return mSampleRate;
			case VDPVAR_MOUSE_RESOLUTION:
				return mResolution;
			case VDPVAR_MOUSE_SCALING:
				return mScaling;
			case VDPVAR_MOUSE_ACCELERATION:
				return mAcceleration;
			case VDPVAR_MOUSE_WHEELACC: {
				auto mouse = getMouse();
				if (mouse) {
					auto & currentAcceleration = mouse->wheelAcceleration();
					return currentAcceleration;
				}
			}
			case VDPVAR_MOUSE_VISIBLE:
				return mouseVisible ? 1 : 0;

			case VDPVAR_KEYEVENT_KEYCODE:
				return _keycode | kbItem.ASCII << 8;
			case VDPVAR_KEYEVENT_VK:
				return kbItem.vk;
			case VDPVAR_KEYEVENT_DOWN:
				return kbItem.down;
			case VDPVAR_KEYEVENT_MODIFIERS:
				return packKeyboardModifiers(&kbItem);
			case VDPVAR_KEYEVENT_CTRL:
				return kbItem.CTRL;
			case VDPVAR_KEYEVENT_LALT:
				return kbItem.LALT;
			case VDPVAR_KEYEVENT_RALT:
				return kbItem.RALT;
			case VDPVAR_KEYEVENT_SHIFT:
				return kbItem.SHIFT;
			case VDPVAR_KEYEVENT_GUI:
				return kbItem.GUI;
			case VDPVAR_KEYEVENT_CAPSLOCK:
				return kbItem.CAPSLOCK;
			case VDPVAR_KEYEVENT_NUMLOCK:
				return kbItem.NUMLOCK;
			case VDPVAR_KEYEVENT_SCROLLLOCK:
				return kbItem.SCROLLLOCK;
			case VDPVAR_KEYEVENT_SCANCODE1:
				return reinterpret_cast<const uint16_t*>(kbItem.scancode)[0];
			case VDPVAR_KEYEVENT_SCANCODE2:
				return reinterpret_cast<const uint16_t*>(kbItem.scancode)[1];
			case VDPVAR_KEYEVENT_SCANCODE3:
				return reinterpret_cast<const uint16_t*>(kbItem.scancode)[2];
			case VDPVAR_KEYEVENT_SCANCODE4:
				return reinterpret_cast<const uint16_t*>(kbItem.scancode)[3];
		}
	}
	return 0;
}

#endif // FEATURE_FLAGS_H

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

// Forward declare getFeatureFlag
uint16_t getFeatureFlag(uint16_t flag);

void setFeatureFlag(uint16_t flag, uint16_t value) {
	if (flag >= FEATUREFLAG_VDU_VARIABLES_START && flag <= FEATUREFLAG_VDU_VARIABLES_END) {
		processor->getContext()->setVariable(flag & FEATUREFLAG_VDU_VARIABLES_MASK, value);
		return;
	}
	if (flag >= FEATUREFLAG_SYSTEM_BEGIN && flag <= FEATUREFLAG_SYSTEM_END) {
		switch (flag) {
			case FEATUREFLAG_RTC_YEAR:
				rtc.setTime(rtc.getSecond(), rtc.getMinute(), rtc.getHour(true), rtc.getDay(), rtc.getMonth(), value);
				return;
			case FEATUREFLAG_RTC_MONTH:
				rtc.setTime(rtc.getSecond(), rtc.getMinute(), rtc.getHour(true), rtc.getDay(), value, rtc.getYear());
				return;
			case FEATUREFLAG_RTC_DAY:
				rtc.setTime(rtc.getSecond(), rtc.getMinute(), rtc.getHour(true), value, rtc.getMonth(), rtc.getYear());
				return;
			case FEATUREFLAG_RTC_HOUR:
				rtc.setTime(rtc.getSecond(), rtc.getMinute(), value, rtc.getDay(), rtc.getMonth(), rtc.getYear());
				return;
			case FEATUREFLAG_RTC_MINUTE:
				rtc.setTime(rtc.getSecond(), value, rtc.getHour(true), rtc.getDay(), rtc.getMonth(), rtc.getYear());
				return;
			case FEATUREFLAG_RTC_SECOND:
				rtc.setTime(value, rtc.getMinute(), rtc.getHour(true), rtc.getDay(), rtc.getMonth(), rtc.getYear());
				return;
			case FEATUREFLAG_RTC_MILLIS:
			case FEATUREFLAG_RTC_WEEKDAY:
			case FEATUREFLAG_RTC_YEARDAY:
			case FEATUREFLAG_FREEPSRAM_LOW:
			case FEATUREFLAG_FREEPSRAM_HIGH:
			case FEATUREFLAG_BUFFERS_USED:
				return;

			case FEATUREFLAG_KEYBOARD_LAYOUT:
				setKeyboardLayout(value);
				return;
			case FEATUREFLAG_KEYBOARD_CTRL_KEYS:
				controlKeys = value != 0;
				return;
			
			case FEATUREFLAG_CONTEXT_ID:
				processor->selectContext(value);
				processor->sendModeInformation();
				return;

			case FEATUREFLAG_MOUSE_CURSOR:	// Mouse cursor ID
				setMouseCursor(value);
				return;
			case FEATUREFLAG_MOUSE_ENABLED:	// Mouse cursor enabled
				if (value) {
					enableMouse();
					processor->sendMouseData();
					processor->bufferCallCallbacks(CALLBACK_MOUSE);
				} else {
					disableMouse();
				}
				return;
			case FEATUREFLAG_MOUSE_XPOS: {	// Mouse cursor X position (pixel coords)
				uint16_t mouseY = getFeatureFlag(FEATUREFLAG_MOUSE_YPOS);
				setMousePos(value, mouseY);
				setMouseCursorPos(value, mouseY);
				processor->sendMouseData();
				processor->bufferCallCallbacks(CALLBACK_MOUSE);
				return;
			};
			case FEATUREFLAG_MOUSE_YPOS: {	// Mouse cursor Y position (pixel coords)
				uint16_t mouseX = getFeatureFlag(FEATUREFLAG_MOUSE_XPOS);
				setMousePos(mouseX, value);
				setMouseCursorPos(mouseX, value);
				processor->sendMouseData();
				processor->bufferCallCallbacks(CALLBACK_MOUSE);
				return;
			};
			case FEATUREFLAG_MOUSE_BUTTONS:	// Mouse cursor button status
			case FEATUREFLAG_MOUSE_WHEEL:	// Mouse wheel
				return;
			case FEATUREFLAG_MOUSE_SAMPLERATE:	// Mouse sample rate
				setMouseSampleRate(value);
				return;
			case FEATUREFLAG_MOUSE_RESOLUTION:	// Mouse resolution
				setMouseResolution(value);
				return;
			case FEATUREFLAG_MOUSE_SCALING:	// Mouse scaling
				setMouseScaling(value);
				return;
			case FEATUREFLAG_MOUSE_ACCELERATION:	// Mouse acceleration
				setMouseAcceleration(value);
				return;
			case FEATUREFLAG_MOUSE_WHEELACC:	// Mouse wheel acceleration
				setMouseWheelAcceleration(value);
				return;
			case FEATUREFLAG_MOUSE_VISIBLE:	// Mouse cursor visible (1) or hidden (0)
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
		case FEATUREFLAG_FULL_DUPLEX:
			setVDPProtocolDuplex(value != 0);
			debug_log("Full duplex mode requested\n\r");
			break;
		case FEATUREFLAG_ECHO:
			debug_log("Echo mode requested\n\r");
			processor->setEcho(value != 0);
			break;
		case FEATUREFLAG_MOS_VDPP_BUFFERSIZE:
			debug_log("Echo buffer size requested: %d\n\r", value);
			break;
	}

	featureFlags[flag] = value;
}

void clearFeatureFlag(uint16_t flag) {
	auto flagIter = featureFlags.find(flag);

	switch (flag) {
		case FEATUREFLAG_FULL_DUPLEX:
			setVDPProtocolDuplex(false);
			debug_log("Full duplex mode disabled\n\r");
			break;

		case FEATUREFLAG_ECHO:
			debug_log("Echo mode disabled\n\r");
			processor->setEcho(false);
			break;

		case FEATUREFLAG_MOUSE_CURSOR:	// Mouse cursor ID
			setMouseCursor(MOUSE_DEFAULT_CURSOR);
			hideMouseCursor();
			return;
		case FEATUREFLAG_MOUSE_ENABLED:
			disableMouse();
			return;
		case FEATUREFLAG_MOUSE_VISIBLE:
			hideMouseCursor();
			return;
	}

	if (flagIter != featureFlags.end()) {
		featureFlags.erase(flagIter);
	}
}

bool isFeatureFlagSet(uint16_t flag) {
	if (flag >= FEATUREFLAG_VDU_VARIABLES_START && flag <= FEATUREFLAG_VDU_VARIABLES_END) {
		return processor->getContext()->readVariable(flag & FEATUREFLAG_VDU_VARIABLES_MASK, nullptr);
	}
	if (flag >= FEATUREFLAG_SYSTEM_BEGIN && flag <= FEATUREFLAG_SYSTEM_END) {
		switch (flag) {
			case FEATUREFLAG_RTC_YEAR:
			case FEATUREFLAG_RTC_MONTH:
			case FEATUREFLAG_RTC_DAY:
			case FEATUREFLAG_RTC_HOUR:
			case FEATUREFLAG_RTC_MINUTE:
			case FEATUREFLAG_RTC_SECOND:
			case FEATUREFLAG_RTC_MILLIS:
			case FEATUREFLAG_RTC_WEEKDAY:
			case FEATUREFLAG_RTC_YEARDAY:
			case FEATUREFLAG_FREEPSRAM_LOW:
			case FEATUREFLAG_FREEPSRAM_HIGH:
			case FEATUREFLAG_BUFFERS_USED:
			case FEATUREFLAG_KEYBOARD_LAYOUT:
			case FEATUREFLAG_KEYBOARD_CTRL_KEYS:
			case FEATUREFLAG_CONTEXT_ID:
			case FEATUREFLAG_MOUSE_CURSOR:
			case FEATUREFLAG_MOUSE_ENABLED:
			case FEATUREFLAG_MOUSE_XPOS:
			case FEATUREFLAG_MOUSE_YPOS:
			case FEATUREFLAG_MOUSE_BUTTONS:
			case FEATUREFLAG_MOUSE_WHEEL:
			case FEATUREFLAG_MOUSE_SAMPLERATE:
			case FEATUREFLAG_MOUSE_RESOLUTION:
			case FEATUREFLAG_MOUSE_SCALING:
			case FEATUREFLAG_MOUSE_ACCELERATION:
			case FEATUREFLAG_MOUSE_WHEELACC:
			case FEATUREFLAG_MOUSE_VISIBLE:
				return true;
		}
	}
	auto flagIter = featureFlags.find(flag);
	return flagIter != featureFlags.end();
}

uint16_t getFeatureFlag(uint16_t flag) {
	if (flag >= FEATUREFLAG_VDU_VARIABLES_START && flag <= FEATUREFLAG_VDU_VARIABLES_END) {
		uint16_t value = 0;
		processor->getContext()->readVariable(flag & FEATUREFLAG_VDU_VARIABLES_MASK, &value);
		return value;
	}
	auto flagIter = featureFlags.find(flag);
	if (flagIter != featureFlags.end()) {
		return featureFlags[flag];
	} else {
		switch (flag) {
			case FEATUREFLAG_RTC_YEAR:
				return rtc.getYear();
			case FEATUREFLAG_RTC_MONTH:
				return rtc.getMonth();		// 0 - 11
			case FEATUREFLAG_RTC_DAY:
				return rtc.getDay();		// 1 - 31
			case FEATUREFLAG_RTC_HOUR:
				return rtc.getHour(true);	// 0 - 23
			case FEATUREFLAG_RTC_MINUTE:
				return rtc.getMinute();		// 0 - 59
			case FEATUREFLAG_RTC_SECOND:
				return rtc.getSecond();		// 0 - 59
			case FEATUREFLAG_RTC_MILLIS:
				return rtc.getMillis();		// 0 - 999
			case FEATUREFLAG_RTC_WEEKDAY:
				return rtc.getDayofWeek();	// 0 - 6
			case FEATUREFLAG_RTC_YEARDAY:
				return rtc.getDayofYear();	// 0 - 365

			case FEATUREFLAG_FREEPSRAM_LOW:
				return heap_caps_get_free_size(MALLOC_CAP_SPIRAM) & 0xFFFF;
			case FEATUREFLAG_FREEPSRAM_HIGH:
				return heap_caps_get_free_size(MALLOC_CAP_SPIRAM) >> 16;

			case FEATUREFLAG_BUFFERS_USED:
				return buffers.size();

			case FEATUREFLAG_KEYBOARD_LAYOUT:
				return kbRegion;
			case FEATUREFLAG_KEYBOARD_CTRL_KEYS:
				return controlKeys ? 1 : 0;
			
			case FEATUREFLAG_CONTEXT_ID:
				return processor->contextId;

			case FEATUREFLAG_MOUSE_CURSOR:
				return mCursor;
			case FEATUREFLAG_MOUSE_ENABLED:
				return mouseEnabled ? 1 : 0;
			case FEATUREFLAG_MOUSE_XPOS: {
				auto mouse = getMouse();
				if (mouse) {
					auto mStatus = mouse->status();
					return mStatus.X;
				}
			};
			case FEATUREFLAG_MOUSE_YPOS: {
				auto mouse = getMouse();
				if (mouse) {
					auto mStatus = mouse->status();
					return mStatus.Y;
				}
			};
			case FEATUREFLAG_MOUSE_BUTTONS: {
				auto mouse = getMouse();
				if (mouse) {
					auto mStatus = mouse->status();
					return mStatus.buttons.left << 0 | mStatus.buttons.right << 1 | mStatus.buttons.middle << 2;
				}
			};
			case FEATUREFLAG_MOUSE_WHEEL: {
				auto mouse = getMouse();
				if (mouse) {
					auto mStatus = mouse->status();
					return mStatus.wheelDelta;
				}
			};
			case FEATUREFLAG_MOUSE_SAMPLERATE:
				return mSampleRate;
			case FEATUREFLAG_MOUSE_RESOLUTION:
				return mResolution;
			case FEATUREFLAG_MOUSE_SCALING:
				return mScaling;
			case FEATUREFLAG_MOUSE_ACCELERATION:
				return mAcceleration;
			case FEATUREFLAG_MOUSE_WHEELACC: {
				auto mouse = getMouse();
				if (mouse) {
					auto & currentAcceleration = mouse->wheelAcceleration();
					return currentAcceleration;
				}
			};
			case FEATUREFLAG_MOUSE_VISIBLE:
				return mouseVisible ? 1 : 0;
		}
	}
	return 0;
}

#endif // FEATURE_FLAGS_H

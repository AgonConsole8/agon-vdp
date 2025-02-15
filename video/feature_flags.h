#ifndef FEATURE_FLAGS_H
#define FEATURE_FLAGS_H

#include <memory>
#include <unordered_map>

#include "agon.h"
#include "vdu_stream_processor.h"
#include "vdp_protocol.h"

std::unordered_map<uint16_t, uint16_t> featureFlags;	// Feature/Test flags
extern VDUStreamProcessor *processor;

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
		}
	}
	return 0;
}

#endif // FEATURE_FLAGS_H

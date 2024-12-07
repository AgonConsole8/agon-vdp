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

inline bool isFeatureFlagSet(uint16_t flag) {
	auto flagIter = featureFlags.find(flag);
	return flagIter != featureFlags.end();
}

inline uint16_t getFeatureFlag(uint16_t flag) {
	auto flagIter = featureFlags.find(flag);
	if (flagIter != featureFlags.end()) {
		return featureFlags[flag];
		// return flagIter->second;
	}
	return 0;
}

#endif // FEATURE_FLAGS_H

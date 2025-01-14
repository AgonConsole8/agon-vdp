#ifndef FEATURE_FLAGS_H
#define FEATURE_FLAGS_H

#include <memory>
#include <unordered_map>

#include "agon.h"
#include "vdp_protocol.h"

std::unordered_map<uint16_t, uint16_t> featureFlags;	// Feature/Test flags

void setFeatureFlag(uint16_t flag, uint16_t value) {
	featureFlags[flag] = value;

	switch (flag) {
		case FEATUREFLAG_FULL_DUPLEX:
			setVDPProtocolDuplex(value != 0);
			debug_log("Full duplex mode requested\n\r");
			break;
	}
}

inline void clearFeatureFlag(uint16_t flag) {
	auto flagIter = featureFlags.find(flag);
	if (flagIter != featureFlags.end()) {
		featureFlags.erase(flagIter);
	}

	switch (flag) {
		case FEATUREFLAG_FULL_DUPLEX:
			setVDPProtocolDuplex(false);
			debug_log("Full duplex mode disabled\n\r");
			break;
	}
}

inline bool isFeatureFlagSet(uint16_t flag) {
	auto flagIter = featureFlags.find(flag);
	return flagIter != featureFlags.end();
}

inline uint16_t getFeatureFlag(uint16_t flag) {
	auto flagIter = featureFlags.find(flag);
	if (flagIter != featureFlags.end()) {
		return flagIter->second;
	}
	return 0;
}

#endif // FEATURE_FLAGS_H

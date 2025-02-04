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
		processor->getContext()->setVariable(flag & 0xFF, value);
		return;
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
		return processor->getContext()->readVariable(flag & 0xFF, nullptr);
	}
	auto flagIter = featureFlags.find(flag);
	return flagIter != featureFlags.end();
}

uint16_t getFeatureFlag(uint16_t flag) {
	if (flag >= FEATUREFLAG_VDU_VARIABLES_START && flag <= FEATUREFLAG_VDU_VARIABLES_END) {
		uint16_t value = 0;
		processor->getContext()->readVariable(flag & 0xFF, &value);
		return value;
	}
	auto flagIter = featureFlags.find(flag);
	if (flagIter != featureFlags.end()) {
		return featureFlags[flag];
	}
	return 0;
}

#endif // FEATURE_FLAGS_H

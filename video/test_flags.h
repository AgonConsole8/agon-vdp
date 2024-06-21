#ifndef TEST_FLAGS_H
#define TEST_FLAGS_H

#include <memory>
#include <unordered_map>

std::unordered_map<uint16_t, uint16_t> testFlags;	// Test flags

inline void setTestFlag(uint16_t flag, uint16_t value) {
	testFlags[flag] = value;
}

inline void clearTestFlag(uint16_t flag) {
	auto flagIter = testFlags.find(flag);
	if (flagIter != testFlags.end()) {
		testFlags.erase(flagIter);
	}
}

inline bool isTestFlagSet(uint16_t flag) {
	auto flagIter = testFlags.find(flag);
	return flagIter != testFlags.end();
}

inline uint16_t getTestFlag(uint16_t flag) {
	auto flagIter = testFlags.find(flag);
	if (flagIter != testFlags.end()) {
		return flagIter->second;
	}
	return 0;
}

#endif // TEST_FLAGS_H

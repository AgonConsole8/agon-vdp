#ifndef BUFFERS_H
#define BUFFERS_H

#include <memory>
#include <vector>
#include <unordered_map>

#include "buffer_stream.h"
#include "span.h"

std::unordered_map<uint16_t, std::vector<std::shared_ptr<BufferStream>>> buffers;

// Utility functions for buffer management:

// Resolve a buffer id
int32_t resolveBufferId(int32_t bufferId, uint16_t currentId) {
	if (bufferId == 65535) {
		// buffer ID of 65535 means use the current buffer
		if (currentId == 65535) {
			// this is an error, we can't resolve a buffer id if we're not in a buffer call
			return -1;
		}
		return currentId;
	}
	return bufferId;
}

// Reverse values in a buffer
void reverseValues(uint8_t * data, uint32_t length, uint16_t valueSize) {
	// get last offset into buffer
	auto bufferEnd = length - valueSize;

	if (valueSize == 1) {
		// reverse the data
		for (auto i = 0; i <= (bufferEnd / 2); i++) {
			auto temp = data[i];
			data[i] = data[bufferEnd - i];
			data[bufferEnd - i] = temp;
		}
	} else {
		// reverse the data in chunks
		for (auto i = 0; i <= (bufferEnd / (valueSize * 2)); i++) {
			auto sourceOffset = i * valueSize;
			auto targetOffset = bufferEnd - sourceOffset;
			for (auto j = 0; j < valueSize; j++) {
				auto temp = data[sourceOffset + j];
				data[sourceOffset + j] = data[targetOffset + j];
				data[targetOffset + j] = temp;
			}
		}
	}
}

// Work out which buffer to use next
// Returns whether to continue iterating, which the caller uses to decide whether to clear buffers
bool updateTarget(tcb::span<uint16_t> targets, tcb::span<uint16_t>::iterator &targetIter, bool iterate) {
	if (iterate) {
		// if the iterate flag is set, we just use the next buffer
		auto &targetId = *targetIter;
		if (targetId >= 65534) {
			// in future iterations, loop over the single buffer in the list and don't clear it
			return false;
		}
		// update the list in-place
		targetId++;
		return true;
	} else {
		// use the next buffer in the list, or loop back to the start
		targetIter++;
		if (targetIter == targets.end()) {
			targetIter = targets.begin();
		}
		return false;
	}
}

// consolidate blocks/streams into a single buffer
std::shared_ptr<BufferStream> consolidateBuffers(const std::vector<std::shared_ptr<BufferStream>> &streams) {
	// don't do anything if only one stream
	if (streams.size() == 1) {
		return streams.front();
	}
	// work out total length of buffer
	uint32_t length = 0;
	for (auto &block : streams) {
		length += block->size();
	}
	auto bufferStream = make_shared_psram<BufferStream>(length);
	if (!bufferStream || !bufferStream->getBuffer()) {
		// buffer couldn't be created
		return nullptr;
	}
	auto destination = bufferStream->getBuffer();
	for (auto &block : streams) {
		auto bufferLength = block->size();
		memcpy(destination, block->getBuffer(), bufferLength);
		destination += bufferLength;
	}
	return bufferStream;
}

// split a buffer into multiple blocks/chunks
std::vector<std::shared_ptr<BufferStream>> splitBuffer(std::shared_ptr<BufferStream> buffer, uint16_t length) {
	std::vector<std::shared_ptr<BufferStream>> chunks;
	auto totalLength = buffer->size();
	auto remaining = totalLength;
	auto sourceData = buffer->getBuffer();

	// chop up source data by length, storing into new buffers
	// looping the buffer list until we have no data left
	while (remaining > 0) {
		auto bufferLength = length;
		if (remaining < bufferLength) {
			bufferLength = remaining;
		}
		auto chunk = make_shared_psram<BufferStream>(bufferLength);
		if (!chunk || !chunk->getBuffer()) {
			// buffer couldn't be created, so return an empty vector
			chunks.clear();
			break;
		}
		memcpy(chunk->getBuffer(), sourceData, bufferLength);
		chunks.push_back(std::move(chunk));
		sourceData += bufferLength;
		remaining -= bufferLength;
	}
	return chunks;
}

#endif // BUFFERS_H

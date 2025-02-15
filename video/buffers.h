#ifndef BUFFERS_H
#define BUFFERS_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mat.h>

#include "agon.h"
#include "buffer_stream.h"
#include "span.h"
#include "types.h"

using BufferVector = std::vector<std::shared_ptr<BufferStream>, psram_allocator<std::shared_ptr<BufferStream>>>;
std::unordered_map<uint16_t, BufferVector, std::hash<uint16_t>, std::equal_to<uint16_t>, psram_allocator<std::pair<const uint16_t, BufferVector>>> buffers;
std::unordered_map<uint16_t, std::unordered_set<uint16_t>> callbackBuffers;

struct AdvancedOffset {
	uint32_t blockOffset = 0;
	size_t blockIndex = 0;
};

typedef union {
	struct {
		uint8_t rows : 4;
		uint8_t columns : 4;
	};
	uint8_t value = 0;
	uint8_t size() const {
		return rows * columns;
	}
	uint8_t rowSizeBytes() const {
		return columns * sizeof(float);
	}
	uint8_t sizeBytes() const {
		return size() * sizeof(float);
	}
} MatrixSize;

std::unordered_map<uint16_t, MatrixSize, std::hash<uint16_t>, std::equal_to<uint16_t>, psram_allocator<std::pair<const uint16_t, MatrixSize>>> matrixMetadata;

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
std::shared_ptr<BufferStream> consolidateBuffers(const BufferVector &streams) {
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
BufferVector splitBuffer(std::shared_ptr<BufferStream> buffer, uint16_t length) {
	BufferVector chunks;
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

// check buffer looks like a transform, and ensure there's an inverse
bool checkTransformBuffer(BufferVector &transformBuffer) {
	int const matrixSize = sizeof(float) * 9;

	if (transformBuffer.size() == 1) {
		// make sure we have an inverse matrix cached
		if (transformBuffer[0]->size() < matrixSize) {
			return false;
		}
		// create an inverse matrix, and push that to the buffer
		auto transform = (float *)transformBuffer[0]->getBuffer();
		auto matrix = dspm::Mat(transform, 3, 3).inverse();
		auto bufferStream = make_shared_psram<BufferStream>(matrixSize);
		bufferStream->writeBuffer((uint8_t *)matrix.data, matrixSize);
		transformBuffer.push_back(bufferStream);
	}

	if (transformBuffer[0]->size() < matrixSize || transformBuffer[1]->size() < matrixSize) {
		return false;
	}

	return true;
}

void extractFormatInfo(uint8_t format, bool &isFixed, bool &is16Bit, int8_t &shift) {
	isFixed = format & FLOAT_FORMAT_FIXED;
	is16Bit = format & FLOAT_FORMAT_16BIT;
	shift = format & FLOAT_FORMAT_SHIFT_MASK;
	// ensure our size value obeys negation
	if (is16Bit && (shift & FLOAT_FORMAT_SHIFT_TOPBIT)) {
		// top bit was set, so it's a negative - so we need to set the top bits of the size
		shift = shift | FLOAT_FORMAT_FLAGS;
	}
};

// Get the longest contiguous span at the given buffer offset. Updates the offset to the correct block index.
// accepts a size to dictate the minimum span size, and will align offset if block didn't contain the required size of data
tcb::span<uint8_t> getBufferSpan(const BufferVector &buffer, AdvancedOffset &offset, uint8_t size = 1) {
	while (offset.blockIndex < buffer.size()) {
		// check for available bytes in the current block
		auto &block = buffer[offset.blockIndex];
		if ((offset.blockOffset + size) <= block->size()) {
			return { block->getBuffer() + offset.blockOffset, block->size() - offset.blockOffset };
		}
		// if offset exceeds the block size, loop to find the correct block
		// ensuring our offset doesn't go below zero in the event that we didn't have enough data in the block
		offset.blockOffset = std::max<int32_t>(0, offset.blockOffset - block->size());
		offset.blockIndex++;
	}
	// offset not found in buffer
	return {};
}

tcb::span<uint8_t> getBufferSpan(const uint16_t bufferId, AdvancedOffset &offset, uint8_t size = 1) {
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {	// buffer not found
		return {};
	}
	return getBufferSpan(bufferIter->second, offset, size);
}

// Utility call to read a byte from a buffer at the given offset
int16_t getBufferByte(const BufferVector &buffer, AdvancedOffset &offset, bool iterate = false) {
	auto bufferSpan = getBufferSpan(buffer, offset);
	if (bufferSpan.empty()) {	// offset not found in buffer
		return -1;
	}
	if (iterate) {
		offset.blockOffset++;
	}
	return bufferSpan.front();
}

// utility call to read multiple bytes from a buffer at a given offset
bool readBufferBytes(const uint16_t bufferId, AdvancedOffset &offset, void *target, uint16_t size, bool iterate = false) {
	auto bufferSpan = getBufferSpan(bufferId, offset, size);
	if (bufferSpan.empty()) {	// offset not found in buffer
		return false;
	}
	memcpy(target, bufferSpan.data(), size);
	if (iterate) {
		offset.blockOffset += size;
	}
	return true;
}

// Utility call to read a float from a buffer at the given offset, in the given format
// NB float must not span over buffer block boundaries
float readBufferFloat(uint32_t sourceBufferId, AdvancedOffset &offset, bool is16Bit, bool isFixed, int8_t shift, bool iterate = false) {
	uint32_t rawValue = 0;
	if (!readBufferBytes(sourceBufferId, offset, &rawValue, is16Bit ? 2 : 4, iterate)) {
		// failed to read value from buffer
		return INFINITY;
	}
	return convertValueToFloat(rawValue, is16Bit, isFixed, shift);
};

// Utility call to set a byte in a buffer at the given offset
bool setBufferByte(uint8_t value, const BufferVector &buffer, AdvancedOffset &offset, bool iterate = false) {
	auto bufferSpan = getBufferSpan(buffer, offset);
	if (bufferSpan.empty()) {
		// offset not found in buffer
		return false;
	}
	bufferSpan.front() = value;
	if (iterate) {
		offset.blockOffset++;
	}
	return true;
}

MatrixSize getMatrixSize(uint16_t bufferId) {
	auto sizeIter = matrixMetadata.find(bufferId);
	if (sizeIter == matrixMetadata.end()) {
		return {};
	}
	return sizeIter->second;
}

bool getMatrixFromBuffer(uint16_t bufferId, float* matrix, MatrixSize size, bool allowSubmatrix = true) {
	// Does the buffer contain the matrix?
	auto sourceSize = getMatrixSize(bufferId);
	if (sourceSize.size() == 0) {
		// No size information for this buffer
		return false;
	}
	if (buffers.find(bufferId) == buffers.end()) {
		// This check should be unnecessary, but it's here for safety
		return false;
	}

	if (sourceSize.rows == size.rows && sourceSize.columns == size.columns) {
		// The matrix is the correct size, so we can copy it directly
		AdvancedOffset offset = {};
		return readBufferBytes(bufferId, offset, matrix, size.sizeBytes());
	}

	if (!allowSubmatrix) {
		// Matrix sizes don't match, and we're not allowed to grab a submatrix
		return false;
	}
	// Matrix sizes don't match, so we will grab a submatrix
	// Assumption is that matrix pointer is pre-filled as appropriate (probably with an identity matrix, or zeros)

	auto rows = std::min(sourceSize.rows, size.rows);
	auto columns = std::min(sourceSize.columns, size.columns);
	for (auto i = 0; i < rows; i++) {
		AdvancedOffset offset = {};
		offset.blockOffset = i * sourceSize.rowSizeBytes();
		for (auto j = 0; j < columns; j++) {
			auto value = readBufferFloat(bufferId, offset, false, false, 0, true);
			if (value == INFINITY) {
				// failed to read value from buffer
				return false;
			}
			matrix[i * size.columns + j] = value;
		}
	}

	return true;
}

#endif // BUFFERS_H

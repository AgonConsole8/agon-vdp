#ifndef MULTI_BUFFER_STREAM_H
#define MULTI_BUFFER_STREAM_H

#include <memory>
#include <vector>
#include <Stream.h>

#include "buffer_stream.h"
#include "types.h"

class MultiBufferStream : public Stream {
	public:
		MultiBufferStream(std::vector<std::shared_ptr<BufferStream>> buffers);
		int available();
		int read();
		int peek();
		virtual size_t readBytes(char * outBuffer, size_t length); // read chars from stream into buffer
		virtual size_t readBytes(uint8_t * outBuffer, size_t length) {
			return readBytes((char *)outBuffer, length);
		}
		size_t write(uint8_t b);
		void rewind(size_t bufferIndex = 0);
		void seekTo(uint32_t position, size_t bufferIndex = 0);
		uint32_t size();
		const std::vector<std::shared_ptr<BufferStream>> &tellBuffer(uint32_t &blockOffset, size_t &blockIndex);
	private:
		std::vector<std::shared_ptr<BufferStream>> buffers;
		BufferStream * getBuffer();
		size_t currentBufferIndex = 0;
};

MultiBufferStream::MultiBufferStream(std::vector<std::shared_ptr<BufferStream>> buffers) : buffers(std::move(buffers)) {
	// rewind to the start of the first buffer
	rewind();
}

int MultiBufferStream::available() {
	auto buffer = getBuffer();
	if (buffer) {
		return buffer->available();
	}
	return 0;
}

int MultiBufferStream::read() {
	auto buffer = getBuffer();
	return buffer->read();
}

int MultiBufferStream::peek() {
	auto buffer = getBuffer();
	return buffer->peek();
}

size_t MultiBufferStream::readBytes(char * outBuffer, size_t length) {
	size_t readAmount = 0;
	while (readAmount < length) {
		auto buffer = getBuffer();
		if (!buffer) {
			break;
		}
		readAmount += buffer->readBytes(outBuffer + readAmount, length - readAmount);
	}
	return readAmount;
}

size_t MultiBufferStream::write(uint8_t b) {
	// write is not supported
	return 0;
}

void MultiBufferStream::rewind(size_t bufferIndex) {
	currentBufferIndex = bufferIndex;
	if (currentBufferIndex < buffers.size()) {
		buffers[currentBufferIndex]->rewind();
	}
}

void MultiBufferStream::seekTo(uint32_t position, size_t bufferIndex) {
	// find the buffer that contains the position we want
	// keeping track of an offset into the whole buffer
	auto offset = position;
	for (auto i = bufferIndex; i < buffers.size(); i++) {
		auto &stream = buffers[i];
		auto bufferSize = stream->size();
		if (offset < bufferSize) {
			// this is the buffer we want
			currentBufferIndex = i;
			stream->seekTo(offset);
			return;
		}
		// reduce the offset by the size of this buffer, as position wasn't there
		offset -= bufferSize;
	}

	// if we get here, we've gone past the end of the buffers
	// so just seek past the end of the last buffer
	currentBufferIndex = buffers.size();
}

uint32_t MultiBufferStream::size() {
	uint32_t totalSize = 0;
	for (auto &buffer : buffers) {
		totalSize += buffer->size();
	}
	return totalSize;
}

const std::vector<std::shared_ptr<BufferStream>> &MultiBufferStream::tellBuffer(uint32_t &blockOffset, size_t &blockIndex) {
	auto buffer = getBuffer();
	blockOffset = buffer ? buffer->tell() : 0;
	blockIndex = currentBufferIndex;
	return buffers;
}

inline BufferStream * MultiBufferStream::getBuffer() {
	while (currentBufferIndex < buffers.size() && !buffers[currentBufferIndex]->available()) {
		rewind(currentBufferIndex + 1);
	}
	if (currentBufferIndex >= buffers.size()) {
		return nullptr;
	}
	return buffers[currentBufferIndex].get();
}

#endif // MULTI_BUFFER_STREAM_H

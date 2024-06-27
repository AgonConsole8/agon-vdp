#ifndef VDU_BUFFERED_H
#define VDU_BUFFERED_H

#include <algorithm>
#include <memory>
#include <vector>
#include <unordered_map>
#include <esp_heap_caps.h>
#include <mat.h>
#include <dspm_mult.h>

#include "agon.h"
#include "agon_fonts.h"
#include "buffers.h"
#include "buffer_stream.h"
#include "compression.h"
#include "mem_helpers.h"
#include "multi_buffer_stream.h"
#include "sprites.h"
#include "test_flags.h"
#include "types.h"
#include "vdu_stream_processor.h"
#include "pingo_3d.h"

// VDU 23, 0, &A0, bufferId; command: Buffered command support
//
void IRAM_ATTR VDUStreamProcessor::vdu_sys_buffered() {
	auto bufferId = readWord_t(); if (bufferId == -1) return;
	auto command = readByte_t(); if (command == -1) return;

	switch (command) {
		case BUFFERED_WRITE: {
			auto length = readWord_t(); if (length == -1) return;
			bufferWrite(bufferId, length);
		}	break;
		case BUFFERED_CALL: {
			bufferCall(bufferId, {});
		}	break;
		case BUFFERED_CLEAR: {
			bufferClear(bufferId);
		}	break;
		case BUFFERED_CREATE: {
			auto size = readWord_t(); if (size == -1) return;
			auto buffer = bufferCreate(bufferId, size);	
			if (buffer) {
				// Ensure buffer is empty
				memset(buffer->getBuffer(), 0, size);
			}
		}	break;
		case BUFFERED_SET_OUTPUT: {
			setOutputStream(bufferId);
		}	break;
		case BUFFERED_ADJUST: {
			bufferAdjust(bufferId);
		}	break;
		case BUFFERED_COND_CALL: {
			// VDU 23, 0, &A0, bufferId; 6, <conditional arguments>  : Conditional call
			if (bufferConditional()) {
				bufferCall(bufferId, {});
			}
		}	break;
		case BUFFERED_JUMP: {
			// VDU 23, 0, &A0, bufferId; 7: Jump to buffer
			// a "jump" (without an offset) to buffer 65535 (-1) indicates a "jump to end"
			AdvancedOffset offset = {};
			offset.blockIndex = bufferId == 65535 ? -1 : 0;
			bufferJump(bufferId, offset);
		}	break;
		case BUFFERED_COND_JUMP: {
			// VDU 23, 0, &A0, bufferId; 8, <conditional arguments>  : Conditional jump
			if (bufferConditional()) {
				// ensure offset-less jump to buffer 65535 (-1) is treated as a "jump to end"
				AdvancedOffset offset = {};
				offset.blockIndex = bufferId == 65535 ? -1 : 0;
				bufferJump(bufferId, offset);
			}
		}	break;
		case BUFFERED_OFFSET_JUMP: {
			// VDU 23, 0, &A0, bufferId; 9, offset; offsetHighByte  : Offset jump
			auto offset = getOffsetFromStream(true); if (offset.blockOffset == -1) return;
			bufferJump(bufferId, offset);
		}	break;
		case BUFFERED_OFFSET_COND_JUMP: {
			// VDU 23, 0, &A0, bufferId; &0A, offset; offsetHighByte, <conditional arguments>  : Conditional jump with offset
			auto offset = getOffsetFromStream(true); if (offset.blockOffset == -1) return;
			if (bufferConditional()) {
				bufferJump(bufferId, offset);
			}
		}	break;
		case BUFFERED_OFFSET_CALL: {
			// VDU 23, 0, &A0, bufferId; &0B, offset; offsetHighByte  : Offset call
			auto offset = getOffsetFromStream(true); if (offset.blockOffset == -1) return;
			bufferCall(bufferId, offset);
		}	break;
		case BUFFERED_OFFSET_COND_CALL: {
			// VDU 23, 0, &A0, bufferId; &0C, offset; offsetHighByte, <conditional arguments>  : Conditional call with offset
			auto offset = getOffsetFromStream(true); if (offset.blockOffset == -1) return;
			if (bufferConditional()) {
				bufferCall(bufferId, offset);
			}
		}	break;
		case BUFFERED_COPY: {
			// read list of source buffer IDs
			auto sourceBufferIds = getBufferIdsFromStream();
			if (sourceBufferIds.empty()) {
				debug_log("vdu_sys_buffered: no source buffer IDs\n\r");
				return;
			}
			bufferCopy(bufferId, sourceBufferIds);
		}	break;
		case BUFFERED_CONSOLIDATE: {
			bufferConsolidate(bufferId);
		}	break;
		case BUFFERED_SPLIT: {
			auto length = readWord_t(); if (length == -1) return;
			uint16_t target[] = { (uint16_t)bufferId };
			bufferSplitInto(bufferId, length, target, false);
		}	break;
		case BUFFERED_SPLIT_INTO: {
			auto length = readWord_t(); if (length == -1) return;
			auto targetBufferIds = getBufferIdsFromStream();
			if (targetBufferIds.empty()) {
				debug_log("vdu_sys_buffered: no target buffer IDs\n\r");
				return;
			}
			bufferSplitInto(bufferId, length, targetBufferIds, false);
		}	break;
		case BUFFERED_SPLIT_FROM: {
			auto length = readWord_t(); if (length == -1) return;
			auto targetStart = readWord_t(); if (targetStart == -1 || targetStart == 65535) return;
			uint16_t target[] = { (uint16_t)targetStart };
			bufferSplitInto(bufferId, length, target, true);
		}	break;
		case BUFFERED_SPLIT_BY: {
			auto width = readWord_t(); if (width == -1) return;
			auto chunks = readWord_t(); if (chunks == -1) return;
			uint16_t target[] = { (uint16_t)bufferId };
			bufferSplitByInto(bufferId, width, chunks, target, false);
		}	break;
		case BUFFERED_SPLIT_BY_INTO: {
			auto width = readWord_t(); if (width == -1) return;
			auto targetBufferIds = getBufferIdsFromStream();
			auto chunks = targetBufferIds.size();
			if (chunks == 0) {
				debug_log("vdu_sys_buffered: no target buffer IDs\n\r");
				return;
			}
			bufferSplitByInto(bufferId, width, chunks, targetBufferIds, false);
		}	break;
		case BUFFERED_SPLIT_BY_FROM: {
			auto width = readWord_t(); if (width == -1) return;
			auto chunks = readWord_t(); if (chunks == -1) return;
			auto targetStart = readWord_t(); if (targetStart == -1 || targetStart == 65535) return;
			uint16_t target[] = { (uint16_t)targetStart };
			bufferSplitByInto(bufferId, width, chunks, target, true);
		}	break;
		case BUFFERED_SPREAD_INTO: {
			auto targetBufferIds = getBufferIdsFromStream();
			if (targetBufferIds.empty()) {
				debug_log("vdu_sys_buffered: no target buffer IDs\n\r");
				return;
			}
			bufferSpreadInto(bufferId, targetBufferIds, false);
		}	break;
		case BUFFERED_SPREAD_FROM: {
			auto targetStart = readWord_t(); if (targetStart == -1 || targetStart == 65535) return;
			uint16_t target[] = { (uint16_t)targetStart };
			bufferSpreadInto(bufferId, target, true);
		}	break;
		case BUFFERED_REVERSE_BLOCKS: {
			bufferReverseBlocks(bufferId);
		}	break;
		case BUFFERED_REVERSE: {
			auto options = readByte_t(); if (options == -1) return;
			bufferReverse(bufferId, options);
		}	break;
		case BUFFERED_COPY_REF: {
			// read list of source buffer IDs
			auto sourceBufferIds = getBufferIdsFromStream();
			if (sourceBufferIds.empty()) {
				debug_log("vdu_sys_buffered: no source buffer IDs\n\r");
				return;
			}
			bufferCopyRef(bufferId, sourceBufferIds);
		}	break;
		case BUFFERED_COPY_AND_CONSOLIDATE: {
			// read list of source buffer IDs
			auto sourceBufferIds = getBufferIdsFromStream();
			if (sourceBufferIds.empty()) {
				debug_log("vdu_sys_buffered: no source buffer IDs\n\r");
				return;
			}
			bufferCopyAndConsolidate(bufferId, sourceBufferIds);
		}	break;
		case BUFFERED_AFFINE_TRANSFORM: {
			if (isTestFlagSet(TEST_FLAG_AFFINE_TRANSFORM)) {
				bufferAffineTransform(bufferId);
			}
		}	break;
		case BUFFERED_COMPRESS: {
			auto sourceBufferId = readWord_t();
			if (sourceBufferId == -1) return;
			bufferCompress(bufferId, sourceBufferId);
		}	break;
		case BUFFERED_DECOMPRESS: {
			auto sourceBufferId = readWord_t();
			if (sourceBufferId == -1) return;
			bufferDecompress(bufferId, sourceBufferId);
		}	break;
		case BUFFERED_EXPAND_BITMAP: {
			auto options = readByte_t(); if (options == -1) return;
			auto sourceBufferId = readWord_t();
			if (sourceBufferId == -1) return;
			bufferExpandBitmap(bufferId, options, sourceBufferId);
		}	break;
        case BUFFERED_PINGO_3D: {
			bufferUsePingo3D(bufferId);
        }   break;
		case BUFFERED_DEBUG_INFO: {
			// force_debug_log("vdu_sys_buffered: debug info stack highwater %d\n\r",uxTaskGetStackHighWaterMark(nullptr));
			debug_log("vdu_sys_buffered: buffer %d, %d streams stored\n\r", bufferId, buffers[bufferId].size());
			if (buffers[bufferId].empty()) {
				return;
			}
			// output contents of buffer stream 0
			auto buffer = buffers[bufferId][0];
			auto bufferLength = buffer->size();
			for (auto i = 0; i < bufferLength; i++) {
				auto data = buffer->getBuffer()[i];
				debug_log("%02X ", data);
			}
			debug_log("\n\r");
		}	break;
		default: {
			debug_log("vdu_sys_buffered: unknown command %d, buffer %d\n\r", command, bufferId);
		}	break;
	}
}

// VDU 23, 0, &A0, bufferId; 0, length; data...: store stream into buffer
// This adds a new stream to the given bufferId
// allowing a single bufferId to store multiple streams of data
//
uint32_t VDUStreamProcessor::bufferWrite(uint16_t bufferId, uint32_t length) {
	auto bufferStream = make_shared_psram<BufferStream>(length);

	debug_log("bufferWrite: storing stream into buffer %d, length %d\n\r", bufferId, length);

	auto remaining = readIntoBuffer(bufferStream->getBuffer(), length);
	if (remaining > 0) {
		// NB this discards the data we just read
		debug_log("bufferWrite: timed out write for buffer %d (%d bytes remaining)\n\r", bufferId, remaining);
		return remaining;
	}

	if (bufferId == 65535) {
		// buffer ID of -1 (65535) reserved so we don't store it
		debug_log("bufferWrite: ignoring buffer 65535\n\r");
		return remaining;
	}

	buffers[bufferId].push_back(std::move(bufferStream));
	debug_log("bufferWrite: stored stream in buffer %d, length %d, %d streams stored\n\r", bufferId, length, buffers[bufferId].size());
	return remaining;
}

// VDU 23, 0, &A0, bufferId; 1: Call buffer
// VDU 23, 0, &A0, bufferId; &0B, offset; offsetHighByte  : Offset call
// Processes all commands from the streams stored against the given bufferId
//
void VDUStreamProcessor::bufferCall(uint16_t callBufferId, AdvancedOffset offset) {
	debug_log("bufferCall: buffer %d\n\r", callBufferId);
	auto bufferId = resolveBufferId(callBufferId, id);
	if (bufferId == -1) {
		debug_log("bufferCall: no buffer ID\n\r");
		return;
	}
	AdvancedOffset returnOffset;
	if (id != 65535) {
		if (inputStream->available() == 0) {
			// tail-call optimise - turn the call into a jump
			bufferJump(bufferId, offset);
			return;
		}
		// get the return offset before doing any BufferStream operations
		auto multiBufferStream = (MultiBufferStream *)inputStream.get();
		multiBufferStream->tellBuffer(returnOffset.blockOffset, returnOffset.blockIndex);
		if (id == bufferId) {
			// calling ourselves, just seek to the old offset after returning
			multiBufferStream->seekTo(offset.blockOffset, offset.blockIndex);
			processAllAvailable();
			multiBufferStream->seekTo(returnOffset.blockOffset, returnOffset.blockIndex);
			return;
		}
	}
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferCall: buffer %d not found\n\r", bufferId);
		return;
	}
	auto &streams = bufferIter->second;
	std::shared_ptr<Stream> callInputStream = make_shared_psram<MultiBufferStream>(streams);
	if (offset.blockOffset != 0 || offset.blockIndex != 0) {
		auto multiBufferStream = (MultiBufferStream *)callInputStream.get();
		multiBufferStream->seekTo(offset.blockOffset, offset.blockIndex);
	}
	// use the current VDUStreamProcessor, swapping out the stream
	std::swap(id, callBufferId);
	std::swap(inputStream, callInputStream);
	processAllAvailable();
	// restore the original buffer id and stream
	id = callBufferId;
	inputStream = std::move(callInputStream);
	if (id != 65535) {
		// return to the appropriate offset
		auto multiBufferStream = (MultiBufferStream *)inputStream.get();
		multiBufferStream->seekTo(returnOffset.blockOffset, returnOffset.blockIndex);
	}
}

void VDUStreamProcessor::bufferRemoveUsers(uint16_t bufferId) {
	// remove all users of the given buffer
	context->unmapBitmapFromChars(bufferId);
	clearBitmap(bufferId);
	clearFont(bufferId);
	clearSample(bufferId);
}

// VDU 23, 0, &A0, bufferId; 2: Clear buffer
// Removes all streams stored against the given bufferId
// sending a bufferId of 65535 (i.e. -1) clears all buffers
//
void VDUStreamProcessor::bufferClear(uint16_t bufferId) {
	debug_log("bufferClear: buffer %d\n\r", bufferId);
	if (bufferId == 65535) {
		buffers.clear();
		resetBitmaps();
		// TODO reset current bitmaps in all processors
		context->setCurrentBitmap(BUFFERED_BITMAP_BASEID);
		context->resetCharToBitmap();
		resetFonts();
		resetSamples();
		return;
	}
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferClear: buffer %d not found\n\r", bufferId);
		return;
	}
	buffers.erase(bufferIter);
	bufferRemoveUsers(bufferId);
	debug_log("bufferClear: cleared buffer %d\n\r", bufferId);
}

// VDU 23, 0, &A0, bufferId; 3, size; : Create a writeable buffer
// This is used for creating buffers to redirect output to
//
std::shared_ptr<WritableBufferStream> VDUStreamProcessor::bufferCreate(uint16_t bufferId, uint32_t size) {
	if (bufferId == 65535) {
		debug_log("bufferCreate: bufferId %d is reserved\n\r", bufferId);
		return nullptr;
	}
	if (buffers.find(bufferId) != buffers.end()) {
		debug_log("bufferCreate: buffer %d already exists\n\r", bufferId);
		return nullptr;
	}
	auto buffer = make_shared_psram<WritableBufferStream>(size);
	if (!buffer) {
		debug_log("bufferCreate: failed to create buffer %d\n\r", bufferId);
		return nullptr;
	}
	buffers[bufferId].push_back(buffer);
	debug_log("bufferCreate: created buffer %d, size %d\n\r", bufferId, size);
	return buffer;
}

// VDU 23, 0, &A0, bufferId; 4: Set output to buffer
// use an ID of -1 (65535) to clear the output buffer (no output)
// use an ID of 0 to reset the output buffer to it's original value
//
// TODO add a variant/command to adjust offset inside output stream
void VDUStreamProcessor::setOutputStream(uint16_t bufferId) {
	if (bufferId == 65535) {
		outputStream = nullptr;
		return;
	}
	// bufferId of 0 resets output buffer to it's original value
	// which will usually be the z80 serial port
	if (bufferId == 0) {
		outputStream = originalOutputStream;
		return;
	}
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end() || bufferIter->second.empty()) {
		debug_log("setOutputStream: buffer %d not found\n\r", bufferId);
		return;
	}
	auto &output = bufferIter->second.front();
	if (output->isWritable()) {
		outputStream = output;
	} else {
		debug_log("setOutputStream: buffer %d is not writable\n\r", bufferId);
	}
}

// Utility call to read offset from stream, supporting advanced offsets
VDUStreamProcessor::AdvancedOffset VDUStreamProcessor::getOffsetFromStream(bool isAdvanced) {
	AdvancedOffset offset = {};
	if (isAdvanced) {
		offset.blockOffset = read24_t();
		if (offset.blockOffset != -1 && offset.blockOffset & 0x00800000) {
			// top bit of 24-bit offset is set, so we have a block index too
			auto blockIndex = readWord_t();
			if (blockIndex == -1) {
				offset.blockOffset = -1;
			} else {
				offset.blockOffset &= 0x007FFFFF;
				offset.blockIndex = blockIndex;
			}
		}
	} else {
		offset.blockOffset = readWord_t();
	}
	return offset;
}

// Utility call to read a sequence of buffer IDs from the stream
std::vector<uint16_t> VDUStreamProcessor::getBufferIdsFromStream() {
	// read buffer IDs until we get a 65535 (end of list) or a timeout
	std::vector<uint16_t> bufferIds;
	uint32_t bufferId;
	bool looping = true;
	while (looping) {
		bufferId = readWord_t();
		looping = (bufferId != 65535 && bufferId != -1);
		if (looping) {
			bufferIds.push_back(bufferId);
		}
	}
	if (bufferId == -1) {
		// timeout
		bufferIds.clear();
	}

	return bufferIds;
}

// Get the longest contiguous span at the given buffer offset. Updates the offset to the correct block index.
tcb::span<uint8_t> VDUStreamProcessor::getBufferSpan(const std::vector<std::shared_ptr<BufferStream>> &buffer, AdvancedOffset &offset) {
	while (offset.blockIndex < buffer.size()) {
		// check for available bytes in the current block
		auto &block = buffer[offset.blockIndex];
		if (offset.blockOffset < block->size()) {
			return { block->getBuffer() + offset.blockOffset, block->size() - offset.blockOffset };
		}
		// if offset exceeds the block size, loop to find the correct block
		offset.blockOffset -= block->size();
		offset.blockIndex++;
	}
	// offset not found in buffer
	return {};
}

// Utility call to read a byte from a buffer at the given offset
int16_t VDUStreamProcessor::getBufferByte(const std::vector<std::shared_ptr<BufferStream>> &buffer, AdvancedOffset &offset, bool iterate) {
	auto bufferSpan = getBufferSpan(buffer, offset);
	if (bufferSpan.empty()) {
		// offset not found in buffer
		return -1;
	}
	auto value = bufferSpan.front();
	if (iterate) {
		offset.blockOffset++;
	}
	return value;
}

// Utility call to set a byte in a buffer at the given offset
bool VDUStreamProcessor::setBufferByte(uint8_t value, const std::vector<std::shared_ptr<BufferStream>> &buffer, AdvancedOffset &offset, bool iterate) {
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

// Utility classes for specializing buffer adjust operations
// AdjustSingle must be specialized for every operation type
// The other classes provide default implementations using AdjustSingle,
// and may be specialized for specific operation types.
template<uint8_t Operator>
struct AdjustSingle {
	// Default implementation for unimplemented operations
	static inline uint_fast8_t adjust(uint_fast8_t target, uint_fast8_t, bool &) {
		return target;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t target, uint_fast16_t, bool &) {
		return target;
	}
	static inline uint32_t adjustWord(uint32_t target, uint32_t, bool&) {
		return target;
	}
};
template<>
struct AdjustSingle<ADJUST_NOT> {
	static inline uint_fast8_t adjust(uint_fast8_t target, uint_fast8_t, bool &) {
		return ~target;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t target, uint_fast16_t, bool &) {
		return ~target;
	}
	static inline uint32_t adjustWord(uint32_t target, uint32_t, bool &) {
		return ~target;
	}
};
template<>
struct AdjustSingle<ADJUST_NEG> {
	static inline uint_fast8_t adjust(uint_fast8_t target, uint_fast8_t, bool &) {
		return -target;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t target, uint_fast16_t, bool &) {
		static constexpr uint_fast16_t CARRY_MASK = 0x100;
		uint_fast16_t result = -target;
		return result + ((result ^ target) & CARRY_MASK);
	}
	static inline uint32_t adjustWord(uint32_t target, uint32_t, bool &) {
		static constexpr uint32_t SIGN_MASK = 0x7F7F7F7F;
		return ((target & SIGN_MASK) + SIGN_MASK) ^ (target | SIGN_MASK);
	}
};
template<>
struct AdjustSingle<ADJUST_SET> {
	static inline uint_fast8_t adjust(uint_fast8_t, uint_fast8_t operand, bool &) {
		return operand;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t, uint_fast16_t operand, bool &) {
		return operand;
	}
	static inline uint32_t adjustWord(uint32_t, uint32_t operand, bool &) {
		return operand;
	}
};
template<>
struct AdjustSingle<ADJUST_ADD> {
	// byte-wise add - no carry, so bytes may overflow
	static inline uint_fast8_t adjust(uint_fast8_t target, uint_fast8_t operand, bool &) {
		return target + operand;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t target, uint_fast16_t operand, bool &) {
		static constexpr uint_fast16_t CARRY_MASK = 0x100;
		uint_fast16_t result = target + operand;
		return result - ((result ^ target ^ operand) & CARRY_MASK);
	}
	static inline uint32_t adjustWord(uint32_t target, uint32_t operand, bool &) {
		static constexpr uint32_t SIGN_MASK = 0x7F7F7F7F;
		return ((target & SIGN_MASK) + (operand & SIGN_MASK)) ^ ((target ^ operand) & ~SIGN_MASK);
	}
	static inline uint_fast8_t fold(uint32_t accumulator) {
		static constexpr uint32_t BYTE_MASK = 0x00FF00FF;
		accumulator = (accumulator & BYTE_MASK) + ((accumulator >> 8) & BYTE_MASK);
		return accumulator + (accumulator >> 16);
	}
};
template<>
struct AdjustSingle<ADJUST_ADD_CARRY> {
	// byte-wise add with carry
	// bytes are treated as being in little-endian order
	static inline uint_fast8_t adjust(uint_fast8_t target, uint_fast8_t operand, bool &carry) {
		uint_fast16_t sum = (uint_fast16_t)(uint8_t)target + (uint8_t)operand + carry;
		carry = sum >> 8;
		return sum;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t target, uint_fast16_t operand, bool &carry) {
		// convert from little-endian to native and back
		uint32_t sum = (uint32_t)from_le16(target) + from_le16(operand) + carry;
		carry = sum >> 16;
		return to_le16(sum);
	}
	static inline uint32_t adjustWord(uint32_t target, uint32_t operand, bool &carry) {
		// convert from little-endian to native and back
		target = from_le32(target);
		operand = from_le32(operand);
#if defined(__XTENSA__) && XCHAL_HAVE_MINMAX
		operand += carry;
		target += operand;
		// Do branchless carry handling
		uint32_t min; //= std::min(target, operand);
		// Use assembly so the compiler can't reverse the min operation back into a branchy less-than
		asm("minu\t%0, %1, %2"
		  : "=r" (min)
		  : "r" (target), "r" (operand));
		// The following should compile to conditional moves
		// Intentionally preserves input carry if carry-adjusted operand is zero
		if (operand != 0) carry = false; // Initialize carry to false if carry-adjusted operand is non-zero
		if (min != operand) carry = true; // Set carry to true if sum < carry-adjusted operand
#elif __has_builtin(__builtin_addc)
		unsigned int carry_out;
		target = __builtin_addc(target, operand, carry, &carry_out);
		carry = carry_out;
#else
		uint64_t sum = (uint64_t)target + operand + carry;
		carry = sum >> 32;
		target = sum;
#endif
		return to_le32(target);
	}
};
template<>
struct AdjustSingle<ADJUST_AND> {
	static inline uint_fast8_t adjust(uint_fast8_t target, uint_fast8_t operand, bool &) {
		return target & operand;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t target, uint_fast16_t operand, bool &) {
		return target & operand;
	}
	static inline uint32_t adjustWord(uint32_t target, uint32_t operand, bool &) {
		return target & operand;
	}
	static inline uint_fast8_t fold(uint32_t accumulator) {
		accumulator &= accumulator >> 16;
		accumulator &= accumulator >> 8;
		return accumulator;
	}
};
template<>
struct AdjustSingle<ADJUST_OR> {
	static inline uint_fast8_t adjust(uint_fast8_t target, uint_fast8_t operand, bool &) {
		return target | operand;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t target, uint_fast16_t operand, bool &) {
		return target | operand;
	}
	static inline uint32_t adjustWord(uint32_t target, uint32_t operand, bool &) {
		return target | operand;
	}
	static inline uint_fast8_t fold(uint32_t accumulator) {
		accumulator |= accumulator >> 16;
		accumulator |= accumulator >> 8;
		return accumulator;
	}
};
template<>
struct AdjustSingle<ADJUST_XOR> {
	static inline uint_fast8_t adjust(uint_fast8_t target, uint_fast8_t operand, bool &) {
		return target ^ operand;
	}
	static inline uint_fast16_t adjustHalfWord(uint_fast16_t target, uint_fast16_t operand, bool &) {
		return target ^ operand;
	}
	static inline uint32_t adjustWord(uint32_t target, uint32_t operand, bool &) {
		return target ^ operand;
	}
	static inline uint_fast8_t fold(uint32_t accumulator) {
		accumulator ^= accumulator >> 16;
		accumulator ^= accumulator >> 8;
		return accumulator;
	}
};
template<uint8_t Operator>
struct AdjustMultiSingle {
	// Input operand is duplicated into all 4 bytes
	static void adjust(uint8_t * target, uint32_t operand, bool &carry, size_t count) {
		bool localCarry = carry;
		if (count >= sizeof(uint32_t)) {
			if (reinterpret_cast<uintptr_t>(target) & 1) {
				*target = AdjustSingle<Operator>::adjust(*target, operand, localCarry);
				target++, count--;
			}
			if (reinterpret_cast<uintptr_t>(target) & 2) {
				write16_aligned(target, AdjustSingle<Operator>::adjustHalfWord(read16_aligned(target), operand, localCarry));
				target += sizeof(uint16_t), count -= sizeof(uint16_t);
			}
			while (count >= sizeof(uint32_t)) {
				write32_aligned(target, AdjustSingle<Operator>::adjustWord(read32_aligned(target), operand, localCarry));
				target += sizeof(uint32_t), count -= sizeof(uint32_t);
			}
		}
		if (count & 2) {
			write16_unaligned(target, AdjustSingle<Operator>::adjustHalfWord(read16_unaligned(target), operand, localCarry));
			target += sizeof(uint16_t);
		}
		if (count & 1) {
			*target = AdjustSingle<Operator>::adjust(*target, operand, localCarry);
		}
		if (Operator == ADJUST_ADD_CARRY) {
			carry = localCarry;
		}
	}
};
template<uint8_t Operator, typename = void>
struct AdjustSingleMulti {
	static uint_fast8_t adjust(uint_fast8_t target, uint8_t * operand, bool &carry, size_t count) {
		bool localCarry = carry;
		for (size_t i = 0; i < count; i++) {
			target = AdjustSingle<Operator>::adjust(target, operand[i], localCarry);
		}
		if (Operator == ADJUST_ADD_CARRY) {
			carry = localCarry;
		}
		return target;
	}
};
// Specialization if a fold operation is provided
template<uint8_t Operator>
struct AdjustSingleMulti<Operator, decltype(void(AdjustSingle<Operator>::fold))> {
	static uint_fast8_t adjust(uint_fast8_t target, uint8_t * operand, bool &carry, size_t count) {
		bool localCarry = carry;
		if (count >= sizeof(uint32_t)) {
			if (reinterpret_cast<uintptr_t>(operand) & 1) {
				target = AdjustSingle<Operator>::adjust(target, *operand, localCarry);
				operand++, count--;
			}
			if (reinterpret_cast<uintptr_t>(operand) & 2) {
				target = AdjustSingle<Operator>::adjust(target, *operand, localCarry);
				operand++, count--;
				target = AdjustSingle<Operator>::adjust(target, *operand, localCarry);
				operand++, count--;
			}
			if (count >= sizeof(uint32_t)) {
				uint32_t accumulator = read32_aligned(operand);
				operand += sizeof(uint32_t), count -= sizeof(uint32_t);
				while (count >= sizeof(uint32_t)) {
					accumulator = AdjustSingle<Operator>::adjustWord(accumulator, read32_aligned(operand), localCarry);
					operand += sizeof(uint32_t), count -= sizeof(uint32_t);
				}
				target = AdjustSingle<Operator>::adjust(target, AdjustSingle<Operator>::fold(accumulator), localCarry);
			}
		}
		if (count & 2) {
			target = AdjustSingle<Operator>::adjust(target, *operand, localCarry);
			operand++;
			target = AdjustSingle<Operator>::adjust(target, *operand, localCarry);
			operand++;
		}
		if (count & 1) {
			target = AdjustSingle<Operator>::adjust(target, *operand, localCarry);
		}
		if (Operator == ADJUST_ADD_CARRY) {
			carry = localCarry;
		}
		return target;
	}
};
template<uint8_t Operator>
struct AdjustMulti {
	static void adjust(uint8_t * target, uint8_t * operand, bool &carry, size_t count, bool sameBuffer) {
		bool localCarry = carry;
		if (count >= sizeof(uint32_t) && (!sameBuffer || target <= operand || target >= operand + sizeof(uint32_t))) {
			if (reinterpret_cast<uintptr_t>(target) & 1) {
				*target = AdjustSingle<Operator>::adjust(*target, *operand, localCarry);
				target++, operand++, count--;
			}
			if (reinterpret_cast<uintptr_t>(target) & 2) {
				write16_aligned(target, AdjustSingle<Operator>::adjustHalfWord(read16_aligned(target), read16_unaligned(operand), localCarry));
				target += sizeof(uint16_t), operand += sizeof(uint16_t), count -= sizeof(uint16_t);
			}
			while (count >= sizeof(uint32_t)) {
				write32_aligned(target, AdjustSingle<Operator>::adjustWord(read32_aligned(target), read32_unaligned(operand), localCarry));
				target += sizeof(uint32_t), operand += sizeof(uint32_t), count -= sizeof(uint32_t);
			}
		}
		// Target pointer may be immediately ahead of operand pointer, so simply loop bytes
		for (size_t i = 0; i < count; i++) {
			target[i] = AdjustSingle<Operator>::adjust(target[i], operand[i], localCarry);
		}
		if (Operator == ADJUST_ADD_CARRY) {
			carry = localCarry;
		}
	}
};

// Automatically generates a function table for all operators. Would be easier with C++14
template <template<uint8_t, typename...> typename T>
class AdjustFuncTable {
	using FuncPtr = decltype(T<ADJUST_NOT>::adjust)*;
	std::array<FuncPtr, ADJUST_OP_MASK+1> table;

	template<uint8_t... Operator>
	struct operator_sequence {};
	template<size_t N, uint8_t... Operator>
	struct make_operator_sequence : make_operator_sequence<N-1, N-1, Operator...> {};
	template<uint8_t... Operator>
	struct make_operator_sequence<0, Operator...> : operator_sequence<Operator...> {};

	template<uint8_t... Operator>
	constexpr AdjustFuncTable(operator_sequence<Operator...>)
		: table({{ &T<Operator>::adjust... }})
	{
	}

public:
	constexpr AdjustFuncTable()
		: AdjustFuncTable(make_operator_sequence<ADJUST_OP_MASK+1>{})
	{
	}

	constexpr inline FuncPtr operator[](uint8_t op) const {
		return table[op];
	}
};

// VDU 23, 0, &A0, bufferId; 5, operation, offset; [count;] [operand]: Adjust buffer
// This is used for adjusting the contents of a buffer
// It can be used to overwrite bytes, insert bytes, increment bytes, etc
// Basic operation are not, neg, set, add, add-with-carry, and, or, xor
// Upper bits of operation byte are used to indicate:
// - whether to use a long offset (24-bit) or short offset (16-bit)
// - whether the operand is a buffer-originated value or an immediate value
// - whether to adjust a single target or multiple targets
// - whether to use a single operand or multiple operands
//
void VDUStreamProcessor::bufferAdjust(uint16_t adjustBufferId) {
	static constexpr AdjustFuncTable<AdjustSingle> adjustSingleFuncs;
	static constexpr AdjustFuncTable<AdjustMultiSingle> adjustMultiSingleFuncs;
	static constexpr AdjustFuncTable<AdjustSingleMulti> adjustSingleMultiFuncs;
	static constexpr AdjustFuncTable<AdjustMulti> adjustMultiFuncs;

	const auto command = readByte_t();

	const bool useAdvancedOffsets = command & ADJUST_ADVANCED_OFFSETS;
	const bool useBufferValue = command & ADJUST_BUFFER_VALUE;
	const bool useMultiTarget = command & ADJUST_MULTI_TARGET;
	const bool useMultiOperand = command & ADJUST_MULTI_OPERAND;
	const uint8_t op = command & ADJUST_OP_MASK;
	// Operators that are greater than NEG have an operand value
	const bool hasOperand = op > ADJUST_NEG;

	auto offset = getOffsetFromStream(useAdvancedOffsets);
	const std::vector<std::shared_ptr<BufferStream>> * operandBuffer = nullptr;
	auto operandBufferId = 0;
	AdvancedOffset operandOffset = {};
	auto count = 1;

	if (useMultiTarget || useMultiOperand) {
		count = useAdvancedOffsets ? read24_t() : readWord_t();
	}
	if (useBufferValue && hasOperand) {
		operandBufferId = resolveBufferId(readWord_t(), id);
		operandOffset = getOffsetFromStream(useAdvancedOffsets);
		if (operandBufferId == -1) {
			debug_log("bufferAdjust: no operand buffer ID\n\r");
			return;
		}
		auto operandBufferIter = buffers.find(operandBufferId);
		if (operandBufferIter == buffers.end()) {
			debug_log("bufferAdjust: buffer %d not found\n\r", operandBufferId);
			return;
		}
		operandBuffer = &operandBufferIter->second;
	}

	auto bufferId = resolveBufferId(adjustBufferId, id);
	if (bufferId == -1) {
		debug_log("bufferAdjust: no target buffer ID\n\r");
		return;
	}
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferAdjust: buffer %d not found\n\r", bufferId);
		return;
	}
	auto &buffer = bufferIter->second;

	if (command == -1 || count == -1 || offset.blockOffset == -1 || operandOffset.blockOffset == -1) {
		debug_log("bufferAdjust: invalid command, count, offset or operand value\n\r");
		return;
	}

	MultiBufferStream * instream = nullptr;
	tcb::span<uint8_t> targetSpan;
	uint_fast8_t sourceValue = 0;
	auto operandValue = 0;
	bool carryValue = false;

	// if useMultiTarget is set, the we're updating multiple source values
	// if useMultiOperand is also set, we get multiple operand values
	// so...
	// if both useMultiTarget and useMultiOperand are false we're updating a single source value with a single operand
	// if useMultiTarget is false and useMultiOperand is true we're adding all operand values to the same source value
	// if useMultiTarget is true and useMultiOperand is false we're adding the same operand to all source values
	// if both useMultiTarget and useMultiOperand are true we're adding each operand value to the corresponding source value

	if (hasOperand) {
		if (!useMultiOperand) {
			// we have a singular operand value
			operandValue = operandBuffer ? getBufferByte(*operandBuffer, operandOffset) : readByte_t();
			if (operandValue == -1) {
				debug_log("bufferAdjust: invalid operand value\n\r");
				return;
			}
		} else if (!useBufferValue && id != 65535) {
			// multiple inline operands, get the underlying buffer if executing from one
			instream = (MultiBufferStream *)inputStream.get();
			operandBuffer = &instream->tellBuffer(operandOffset.blockOffset, operandOffset.blockIndex);
		}
	}
	if (!useMultiTarget) {
		// we have a singular target value
		targetSpan = getBufferSpan(buffer, offset);
		if (targetSpan.empty()) {
			debug_log("bufferAdjust: invalid target offset\n\r");
			return;
		}
		sourceValue = targetSpan.front();
	}

	debug_log("bufferAdjust: command %d, offset %d:%d, count %d, operandBufferId %d, operandOffset %d:%d, sourceValue %d, operandValue %d\n\r",
		command, (int)offset.blockIndex, offset.blockOffset, count, operandBufferId, (int)operandOffset.blockIndex, operandOffset.blockOffset, sourceValue, operandValue);
	debug_log("useMultiTarget %d, useMultiOperand %d, useAdvancedOffsets %d, useBufferValue %d\n\r", useMultiTarget, useMultiOperand, useAdvancedOffsets, useBufferValue);

	if (!useMultiTarget) {
		if (!hasOperand || !useMultiOperand) {
			auto func = adjustSingleFuncs[op];
			sourceValue = func(sourceValue, operandValue, carryValue);
		} else if (operandBuffer) {
			auto func = adjustSingleMultiFuncs[op];
			while (count > 0) {
				auto operandSpan = getBufferSpan(*operandBuffer, operandOffset);
				auto iterCount = std::min<size_t>(operandSpan.size(), count);
				if (iterCount == 0) {
					debug_log("bufferAdjust: operand buffer overflow\n\r");
					if (instream) {
						instream->seekTo(operandOffset.blockOffset, operandOffset.blockIndex);
					}
					return;
				}
				sourceValue = func(sourceValue, operandSpan.data(), carryValue, iterCount);
				operandOffset.blockOffset += iterCount;
				count -= iterCount;
			}
			if (instream) {
				instream->seekTo(operandOffset.blockOffset, operandOffset.blockIndex);
			}
		} else {
			auto func = adjustSingleFuncs[op];
			while (count > 0) {
				operandValue = readByte_t();
				if (operandValue == -1) {
					debug_log("bufferAdjust: operand timeout\n\r");
					return;
				}
				sourceValue = func(sourceValue, operandValue, carryValue);
				count--;
			}
		}
		debug_log("bufferAdjust: result %d\n\r", sourceValue);
		targetSpan.front() = sourceValue;
		// increment offset in case carry is used
		offset.blockOffset++;
	} else {
		if (!hasOperand || !useMultiOperand) {
			auto func = adjustMultiSingleFuncs[op];
			auto operandWord = (uint8_t)operandValue * (uint32_t)0x01010101;
			while (count > 0) {
				targetSpan = getBufferSpan(buffer, offset);
				auto iterCount = std::min<size_t>(targetSpan.size(), count);
				if (iterCount == 0) {
					debug_log("bufferAdjust: target buffer overflow\n\r");
					return;
				}
				func(targetSpan.data(), operandWord, carryValue, iterCount);
				offset.blockOffset += iterCount;
				count -= iterCount;
			}
		} else if (operandBuffer) {
			auto func = adjustMultiFuncs[op];
			while (count > 0) {
				targetSpan = getBufferSpan(buffer, offset);
				auto operandSpan = getBufferSpan(*operandBuffer, operandOffset);
				auto iterCount = std::min<size_t>(std::min(targetSpan.size(), operandSpan.size()), count);
				if (iterCount == 0) {
					debug_log("bufferAdjust: target or operand buffer overflow\n\r");
					if (instream) {
						instream->seekTo(operandOffset.blockOffset, operandOffset.blockIndex);
					}
					return;
				}
				bool sameBuffer = buffer[offset.blockIndex] == (*operandBuffer)[operandOffset.blockIndex];
				func(targetSpan.data(), operandSpan.data(), carryValue, iterCount, sameBuffer);
				offset.blockOffset += iterCount;
				operandOffset.blockOffset += iterCount;
				count -= iterCount;
			}
			if (instream) {
				instream->seekTo(operandOffset.blockOffset, operandOffset.blockIndex);
			}
		} else {
			auto func = adjustSingleFuncs[op];
			while (count > 0) {
				targetSpan = getBufferSpan(buffer, offset);
				auto iterCount = std::min<size_t>(targetSpan.size(), count);
				if (iterCount == 0) {
					debug_log("bufferAdjust: target buffer overflow\n\r");
					return;
				}
				for (size_t i = 0; i < iterCount; i++) {
					operandValue = readByte_t();
					if (operandValue == -1) {
						debug_log("bufferAdjust: operand timeout\n\r");
						return;
					}
					targetSpan[i] = func(targetSpan[i], operandValue, carryValue);
				}
				offset.blockOffset += iterCount;
				count -= iterCount;
			}
		}
	}

	if (op == ADJUST_ADD_CARRY) {
		// if we were using carry, store the final carry value
		if (!setBufferByte(carryValue, buffer, offset)) {
			debug_log("bufferAdjust: failed to set carry value %d at offset %d:%d\n\r", carryValue, (int)offset.blockIndex, offset.blockOffset);
			return;
		}
	}
}

// returns true or false depending on whether conditions are met
// Will read the following arguments from the stream
// operation, checkBufferId; offset; [operand]
// This works in a similar manner to bufferAdjust
// for now, this only supports single-byte comparisons
// as multi-byte comparisons are a bit more complex
//
bool VDUStreamProcessor::bufferConditional() {
	auto command = readByte_t();
	auto checkBufferId = resolveBufferId(readWord_t(), id);

	bool useAdvancedOffsets = command & COND_ADVANCED_OFFSETS;
	bool useBufferValue = command & COND_BUFFER_VALUE;
	uint8_t op = command & COND_OP_MASK;
	// conditional operators that are greater than NOT_EXISTS require an operand
	bool hasOperand = op > COND_NOT_EXISTS;

	auto offset = getOffsetFromStream(useAdvancedOffsets);
	const std::vector<std::shared_ptr<BufferStream>> * operandBuffer = nullptr;
	auto operandBufferId = 0;
	AdvancedOffset operandOffset = {};

	if (useBufferValue && hasOperand) {
		operandBufferId = resolveBufferId(readWord_t(), id);
		operandOffset = getOffsetFromStream(useAdvancedOffsets);
		if (operandBufferId == -1) {
			debug_log("bufferConditional: no operand buffer ID\n\r");
			return false;
		}
		auto operandBufferIter = buffers.find(operandBufferId);
		if (operandBufferIter == buffers.end()) {
			debug_log("bufferConditional: buffer %d not found\n\r", operandBufferId);
			return false;
		}
		operandBuffer = &operandBufferIter->second;
	}

	if (command == -1 || checkBufferId == -1 || offset.blockOffset == -1 || operandOffset.blockOffset == -1) {
		debug_log("bufferConditional: invalid command, checkBufferId, offset or operand value\n\r");
		return false;
	}

	auto checkBufferIter = buffers.find(checkBufferId);
	if (checkBufferIter == buffers.end()) {
		debug_log("bufferConditional: buffer %d not found\n\r", checkBufferId);
		return false;
	}
	auto &checkBuffer = checkBufferIter->second;
	auto sourceValue = getBufferByte(checkBuffer, offset);
	int16_t operandValue = 0;
	if (hasOperand) {
		operandValue = operandBuffer ? getBufferByte(*operandBuffer, operandOffset) : readByte_t();
	}

	debug_log("bufferConditional: command %d, checkBufferId %d, offset %d:%d, operandBufferId %d, operandOffset %d:%d, sourceValue %d, operandValue %d\n\r",
		command, checkBufferId, (int)offset.blockIndex, offset.blockOffset, operandBufferId, (int)operandOffset.blockIndex, operandOffset.blockOffset, sourceValue, operandValue);

	if (sourceValue == -1 || operandValue == -1) {
		debug_log("bufferConditional: invalid source or operand value\n\r");
		return false;
	}

	bool shouldCall = false;

	switch (op) {
		case COND_EXISTS: {
			shouldCall = sourceValue != 0;
		}	break;
		case COND_NOT_EXISTS: {
			shouldCall = sourceValue == 0;
		}	break;
		case COND_EQUAL: {
			shouldCall = sourceValue == operandValue;
		}	break;
		case COND_NOT_EQUAL: {
			shouldCall = sourceValue != operandValue;
		}	break;
		case COND_LESS: {
			shouldCall = sourceValue < operandValue;
		}	break;
		case COND_GREATER: {
			shouldCall = sourceValue > operandValue;
		}	break;
		case COND_LESS_EQUAL: {
			shouldCall = sourceValue <= operandValue;
		}	break;
		case COND_GREATER_EQUAL: {
			shouldCall = sourceValue >= operandValue;
		}	break;
		case COND_AND: {
			shouldCall = sourceValue && operandValue;
		}	break;
		case COND_OR: {
			shouldCall = sourceValue || operandValue;
		}	break;
	}

	debug_log("bufferConditional: evaluated as %s\n\r", shouldCall ? "true" : "false");

	return shouldCall;
}

// VDU 23, 0, &A0, bufferId; 7: Jump to a buffer
// VDU 23, 0, &A0, bufferId; 9, offset; offsetHighByte : Jump to (advanced) offset within buffer
// Change execution to given buffer (from beginning or at an offset)
//
void VDUStreamProcessor::bufferJump(uint16_t bufferId, AdvancedOffset offset) {
	debug_log("bufferJump: buffer %d\n\r", bufferId);
	if (id == 65535) {
		// we're currently the top-level stream, so we can't jump
		// so have to call instead
		return bufferCall(bufferId, offset);
	}
	if (bufferId == 65535 || bufferId == id) {
		// a buffer ID of 65535 is used to indicate current buffer, so we seek to offset
		auto instream = (MultiBufferStream *)inputStream.get();
		instream->seekTo(offset.blockOffset, offset.blockIndex);
		return;
	}
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferJump: buffer %d not found\n\r", bufferId);
		return;
	}
	// replace our input stream with a new one
	auto &streams = bufferIter->second;
	auto multiBufferStream = make_shared_psram<MultiBufferStream>(streams);
	if (offset.blockOffset != 0 || offset.blockIndex != 0) {
		multiBufferStream->seekTo(offset.blockOffset, offset.blockIndex);
	}
	id = bufferId;
	inputStream = std::move(multiBufferStream);
}

// VDU 23, 0, &A0, bufferId; &0D, sourceBufferId; sourceBufferId; ...; 65535; : Copy blocks from buffers
// Copy (blocks from) a list of buffers into a new buffer
// list is terminated with a bufferId of 65535 (-1)
// Replaces the target buffer with the new one
// This is useful to construct a single buffer from multiple buffers
// which can be used to construct more complex commands
// Target buffer ID can be included in the source list
//
void VDUStreamProcessor::bufferCopy(uint16_t bufferId, tcb::span<const uint16_t> sourceBufferIds) {
	if (bufferId == 65535) {
		debug_log("bufferCopy: ignoring buffer %d\n\r", bufferId);
		return;
	}
	// prepare a vector for storing our buffers
	std::vector<std::shared_ptr<BufferStream>, psram_allocator<std::shared_ptr<BufferStream>>> streams;
	// loop thru buffer IDs
	for (const auto sourceId : sourceBufferIds) {
		auto sourceBufferIter = buffers.find(sourceId);
		if (sourceBufferIter != buffers.end()) {
			// buffer ID exists
			// loop thru blocks stored against this ID
			for (const auto &block : sourceBufferIter->second) {
				// push a copy of the block into our vector
				auto bufferStream = make_shared_psram<BufferStream>(block->size());
				if (!bufferStream || !bufferStream->getBuffer()) {
					debug_log("bufferCopy: failed to create buffer\n\r");
					return;
				}
				debug_log("bufferCopy: copying stream %d bytes\n\r", block->size());
				bufferStream->writeBuffer(block->getBuffer(), block->size());
				streams.push_back(std::move(bufferStream));
			}
		} else {
			debug_log("bufferCopy: buffer %d not found\n\r", sourceId);
		}
	}
	// replace buffer with new one
	bufferRemoveUsers(bufferId);
	buffers[bufferId].assign(std::make_move_iterator(streams.begin()), std::make_move_iterator(streams.end()));
	debug_log("bufferCopy: copied %d streams into buffer %d (%d)\n\r", streams.size(), bufferId, buffers[bufferId].size());
}

// VDU 23, 0, &A0, bufferId; &0E : Consolidate blocks within buffer
// Consolidate multiple streams/blocks into a single block
// This is useful for using bitmaps sent in multiple blocks
//
void VDUStreamProcessor::bufferConsolidate(uint16_t bufferId) {
	// Create a new stream big enough to contain all streams in the given buffer
	// Copy all streams into the new stream
	// Replace the given buffer with the new stream
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferConsolidate: buffer %d not found\n\r", bufferId);
		return;
	}
	auto &buffer = bufferIter->second;
	if (buffer.size() == 1) {
		// only one stream, so nothing to consolidate
		return;
	}
	// buffer ID exists
	auto bufferStream = consolidateBuffers(buffer);
	if (!bufferStream) {
		debug_log("bufferConsolidate: failed to create buffer\n\r");
		return;
	}
	bufferRemoveUsers(bufferId);
	buffer.clear();
	buffer.push_back(std::move(bufferStream));
	debug_log("bufferConsolidate: consolidated %d streams into buffer %d\n\r", buffer.size(), bufferId);
}

void VDUStreamProcessor::clearTargets(tcb::span<const uint16_t> targets) {
	for (const auto target : targets) {
		bufferClear(target);
	}
}

// VDU 23, 0, &A0, bufferId; &0F, length; : Split buffer into blocks by length
// VDU 23, 0, &A0, bufferId; &10, length; <bufferIds>; 65535; : Split buffer by length to new buffers
// VDU 23, 0, &A0, bufferId; &11, length; targetBufferId; : Split buffer by length to new buffers from target onwards
// Split a buffer into multiple blocks/streams to new buffers
// Will overwrite any existing buffers
//
void VDUStreamProcessor::bufferSplitInto(uint16_t bufferId, uint16_t length, tcb::span<uint16_t> newBufferIds, bool iterate) {
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferSplitInto: buffer %d not found\n\r", bufferId);
		return;
	}
	// get a consolidated version of the buffer
	auto bufferStream = consolidateBuffers(bufferIter->second);
	if (!bufferStream) {
		debug_log("bufferSplitInto: failed to create buffer\n\r");
		return;
	}
	if (!iterate) {
		clearTargets(newBufferIds);
	}

	auto chunks = splitBuffer(std::move(bufferStream), length);
	if (chunks.empty()) {
		debug_log("bufferSplitInto: failed to split buffer\n\r");
		return;
	}
	// distribute our chunks to destination buffers
	auto targetIter = newBufferIds.begin();
	for (auto &chunk : chunks) {
		auto targetId = *targetIter;
		if (iterate) {
			bufferClear(targetId);
		}
		buffers[targetId].push_back(std::move(chunk));
		iterate = updateTarget(newBufferIds, targetIter, iterate);
	}
	debug_log("bufferSplitInto: split buffer %d into %d blocks of length %d\n\r", bufferId, chunks.size(), length);
}

// VDU 23, 0, &A0, bufferId; &12, width; chunkCount; : Split buffer by width (in-place)
// VDU 23, 0, &A0, bufferId; &13, width; <bufferIds>; 65535; : Split buffer by width to new buffers
// VDU 23, 0, &A0, bufferId; &14, width; chunkCount; targetBufferId; : Split buffer by width to new buffers from ID onwards
// Split a buffer into multiple blocks/streams to new buffers/chunks by width
// Will overwrite any existing buffers
//
void VDUStreamProcessor::bufferSplitByInto(uint16_t bufferId, uint16_t width, uint16_t chunkCount, tcb::span<uint16_t> newBufferIds, bool iterate) {
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferSplitByInto: buffer %d not found\n\r", bufferId);
		return;
	}
	// get a consolidated version of the buffer
	auto bufferStream = consolidateBuffers(bufferIter->second);
	if (!bufferStream) {
		debug_log("bufferSplitByInto: failed to create buffer\n\r");
		return;
	}
	if (!iterate) {
		clearTargets(newBufferIds);
	}

	std::vector<std::vector<std::shared_ptr<BufferStream>>> chunks;
	chunks.resize(chunkCount);
	{
		// split to get raw chunks
		auto rawchunks = splitBuffer(std::move(bufferStream), width);
		if (rawchunks.empty()) {
			debug_log("bufferSplitByInto: failed to split buffer\n\r");
			return;
		}
		// and re-jig into our chunks vector
		auto chunkIndex = 0;
		for (auto &chunk : rawchunks) {
			chunks[chunkIndex].push_back(std::move(chunk));
			chunkIndex++;
			if (chunkIndex >= chunkCount) {
				chunkIndex = 0;
			}
		}
	}

	// consolidate our chunks, and distribute to buffers
	auto targetIter = newBufferIds.begin();
	for (auto &stream : chunks) {
		auto targetId = *targetIter;
		if (iterate) {
			bufferClear(targetId);
		}
		auto chunk = consolidateBuffers(stream);
		if (!chunk) {
			debug_log("bufferSplitByInto: failed to create buffer\n\r");
			return;
		}
		buffers[targetId].push_back(std::move(chunk));
		iterate = updateTarget(newBufferIds, targetIter, iterate);
	}

	debug_log("bufferSplitByInto: split buffer %d into %d chunks of width %d\n\r", bufferId, chunkCount, width);
}

// VDU 23, 0, &A0, bufferId; &15, <bufferIds>; 65535; : Spread blocks from buffer into new buffers
// VDU 23, 0, &A0, bufferId; &16, targetBufferId; : Spread blocks from target buffer onwards
//
void VDUStreamProcessor::bufferSpreadInto(uint16_t bufferId, tcb::span<uint16_t> newBufferIds, bool iterate) {
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferSpreadInto: buffer %d not found\n\r", bufferId);
		return;
	}
	auto &buffer = bufferIter->second;
	// swap the source buffer contents into a local vector so it can be iterated safely even if it's a target
	std::vector<std::shared_ptr<BufferStream>> localBuffer;
	localBuffer.swap(buffer);
	if (!iterate) {
		clearTargets(newBufferIds);
	}
	// iterate over its blocks and send to targets
	auto targetIter = newBufferIds.begin();
	for (const auto &block : localBuffer) {
		auto targetId = *targetIter;
		if (iterate) {
			bufferClear(targetId);
		}
		buffers[targetId].push_back(block);
		iterate = updateTarget(newBufferIds, targetIter, iterate);
	}
	// if the source buffer is still empty, move the original contents back
	if (buffer.empty()) {
		buffer = std::move(localBuffer);
	}
}

// VDU 23, 0, &A0, bufferId; &17 : Reverse blocks within buffer
// Reverses the order of blocks within a buffer
// may be useful for mirroring bitmaps if they have been split by row
//
void VDUStreamProcessor::bufferReverseBlocks(uint16_t bufferId) {
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter != buffers.end()) {
		// reverse the order of the streams
		auto &buffer = bufferIter->second;
		std::reverse(buffer.begin(), buffer.end());
		debug_log("bufferReverseBlocks: reversed blocks in buffer %d\n\r", bufferId);
	}
}

// VDU 23, 0, &A0, bufferId; &18, options, <parameters> : Reverse buffer
// Reverses the contents of blocks within a buffer
// may be useful for mirroring bitmaps
//
void VDUStreamProcessor::bufferReverse(uint16_t bufferId, uint8_t options) {
	auto bufferIter = buffers.find(bufferId);
	if (bufferIter == buffers.end()) {
		debug_log("bufferReverse: buffer %d not found\n\r", bufferId);
		return;
	}
	auto &buffer = bufferIter->second;
	bool use16Bit = options & REVERSE_16BIT;
	bool use32Bit = options & REVERSE_32BIT;
	bool useSize  = (options & REVERSE_SIZE) == REVERSE_SIZE;
	bool useChunks = options & REVERSE_CHUNKED;
	bool reverseBlocks = options & REVERSE_BLOCK;
	uint8_t unused = options & REVERSE_UNUSED_BITS;

	// future expansion may include:
	// reverse at an offset for a set length (within blocks)
	// reversing across whole buffer (not per block)

	if (unused != 0) {
		debug_log("bufferReverse: warning - unused bits in options byte\n\r");
	}

	auto valueSize = 1;
	auto chunkSize = 0;

	if (useSize) {
		// size follows as a word
		valueSize = readWord_t();
		if (valueSize == -1) {
			return;
		}
	} else if (use32Bit) {
		valueSize = 4;
	} else if (use16Bit) {
		valueSize = 2;
	}

	if (useChunks) {
		chunkSize = readWord_t();
		if (chunkSize == -1) {
			return;
		}
	}

	// verify that our blocks are a multiple of valueSize
	for (const auto &block : buffer) {
		auto size = block->size();
		if (size % valueSize != 0 || (chunkSize != 0 && size % chunkSize != 0)) {
			debug_log("bufferReverse: error - buffer %d contains block not a multiple of value/chunk size %d\n\r", bufferId, valueSize);
			return;
		}
	}

	debug_log("bufferReverse: reversing buffer %d, value size %d, chunk size %d\n\r", bufferId, valueSize, chunkSize);

	for (const auto &block : buffer) {
		if (chunkSize == 0) {
			// no chunking, so simpler reverse
			reverseValues(block->getBuffer(), block->size(), valueSize);
		} else {
			// reverse in chunks
			auto data = block->getBuffer();
			auto chunkCount = block->size() / chunkSize;
			for (auto i = 0; i < chunkCount; i++) {
				reverseValues(data + (i * chunkSize), chunkSize, valueSize);
			}
		}
	}

	if (reverseBlocks) {
		// reverse the order of the streams
		std::reverse(buffer.begin(), buffer.end());
		debug_log("bufferReverse: reversed blocks in buffer %d\n\r", bufferId);
	}

	debug_log("bufferReverse: reversed buffer %d\n\r", bufferId);
}

// VDU 23, 0, &A0, bufferId; &19, sourceBufferId; sourceBufferId; ...; 65535; : Copy references to blocks from buffers
// Copy references to (blocks from) a list of buffers into a new buffer
// list is terminated with a bufferId of 65535 (-1)
// Replaces the target buffer with the new one
// This is useful to construct a single buffer from multiple buffers without the copy overhead
// If target buffer is included in the source list it will be skipped to prevent a reference loop
//
void VDUStreamProcessor::bufferCopyRef(uint16_t bufferId, tcb::span<const uint16_t> sourceBufferIds) {
	if (bufferId == 65535) {
		debug_log("bufferCopyRef: ignoring buffer %d\n\r", bufferId);
		return;
	}
	bufferClear(bufferId);
	auto &buffer = buffers[bufferId];

	// loop thru buffer IDs
	for (const auto sourceId : sourceBufferIds) {
		if (sourceId == bufferId) {
			debug_log("bufferCopyRef: skipping buffer %d as it's the target\n\r", sourceId);
			continue;
		}
		auto sourceBufferIter = buffers.find(sourceId);
		if (sourceBufferIter != buffers.end()) {
			// buffer ID exists
			auto &sourceBuffer = sourceBufferIter->second;
			// push pointers to the blocks into our target buffer
			buffer.insert(buffer.end(), sourceBuffer.begin(), sourceBuffer.end());
		} else {
			debug_log("bufferCopyRef: buffer %d not found\n\r", sourceId);
		}
	}
	debug_log("bufferCopyRef: copied %d block references into buffer %d\n\r", buffers[bufferId].size(), bufferId);
}

// VDU 23, 0, &A0, bufferId; &1A, sourceBufferId; sourceBufferId; ...; 65535; : Copy blocks from buffers and consolidate
// Copy (blocks from) a list of buffers into a new buffer and consolidate them
// list is terminated with a bufferId of 65535 (-1)
// Replaces the target buffer with the new one, but will re-use the memory if it is the same size
// This is useful for constructing bitmaps from multiple buffers without needing an extra consolidate step
// If target buffer is included in the source list it will be skipped.
//
void VDUStreamProcessor::bufferCopyAndConsolidate(uint16_t bufferId, tcb::span<const uint16_t> sourceBufferIds) {
	if (bufferId == 65535) {
		debug_log("bufferCopyAndConsolidate: ignoring buffer %d\n\r", bufferId);
		return;
	}

	// work out total length of buffer
	uint32_t length = 0;
	for (const auto sourceId : sourceBufferIds) {
		if (sourceId == bufferId) {
			continue;
		}
		auto sourceBufferIter = buffers.find(sourceId);
		if (sourceBufferIter != buffers.end()) {
			auto &sourceBuffer = sourceBufferIter->second;
			for (const auto &block : sourceBuffer) {
				length += block->size();
			}
		}
	}

	// Ensure the buffer has 1 block of the correct size
	auto &buffer = buffers[bufferId];
	if (buffer.size() != 1 || buffer.front()->size() != length) {
		bufferRemoveUsers(bufferId);
		buffer.clear();
		auto bufferStream = make_shared_psram<BufferStream>(length);
		if (!bufferStream || !bufferStream->getBuffer()) {
			// buffer couldn't be created
			debug_log("bufferCopyAndConsolidate: failed to create buffer %d\n\r", bufferId);
			return;
		}
		buffer.push_back(std::move(bufferStream));
	}

	auto destination = buffer.front()->getBuffer();

	// loop thru buffer IDs
	for (const auto sourceId : sourceBufferIds) {
		if (sourceId == bufferId) {
			debug_log("bufferCopyAndConsolidate: skipping buffer %d as it's the target\n\r", sourceId);
			continue;
		}
		auto sourceBufferIter = buffers.find(sourceId);
		if (sourceBufferIter != buffers.end()) {
			// buffer ID exists
			auto &sourceBuffer = sourceBufferIter->second;
			// loop thru blocks stored against this ID
			for (const auto &block : sourceBuffer) {
				// copy the block into our target buffer
				auto bufferLength = block->size();
				memcpy(destination, block->getBuffer(), bufferLength);
				destination += bufferLength;
			}
		} else {
			debug_log("bufferCopyAndConsolidate: buffer %d not found\n\r", sourceId);
		}
	}
	debug_log("bufferCopyAndConsolidate: copied %d bytes into buffer %d\n\r", length, bufferId);
}

// VDU 23, 0, &A0, bufferId; &20, operation, <args> : Affine transform creation/combination
// Create or combine an affine transformaiton matrix
//
void VDUStreamProcessor::bufferAffineTransform(uint16_t bufferId) {
	const auto command = readByte_t();
	const auto op = command & AFFINE_OP_MASK;
	const bool useAdvancedOffsets = command & AFFINE_OP_ADVANCED_OFFSETS;
	const bool useBufferValue = command & AFFINE_OP_BUFFER_VALUE;
	const bool useMultiFormat = command & AFFINE_OP_MULTI_FORMAT;

	// transform is a 3x3 matrix of float values
	float transform[9] = {0.0f};
	transform[0] = 1.0f;
	transform[4] = 1.0f;
	transform[8] = 1.0f;
	auto matrixSize = 9 * sizeof(float);

	// get a MultiBufferStream object for a buffer
	// NB this will rewind all the streams in the buffer
	auto getMultiBufferStream = [this](uint16_t bufferId) -> std::unique_ptr<MultiBufferStream> {
		auto bufferIter = buffers.find(bufferId);
		if (bufferIter == buffers.end()) {
			debug_log("bufferAffineTransform: buffer %d not found\n\r", bufferId);
			return nullptr;
		}
		auto buffer = bufferIter->second;
		return std::unique_ptr<MultiBufferStream>(new MultiBufferStream(buffer));
	};

	auto getFormatInfo = [this, useAdvancedOffsets, useBufferValue](bool &isFixed, bool &is16Bit, int8_t &shift, int16_t &sourceBufferId, AdvancedOffset &offset) -> bool {
		auto format = readByte_t();
		if (format == -1) {
			return false;
		}
		isFixed = format & AFFINE_FORMAT_FIXED;
		is16Bit = format & AFFINE_FORMAT_16BIT;
		shift = format & AFFINE_FORMAT_SHIFT_MASK;
		// ensure our size value obeys negation
		if (is16Bit && (shift & AFFINE_FORMAT_SHIFT_TOPBIT)) {
			// top bit was set, so it's a negative - so we need to set the top bits of the size
			shift = shift | AFFINE_FORMAT_FLAGS;
		}
		if (useBufferValue) {
			sourceBufferId = readWord_t();
			if (sourceBufferId == -1) {
				return false;
			}
			offset = getOffsetFromStream(useAdvancedOffsets);
			return offset.blockOffset != -1;
		}
		return true;
	};

	auto convertValueToFloat = [](uint32_t rawValue, bool is16Bit, bool isFixed, int8_t shift) -> float {
		if (isFixed) {
			// fixed point value
			// we will scale our rawValue by a factor to create our float
			// this version assumes base value is -1 to +1, multiplied by 2^shift
			// so binary point starts at bit 31 and is moved right by shift
			// auto scale = (1u << shift) / (float)(1u << 31);
			// if (is16Bit) {
			// 	// ensure 16-bit values are 32-bit values with their buttom 16 bits empty
			// 	rawValue <<= 16;
			// }
			// in this version the binary point starts after bit 0 and is moved left by shift
			// which is arguably a bit more intuitive in use, and matches the xtensa fixed point instruction support
			// we need to use our shift value to determine how far to scale
			auto scale = shift < 0 ? 1 << -shift : 1.0f / (1 << shift);
			if (is16Bit) {
				return (float)(int16_t)rawValue * scale;
			}
			debug_log("bufferAffineTransform: rawValue %d, shift %d, scale %f\n\r", rawValue, shift, scale);
			return (float)(int32_t)rawValue * scale;
		} else {
			// floating point value - shift ignored
			// if we're reading a 16-bit value, we need to convert to 32-bit float
			if (is16Bit) {
				return float16ToFloat32(rawValue);
			}
			// take our raw value and interpret its bits as a float
			return *(float*)&rawValue;
		}
	};

	auto readValueFromBuffer = [getMultiBufferStream, convertValueToFloat](uint32_t sourceBufferId, AdvancedOffset &offset, bool is16Bit, bool isFixed, int8_t shift) -> float {
		// get the value that we're dealing with
		uint32_t rawValue = 0;

		std::unique_ptr<MultiBufferStream> buff = getMultiBufferStream(sourceBufferId);
		if (!buff) {
			return INFINITY;
		}
		buff->seekTo(offset.blockOffset, offset.blockIndex);

		auto bytesToRead = is16Bit ? 2 : 4;
		auto read = buff->readBytes((uint8_t *)&rawValue, bytesToRead);
		if (read != bytesToRead) {
			debug_log("bufferAffineTransform: failed to read %d bytes from buffer %d\n\r", bytesToRead, sourceBufferId);
			return INFINITY;
		}
		// update our offset so repeated reads don't just re-read the same value
		buff->tellBuffer(offset.blockOffset, offset.blockIndex);
		return convertValueToFloat(rawValue, is16Bit, isFixed, shift);
	};

	auto readValueFromStream = [this, convertValueToFloat](bool is16Bit, bool isFixed, int8_t shift) -> float {
		// get the value that we're dealing with
		uint32_t rawValue = 0;
		auto bytesToRead = is16Bit ? 2 : 4;
		// read the value from the stream
		if (readIntoBuffer((uint8_t *)&rawValue, bytesToRead) != 0) {
			debug_log("bufferAffineTransform: failed to read %d bytes from stream\n\r", bytesToRead);
			return INFINITY;
		}
		return convertValueToFloat(rawValue, is16Bit, isFixed, shift);
	};

	auto readFloatArgument = [getFormatInfo, readValueFromBuffer, readValueFromStream, useBufferValue]() -> float {
		bool isFixed, is16Bit;
		int8_t shift;
		int16_t sourceBufferId = -1;
		AdvancedOffset offset = {};
		if (!getFormatInfo(isFixed, is16Bit, shift, sourceBufferId, offset)) {
			return INFINITY;
		}
		if (useBufferValue) {
			return readValueFromBuffer(sourceBufferId, offset, is16Bit, isFixed, shift);
		}
		return readValueFromStream(is16Bit, isFixed, shift);
	};

	auto readMultipleArgs = [this, getFormatInfo, readValueFromBuffer, readValueFromStream, useBufferValue, useMultiFormat](float *values, int count) -> bool {
		bool isFixed, is16Bit;
		int8_t shift;
		int16_t sourceBufferId = -1;
		AdvancedOffset offset = {};

		for (int i = 0; i < count; i++) {
			if (i == 0 || useMultiFormat) {
				if (!getFormatInfo(isFixed, is16Bit, shift, sourceBufferId, offset)) {
					return false;
				}
			}
			values[i] = useBufferValue ? readValueFromBuffer(sourceBufferId, offset, is16Bit, isFixed, shift) : readValueFromStream(is16Bit, isFixed, shift);
			if (values[i] == INFINITY) {
				return false;
			}
		}
		return true;
	};

	auto readMatrixFromBuffer = [getMultiBufferStream, matrixSize](uint16_t sourceBufferId, void *matrix) -> bool {
		auto buff = getMultiBufferStream(sourceBufferId);
		if (!buff) {
			return false;
		}
		if (buff->size() < matrixSize) {
			debug_log("bufferAffineTransform: buffer %d too small for matrix\n\r", sourceBufferId);
			return false;
		}
		auto read = buff->readBytes((uint8_t *)matrix, matrixSize);
		if (read != matrixSize) {
			debug_log("bufferAffineTransform: failed to read 9 floats from buffer %d\n\r", sourceBufferId);
			return false;
		}
		return true;
	};

	bool replace = false;
	switch (op) {
		case AFFINE_IDENTITY: {
			// create an identity matrix
			replace = true;
		}	break;
		case AFFINE_INVERT: {
			// this will only work if we have a 3x3 matrix...
			// first of all, get our existing matrix
			if (!readMatrixFromBuffer(bufferId, &transform)) {
				return;
			}
			auto matrix = dspm::Mat(transform, 3, 3).inverse();
			// copy data from matrix back to our working transform matrix
			memcpy(transform, matrix.data, matrixSize);
			replace = true;
		}	break;
		case AFFINE_ROTATE:
		case AFFINE_ROTATE_RAD:
		{
			// rotate anticlockwise (given inverted Y axis) by a given angle in degrees
			float angle = readFloatArgument();
			if (angle == INFINITY) {
				return;
			}
			if (op == AFFINE_ROTATE) angle = DEG_TO_RAD * angle;
			const auto cosAngle = cosf(angle);
			const auto sinAngle = sinf(angle);
			transform[0] = cosAngle;
			transform[1] = sinAngle;
			transform[3] = -sinAngle;
			transform[4] = cosAngle;
			transform[8] = 1.0f;
		}	break;
		case AFFINE_MULTIPLY: {
			// multiply values in a matrix by (single) argument value
			// fetch matrix buffer, then multiply all values except last row by value
			float scalar = readFloatArgument();
			if (scalar == INFINITY) {
				return;
			}
			if (!readMatrixFromBuffer(bufferId, &transform)) {
				return;
			}
			// scale transform matrix by scalar, skipping last value (as that needs to remain a 1)
			for (int i = 0; i < 7; i++) {
				transform[i] *= scalar;
			}
			replace = true;
		}	break;
		case AFFINE_SCALE: {
			// scale by a given factor
			// get an array of 2 float values
			float scales[2] = {0.0f, 0.0f};
			if (!readMultipleArgs(scales, 2)) {
				return;
			}
			transform[0] = scales[0];
			transform[4] = scales[1];
			transform[8] = 1.0f;
		}	break;
		case AFFINE_TRANSLATE: {
			// translate by a given amount of pixels
			float translateXY[2] = {0.0f, 0.0f};
			if (!readMultipleArgs(translateXY, 2)) {
				return;
			}
			transform[2] = translateXY[0];
			transform[5] = translateXY[1];
			transform[8] = 1.0f;
		}	break;
		case AFFINE_TRANSLATE_OS_COORDS: {
			// translate by a given amount of pixels
			float translateXY[2] = {0.0f, 0.0f};
			if (!readMultipleArgs(translateXY, 2)) {
				return;
			}
			auto scaled = context->scale(translateXY[0], translateXY[1]);
			transform[2] = scaled.X;
			transform[5] = scaled.Y;
			transform[8] = 1.0f;
		}	break;
		case AFFINE_SHEAR: {
			// shear by a given amount
			float shearXY[2] = {0.0f, 0.0f};
			if (!readMultipleArgs(shearXY, 2)) {
				return;
			}
			transform[1] = shearXY[0];
			transform[3] = shearXY[1];
			transform[8] = 1.0f;
		}	break;
		case AFFINE_SKEW: {
			// skew by a given amount (angle)
			float skewXY[2] = {0.0f, 0.0f};
			if (!readMultipleArgs(skewXY, 2)) {
				return;
			}
			transform[1] = tanf(DEG_TO_RAD * skewXY[0]);
			transform[3] = tanf(DEG_TO_RAD * skewXY[1]);
			transform[8] = 1.0f;
		}	break;
		case AFFINE_SKEW_RAD: {
			// skew by a given amount (angle)
			float skewXY[2] = {0.0f, 0.0f};
			if (!readMultipleArgs(skewXY, 2)) {
				return;
			}
			transform[1] = tanf(skewXY[0]);
			transform[3] = tanf(skewXY[1]);
			transform[8] = 1.0f;
		}	break;
		case AFFINE_TRANSFORM: {
			// we'll either be creating a new matrix, or combining with an existing one
			if (!readMultipleArgs(transform, 6)) {
				return;
			}
			transform[8] = 1.0f;
		}	break;
		default:
			debug_log("bufferAffineTransform: unknown operation %d\n\r", op);
			return;
	}

	auto bufferStream = make_shared_psram<BufferStream>(matrixSize);
	if (!bufferStream || !bufferStream->getBuffer()) {
		// buffer couldn't be created
		debug_log("bufferAffineTransform: failed to create buffer %d\n\r", bufferId);
		return;
	}

	if (!replace) {
		// we are combining, if we have an existing matrix
		float existing[9] = {0.0f};
		if (readMatrixFromBuffer(bufferId, &existing)) {
			// combine the two matrices together
			float newTransform[9] = {0.0f};
			dspm_mult_f32(transform, existing, newTransform, 3, 3, 3);
			// copy data from matrix back to our working transform matrix
			memcpy(transform, newTransform, matrixSize);
		}
	}

	bufferStream->writeBuffer((uint8_t *)transform, matrixSize);
	bufferClear(bufferId);
	buffers[bufferId].push_back(std::move(bufferStream));
	debug_log("bufferAffineTransform: created new matrix buffer %d\n\r", bufferId);

	debug_log(" %f %f %f\n\r", transform[0], transform[1], transform[2]);
	debug_log(" %f %f %f\n\r", transform[3], transform[4], transform[5]);
	debug_log(" %f %f %f\n\r", transform[6], transform[7], transform[8]);
}


// VDU 23, 0, &A0, bufferId; &40, sourceBufferId; : Compress blocks from a buffer
// Compress (blocks from) a buffer into a new buffer.
// Replaces the target buffer with the new one.
//
void VDUStreamProcessor::bufferCompress(uint16_t bufferId, uint16_t sourceBufferId) {
	debug_log("Compressing into buffer %u\n\r", bufferId);

	auto sourceBufferIter = buffers.find(sourceBufferId);
	if (sourceBufferIter == buffers.end()) {
		debug_log("bufferCompress: buffer %d not found\n\r", sourceBufferId);
		return;
	}

	// create a temporary output buffer, which may be expanded during compression
	uint8_t* p_temp = (uint8_t*) ps_malloc(COMPRESSION_OUTPUT_CHUNK_SIZE);
	if (p_temp) {
		// prepare for doing compression
		CompressionData cd;
		agon_init_compression(&cd, &p_temp, &local_write_compressed_byte);

		// Output the compression header
		CompressionFileHeader hdr;
		hdr.marker[0] = 'C';
		hdr.marker[1] = 'm';
		hdr.marker[2] = 'p';
		hdr.type = COMPRESSION_TYPE_TURBO;
		hdr.orig_size = 0;

		auto p_hdr_bytes = hdr.marker;
		for (int i = 0; i < sizeof(hdr); i++) {
			local_write_compressed_byte(&cd, *p_hdr_bytes++);
		}

		// loop thru blocks stored against the source buffer ID
		uint32_t orig_size = 0;
		auto &sourceBuffer = sourceBufferIter->second;
		for (const auto &block : sourceBuffer) {
			// compress the block into our temporary buffer
			auto bufferLength = block->size();
			auto p_data = block->getBuffer();
			orig_size += bufferLength;
			debug_log(" from buffer %u [%08X] %u bytes\n\r", sourceBufferId, p_data, bufferLength);
			debug_log(" %02hX %02hX %02hX %02hX\n\r",
						p_data[0], p_data[1], p_data[2], p_data[3]);
			cd.input_count += bufferLength;
			while (bufferLength--) {
				agon_compress_byte(&cd, *p_data++);
			}
		}
		agon_finish_compression(&cd);

		// make a single buffer with all of the temporary output data
		auto bufferStream = make_shared_psram<BufferStream>(cd.output_count);
		if (!bufferStream || !bufferStream->getBuffer()) {
			// buffer couldn't be created
			debug_log("bufferCompress: failed to create buffer %d\n\r", bufferId);
			return;
		}

		auto p_hdr = (CompressionFileHeader*) p_temp;
		p_hdr->orig_size = orig_size;

		auto destination = bufferStream->getBuffer();
		debug_log(" %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX\n\r",
					p_temp[0], p_temp[1], p_temp[2], p_temp[3],
					p_temp[4], p_temp[5], p_temp[6], p_temp[7],
					p_temp[8], p_temp[9], p_temp[10], p_temp[11]);
		memcpy(destination, p_temp, cd.output_count);
		heap_caps_free(p_temp);
		bufferClear(bufferId);
		buffers[bufferId].push_back(bufferStream);

		uint32_t pct = (cd.output_count * 100) / cd.input_count;
		debug_log("Compressed %u input bytes to %u output bytes (%u%%) at %08X\n\r",
				cd.input_count, cd.output_count, pct, destination);
	} else {
		debug_log("bufferCompress: cannot allocate temporary buffer of %d bytes\n\r", COMPRESSION_OUTPUT_CHUNK_SIZE);
	}
}

// VDU 23, 0, &A0, bufferId; &41, sourceBufferId; : Decompress blocks from a buffer
// Decompress (blocks from) a buffer into a new buffer.
// Replaces the target buffer with the new one.
//
void VDUStreamProcessor::bufferDecompress(uint16_t bufferId, uint16_t sourceBufferId) {
	#ifdef DEBUG
	auto start = millis();
	#endif
	auto sourceBufferIter = buffers.find(sourceBufferId);
	if (sourceBufferIter == buffers.end()) {
		debug_log("bufferDeompress: buffer %d not found\n\r", sourceBufferId);
		return;
	}
	auto &sourceBuffer = sourceBufferIter->second;

	// Validate the compression header
	if (sourceBuffer.size() >= 1 && sourceBuffer[0]->size() < sizeof(CompressionFileHeader)) {
		debug_log("bufferDecompress: buffer too small for header\n\r");
		return;
	}
	
	auto p_hdr = (const CompressionFileHeader*) sourceBuffer[0]->getBuffer();
	if (p_hdr->marker[0] != 'C' ||
		p_hdr->marker[1] != 'm' ||
		p_hdr->marker[2] != 'p' ||
		p_hdr->type != COMPRESSION_TYPE_TURBO) {
		debug_log("bufferDecompress: header is invalid\n\r");
		return;
	}
	auto orig_size = p_hdr->orig_size;

	debug_log("Decompressing into buffer %u\n\r", bufferId);

	// create output buffer
	auto bufferStream = make_shared_psram<BufferStream>(orig_size);
	if (!bufferStream || !bufferStream->getBuffer()) {
		// buffer couldn't be created
		debug_log("bufferDecompress: failed to create buffer %d\n\r", bufferId);
		return;
	}

	// prepare for doing compression
	auto buffer = bufferStream->getBuffer();
	DecompressionData dd;
	agon_init_decompression(&dd, &buffer, &local_write_decompressed_byte, orig_size);

	// loop thru blocks stored against the source buffer ID
	uint32_t skip_hdr = sizeof(CompressionFileHeader);
	dd.input_count = skip_hdr;
	for (const auto &block : sourceBuffer) {
		// decompress the block into our temporary buffer
		auto bufferLength = block->size() - skip_hdr;
		auto p_data = block->getBuffer();
		debug_log(" from buffer %u [%08X] %u bytes\n\r", sourceBufferId, p_data, bufferLength);
		debug_log(" %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX\n\r",
					p_data[0], p_data[1], p_data[2], p_data[3],
					p_data[4], p_data[5], p_data[6], p_data[7],
					p_data[8], p_data[9], p_data[10], p_data[11]);
		p_data += skip_hdr;
		skip_hdr = 0;
		dd.input_count += bufferLength;
		while (bufferLength--) {
			agon_decompress_byte(&dd, *p_data++);
		}
	}

	debug_log(" %02hX %02hX %02hX %02hX\n\r",
				buffer[0], buffer[1], buffer[2], buffer[3]);
	bufferClear(bufferId);
	buffers[bufferId].push_back(bufferStream);

	uint32_t pct = (dd.output_count * 100) / dd.input_count;
	debug_log("Decompressed %u input bytes to %u output bytes (%u%%) at %08X\n\r",
				dd.input_count, dd.output_count, pct, buffer);

	if (dd.output_count != orig_size) {
		debug_log("Decompressed buffer size %u does not equal original size %u\r\n",
					dd.output_count, orig_size);
	}
	#ifdef DEBUG
	debug_log("Decompress took %u ms\n\r", millis() - start);
	#endif
}

// VDU 23, 0, &A0, bufferId; &48, options, sourceBufferId; [width;] [mapBufferId;] [mapValues...] : Expand a bitmap buffer
// Expands a bitmap buffer into a new buffer with 8-bit values
// options dictates how the expansion is done
// width will be provided to give a pixel width at which a byte-align is done
//
void VDUStreamProcessor::bufferExpandBitmap(uint16_t bufferId, uint8_t options, uint16_t sourceBufferId) {
	auto sourceBufferIter = buffers.find(sourceBufferId);
	if (sourceBufferIter == buffers.end()) {
		debug_log("bufferExpandBitmap: source buffer %d not found\n\r", sourceBufferId);
		return;
	}
	auto &sourceBuffer = sourceBufferIter->second;

	// pixelSize is our number of bits in a pixel
	auto pixelSize = options & EXPAND_BITMAP_SIZE;
	if (pixelSize == 0) {
		pixelSize = 8;
	}
	// do we have an aligned width?
	bool aligned = options & EXPAND_BITMAP_ALIGNED;
	// do we have a map buffer, or are we just reading the values from the stream?
	bool useBuffer = options & EXPAND_BITMAP_USEBUFFER;
	int16_t width = -1;

	uint8_t * mapValues = nullptr;

	if (aligned) {
		width = readWord_t();
		if (width == -1) {
			debug_log("bufferExpandBitmap: failed to read width\n\r");
			return;
		}
	}

	auto numValues = 1 << pixelSize;

	if (useBuffer) {
		auto mapId = readWord_t();
		if (mapId == -1) {
			debug_log("bufferExpandBitmap: failed to read map buffer ID\n\r");
			return;
		}
		auto bufferIter = buffers.find(mapId);
		if (bufferIter == buffers.end()) {
			debug_log("bufferExpandBitmap: map buffer %d not found\n\r", mapId);
			return;
		}
		auto &buffer = bufferIter->second;
		if (buffer.size() != 1) {
			debug_log("bufferExpandBitmap: map buffer %d does not contain a single block\n\r", mapId);
			return;
		}
		if (buffer[0]->size() < numValues) {
			debug_log("bufferExpandBitmap: map buffer %d does not contain at least %d values\n\r", mapId, numValues);
			return;
		}
		mapValues = buffer[0]->getBuffer();
	} else {
		// read pixelSize bytes from stream
		mapValues = (uint8_t *) ps_malloc(numValues);
		if (!mapValues) {
			debug_log("bufferExpandBitmap: failed to allocate map values\n\r");
			return;
		}
		if (readIntoBuffer(mapValues, 1 << pixelSize) != 0) {
			debug_log("bufferExpandBitmap: failed to read map values\n\r");
			free(mapValues);
			return;
		}
		debug_log("bufferExpandBitmap: read map values ");
		for (int i = 0; i < numValues; i++) {
			debug_log("%02hX ", mapValues[i]);
		}
		debug_log("\n\r");
	}

	// work out source size
	uint32_t sourceSize = 0;
	for (const auto &block : sourceBuffer) {
		sourceSize += block->size();
	}

	// if we are aligning we need to work out our byte width, based off the pixel width
	uint32_t byteWidth = 0;	
	if (aligned) {
		byteWidth = ((pixelSize * width) + (8 - pixelSize)) / 8;
	}

	// work out our output size
	uint32_t outputSize = 0;
	if (aligned) {
		outputSize = (sourceSize / byteWidth) * width;
	} else {
		outputSize = (sourceSize * 8) / pixelSize;
	}

	debug_log("bufferExpandBitmap: source size %d, output size %d, pixel size %d, width %d, byte width %d\n\r",
		sourceSize, outputSize, pixelSize, width, byteWidth);

	// create output buffer
	auto bufferStream = make_shared_psram<BufferStream>(outputSize);

	if (!bufferStream || !bufferStream->getBuffer()) {
		// buffer couldn't be created
		debug_log("bufferExpandBitmap: failed to create buffer %d\n\r", bufferId);
		if (!useBuffer) {
			free(mapValues);
		}
		return;
	}

	auto destination = bufferStream->getBuffer();

	// iterate through source buffer
	auto p_data = destination;
	uint8_t bit = 0;
	uint8_t pixel = 0;
	uint16_t pixelCount = 0;
	for (const auto &block : sourceBuffer) {
		auto bufferLength = block->size();
		auto p_source = block->getBuffer();

		// go through one byte at a time,
		// and expand the pixels into the destination buffer
		// aligning when our pixel count reaches our pixel width if required

		while (bufferLength--) {
			auto value = *p_source++;
			for (uint8_t i = 0; i < 8; i++) {
				debug_log("pixel bit is %d ", ((value >> 7 - i) & 1));
				pixel = (pixel << 1) | ((value >> (7 - i)) & 1);
				bit++;
				if (bit == pixelSize) {
					bit = 0;
					*p_data++ = mapValues[pixel];
					debug_log(" %02hX %02hX (%02hX) %d\n\r", pixel, mapValues[pixel], value, i);
					pixel = 0;
					if (aligned) {
						if (++pixelCount == width) {
							// byte align
							debug_log("aligned... skipping to next byte at byte bit %d\n\r", i);
							pixelCount = 0;
							// jump to next byte
							break;
						}
					}
				}
			}
		}
	}

	// save our bufferStream to the buffer
	bufferClear(bufferId);
	buffers[bufferId].push_back(std::move(bufferStream));
	if (!useBuffer) {
		free(mapValues);
	}
	debug_log("bufferExpandBitmap: expanded %d bytes into buffer %d\n\r", outputSize, bufferId);
}

// VDU 23, 0, &A0, bufferId; &49, subcommand - Configure/render using Pingo 3D
//
// VDU 23, 0, &A0, sid; &49, 0, w; h; :  Create Control Structure
// VDU 23, 0, &A0, sid; &49, 1, mid; n; x0; y0; z0; ... :  Define Mesh Vertices
// VDU 23, 0, &A0, sid; &49, 2, mid; n; i0; ... :  Set Mesh Vertex Indexes
// VDU 23, 0, &A0, sid; &49, 3, mid; n; u0; v0; ... :  Define Mesh Texture Coordinates
// VDU 23, 0, &A0, sid; &49, 4, mid; n; i0; ... :  Set Texture Coordinate Indexes
// VDU 23, 0, &A0, sid; &49, 5, oid; mid; bmid; :  Create Object
// VDU 23, 0, &A0, sid; &49, 40, oid; n; u0; v0; ... :  Define Object Texture Coordinates
// VDU 23, 0, &A0, sid; &49, 6, oid; scalex; :  Set Object X Scale Factor
// VDU 23, 0, &A0, sid; &49, 7, oid; scaley; :  Set Object Y Scale Factor
// VDU 23, 0, &A0, sid; &49, 8, oid; scalez; :  Set Object Z Scale Factor
// VDU 23, 0, &A0, sid; &49, 9, oid; scalex; scaley; scalez :  Set Object XYZ Scale Factors
// VDU 23, 0, &A0, sid; &49, 10, oid; anglex; :  Set Object X Rotation Angle
// VDU 23, 0, &A0, sid; &49, 11, oid; angley; :  Set Object Y Rotation Angle
// VDU 23, 0, &A0, sid; &49, 12, oid; anglez; :  Set Object Z Rotation Angle
// VDU 23, 0, &A0, sid; &49, 13, oid; anglex; angley; anglez; :  Set Object XYZ Rotation Angles
// VDU 23, 0, &A0, sid; &49, 14, oid; distx; :  Set Object X Translation Distance
// VDU 23, 0, &A0, sid; &49, 15, oid; disty; :  Set Object Y Translation Distance
// VDU 23, 0, &A0, sid; &49, 16, oid; distz; :  Set Object Z Translation Distance
// VDU 23, 0, &A0, sid; &49, 17, oid; distx; disty; distz :  Set Object XYZ Translation Distances
// VDU 23, 0, &A0, sid; &49, 18, anglex;</b> :  Set Camera X Rotation Angle
// VDU 23, 0, &A0, sid; &49, 19, angley;</b> :  Set Camera Y Rotation Angle
// VDU 23, 0, &A0, sid; &49, 20, anglez;</b> :  Set Camera Z Rotation Angle
// VDU 23, 0, &A0, sid; &49, 21, anglex; angley; anglez;</b> :  Set Camera XYZ Rotation Angles
// VDU 23, 0, &A0, sid; &49, 22, distx;</b> :  Set Camera X Translation Distance
// VDU 23, 0, &A0, sid; &49, 23, disty;</b> :  Set Camera Y Translation Distance
// VDU 23, 0, &A0, sid; &49, 24, distz;</b> :  Set Camera Z Translation Distance
// VDU 23, 0, &A0, sid; &49, 25, distx; disty; distz;</b> :  Set Camera XYZ Translation Distances
// VDU 23, 0, &A0, sid; &49, 26, scalex;</b> :  Set Scene X Scale Factor
// VDU 23, 0, &A0, sid; &49, 27, scaley;</b> :  Set Scene Y Scale Factor
// VDU 23, 0, &A0, sid; &49, 28, scalez;</b> :  Set Scene Z Scale Factor
// VDU 23, 0, &A0, sid; &49, 29, scalex; scaley; scalez;</b> :  Set Scene XYZ Scale Factors
// VDU 23, 0, &A0, sid; &49, 30, anglex;</b> :  Set Scene X Rotation Angle
// VDU 23, 0, &A0, sid; &49, 31, angley;</b> :  Set Scene Y Rotation Angle
// VDU 23, 0, &A0, sid; &49, 32, anglez;</b> :  Set Scene Z Rotation Angle
// VDU 23, 0, &A0, sid; &49, 33, anglex; angley; anglez;</b> :  Set Scene XYZ Rotation Angles
// VDU 23, 0, &A0, sid; &49, 34, distx;</b> :  Set Scene X Translation Distance
// VDU 23, 0, &A0, sid; &49, 35, disty;</b> :  Set Scene Y Translation Distance
// VDU 23, 0, &A0, sid; &49, 36, distz;</b> :  Set Scene Z Translation Distance
// VDU 23, 0, &A0, sid; &49, 37, distx; disty; distz;</b> :  Set Scene XYZ Translation Distances
// VDU 23, 0, &A0, sid; &49, 38, bmid; :  Render To Bitmap
// VDU 23, 0, &A0, sid; &49, 39 :  Delete Control Structure (not implemented yet)
//
void VDUStreamProcessor::bufferUsePingo3D(uint16_t bufferId) {
    auto subcmd = readByte_t();
    if (subcmd == 0) {
		// Create the buffer if necessary
		// Initialize the control structure
		auto w = readWord_t();
		if (w > 0) {
			auto h = readWord_t();
			if (h > 0) {
				auto buffer = bufferCreate(bufferId, sizeof(Pingo3dControl));
				if (buffer) {
					auto ctrl = (Pingo3dControl*) buffer->getBuffer();
					ctrl->initialize(*this, (uint16_t)w, (uint16_t)h);
				}
			} else {
				debug_log("bufferUsePingo3D: buffer %d missing height\n\r", bufferId);
			}
		} else {
			debug_log("bufferUsePingo3D: buffer %d missing width\n\r", bufferId);
		}
	} else if (subcmd == 39) {
		// Deinitialize the control structure
		// Delete the buffer
		auto bufferIter = buffers.find(bufferId);
		if (bufferIter != buffers.end()) {
			auto &buffer = bufferIter->second;
			auto ctrl = (Pingo3dControl*) buffer.begin()->get()->getBuffer();
			if (ctrl->validate()) {
				ctrl->deinitialize(*this);
				buffers.erase(bufferIter);
			} else {
				debug_log("bufferUsePingo3D: buffer %d is invalid\n\r", bufferId);
			}
		} else {
			debug_log("bufferUsePingo3D: buffer %d not found\n\r", bufferId);
		}            
    } else {
        auto bufferIter = buffers.find(bufferId);
        if (bufferIter != buffers.end()) {
			auto &buffer = bufferIter->second;
			auto ctrl = (Pingo3dControl*) buffer.begin()->get()->getBuffer();
            if (ctrl->validate()) {
                ctrl->handle_subcommand(*this, subcmd);
            } else {
           		debug_log("bufferUsePingo3D: buffer %d is invalid\n\r", bufferId);
            }
        } else {
    		debug_log("bufferUsePingo3D: buffer %d not found\n\r", bufferId);
        }
    }
}

#endif // VDU_BUFFERED_H

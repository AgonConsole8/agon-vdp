#ifndef VDU_STREAM_PROCESSOR_H
#define VDU_STREAM_PROCESSOR_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <Stream.h>
#include <fabgl.h>

#include "agon.h"
#include "buffers.h"
#include "context.h"
#include "buffer_stream.h"
#include "span.h"
#include "types.h"

using ContextVector = std::vector<std::shared_ptr<Context>, psram_allocator<std::shared_ptr<Context>>>;
using ContextVectorPtr = std::shared_ptr<ContextVector>;
std::unordered_map<uint16_t, ContextVectorPtr,
	std::hash<uint16_t>, std::equal_to<uint16_t>,
	psram_allocator<std::pair<const uint16_t, ContextVectorPtr>>> contextStacks;

class VDUStreamProcessor {
	private:
		std::shared_ptr<Stream> inputStream;
		std::shared_ptr<Stream> outputStream;
		std::shared_ptr<Stream> originalOutputStream;

		// Graphics context storage and management
		std::shared_ptr<Context> context;		// Current active context
		ContextVectorPtr contextStack;			// Current active context stack

		bool commandsEnabled = true;

		int16_t readByte_t(uint16_t timeout);
		int32_t readWord_t(uint16_t timeout);
		int32_t read24_t(uint16_t timeout);
		uint8_t readByte_b();
		uint32_t readIntoBuffer(uint8_t * buffer, uint32_t length, uint16_t timeout);
		uint32_t discardBytes(uint32_t length, uint16_t timeout);
		int16_t peekByte_t(uint16_t timeout);
		float readFloat_t(bool is16Bit, bool isFixed, int8_t shift, uint16_t timeout);
		bool readFloatArguments(float *values, int count, bool useBufferValue, bool useAdvancedOffsets, bool useMultiFormat);

		void vdu_print(char c, bool usePeek);
		void vdu_colour();
		void vdu_gcol();
		void vdu_palette();
		void vdu_mode();
		void vdu_graphicsViewport();
		void vdu_plot();
		void vdu_resetViewports();
		void vdu_textViewport();
		void vdu_origin();
		void vdu_cursorTab();

		void vdu_sys();
		void vdu_sys_video();
		void sendGeneralPoll();
		void vdu_sys_video_kblayout();
		void sendCursorPosition();
		void sendScreenChar(char c);
		void sendScreenPixel(uint16_t x, uint16_t y);
		void sendColour(uint8_t colour);
		void printBuffer(uint16_t bufferId);
		void sendTime();
		void vdu_sys_video_time();
		void sendKeyboardState();
		void vdu_sys_keystate();
		void vdu_sys_mouse();
		void vdu_sys_scroll();
		void vdu_sys_cursorBehaviour();
		void vdu_sys_udg(char c);

		void vdu_sys_audio();
		void sendAudioStatus(uint8_t channel, uint8_t status);
		uint8_t loadSample(uint16_t bufferId, uint32_t length);
		uint8_t createSampleFromBuffer(uint16_t bufferId, uint8_t format, uint16_t sampleRate);
		uint8_t setVolumeEnvelope(uint8_t channel, uint8_t type);
		uint8_t setFrequencyEnvelope(uint8_t channel, uint8_t type);
		uint8_t setSampleFrequency(uint16_t bufferId, uint16_t frequency);
		uint8_t setSampleRepeatStart(uint16_t bufferId, uint32_t offset);
		uint8_t setSampleRepeatLength(uint16_t bufferId, uint32_t length);
		uint8_t setParameter(uint8_t channel, uint8_t parameter, uint16_t value);

		void vdu_sys_font();

		void vdu_sys_context();
		void selectContext(uint8_t contextId);
		bool resetContext(uint8_t flags);
		void saveContext();
		void restoreContext();
		void saveAndSelectContext(uint8_t contextId);
		void restoreAllContexts();
		void clearContextStack();
		void resetAllContexts();

		void vdu_sys_sprites();
		void receiveBitmap(uint16_t bufferId, uint16_t width, uint16_t height);
		void createBitmapFromScreen(uint16_t bufferId);
		void createEmptyBitmap(uint16_t bufferId, uint16_t width, uint16_t height, uint32_t color);
		void createBitmapFromBuffer(uint16_t bufferId, uint8_t format, uint16_t width, uint16_t height);

		void vdu_sys_hexload(void);
		void sendKeycodeByte(uint8_t b, bool waitack);

		void vdu_sys_buffered();
		uint32_t bufferWrite(uint16_t bufferId, uint32_t size);
		void bufferCall(uint16_t bufferId, AdvancedOffset offset);
		void bufferRemoveUsers(uint16_t bufferId);
		void bufferClear(uint16_t bufferId);
		std::shared_ptr<WritableBufferStream> bufferCreate(uint16_t bufferId, uint32_t size);
		void setOutputStream(uint16_t bufferId);
		AdvancedOffset getOffsetFromStream(bool isAdvanced);
		std::vector<uint16_t> getBufferIdsFromStream();
		void bufferAdjust(uint16_t bufferId);
		bool bufferConditional();
		void bufferJump(uint16_t bufferId, AdvancedOffset offset);
		void bufferCopy(uint16_t bufferId, tcb::span<const uint16_t> sourceBufferIds);
		void bufferConsolidate(uint16_t bufferId);
		void clearTargets(tcb::span<const uint16_t> targets);
		void bufferSplitInto(uint16_t bufferId, uint16_t length, tcb::span<uint16_t> newBufferIds, bool iterate);
		void bufferSplitByInto(uint16_t bufferId, uint16_t width, uint16_t chunkCount, tcb::span<uint16_t> newBufferIds, bool iterate);
		void bufferSpreadInto(uint16_t bufferId, tcb::span<uint16_t> newBufferIds, bool iterate);
		void bufferReverseBlocks(uint16_t bufferId);
		void bufferReverse(uint16_t bufferId, uint8_t options);
		void bufferCopyRef(uint16_t bufferId, tcb::span<const uint16_t> sourceBufferIds);
		void bufferCopyAndConsolidate(uint16_t bufferId, tcb::span<const uint16_t> sourceBufferIds);
		void bufferAffineTransform(uint16_t bufferId, uint8_t command, bool is3D);
		void bufferMatrixManipulate(uint16_t bufferId, uint8_t command, MatrixSize size);
		void bufferTransformBitmap(uint16_t bufferId, uint8_t options, uint16_t transformBufferId, uint16_t sourceBufferId);
		void bufferTransformData(uint16_t bufferId, uint8_t options, uint8_t format, uint16_t transformBufferId, uint16_t sourceBufferId);
		void bufferCompress(uint16_t bufferId, uint16_t sourceBufferId);
		void bufferDecompress(uint16_t bufferId, uint16_t sourceBufferId);
		void bufferExpandBitmap(uint16_t bufferId, uint8_t options, uint16_t sourceBufferId);

		void vdu_sys_updater();
		void unlock();
		void receiveFirmware();
		void switchFirmware();

	public:
		uint16_t id = 65535;

		VDUStreamProcessor(std::shared_ptr<Context> _context, std::shared_ptr<Stream> input, std::shared_ptr<Stream> output, uint16_t bufferId) :
			context(_context), inputStream(std::move(input)), outputStream(std::move(output)), originalOutputStream(outputStream), id(bufferId) {
				// NB this will become obsolete when merging in the buffered command optimisations
				context = make_shared_psram<Context>(*_context);
				contextStack = make_shared_psram<ContextVector>();
				contextStack->push_back(context);
			}
		VDUStreamProcessor(Stream *input) :
			inputStream(std::shared_ptr<Stream>(input)), outputStream(inputStream), originalOutputStream(inputStream) {
				context = make_shared_psram<Context>();
				contextStack = make_shared_psram<ContextVector>();
				contextStack->push_back(context);
			}

		inline bool byteAvailable() {
			return inputStream->available() > 0;
		}
		inline uint8_t readByte() {
			return inputStream->read();
		}
		inline void writeByte(uint8_t b) {
			if (outputStream) {
				outputStream->write(b);
			}
		}
		void send_packet(uint8_t code, uint16_t len, uint8_t data[]);

		void sendMouseData(MouseDelta * delta);

		void processAllAvailable();
		void processNext();
		void doCursorFlash() {
			context->doCursorFlash();
		}
		void hideCursor() {
			context->hideCursor();
		}
		void showCursor() {
			context->showCursor();
		}

		void vdu(uint8_t c, bool usePeek = true);

		void wait_eZ80();
		void sendModeInformation();

		std::shared_ptr<Context> getContext() {
			return context;
		}
		bool contextExists(uint8_t id) {
			return contextStacks.find(id) != contextStacks.end();
		}
};

// Read an unsigned byte from the serial port, with a timeout
// Returns:
// - Byte value (0 to 255) if value read, otherwise -1
//
int16_t inline VDUStreamProcessor::readByte_t(uint16_t timeout = COMMS_TIMEOUT) {
	auto read = inputStream->read();
	if (read != -1) {
		return read;
	}

	auto start = xTaskGetTickCountFromISR();
	const auto timeCheck = pdMS_TO_TICKS(timeout);

	do {
		read = inputStream->read();
		if (read != -1) {
			return read;
		}
	} while (xTaskGetTickCountFromISR() - start < timeCheck);

	return read;
}

// Read an unsigned word from the serial port, with a timeout
// Returns:
// - Word value (0 to 65535) if 2 bytes read, otherwise -1
//
int32_t VDUStreamProcessor::readWord_t(uint16_t timeout = COMMS_TIMEOUT) {
	auto l = readByte_t(timeout);
	if (l != -1) {
		auto h = readByte_t(timeout);
		if (h != -1) {
			return (h << 8) | l;
		}
	}
	return -1;
}

// Read an unsigned 24-bit value from the serial port, with a timeout
// Returns:
// - Value (0 to 16777215) if 3 bytes read, otherwise -1
//
int32_t VDUStreamProcessor::read24_t(uint16_t timeout = COMMS_TIMEOUT) {
	auto l = readByte_t(timeout);
	if (l != -1) {
		auto m = readByte_t(timeout);
		if (m != -1) {
			auto h = readByte_t(timeout);
			if (h != -1) {
				return (h << 16) | (m << 8) | l;
			}
		}
	}
	return -1;
}

// Read an unsigned byte from the serial port (blocking)
//
uint8_t VDUStreamProcessor::readByte_b() {
	while (inputStream->available() == 0);
	return readByte();
}

// Read a given number of bytes from the serial port into a buffer
// Returns number of remaining bytes
// which should be zero if all bytes were read
// but will be non-zero if the read timed out
//
uint32_t VDUStreamProcessor::readIntoBuffer(uint8_t * buffer, uint32_t length, uint16_t timeout = COMMS_TIMEOUT) {
	uint32_t remaining = length;
	if (buffer == nullptr) {
		debug_log("readIntoBuffer: buffer is null\n\r");
		return remaining;
	}

	while (remaining > 0) {
		auto read = inputStream->readBytes(buffer, remaining);
		if (read == 0) {
			// timed out - perform a single retry
			read = inputStream->readBytes(buffer, remaining);
			if (read == 0) {
				debug_log("readIntoBuffer: timed out\n\r");
				return remaining;
			}
		}
		buffer += read;
		remaining -= read;
	}
	return remaining;
}

// Discard a given number of bytes from input stream
// Returns 0 on success, or the number of bytes remaining if timed out
//
uint32_t VDUStreamProcessor::discardBytes(uint32_t length, uint16_t timeout = COMMS_TIMEOUT) {
	uint32_t remaining = length;
	auto bufferSize = 64;
	auto readSize = bufferSize;
	auto buffer = make_unique_psram_array<uint8_t>(bufferSize);

	while (remaining > 0) {
		if (remaining < readSize) {
			readSize = remaining;
		}
		if (readIntoBuffer(buffer.get(), readSize, timeout) != 0) {
			// timed out
			return remaining;
		}
		remaining -= readSize;
	}
	return remaining;
}

// Peek at the next byte in the command stream
// returns -1 if timed out, or the byte value (0 to 255)
//
int16_t VDUStreamProcessor::peekByte_t(uint16_t timeout = COMMS_TIMEOUT) {
	auto start = xTaskGetTickCountFromISR();
	const auto timeCheck = pdMS_TO_TICKS(timeout);

	while (xTaskGetTickCountFromISR() - start < timeCheck) {
		if (inputStream->available() > 0) {
			return inputStream->peek();
		}
	}
	return -1;
}

// Read a float value from the stream, given the specified format
// Returns the float value, or INFINITY if timed out
//
float VDUStreamProcessor::readFloat_t(bool is16Bit, bool isFixed, int8_t shift, uint16_t timeout = COMMS_TIMEOUT) {
	// get the value that we're dealing with
	uint32_t rawValue = 0;
	auto bytesToRead = is16Bit ? 2 : 4;
	// read the value from the stream
	if (readIntoBuffer((uint8_t *)&rawValue, bytesToRead, timeout) != 0) {
		return INFINITY;
	}
	return convertValueToFloat(rawValue, is16Bit, isFixed, shift);
}

// Reads a series of float values from the stream
// Returns true if all values were read,
// or false if timed out or another issue occurred, such as a non-existant buffer was specified
// Stream will contain a format byte, followed by float values
// if useMultiFormat is true, then each float will be preceded by a format byte
// if useBufferValue is true, then the "value" will be a buffer ID and offset to fetch the value from
//
bool VDUStreamProcessor::readFloatArguments(float *values, int count, bool useBufferValue, bool useAdvancedOffsets, bool useMultiFormat) {
	bool isFixed, is16Bit;
	int8_t shift;
	int32_t sourceBufferId = -1;
	AdvancedOffset offset = {};

	for (int i = 0; i < count; i++) {
		if (i == 0 || useMultiFormat) {
			auto format = readByte_t();
			if (format == -1) {
				return false;
			}
			extractFormatInfo(format, isFixed, is16Bit, shift);
			if (useBufferValue) {
				sourceBufferId = readWord_t();
				if (sourceBufferId == -1) {
					return false;
				}
				offset = getOffsetFromStream(useAdvancedOffsets);
				if (offset.blockOffset == -1) {
					return false;
				}
			}
		}
		values[i] = useBufferValue ? readBufferFloat(sourceBufferId, offset, is16Bit, isFixed, shift, true) : readFloat_t(is16Bit, isFixed, shift);
		if (values[i] == INFINITY) {
			return false;
		}
	}
	return true;
};


// Send a packet of data to the MOS
//
void VDUStreamProcessor::send_packet(uint8_t code, uint16_t len, uint8_t data[]) {
	writeByte(code + 0x80);
	writeByte(len);
	for (int i = 0; i < len; i++) {
		writeByte(data[i]);
	}
}

void VDUStreamProcessor::sendMouseData(MouseDelta * delta = nullptr) {
	auto mouse = getMouse();
	uint16_t mouseX = 0;
	uint16_t mouseY = 0;
	uint8_t buttons = 0;
	uint8_t wheelDelta = 0;
	uint16_t deltaX = 0;
	uint16_t deltaY = 0;
	if (delta) {
		deltaX = delta->deltaX;
		deltaY = delta->deltaY;
	}
	if (mouse) {
		auto mStatus = mouse->status();
		auto mousePos = context->toCurrentCoordinates(mStatus.X, mStatus.Y);
		mouseX = mousePos.X;
		mouseY = mousePos.Y;
		buttons = mStatus.buttons.left << 0 | mStatus.buttons.right << 1 | mStatus.buttons.middle << 2;
		wheelDelta = mStatus.wheelDelta;
	}
	debug_log("sendMouseData: %d %d %d %d %d %d %d %d %d %d\n\r", mouseX, mouseY, buttons, wheelDelta, deltaX, deltaY);
	uint8_t packet[] = {
		(uint8_t) (mouseX & 0xFF),
		(uint8_t) ((mouseX >> 8) & 0xFF),
		(uint8_t) (mouseY & 0xFF),
		(uint8_t) ((mouseY >> 8) & 0xFF),
		(uint8_t) buttons,
		(uint8_t) wheelDelta,
		(uint8_t) (deltaX & 0xFF),
		(uint8_t) ((deltaX >> 8) & 0xFF),
		(uint8_t) (deltaY & 0xFF),
		(uint8_t) ((deltaY >> 8) & 0xFF),
	};
	send_packet(PACKET_MOUSE, sizeof packet, packet);
}
// Process all available commands from the stream
//
void VDUStreamProcessor::processAllAvailable() {
	while (byteAvailable()) {
		vdu(readByte());
	}
}

// Process next command from the stream
//
void VDUStreamProcessor::processNext() {
	if (byteAvailable()) {
		vdu(readByte());
	}
}

#include "vdu.h"

#endif // VDU_STREAM_PROCESSOR_H

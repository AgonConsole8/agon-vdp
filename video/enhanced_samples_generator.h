#ifndef ENHANCED_SAMPLES_GENERATOR_H
#define ENHANCED_SAMPLES_GENERATOR_H

#include <memory>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <fabgl.h>

#include "audio_sample.h"
#include "types.h"

// Enhanced samples generator
//
class EnhancedSamplesGenerator : public WaveformGenerator {
	public:
		EnhancedSamplesGenerator(std::shared_ptr<AudioSample> sample);

		void setFrequency(int value);
		void setSampleRate(int value);
		int getSample();
		void setSample(std::shared_ptr<AudioSample> sample);

		int getDuration(uint16_t frequency);

		void seekTo(uint32_t position);
	private:
		std::weak_ptr<AudioSample> _sample;

		// TODO we may need to use a mutex to protect these values
		uint32_t	index;				// Current index inside the current sample block
		uint32_t	blockIndex;			// Current index into the sample data blocks
		int32_t		repeatCount;		// Sample count when repeating
		// TODO consider whether repeatStart and repeatLength may need to be here
		// which would allow for per-channel repeat settings

		std::atomic<int>		frequency;
		std::atomic<int>		previousSample;
		std::atomic<int>		currentSample;
		std::atomic<double>		samplesPerGet;
		std::atomic<double>		fractionalSampleOffset;

		double calculateSamplerate(uint16_t frequency);
		int8_t getNextSample(std::shared_ptr<AudioSample> sample);
};

EnhancedSamplesGenerator::EnhancedSamplesGenerator(std::shared_ptr<AudioSample> sample)
	: _sample(sample), repeatCount(0), index(0), blockIndex(0), frequency(0), previousSample(0), currentSample(0), samplesPerGet(1.0), fractionalSampleOffset(0.0)
{}

void EnhancedSamplesGenerator::setFrequency(int value) {
	frequency = value;
	samplesPerGet = calculateSamplerate(value);
}

void EnhancedSamplesGenerator::setSampleRate(int value) {
	WaveformGenerator::setSampleRate(value);
	samplesPerGet = calculateSamplerate(frequency);
}

int EnhancedSamplesGenerator::getSample() {
	if (duration() == 0 || _sample.expired()) {
		return 0;
	}

	auto samplePtr = _sample.lock();
	if (!samplePtr) {
		return 0;
	}

	// if we've moved far enough along, read the next sample
	while (fractionalSampleOffset >= 1.0) {
		previousSample = currentSample.load();
		currentSample = getNextSample(samplePtr);
		fractionalSampleOffset = fractionalSampleOffset - 1.0;
	}

	 // Interpolate between the samples to reduce aliasing
	int sample = currentSample * fractionalSampleOffset + previousSample * (1.0-fractionalSampleOffset);

	fractionalSampleOffset = fractionalSampleOffset + samplesPerGet;

	// process volume
	sample = sample * volume() / 127;

	decDuration();

	return sample;
}

void EnhancedSamplesGenerator::setSample(std::shared_ptr<AudioSample> sample) {
	_sample = sample;
	seekTo(0);
}

int EnhancedSamplesGenerator::getDuration(uint16_t frequency) {
	// TODO this will produce an incorrect duration if the sample rate for the channel has been
	// adjusted to differ from the underlying audio system sample rate
	// At this point it's not clear how to resolve this, so we'll assume it hasn't been adjusted
	auto samplePtr = _sample.lock();
	if (!samplePtr) {
		return 0;
	}
	return (samplePtr->getSize() * 1000 / sampleRate()) / calculateSamplerate(frequency);
}

void EnhancedSamplesGenerator::seekTo(uint32_t position) {
	auto samplePtr = _sample.lock();
	if (samplePtr) {
		samplePtr->seekTo(position, index, blockIndex, repeatCount);

		// prepare our fractional sample data for playback
		fractionalSampleOffset = 0.0;
		previousSample = samplePtr->getSample(index, blockIndex);
		currentSample = samplePtr->getSample(index, blockIndex);
	}
}

double EnhancedSamplesGenerator::calculateSamplerate(uint16_t frequency) {
	auto samplePtr = _sample.lock();
	if (samplePtr) {
		auto baseFrequency = samplePtr->baseFrequency;
		auto frequencyAdjust = baseFrequency > 0 ? (double)frequency / (double)baseFrequency : 1.0;
		return frequencyAdjust * ((double)samplePtr->sampleRate / (double)(sampleRate()));
	}
	return 1.0;
}

int8_t EnhancedSamplesGenerator::getNextSample(std::shared_ptr<AudioSample> samplePtr) {
	auto sample = samplePtr->getSample(index, blockIndex);
	
	// looping magic
	repeatCount--;
	if (repeatCount == 0) {
		// we've reached the end of the repeat section, so loop back
		seekTo(samplePtr->repeatStart);
	}

	return sample;
}

#endif // ENHANCED_SAMPLES_GENERATOR_H

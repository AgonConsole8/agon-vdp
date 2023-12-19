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
		// std::weak_ptr<AudioSample> _sample;
		std::shared_ptr<AudioSample> _sample;

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
		int8_t getNextSample();
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
	// if (duration() == 0 || _sample.expired()) {
	if (duration() == 0) {
		return 0;
	}

	// auto samplePtr = _sample.lock();
	auto samplePtr = _sample;

	// if we've moved far enough along, read the next sample
	while (fractionalSampleOffset >= 1.0) {
		previousSample = currentSample.load();
		currentSample = getNextSample();
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
	// return _sample.expired() ? 0 : (_sample.lock()->getSize() * 1000 / sampleRate()) / calculateSamplerate(frequency);
	return !_sample ? 0 : (_sample->getSize() * 1000 / sampleRate()) / calculateSamplerate(frequency);
}

void EnhancedSamplesGenerator::seekTo(uint32_t position) {
	// if (!_sample.expired()) {
	if (_sample) {
		// auto samplePtr = _sample.lock();
		auto samplePtr = _sample;
		samplePtr->seekTo(position, index, blockIndex, repeatCount);

		// prepare our fractional sample data for playback
		fractionalSampleOffset = 0.0;
		previousSample = samplePtr->getSample(index, blockIndex);
		currentSample = samplePtr->getSample(index, blockIndex);
	}
}

double EnhancedSamplesGenerator::calculateSamplerate(uint16_t frequency) {
	// if (!_sample.expired()) {
	if (_sample) {
		// auto samplePtr = _sample.lock();
		auto samplePtr = _sample;
		auto baseFrequency = samplePtr->baseFrequency;
		auto frequencyAdjust = baseFrequency > 0 ? (double)frequency / (double)baseFrequency : 1.0;
		return frequencyAdjust * ((double)samplePtr->sampleRate / (double)(sampleRate()));
	}
	return 1.0;
}

int8_t EnhancedSamplesGenerator::getNextSample() {
	// if (!_sample.expired()) {
	if (_sample) {
		// auto samplePtr = _sample.lock();
		auto samplePtr = _sample;
		auto sample = samplePtr->getSample(index, blockIndex);
		
		// looping magic
		repeatCount--;
		if (repeatCount == 0) {
			// we've reached the end of the repeat section, so loop back
			seekTo(samplePtr->repeatStart);
		}

		return sample;
	}
	return 0;
}

#endif // ENHANCED_SAMPLES_GENERATOR_H

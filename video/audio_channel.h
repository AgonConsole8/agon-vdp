#ifndef AUDIO_CHANNEL_H
#define AUDIO_CHANNEL_H

#include <memory>
#include <atomic>
#include <unordered_map>
#include <fabgl.h>

#include "agon.h"
#include "types.h"
#include "envelopes/types.h"

extern std::unique_ptr<fabgl::SoundGenerator> soundGenerator;	// audio handling sub-system
extern void audioTaskAbortDelay(uint8_t channel);

enum AudioState : uint8_t {	// Audio channel state
	Idle = 0,				// currently idle/silent
	Pending,				// note will be played next loop call
	Playing,				// playing (passive)
	PlayLoop,				// active playing loop (used when an envelope is active)
	Release,				// in "release" phase
	Abort					// aborting a note
};

// The audio channel class
//
class AudioChannel {
	public:
		AudioChannel(uint8_t channel);
		~AudioChannel();
		uint8_t		playNote(uint8_t volume, uint16_t frequency, int32_t duration);
		uint8_t		getStatus();
		uint8_t		setWaveform(int8_t waveformType, std::shared_ptr<AudioChannel> channelRef, uint16_t sampleId = 0);
		uint8_t		setVolume(uint8_t volume);
		uint8_t		setFrequency(uint16_t frequency);
		uint8_t		setDuration(int32_t duration);
		uint8_t		setVolumeEnvelope(std::unique_ptr<VolumeEnvelope> envelope);
		uint8_t		setFrequencyEnvelope(std::unique_ptr<FrequencyEnvelope> envelope);
		uint8_t		setSampleRate(uint16_t sampleRate);
		uint8_t		setDutyCycle(uint8_t dutyCycle);
		uint8_t		setParameter(uint8_t parameter, uint16_t value);
		WaveformGenerator * getWaveform() { return this->_waveform.get(); }
		void		attachSoundGenerator();
		void		detachSoundGenerator();
		uint8_t		seekTo(uint32_t position);
		void		loop();
		uint8_t		channel() { return _channel; }
	private:
		std::shared_ptr<WaveformGenerator>	getSampleWaveform(uint16_t sampleId, std::shared_ptr<AudioChannel> channelRef);
		void		waitForAbort();
		uint8_t		getVolume(uint32_t elapsed);
		uint16_t	getFrequency(uint32_t elapsed);
		bool		isReleasing(uint32_t elapsed);
		bool		isFinished(uint32_t elapsed);
		uint8_t		_channel;
		uint8_t		_volume;
		uint16_t	_frequency;
		int32_t		_duration;
		uint32_t	_startTime;
		uint8_t		_waveformType;
		std::atomic<AudioState>				_state;
		std::shared_ptr<WaveformGenerator>	_waveform = nullptr;
		std::unique_ptr<VolumeEnvelope>		_volumeEnvelope;
		std::unique_ptr<FrequencyEnvelope>	_frequencyEnvelope;
};

#include "audio_sample.h"
#include "enhanced_samples_generator.h"
extern std::unordered_map<uint16_t, std::shared_ptr<AudioSample>> samples;	// Storage for the sample data

AudioChannel::AudioChannel(uint8_t channel) : _channel(channel), _state(AudioState::Idle), _volume(64), _frequency(750), _duration(-1) {
	setWaveform(AUDIO_WAVE_DEFAULT, nullptr);
	debug_log("AudioChannel: init %d\n\r", channel);
	debug_log("free mem: %d\n\r", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

AudioChannel::~AudioChannel() {
	debug_log("AudioChannel: deiniting %d\n\r", channel());
	if (this->_waveform != nullptr) {
		this->_waveform->enable(false);
		soundGenerator->detach(getWaveform());
	}
	debug_log("AudioChannel: deinit %d\n\r", channel());
}

uint8_t AudioChannel::playNote(uint8_t volume, uint16_t frequency, int32_t duration) {
	if (!this->_waveform) {
		debug_log("AudioChannel: no waveform on channel %d\n\r", channel());
		return 0;
	}
	if (this->_waveformType == AUDIO_WAVE_SAMPLE && this->_volume == 0 && this->_state != AudioState::Idle) {
		// we're playing a silenced sample, so we're free to play a new note, so abort
		this->_state = AudioState::Abort;
		audioTaskAbortDelay(this->_channel);
		waitForAbort();
	}
	switch (this->_state) {
		case AudioState::Idle:
		case AudioState::Release:
			this->_volume = volume;
			this->_frequency = frequency;
			this->_duration = duration == 65535 ? -1 : duration;
			if (this->_duration == 0 && this->_waveformType == AUDIO_WAVE_SAMPLE) {
				// zero duration means play whole sample
				// NB this can only work out sample duration based on sample provided
				// so if sample data is streaming in an explicit length should be used instead
				this->_duration = ((EnhancedSamplesGenerator *)getWaveform())->getDuration(frequency);
				if (this->_volumeEnvelope) {
					// subtract the "release" time from the duration
					this->_duration -= this->_volumeEnvelope->getRelease();
				}
				if (this->_duration < 0) {
					this->_duration = 1;
				}
			}
			this->_state = AudioState::Pending;
			debug_log("AudioChannel: playNote %d,%d,%d,%d\n\r", channel(), volume, frequency, this->_duration);
			return 1;
	}
	return 0;
}

uint8_t AudioChannel::getStatus() {
	uint8_t status = 0;
	if (this->_waveform && this->_waveform->enabled()) {
		status |= AUDIO_STATUS_ACTIVE;
		if (this->_duration == -1) {
			status |= AUDIO_STATUS_INDEFINITE;
		}
	}
	switch (this->_state) {
		case AudioState::Pending:
		case AudioState::Playing:
		case AudioState::PlayLoop:
			status |= AUDIO_STATUS_PLAYING;
			break;
	}
	if (this->_volumeEnvelope) {
		status |= AUDIO_STATUS_HAS_VOLUME_ENVELOPE;
	}
	if (this->_frequencyEnvelope) {
		status |= AUDIO_STATUS_HAS_FREQUENCY_ENVELOPE;
	}

	debug_log("AudioChannel: getStatus %d\n\r", status);
	return status;
}

std::shared_ptr<WaveformGenerator> AudioChannel::getSampleWaveform(uint16_t sampleId, std::shared_ptr<AudioChannel> channelRef) {
	if (samples.find(sampleId) != samples.end()) {
		auto sample = samples.at(sampleId);
		// if (sample->channels.find(_channel) != sample->channels.end()) {
		// 	// this channel is already playing this sample, so do nothing
		// 	debug_log("AudioChannel: already playing sample %d on channel %d\n\r", sampleId, channel());
		// 	return nullptr;
		// }

		// TODO remove channel tracking??
		// remove this channel from other samples
		// for (auto &samplePair : samples) {
		// 	if (samplePair.second) {
		// 		samplePair.second->channels.erase(_channel);
		// 	}
		// }
		// sample->channels[_channel] = channelRef;

		return std::make_shared<EnhancedSamplesGenerator>(sample);
	}
	debug_log("sample %d not found\n\r", sampleId);
	return nullptr;
}

uint8_t AudioChannel::setWaveform(int8_t waveformType, std::shared_ptr<AudioChannel> channelRef, uint16_t sampleId) {
	std::shared_ptr<WaveformGenerator> newWaveform = nullptr;

	switch (waveformType) {
		case AUDIO_WAVE_SAWTOOTH:
			newWaveform = std::make_shared<SawtoothWaveformGenerator>();
			break;
		case AUDIO_WAVE_SQUARE:
			newWaveform = std::make_shared<SquareWaveformGenerator>();
			break;
		case AUDIO_WAVE_SINE:
			newWaveform = std::make_shared<SineWaveformGenerator>();
			break;
		case AUDIO_WAVE_TRIANGLE:
			newWaveform = std::make_shared<TriangleWaveformGenerator>();
			break;
		case AUDIO_WAVE_NOISE:
			newWaveform = std::make_shared<NoiseWaveformGenerator>();
			break;
		case AUDIO_WAVE_VICNOISE:
			newWaveform = std::make_shared<VICNoiseGenerator>();
			break;
		case AUDIO_WAVE_SAMPLE:
			// Buffer-based sample playback
			debug_log("AudioChannel: using sample buffer %d for waveform on channel %d\n\r", sampleId, channel());
			newWaveform = getSampleWaveform(sampleId, channelRef);
			break;
		default:
			// negative values indicate a sample number
			if (waveformType < 0) {
				// convert our negative sample number to a positive sample number starting at our base buffer ID
				int16_t sampleNum = BUFFERED_SAMPLE_BASEID + (-waveformType - 1);
				debug_log("AudioChannel: using sample %d for waveform (%d) on channel %d\n\r", waveformType, sampleNum, channel());
				newWaveform = getSampleWaveform(sampleNum, channelRef);
				waveformType = AUDIO_WAVE_SAMPLE;
			} else {
				debug_log("AudioChannel: unknown waveform type %d on channel %d\n\r", waveformType, channel());
			}
			break;
	}

	if (newWaveform != nullptr) {
		debug_log("AudioChannel: setWaveform %d on channel %d\n\r", waveformType, channel());
		if (this->_state != AudioState::Idle) {
			debug_log("AudioChannel: aborting current playback\n\r");
			// some kind of playback is happening, so abort any current task delay to allow playback to end
			this->_state = AudioState::Abort;
			audioTaskAbortDelay(this->_channel);
			waitForAbort();
		}
		if (this->_waveform != nullptr) {
			debug_log("AudioChannel: detaching old waveform\n\r");
			detachSoundGenerator();
		}
		this->_waveform = newWaveform;
		_waveformType = waveformType;
		attachSoundGenerator();
		debug_log("AudioChannel: setWaveform %d done on channel %d\n\r", waveformType, channel());
		return 1;
	}
	// waveform not changed, so return a failure
	return 0;
}

uint8_t AudioChannel::setVolume(uint8_t volume) {
	debug_log("AudioChannel: setVolume %d on channel %d\n\r", volume, channel());
	if (volume == 255) {
		return this->_volume;
	}
	if (volume > 127) {
		volume = 127;
	}

	if (this->_waveform) {
		waitForAbort();
		switch (this->_state) {
			case AudioState::Idle:
				if (volume > 0) {
					// new note playback
					this->_volume = volume;
					this->_duration = -1;	// indefinite duration
					this->_state = AudioState::Pending;
				}
				break;
			case AudioState::PlayLoop:
				// we are looping, so an envelope may be active
				if (volume == 0 && this->_waveformType != AUDIO_WAVE_SAMPLE) {
					// silence whilst looping always stops playback - curtail duration
					this->_duration = millis() - this->_startTime;
					// if there's a volume envelope, just allow release to happen, otherwise...
					if (!this->_volumeEnvelope) {
						this->_volume = 0;
					}
				} else {
					// Change base volume level, so next loop iteration will use it
					this->_volume = volume;
				}
				break;
			case AudioState::Pending:
			case AudioState::Release:
				// Set level so next loop will pick up the new volume
				this->_volume = volume;
				break;
			default:
				// All other states we'll set volume immediately
				this->_volume = volume;
				this->_waveform->setVolume(volume);
				if (volume == 0 && this->_waveformType != AUDIO_WAVE_SAMPLE) {
					// we're going silent, so abort any current playback
					this->_state = AudioState::Abort;
					audioTaskAbortDelay(this->_channel);
				}
				break;
		}
		return this->_volume;
	}
	return 255;
}

uint8_t AudioChannel::setFrequency(uint16_t frequency) {
	debug_log("AudioChannel: setFrequency %d on channel %d\n\r", frequency, channel());
	this->_frequency = frequency;

	if (this->_waveform) {
		waitForAbort();
		switch (this->_state) {
			case AudioState::Pending:
			case AudioState::PlayLoop:
			case AudioState::Release:
				// Do nothing as next loop will pick up the new frequency
				break;
			default:
				this->_waveform->setFrequency(frequency);
		}
		return 1;
	}
	return 0;
}

uint8_t AudioChannel::setDuration(int32_t duration) {
	debug_log("AudioChannel: setDuration %d on channel %d\n\r", duration, channel());
	if (duration == 0xFFFFFF) {
		duration = -1;
	}
	this->_duration = duration;

	if (this->_waveform) {
		waitForAbort();
		switch (this->_state) {
			case AudioState::Idle:
				// kick off a new note playback
				this->_state = AudioState::Pending;
				break;
			case AudioState::Playing:
				audioTaskAbortDelay(this->_channel);
				break;
			default:
				// any other state we should be looping so it will just get picked up
				break;
		}
		return 1;
	}
	return 0;
}

uint8_t AudioChannel::setVolumeEnvelope(std::unique_ptr<VolumeEnvelope> envelope) {
	this->_volumeEnvelope = std::move(envelope);
	if (envelope && this->_state == AudioState::Playing) {
		// swap to looping
		this->_state = AudioState::PlayLoop;
		audioTaskAbortDelay(this->_channel);
	}
	return 1;
}

uint8_t AudioChannel::setFrequencyEnvelope(std::unique_ptr<FrequencyEnvelope> envelope) {
	this->_frequencyEnvelope = std::move(envelope);
	if (envelope && this->_state == AudioState::Playing) {
		// swap to looping
		this->_state = AudioState::PlayLoop;
		audioTaskAbortDelay(this->_channel);
	}
	return 1;
}

uint8_t AudioChannel::setSampleRate(uint16_t sampleRate) {
	if (this->_waveform) {
		this->_waveform->setSampleRate(sampleRate);
		return 1;
	}
	return 0;
}

uint8_t AudioChannel::setDutyCycle(uint8_t dutyCycle) {
	if (this->_waveform && this->_waveformType == AUDIO_WAVE_SQUARE) {
		((SquareWaveformGenerator *)getWaveform())->setDutyCycle(dutyCycle);
		return 1;
	}
	return 0;
}

uint8_t AudioChannel::setParameter(uint8_t parameter, uint16_t value) {
	if (this->_waveform) {
		bool use16Bit = parameter & AUDIO_PARAM_16BIT;
		auto param = parameter & AUDIO_PARAM_MASK;
		switch (param) {
			case AUDIO_PARAM_DUTY_CYCLE: {
				return setDutyCycle(value);
			}	break;
			case AUDIO_PARAM_VOLUME: {
				return setVolume(value);
			}	break;
			case AUDIO_PARAM_FREQUENCY: {
				if (!use16Bit) {
					value = _frequency & 0xFF00 | value & 0x00FF;
				}
				return setFrequency(value);
			}	break;
		}
	}
	return 0;
}

void AudioChannel::attachSoundGenerator() {
	if (this->_waveform) {
		soundGenerator->attach(getWaveform());
	}
}

void AudioChannel::detachSoundGenerator() {
	if (this->_waveform) {
		soundGenerator->detach(getWaveform());
	}
}

uint8_t AudioChannel::seekTo(uint32_t position) {
	if (this->_waveformType == AUDIO_WAVE_SAMPLE) {
		((EnhancedSamplesGenerator *)getWaveform())->seekTo(position);
		return 1;
	}
	return 0;
}

void AudioChannel::waitForAbort() {
	while (this->_state == AudioState::Abort) {
		// wait for abort to complete
		vTaskDelay(1);
	}
}

uint8_t AudioChannel::getVolume(uint32_t elapsed) {
	if (this->_volumeEnvelope) {
		return this->_volumeEnvelope->getVolume(this->_volume, elapsed, this->_duration);
	}
	return this->_volume;
}

uint16_t AudioChannel::getFrequency(uint32_t elapsed) {
	if (this->_frequencyEnvelope) {
		return this->_frequencyEnvelope->getFrequency(this->_frequency, elapsed, this->_duration);
	}
	return this->_frequency;
}

bool AudioChannel::isReleasing(uint32_t elapsed) {
	if (this->_volumeEnvelope) {
		return this->_volumeEnvelope->isReleasing(elapsed, this->_duration);
	}
	if (this->_duration == -1) {
		return false;
	}
	return elapsed >= this->_duration;
}

bool AudioChannel::isFinished(uint32_t elapsed) {
	if (this->_volumeEnvelope) {
		return this->_volumeEnvelope->isFinished(elapsed, this->_duration);
	}
	if (this->_duration == -1) {
		return false;
	}
	return (elapsed >= this->_duration);
}

void AudioChannel::loop() {
	int delay = 0;

	switch (this->_state) {
		case AudioState::Pending:
			debug_log("AudioChannel: play %d,%d,%d,%d\n\r", channel(), this->_volume, this->_frequency, this->_duration);
			// we have a new note to play
			this->_startTime = millis();
			// set our initial volume and frequency
			this->_waveform->setVolume(this->getVolume(0));
			this->seekTo(0);
			this->_waveform->setFrequency(this->getFrequency(0));
			this->_waveform->enable(true);
			// if we have an envelope then we loop, otherwise just delay for duration
			if (this->_volumeEnvelope || this->_frequencyEnvelope) {
				this->_state = AudioState::PlayLoop;
			} else {
				this->_state = AudioState::Playing;
				// if delay value is negative then this delays for a super long time
				delay = this->_duration;
			}
			break;

		case AudioState::Playing:
			if (this->_duration >= 0) {
				// simple playback - delay until we have reached our duration
				uint32_t elapsed = millis() - this->_startTime;
				debug_log("AudioChannel: %d elapsed %d\n\r", channel(), elapsed);
				if (elapsed >= this->_duration) {
					this->_waveform->enable(false);
					debug_log("AudioChannel: %d end\n\r", channel());
					this->_state = AudioState::Idle;
				} else {
					debug_log("AudioChannel: %d loop (%d)\n\r", channel(), this->_duration - elapsed);
					delay = this->_duration - elapsed;
				}
			} else {
				// our duration is indefinite, so delay for a long time
				debug_log("AudioChannel: %d loop (indefinite playback)\n\r", channel());
				delay = -1;
			}
			break;

		// loop and release states used for envelopes
		case AudioState::PlayLoop: {
			uint32_t elapsed = millis() - this->_startTime;
			if (isReleasing(elapsed)) {
				debug_log("AudioChannel: releasing %d...\n\r", channel());
				this->_state = AudioState::Release;
			}
			// update volume and frequency as appropriate
			if (this->_volumeEnvelope)
				this->_waveform->setVolume(this->getVolume(elapsed));
			if (this->_frequencyEnvelope)
				this->_waveform->setFrequency(this->getFrequency(elapsed));
			break;
		}

		case AudioState::Release: {
			uint32_t elapsed = millis() - this->_startTime;
			// update volume and frequency as appropriate
			if (this->_volumeEnvelope)
				this->_waveform->setVolume(this->getVolume(elapsed));
			if (this->_frequencyEnvelope)
				this->_waveform->setFrequency(this->getFrequency(elapsed));

			if (isFinished(elapsed)) {
				this->_waveform->enable(false);
				debug_log("AudioChannel: end (released %d)\n\r", channel());
				this->_state = AudioState::Idle;
			}
			break;
		}

		case AudioState::Abort:
			this->_waveform->enable(false);
			debug_log("AudioChannel: abort %d\n\r", channel());
			this->_state = AudioState::Idle;
			break;
	}

	if (delay != 0) {
		vTaskDelay(pdMS_TO_TICKS(delay));
	}
}

#endif // AUDIO_CHANNEL_H

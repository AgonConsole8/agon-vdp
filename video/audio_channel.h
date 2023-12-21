#ifndef AUDIO_CHANNEL_H
#define AUDIO_CHANNEL_H

#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <fabgl.h>
#include <esp_timer.h>

#include "agon.h"
#include "types.h"
#include "envelopes/volume.h"
#include "envelopes/frequency.h"

extern std::unique_ptr<fabgl::SoundGenerator> soundGenerator;	// audio handling sub-system
extern void audioTaskAbortDelay(uint8_t channel);

// The audio channel class
//
class AudioChannel {	
	public:
		AudioChannel(uint8_t channel);
		~AudioChannel();
		uint8_t		playNote(uint8_t volume, uint16_t frequency, int32_t duration);
		uint8_t		getStatus();
		void		setWaveform(int8_t waveformType, std::shared_ptr<AudioChannel> channelRef, uint16_t sampleId = 0);
		void		setVolume(uint8_t volume);
		void		setFrequency(uint16_t frequency);
		void		setDuration(int32_t duration);
		void		setVolumeEnvelope(std::unique_ptr<VolumeEnvelope> envelope);
		void		setFrequencyEnvelope(std::unique_ptr<FrequencyEnvelope> envelope);
		void		setSampleRate(uint16_t sampleRate);
		WaveformGenerator * getWaveform() { return this->_waveform.get(); }
		void		attachSoundGenerator();
		void		seekTo(uint32_t position);
		void		loop();
		uint8_t		channel() { return _channel; }
	private:
		std::shared_ptr<fabgl::WaveformGenerator>	getSampleWaveform(uint16_t sampleId, std::shared_ptr<AudioChannel> channelRef);
		void		waitForAbort();
		uint8_t		getVolume(uint32_t elapsed);
		uint16_t	getFrequency(uint32_t elapsed);
		bool		isReleasing(uint32_t elapsed);
		bool		isFinished(uint32_t elapsed);
		std::atomic<AudioState>				_state;
		std::atomic<std::uint8_t>			_channel;
		std::atomic<std::uint8_t>			_volume;
		std::atomic<std::uint16_t>			_frequency;
		std::atomic<std::int32_t>			_duration;
		// std::atomic<std::uint32_t>		_startTime;
		std::atomic<std::uint64_t>			_startTime;
		std::atomic<std::uint8_t>			_waveformType;
		std::shared_ptr<WaveformGenerator>	_waveform;
		std::unique_ptr<VolumeEnvelope>		_volumeEnvelope;
		std::unique_ptr<FrequencyEnvelope>	_frequencyEnvelope;
		std::mutex							_mutex;
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
	if (this->_waveform) {
		_mutex.lock();
		this->_waveform->enable(false);
		soundGenerator->detach(this->_waveform.get());
		_mutex.unlock();
	}
	debug_log("AudioChannel: deinit %d\n\r", channel());
}

uint8_t AudioChannel::playNote(uint8_t volume, uint16_t frequency, int32_t duration) {
	if (!this->_waveform) {
		debug_log("AudioChannel: no waveform on channel %d\n\r", channel());
		return 0;
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
				this->_duration = ((EnhancedSamplesGenerator *)this->_waveform.get())->getDuration(frequency);
				if (this->_volumeEnvelope) {
					// subtract the "release" time from the duration
					this->_duration -= this->_volumeEnvelope->getRelease();
				}
				if (this->_duration < 0) {
					this->_duration = 1;
				}
			}
			this->_state = AudioState::Pending;
			debug_log("AudioChannel: playNote %d,%d,%d,%d\n\r", channel(), this->_volume.load(), this->_frequency.load(), this->_duration.load());
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

std::shared_ptr<fabgl::WaveformGenerator> AudioChannel::getSampleWaveform(uint16_t sampleId, std::shared_ptr<AudioChannel> channelRef) {
	if (samples.find(sampleId) != samples.end()) {
		auto sample = samples.at(sampleId);
		// if (sample->channels.find(_channel) != sample->channels.end()) {
		// 	// this channel is already playing this sample, so do nothing
		// 	debug_log("AudioChannel: already playing sample %d on channel %d\n\r", sampleId, channel());
		// 	return nullptr;
		// }

		// // if we're already a waveform, then swap sample
		// if (this->_waveformType == AUDIO_WAVE_SAMPLE) {
		// 	// swap sample
		// 	_mutex.lock();
		// 	((EnhancedSamplesGenerator *)this->_waveform.get())->setSample(sample);
		// 	_mutex.unlock();
		// 	debug_log("AudioChannel: setSample %d on channel %d\n\r", sampleId, channel());
		// 	return nullptr;
		// }

		// TODO remove channel tracking??
		// remove this channel from other samples
		// for (auto samplePair : samples) {
		// 	if (samplePair.second) {
		// 		samplePair.second->channels.erase(_channel);
		// 	}
		// }
		// sample->channels[_channel] = channelRef;

		return make_shared_psram<EnhancedSamplesGenerator>(sample);
	}
	return nullptr;
}

void AudioChannel::setWaveform(int8_t waveformType, std::shared_ptr<AudioChannel> channelRef, uint16_t sampleId) {
	std::shared_ptr<fabgl::WaveformGenerator> newWaveform = nullptr;

	switch (waveformType) {
		case AUDIO_WAVE_SAWTOOTH:
			newWaveform = make_shared_psram<SawtoothWaveformGenerator>();
			break;
		case AUDIO_WAVE_SQUARE:
			newWaveform = make_shared_psram<SquareWaveformGenerator>();
			break;
		case AUDIO_WAVE_SINE:
			newWaveform = make_shared_psram<SineWaveformGenerator>();
			break;
		case AUDIO_WAVE_TRIANGLE:
			newWaveform = make_shared_psram<TriangleWaveformGenerator>();
			break;
		case AUDIO_WAVE_NOISE:
			newWaveform = make_shared_psram<NoiseWaveformGenerator>();
			break;
		case AUDIO_WAVE_VICNOISE:
			newWaveform = make_shared_psram<VICNoiseGenerator>();
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

	if (newWaveform) {
		debug_log("AudioChannel: setWaveform %d on channel %d\n\r", waveformType, channel());
		if (this->_state != AudioState::Idle) {
			debug_log("AudioChannel: aborting current playback\n\r");
			// some kind of playback is happening, so abort any current task delay to allow playback to end
			this->_state = AudioState::Abort;
			audioTaskAbortDelay(this->_channel);
			waitForAbort();
		}
		_mutex.lock();
		if (this->_waveform) {
			debug_log("AudioChannel: detaching old waveform\n\r");
			soundGenerator->detach(this->_waveform.get());
		}
		this->_waveform = std::move(newWaveform);
		_waveformType = waveformType;
		attachSoundGenerator();
		debug_log("AudioChannel: setWaveform %d done on channel %d\n\r", waveformType, channel());
		_mutex.unlock();
	}
}

void AudioChannel::setVolume(uint8_t volume) {
	debug_log("AudioChannel: setVolume %d on channel %d\n\r", volume, channel());

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
				if (volume == 0) {
					// silence whilst looping always stops playback - curtail duration
					// this->_duration = millis() - this->_startTime;
					this->_duration = (esp_timer_get_time() / 1000) - this->_startTime;
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
				if (volume == 0) {
					// we're going silent, so abort any current playback
					this->_state = AudioState::Abort;
					audioTaskAbortDelay(this->_channel);
				}
				break;
		}	
	}
}

void AudioChannel::setFrequency(uint16_t frequency) {
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
	}
}

void AudioChannel::setDuration(int32_t duration) {
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
	}
}

void AudioChannel::setVolumeEnvelope(std::unique_ptr<VolumeEnvelope> envelope) {
	this->_volumeEnvelope = std::move(envelope);
	if (envelope && this->_state == AudioState::Playing) {
		// swap to looping
		this->_state = AudioState::PlayLoop;
		audioTaskAbortDelay(this->_channel);
	}
}

void AudioChannel::setFrequencyEnvelope(std::unique_ptr<FrequencyEnvelope> envelope) {
	this->_frequencyEnvelope = std::move(envelope);
	if (envelope && this->_state == AudioState::Playing) {
		// swap to looping
		this->_state = AudioState::PlayLoop;
		audioTaskAbortDelay(this->_channel);
	}
}

void AudioChannel::setSampleRate(uint16_t sampleRate) {
	if (this->_waveform) {
		this->_waveform->setSampleRate(sampleRate);
	}
}

void AudioChannel::attachSoundGenerator() {
	if (this->_waveform) {
		soundGenerator->attach(this->_waveform.get());
	}
}

void AudioChannel::seekTo(uint32_t position) {
	if (this->_waveformType == AUDIO_WAVE_SAMPLE) {
		((EnhancedSamplesGenerator *)this->_waveform.get())->seekTo(position);
	}
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
	// _mutex.lock();
	if (_mutex.try_lock() == false) {
		// can't obtain a lock, so other stuff is happening - just return
		debug_log("AudioChannel: can't obtain lock on channel %d\n\r", channel());
		return;
	}
	int delay = 0;
	switch (this->_state) {
		case AudioState::Pending:
			debug_log("AudioChannel: play %d,%d,%d,%d\n\r", channel(), this->_volume.load(), this->_frequency.load(), this->_duration.load());
			// we have a new note to play
			// this->_startTime = millis();
			this->_startTime = (esp_timer_get_time() / 1000);
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
				// _mutex.unlock();
				delay = pdMS_TO_TICKS(this->_duration);
				// vTaskDelay(pdMS_TO_TICKS(this->_duration));
			}
			break;
		case AudioState::Playing:
			if (this->_duration >= 0) {
				// simple playback - delay until we have reached our duration
				// uint32_t elapsed = millis() - this->_startTime;
				uint32_t elapsed = (esp_timer_get_time() / 1000) - this->_startTime;
				debug_log("AudioChannel: %d elapsed %d\n\r", channel(), elapsed);
				if (elapsed >= this->_duration) {
					this->_waveform->enable(false);
					debug_log("AudioChannel: %d end\n\r", channel());
					this->_state = AudioState::Idle;
				} else {
					debug_log("AudioChannel: %d loop (%d)\n\r", channel(), this->_duration - elapsed);
					// _mutex.unlock();
					delay = pdMS_TO_TICKS(this->_duration - elapsed);
					// vTaskDelay(pdMS_TO_TICKS(this->_duration - elapsed));
				}
			} else {
				// our duration is indefinite, so delay for a long time
				debug_log("AudioChannel: %d loop (indefinite playback)\n\r", channel());
				// _mutex.unlock();
				delay = pdMS_TO_TICKS(-1);
				// vTaskDelay(pdMS_TO_TICKS(-1));
			}
			break;
		// loop and release states used for envelopes
		case AudioState::PlayLoop: {
			// uint32_t elapsed = millis() - this->_startTime;
			uint32_t elapsed = (esp_timer_get_time() / 1000) - this->_startTime;
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
			// uint32_t elapsed = millis() - this->_startTime;
			uint32_t elapsed = (esp_timer_get_time() / 1000) - this->_startTime;
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
	_mutex.unlock();
	if (delay != 0) {
		vTaskDelay(delay);
	}
}

#endif // AUDIO_CHANNEL_H

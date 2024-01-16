//
// Title:			Multi-phase ADSR Envelope support
// Author:			Steve Sims
// Created:			14/01/2024
// Last Updated:	14/01/2024

#ifndef ENVELOPE_MULTIPHASE_ADSR_H
#define ENVELOPE_MULTIPHASE_ADSR_H

#include <memory>
#include <vector>
#include <Arduino.h>

#include "./types.h"

struct VolumeSubPhase {
	uint8_t level;			// relative volume level for sub-phase
	uint16_t duration;		// number of steps
};

class MultiphaseADSREnvelope : public VolumeEnvelope {
	public:
		MultiphaseADSREnvelope(std::shared_ptr<std::vector<VolumeSubPhase>> attack, std::shared_ptr<std::vector<VolumeSubPhase>> sustain, std::shared_ptr<std::vector<VolumeSubPhase>> release);
		uint8_t		getVolume(uint8_t baseVolume, uint32_t elapsed, int32_t duration);
		bool		isReleasing(uint32_t elapsed, int32_t duration);
		bool 		isFinished(uint32_t elapsed, int32_t duration);
		uint32_t	getRelease() {
			return _releaseDuration;
		};
	private:
		uint8_t		getTargetVolume(uint8_t baseVolume, uint8_t level);
		std::shared_ptr<std::vector<VolumeSubPhase>> _attack;
		std::shared_ptr<std::vector<VolumeSubPhase>> _sustain;
		std::shared_ptr<std::vector<VolumeSubPhase>> _release;
		uint32_t	_attackDuration;
		uint32_t	_sustainDuration;
		uint32_t	_releaseDuration;
		uint8_t		_sustainSubphases;
		uint8_t		_attackLevel;			// final levels
		uint8_t		_sustainLevel;
		uint8_t		_releaseLevel;
		bool		_sustainLoops;
};

MultiphaseADSREnvelope::MultiphaseADSREnvelope(std::shared_ptr<std::vector<VolumeSubPhase>> attack, std::shared_ptr<std::vector<VolumeSubPhase>> sustain, std::shared_ptr<std::vector<VolumeSubPhase>> release)
	: _attack(attack), _sustain(sustain), _release(release)
{
	_attackDuration = 0;
	// add durations from attack subphases to attackDuration
	for (const auto& subPhase : *_attack) {
		_attackDuration += subPhase.duration;
	}
	_sustainSubphases = _sustain->size();
	_sustainDuration = 0;
	for (const auto& subPhase : *_sustain) {
		_sustainDuration += subPhase.duration;
	}
	_releaseDuration = 0;
	for (const auto& subPhase : *_release) {
		_releaseDuration += subPhase.duration;
	}
	_attackLevel = 127;
	// set attackLevel to last level from attack vector, if there are values
	if (!_attack->empty()) {
		_attackLevel = _attack->back().level;
	}
	_sustainLevel = 127;
	if (!_sustain->empty()) {
		_sustainLevel = _sustain->back().level;
	}
	_releaseLevel = 0;
	if (!_release->empty()) {
		_releaseLevel = _release->back().level;
	}
	_sustainLoops = false;
	// loop over our sustain entries, and if we have any non-zero duration values set sustainLoops to true
	for (const auto& subPhase : *_sustain) {
		if (subPhase.duration > 0) {
			_sustainLoops = true;
			break;
		}
	}
	debug_log("MultiphaseADSREnvelope created with %d attack, %d sustain, %d release phases\n\r", _attack->size(), _sustain->size(), _release->size());
	debug_log("  attackDuration %d, sustainDuration %d, releaseDuration %d\n\r", _attackDuration, _sustainDuration, _releaseDuration);
	debug_log("  attackLevel %d, sustainLevel %d, releaseLevel %d\n\r", _attackLevel, _sustainLevel, _releaseLevel);
	for (auto subPhase : *this->_attack) {
		debug_log("  level %d, duration %d\n\r", subPhase.level, subPhase.duration);
	}
}

uint8_t MultiphaseADSREnvelope::getVolume(uint8_t baseVolume, uint32_t elapsed, int32_t duration) {
	auto subPhasePos = elapsed;
	uint32_t pos = 0;
	uint8_t startVolume = 0;
	// work out what sub-phase we're on, based on our elapsed time
	if (subPhasePos < _attackDuration) {
		// we're in an attack sub-phase
		for (const auto& subPhase : *this->_attack) {
			if (subPhasePos < subPhase.duration) {
				// we're inside this sub-phase
				return map(subPhasePos, 0, subPhase.duration, startVolume, getTargetVolume(baseVolume, subPhase.level));
			}
			subPhasePos -= subPhase.duration;
			startVolume = getTargetVolume(baseVolume, subPhase.level);
		}
	} else {
		startVolume = getTargetVolume(baseVolume, _attackLevel);
	}
	subPhasePos -= _attackDuration;
	pos += _attackDuration;

	auto sustainVolume = getTargetVolume(baseVolume, _sustainLevel);
	if (_sustainLoops) {
		// if we have sustain data, and it's not just zero duration, then loop around it
		while (pos < duration) {
			// short-cut looping sub-phases if we can for complete loops
			if (subPhasePos > _sustainDuration) {
				subPhasePos -= _sustainDuration;
				pos += _sustainDuration;
				startVolume = sustainVolume;
			} else {
				for (auto subPhase : *this->_sustain) {
					if (subPhasePos < subPhase.duration) {
						// we're in this sub-phase
						return map(subPhasePos, 0, subPhase.duration, startVolume, getTargetVolume(baseVolume, subPhase.level));
					}
					subPhasePos -= subPhase.duration;
					pos += subPhase.duration;
					startVolume = getTargetVolume(baseVolume, subPhase.level);
				}
			}
		}
	} else if (elapsed < duration) {
		// non-looping sustain - so we're spreading time between the phases, if there are any
		if (_sustainSubphases <= 1) {
			return map(subPhasePos, 0, duration - _attackDuration, startVolume, sustainVolume);
		}
		int phaseDuration = (duration - _attackDuration) / _sustainSubphases;
		for (auto subPhase : *this->_sustain) {
			if (subPhasePos < phaseDuration) {
				// we're in this subphase
				return map(subPhasePos, 0, phaseDuration, startVolume, getTargetVolume(baseVolume, subPhase.level));
			}
			subPhasePos -= phaseDuration;
			startVolume = getTargetVolume(baseVolume, subPhase.level);
		}
		return startVolume;
	} else {
		// end of sustain reached for non-looping sustain, so adjust our subPhasePos to our time within release
		subPhasePos = elapsed - duration;
		startVolume = sustainVolume;
	}

	// work out our release phase volume
	for (auto subPhase : *this->_release) {
		if (subPhasePos < subPhase.duration) {
			return map(subPhasePos, 0, subPhase.duration, startVolume, getTargetVolume(baseVolume, subPhase.level));
		}
		subPhasePos -= subPhase.duration;
		startVolume = getTargetVolume(baseVolume, subPhase.level);
	}

	return 0;
}

bool MultiphaseADSREnvelope::isReleasing(uint32_t elapsed, int32_t duration) {
	if (duration < 0) return false;
	auto minDuration = this->_attackDuration;
	if (duration < minDuration) duration = minDuration;

	// NB this is an approximation.  we may not actually be using "release" phase of envelope
	// but we'll consider ourselves to be in the "release" if the following check is true
	// this is good enough for the channel state machine, as the isFinished check is correct
	return (elapsed >= duration);
}

bool MultiphaseADSREnvelope::isFinished(uint32_t elapsed, int32_t duration) {
	if (duration < 0) return false;

	// we're finished if we have reached the end of sustain and then end of release
	auto minDuration = _attackDuration + _sustainDuration;
	if (_sustainDuration != 0) {
		while ((minDuration + _sustainDuration) <= duration) {
			minDuration += _sustainDuration;
		}
	}

	if (duration < minDuration) duration = minDuration;

	return (elapsed >= duration + this->_releaseDuration);
}

uint8_t MultiphaseADSREnvelope::getTargetVolume(uint8_t baseVolume, uint8_t level) {
	return baseVolume * level / 127;
}

#endif // ENVELOPE_MULTIPHASE_ADSR_H

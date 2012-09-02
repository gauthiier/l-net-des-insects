/* 
 Midi.h - Music library
 Copyright (c) 2012 Copenhagen Institute of Interaction Design. 
 All right reserved.
 
 This library is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser Public License for more details.
 
 You should have received a copy of the GNU Lesser Public License
 along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 + author: Jakob Bak
 + contact: j.bak@ciid.dk
 */

// MIDI specific constants

#ifndef CFO_MIDI_CHANNEL
#define CFO_MIDI_CHANNEL 1
#endif

// SYSEX constants
#define SYSEX_LIMIT 16
#define CFO_MANUFACTURER_ID 44
#define CFO_DEVICE_GROUP_ID 3
#define SET_CHANNEL 0



//synth parameters as MIDI controller numbers
#define DETUNE 4
#define WAVEFORM 5
#define PORTAMENTO 6

#define FREQUENCY1 10
#define SEMITONE1 11
#define DETUNE1 12
#define GAIN1 13

#define FREQUENCY2 20
#define SEMITONE2 21
#define DETUNE2 22
#define GAIN2 23

#define FREQUENCY3 30
#define SEMITONE3 31
#define DETUNE3 32
#define GAIN3 33

#define ENV1_ATTACK 114
#define ENV1_DECAY 115
#define ENV1_SUSTAIN 116
#define ENV1_RELEASE 117

#define ENV2_ATTACK 124
#define ENV2_DECAY 125
#define ENV2_SUSTAIN 126
#define ENV2_RELEASE 127



// Synth parameters used in MIDI code
#define ENV_MAX_GAIN (65536 * 4 - 1) 


class MMidi {
public:
	void init();
	void checkMidi();
	void setChannel(uint8_t channel);
	void setID(uint8_t ID);

	void midiHandler();
	void noteOff(uint8_t channel, uint8_t note, uint8_t vel);
	void noteOn(uint8_t channel, uint8_t note, uint8_t vel);
	void aftertouch(uint8_t channel, uint8_t note, uint8_t pressure);
	void controller(uint8_t channel, uint8_t number, uint8_t value);
	void programChange(uint8_t channel, uint8_t number);
	void channelPressure(uint8_t channel, uint8_t pressure);
	void pitchWheel(uint8_t channel, uint8_t highBits, uint8_t lowBits);
	
	void sysexHandler(uint8_t bytes);
	void sysex	(uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t,
				 uint8_t);



private:
	// ID
	uint8_t cfoID;
	
	// MIDI
	uint8_t data;
	bool sysexON;
	//uint16_t midiBuffer[4];
	uint8_t midiBuffer[SYSEX_LIMIT];
	uint8_t midiChannel;
	
	int midiBufferIndex;
	uint16_t frequency;
	//uint32_t midiTime;
	//bool midiNotePlayed;
	
	//synth
	//bool noteTriggered;
	//bool noteReleased;
	//bool envSustainReached;
	uint8_t notePlayed;
	//uint16_t noteFrequency;
	//uint8_t envGain; // maybe another name for the variable
	//uint32_t envAttack;
	//uint32_t envDecay;
	//uint8_t envSustain;
	//uint32_t envRelease;
	//uint32_t envTime;
	//uint32_t envTriggerTime;
	//uint32_t envReleaseTime;
	//uint32_t portamento;
	
};

extern MMidi Midi;
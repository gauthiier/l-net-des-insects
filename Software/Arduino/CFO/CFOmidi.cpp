/* 
 Midi.cpp - Music library
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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <hardwareSerial.h>
//#include <MidiTables.h>
#include <CFOMusic.h>
#include <CFOMidi.h>

prog_uint16_t hertzTable[] PROGMEM = {8,8,9,9,10,10,11,12,12,13,14,15,16,17,18,19,20,21,23,24,25,27,29,30,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73,77,82,87,92,97,103,109,116,123,130,138,146,155,164,174,184,195,207,219,233,246,261,277,293,311,329,349,369,391,415,440,466,493,523,554,587,622,659,698,739,783,830,880,932,987,1046,1108,1174,1244,1318,1396,1479,1567,1661,1760,1864,1975,2093,2217,2349,2489,2637,2793,2959,3135,3322,3520,3729,3951,4186,4434,4698,4978,5274,5587,5919,6271,6644,7040,7458,7902,8372,8869,9397,9956,10548,11175,11839,12543};

MMidi Midi;

bool midiRead = false;

void MMidi::init()
{	
	Serial.begin(115200);

	cfoID = 0;
	sysexON =false;
	midiBufferIndex = 0;
	midiChannel = 0;
	//notePlayed = 0;

	

}


void MMidi::checkMidi()
{
	while(Serial.available() > 0) {

		data = Serial.read();

		if(data == 0xF0) {				// Start of SYSEX
			sysexON = true;
			midiBufferIndex = 0;
			midiRead = true;
		} else if(data & 0x80) {		// Automagically includes EOX (end of sysex, 0xF7)
			sysexON = false;
		}
//		if(data == 0xF7) {
//			sysexON = false;
//			sysexHandler(midiBufferIndex);
//		}
		
		
		if(data & 0x80 && (data & 0x0F) == midiChannel && !sysexON) {	// bitmask with 10000000 to see if byte is over 127 (data&0x80)
			midiBufferIndex = 0;							// and check if the midi channel corresponds to the midiChannel
			midiRead = true;								// the device is set to listen to.
			//if((data & 0xF0) == 0xF0) {
/*			if(data == 0xF0) {
				sysexON = true;
			} else {
				sysexON = false;
			}
 */
//		} else if(sysexON) {
//			midiRead = true;
		} else if(data & 0x80) {								// Else if the byte is over 127 (but not on the device's
			midiRead = false;								// midiChannel, don't read this or any following bytes.
		}

		if(midiRead) {
			midiBuffer[midiBufferIndex] = data;
			midiBufferIndex++;
			if (midiBufferIndex > 2 && !sysexON) {
				midiHandler();
			} else if(sysexON && midiBufferIndex < SYSEX_LIMIT && data == 0xF7) {
				//midiHandler();
				sysexHandler(midiBufferIndex);
			} else if(midiBufferIndex >= SYSEX_LIMIT) {
				midiBufferIndex = 0;
			}
		}
/*
		midiBuffer[midiBufferIndex] = Serial.read();
		if(midiBuffer[midiBufferIndex] == 0xFF) {
			midiHandler();
			midiBufferIndex = 0;
		}    
		else midiBufferIndex++;
*/		
	}
	
}

void MMidi::setChannel(uint8_t channel) {
	if(1 <= channel <= 16) {
		midiChannel = channel-1;
	}
}

void MMidi::setID(uint8_t ID) {
	cfoID = ID;
}


void MMidi::midiHandler() {
	
    //midiTime = millis();
    uint8_t midiChannel = (midiBuffer[0] & 0x0F);
    
	
	switch(midiBuffer[0] & 0xF0) { // bit mask with 11110000 gives us the MIDI command for channel 0
        case 0x80:
			noteOff			(midiBuffer[0] & 0x0F,	// midi channel 0-16
							 midiBuffer[1] & 0x7F,	// note value 0-127
							 midiBuffer[2] & 0x7F);	// note velocity 0-127
			break;
			
        case 0x90:
			noteOn			(midiBuffer[0] & 0x0F,	// midi channel 0-16
							 midiBuffer[1] & 0x7F,	// note value 0-127
							 midiBuffer[2] & 0x7F);	// note velocity 0-127
			break;
			
        case 0xA0:
			aftertouch		(midiBuffer[0] & 0x0F,	// midi channel 0-16
							 midiBuffer[1] & 0x7F,	// note value 0-127
							 midiBuffer[2] & 0x7F);	// note velocity 0-127
			break;
			
        case 0xB0:
			controller		(midiBuffer[0] & 0x0F,	// midi channel 0-16
							 midiBuffer[1] & 0x7F,	// controller number 0-127
							 midiBuffer[2] & 0x7F);	// controller value 0-127
			break;
			
        case 0xC0:
			programChange	(midiBuffer[0] & 0x0F,	// midi channel 0-16
							 midiBuffer[1] & 0x7F);	// program number 0-127
			break;
			
        case 0xD0:
			channelPressure	(midiBuffer[0] & 0x0F,	// midi channel 0-16
							 midiBuffer[1] & 0x7F);	// pressure amount 0-127
			break;
			
        case 0xE0:
			pitchWheel		(midiBuffer[0] & 0x0F,	// midi channel 0-16
							 midiBuffer[1] & 0x7F,	// higher bits 0-6
							 midiBuffer[2] & 0x7F);	// lower bits 7-13
			break;
/*        case 0xF0:
			sysex			(midiBuffer[0],			// sysex start (F0)
							 midiBuffer[1] & 0x7F,	// Manufacturer ID : CFO doesn't have one, but we'll use 0x44 (CASIO :D)
							 midiBuffer[2] & 0x7F,	// CFO Device Group ID : 0x00 (CFO), 0x01 (miniCFO), 0x02 (mikroCFO), 0x03 (l'insect)
							 midiBuffer[3] & 0x7F,	// CFO Device ID : 0-127. Default device ID is 0.
							 midiBuffer[4] & 0x7F,	// Function number: 0-127. 0 is setChannel().
							 midiBuffer[5] & 0x7F,	// value 0
							 midiBuffer[6] & 0x7F,	// value 1
							 midiBuffer[7] & 0x7F,	// value 2
							 midiBuffer[8] & 0x7F,	// value 3
							 midiBuffer[9] & 0x7F,	// value 4
							 midiBuffer[10] & 0x7F,	// value 5
							 midiBuffer[11] & 0x7F, // value 6
							 midiBuffer[12] & 0x7F,	// value 7
							 midiBuffer[13] & 0x7F, // value 8
							 midiBuffer[14] & 0x7F,	// value 9
							 midiBuffer[15] & 0x7F);// sysex stop (F7)
			break;
*/			
        default:
			break;
	}
}


/*
void MMidi::sysex(uint8_t val0,
				  uint8_t val1,
				  uint8_t val2,
				  uint8_t val3,
				  uint8_t val4,
				  uint8_t val5,
				  uint8_t val6,
				  uint8_t val7,
				  uint8_t val8,
				  uint8_t val9,
				  uint8_t valA,
				  uint8_t valB,
				  uint8_t valC,
				  uint8_t valD,
				  uint8_t valE,
				  uint8_t valF)
	{
	
		//ADD SYSEX HANDLING HERE
	
}
*/


void MMidi::sysexHandler(uint8_t bytes) {
	
	if(midiBuffer[1] == CFO_MANUFACTURER_ID) {
		if(midiBuffer[2] == CFO_DEVICE_GROUP_ID) {
			if(midiBuffer[3] == cfoID) {
				switch (midiBuffer[4]) {
					case SET_CHANNEL:
						setChannel(midiBuffer[5]);
						break;
						
					default:
						break;
				}
				
				
			}
		}
	}


}



void MMidi::noteOff(uint8_t channel, uint8_t note, uint8_t vel) {
	
	if(notePlayed == note) {
		Music.setEnvStage(4);
		//ampGain = 0;
		//fltGain = 0;      
	}
    
}


void MMidi::noteOn(uint8_t channel, uint8_t note, uint8_t vel) {
	
	Music.setEnvStage(1);
	//ampGain = 2 * vel << 8;
	//fltGain = 2 * vel << 8;
	Music.setVelSustain(vel);
	notePlayed = note;
	memcpy_P(&frequency, &hertzTable[notePlayed],2);
	Music.setFrequency1(frequency);
	Music.setFrequency2(frequency);
	Music.setFrequency3(frequency);
	
}

void MMidi::aftertouch(uint8_t channel, uint8_t note, uint8_t pressure) {
	// Write code here for Aftertouch 
}

void MMidi::controller(uint8_t channel, uint8_t number, uint8_t value) {
	
	switch(number) {
		case ENV1_ATTACK:
			Music.setAttack(value);
			break;
		case ENV1_DECAY:
			Music.setDecay(value);
			break;
		case ENV1_SUSTAIN:
			Music.setSustain(value);
			break;
		case ENV1_RELEASE:
			Music.setRelease(value);
			break;
		case DETUNE:
			Music.setDetune(value/5120.0);
			break;
		case DETUNE1:
			Music.setDetune1(value/5120.0);
			break;
		case DETUNE2:
			Music.setDetune2(value/5120.0);
			break;
		case DETUNE3:
			Music.setDetune3(value/5120.0);
			break;
		case SEMITONE1:
			if(15 < value && value < 113) {
				int8_t val = (((value-16)/4)-12);
				Music.setSemitone1(val);
			} else if (value < 16) {
				Music.setSemitone1(-12);				
			} else {
				Music.setSemitone1(12);
			}
			break;
		case SEMITONE2:
			if(15 < value && value < 113) {
				int8_t val = (((value-16)/4)-12);
				Music.setSemitone2(val);
			} else if (value < 16) {
				Music.setSemitone2(-12);				
			} else {
				Music.setSemitone2(12);
			}
			break;
		case SEMITONE3:
			if(15 < value && value < 113) {
				int8_t val = (((value-16)/4)-12);
				Music.setSemitone3(val);
			} else if (value < 16) {
				Music.setSemitone3(-12);				
			} else {
				Music.setSemitone3(12);
			}
			break;
		case GAIN1:
			Music.setGain1(uint16_t(value * 512));
			break;
		case GAIN2:
			Music.setGain2(uint16_t(value * 512));
			break;
		case GAIN3:
			Music.setGain3(uint16_t(value * 512));
			break;
		case WAVEFORM:
			value = value / 43;
			if(value == 0) Music.setSine();
			else if(value == 1) Music.setSaw();
			else if(value == 2) Music.setSquare();
			break;
		default:
			break;
	} 

}

void MMidi::programChange(uint8_t channel, uint8_t number) {
	// Write code here for Program Change 
}

void MMidi::channelPressure(uint8_t channel, uint8_t pressure) {
	// Write code here for Channel Pressure 
}

void MMidi::pitchWheel(uint8_t channel, uint8_t highBits, uint8_t lowBits) {
	// Write code here for Pitch Wheel
}
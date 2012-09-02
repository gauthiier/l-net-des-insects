/* 
 Music.cpp - Music library
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

#include <avr/interrupt.h>
#include <avr/pgmspace.h> 

#include <CFOWavetable.h>
#include <CFOMusic.h>
#include <hardwareSerial.h>

// Table of MIDI note values to frequency in Hertz
prog_uint16_t hertsTable[] PROGMEM = {8,8,9,9,10,10,11,12,12,13,14,15,16,17,18,19,20,21,23,24,25,27,29,30,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73,77,82,87,92,97,103,109,116,123,130,138,146,155,164,174,184,195,207,219,233,246,261,277,293,311,329,349,369,391,415,440,466,493,523,554,587,622,659,698,739,783,830,880,932,987,1046,1108,1174,1244,1318,1396,1479,1567,1661,1760,1864,1975,2093,2217,2349,2489,2637,2793,2959,3135,3322,3520,3729,3951,4186,4434,4698,4978,5274,5587,5919,6271,6644,7040,7458,7902,8372,8869,9397,9956,10548,11175,11839,12543};

prog_uint32_t envTimeTable[] PROGMEM = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,33,34,35,36,37,38,39,41,42,43,45,46,48,49,51,53,55,57,59,61,63,65,67,70,73,75,78,81,85,88,92,96,100,104,109,114,119,125,131,138,146,154,163,172,183,195,209,225,242,261,284,310,341,379,425,482,556,654,792,998,1342,2030,4095};

float semitoneTable[] = {0.25,0.2648658,0.2806155,0.29730177,0.31498027,0.33370996,0.35355338,0.37457678,0.39685026,0.4204482,0.44544938,0.47193715,0.5,0.5297315,0.561231,0.59460354,0.62996054,0.6674199,0.70710677,0.74915355,0.7937005,0.8408964,0.8908987,0.9438743,1.0,1.0594631,1.122462,1.1892071,1.2599211,1.3348398,1.4142135,1.4983071,1.587401,1.6817929,1.7817974,1.8877486,2.0,2.1189263,2.244924,2.3784142,2.5198421,2.6696796,2.828427,2.9966142,3.174802,3.3635857,3.563595,3.7754972,4.0};

CFOMusic Music;

// Defining which pins the SPI interface is connected to.
#define SPI_SCK 5
#define SPI_MOSI 3

// timer 2 is audio interrupt timer
ISR(TIMER2_COMPA_vect) {
	
	OCR2A = 127;
	
	Music.synthInterrupt();
	
}




/////////////////////////////////////
//
//	INITIALIZING FUNCTION
//
/////////////////////////////////////

void CFOMusic::init()
{
	// clear interrupts. to make sure the interrupt timer doesn't start until we've set it up.
	cli();
	
	// set up syntheziser
	// this is the timer 2 audio rate timer, fires an interrupt at 15625 Hz sampling rate
	TIMSK2 = 1<<OCIE2A;                      // interrupt enable audio timer
	OCR2A = 127;
	TCCR2A = 2;                              // CTC mode, counts up to 127 then resets
	TCCR2B = 0<<CS22 | 1<<CS21 | 0<<CS20;
	
	// set up SPI port
	SPCR = 0x50;
	SPSR = 0x01;
	
	
	// OUTPUTS
	// sck + mosi + ss
	DDRB = (1 << DDB2) | (1 << DDB3) | (1 << DDB5);
	// dac_cs output
	//DDRB = (1 << DDB2);
		
	
	// DAC frame sync HIGH, so that the SPI port doesn't start wirting straight away
	PORTB |= (1<<2);
	
	// waveform setup
	setSine();
	
	// frequency setup
	setFrequency(110.0f);
	setSemitone1(0);
	setSemitone2(0);
	setSemitone3(0);
	setDetune(0.0f);
	
	// gain setup
	setGain(1.0f);
	setGain1(1.0f);
	setGain2(1.0f);
	setGain3(1.0f);
	
	// envelope setup
	setEnvStage(0);
	disableEnvelope();
	env = 0;
	
	setAttack(4);
	setDecay(90);
	setSustain(32);
	setRelease(64);
	setVelSustain(0);
	
	sei(); // global interrupt enable 
	
	Serial.println("MUSIC INITIALIZED!");
}




/////////////////////////////////////
//
//	FREQUENCY AND DETUNE FUNCTIONS
//
/////////////////////////////////////

void CFOMusic::setFrequency(float freq)
{
	period1 = uint16_t(((freq * semi1 * (1 + detune1 + bend)) * 65536.0) / SAMPLE_RATE);
	period2 = uint16_t(((freq * semi2 * (1 + detune2 + bend)) * 65536.0) / SAMPLE_RATE);
	period3 = uint16_t(((freq * semi3 * (1 + detune3 + bend)) * 65536.0) / SAMPLE_RATE);
	frequency = freq;
	frequency1 = freq;
	frequency2 = freq;
	frequency3 = freq;
}


void CFOMusic::setFrequency1(float freq)
{
	frequency1 = freq;
	period1 = uint16_t(((frequency1 * semi1 * (1 + detune1 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setFrequency2(float freq)
{
	frequency2 = freq;
	period2 = uint16_t(((frequency2 * semi2 * (1 + detune2 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setFrequency3(float freq)
{
	frequency3 = freq;
	period3 = uint16_t(((frequency3 * semi3 * (1 + detune3 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setSemitone1(int8_t semi)
{
	if(-13 < semi && semi < 13){
		semi1 = semitoneTable[semi+12];
	} else if (semi < -12) {
		semi1 = semitoneTable[0];
	} else {
		semi1 = semitoneTable[24];
	}
	period1 = uint16_t(((frequency1 * semi1 * (1 + detune1 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setSemitone2(int8_t semi)
{
	if(-13 < semi && semi < 13){
		semi2 = semitoneTable[semi+12];
	} else if (semi < -12) {
		semi2 = semitoneTable[0];
	} else {
		semi2 = semitoneTable[24];
	}
	period2 = uint16_t(((frequency2 * semi2 * (1 + detune2 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setSemitone3(int8_t semi)
{
	if(-13 < semi && semi < 13){
		semi3 = semitoneTable[semi+12];
	} else if (semi < -12) {
		semi3 = semitoneTable[0];
	} else {
		semi3 = semitoneTable[24];
	}
	period3 = uint16_t(((frequency3 * semi3 * (1 + detune3 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setDetune(float detune)
{
	detune1 = 0.0;
	detune2 = detune;
	detune3 = -detune;
	period2 = uint16_t(((frequency2 * semi2 * (1 + detune2 + bend)) * 65536.0) / SAMPLE_RATE);
	period3 = uint16_t(((frequency3 * semi3 * (1 + detune3 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setDetune1(float detune)
{
	detune1 = detune;
	period1 = uint16_t(((frequency1 * semi1 * (1 + detune1 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setDetune2(float detune)
{
	detune2 = detune;
	period2 = uint16_t(((frequency2 * semi2 * (1 + detune2 + bend)) * 65536.0) / SAMPLE_RATE);
}


void CFOMusic::setDetune3(float detune)
{
	detune3 = detune;
	period3 = uint16_t(((frequency3 * semi3 * (1 + detune3 + bend)) * 65536.0) / SAMPLE_RATE);
}

void CFOMusic::pitchBend(float b)
{
	bend = b;
	period1 = uint16_t(((frequency1 * semi1 * (1 + detune1 + bend)) * 65536.0) / SAMPLE_RATE);
	period2 = uint16_t(((frequency2 * semi2 * (1 + detune2 + bend)) * 65536.0) / SAMPLE_RATE);
	period3 = uint16_t(((frequency3 * semi3 * (1 + detune3 + bend)) * 65536.0) / SAMPLE_RATE);	
}



/////////////////////////////////////
//
//	OSCILLATOR FUNCTIONS
//
/////////////////////////////////////


void CFOMusic::setSine() 
{
	sine = true;
	saw = false;
	square = false;
	waveForm = 0;
}


void CFOMusic::setSaw()
{
	sine = false;
	saw = true;
	square = false;
	waveForm = 1;
}


void CFOMusic::setSquare()
{
	sine = false;
	saw = false;
	square = true;
	waveForm = 2;
}



/////////////////////////////////////
//
//	OVERALL GAIN FUNCTIONS
//
/////////////////////////////////////


void CFOMusic::setGainFloat(float value)
{
	gain = uint16_t(value * 65535);
	gain1 = gain;
	gain2 = gain;
	gain3 = gain;
}


void CFOMusic::setGain16bit(uint16_t value)
{
	gain = value;
	gain1 = value;
	gain2 = value;
	gain3 = value;
}


void CFOMusic::setGain(float value)
{
	gain = uint16_t(value * 65535);
	gain1 = gain;
	gain2 = gain;
	gain3 = gain;
}


void CFOMusic::setGain(uint16_t value)
{
	gain = value;
	gain1 = value;
	gain2 = value;
	gain3 = value;
}



/////////////////////////////////////
//
//	INDIVIDUAL GAIN FUNCTIONS
//
/////////////////////////////////////


void CFOMusic::setGain1(float value)
{
	gain1 = uint16_t(value * 65535);
}


void CFOMusic::setGain2(float value)
{
	gain2 = uint16_t(value * 65535);
}


void CFOMusic::setGain3(float value)
{
	gain3 = uint16_t(value * 65535);
}


void CFOMusic::setGain1(uint16_t value)
{
	gain1 = value;
}


void CFOMusic::setGain2(uint16_t value)
{
	gain2 = value;
}


void CFOMusic::setGain3(uint16_t value)
{
	gain3 = value;
}


float CFOMusic::getGainFloat()
{
	return float(gain);
}


uint16_t CFOMusic::getGain()
{
	return gain;
}




/////////////////////////////////////
//
//	NOTE FUNCTIONS
//
/////////////////////////////////////


void CFOMusic::noteOn(uint8_t note, uint8_t vel)
{	
	envStage = 1;
	velSustain = vel * (sustain/128);
	notePlayed = note;
	memcpy_P(&frequency16bit, &hertsTable[notePlayed],2);
	setFrequency1(frequency16bit);
	setFrequency2(frequency16bit);
	setFrequency3(frequency16bit);
}


void CFOMusic::noteOn(uint8_t note)
{	
	envStage = 1;
	velSustain = sustain;
	notePlayed = note;
	memcpy_P(&frequency16bit, &hertsTable[notePlayed],2);
	setFrequency1(frequency16bit);
	setFrequency2(frequency16bit);
	setFrequency3(frequency16bit);
}


void CFOMusic::noteOff(uint8_t note)
{	
	if(notePlayed == note) {
		envStage = 4;
	}    
}


void CFOMusic::noteOff()
{	
	envStage = 4;
}


uint16_t CFOMusic::getNoteFrequency(uint8_t note)
{
	if(note > 127) note = 127;
	uint16_t freq;
	memcpy_P(&freq, &hertsTable[note],2);
	return freq;
}



/////////////////////////////////////
//
//	ENVELOPE FUNCTIONS
//
/////////////////////////////////////

void CFOMusic::enableEnvelope()
{
	envelopeOn = true;
}


void CFOMusic::disableEnvelope()
{
	envelopeOn = false;
}


void CFOMusic::setEnvStage(uint8_t stage)
{
	envStage = stage;
}


void CFOMusic::setAttack16bit(uint16_t att)
{
	attack = att;
}


void CFOMusic::setDecay16bit(uint16_t dec)
{
	decay = dec;
}


void CFOMusic::setSustain16bit(uint16_t sus)
{
	sustain = sus;
}


void CFOMusic::setRelease16bit(uint16_t rel)
{
	release = rel;
}


void CFOMusic::setAttack(uint8_t att)
{
	if(att>127) att = 127;
	memcpy_P(&attack, &envTimeTable[127 - att],2);
	//attack = envTimeTable[127 - att];
}


void CFOMusic::setDecay(uint8_t dec)
{
	if(dec>127) dec = 127;
	memcpy_P(&decay, &envTimeTable[127 - dec],2);
	//decay = envTimeTable[127 - dec];
}


void CFOMusic::setSustain(uint8_t sus)
{
	sustain = sus * (MAX_ENV_GAIN/128);	
}


void CFOMusic::setRelease(uint8_t rel)
{
	if(rel>127) rel = 127;
	memcpy_P(&release, &envTimeTable[127 - rel],2);
	//release = envTimeTable[127 - rel];
}


void CFOMusic::setVelSustain(uint8_t vel)
{
	velSustain = vel * (sustain / 128);	
}




/////////////////////////////////////
//
//	AUDIO INTERRUPT SERVICE ROUTINE
//
/////////////////////////////////////


void CFOMusic::synthInterrupt()
{
	// Frame sync low for SPI (making it low here so that we can measure lenght of interrupt with scope)
	PORTB &= ~(1<<2);
	
	// The accumulator (16bit) keeps track of the pitch by adding the 
	// the amount of "index" points that the frequency has "travelled" 
	// since the last sample was sent to the DAC, i.e. the current phase
	// of the waveform.
	accumulator1 = accumulator1 + period1;
	accumulator2 = accumulator2 + period2;
	accumulator3 = accumulator3 + period3;

	// To use the accumulator position to find the right index in the 
	// waveform look-up table, we truncate it to 12bit.
	index1 = accumulator1 >> 4;
	index2 = accumulator2 >> 4;
	index3 = accumulator3 >> 4;
	
	// SINE WAVE
	// Because the waveform look-up table resides in program memory
	// we most use memcpy_P to copy the data from that table to our
	// oscilator variable.
	if(sine) {
		memcpy_P(&oscil1, &sineTable[index1],2);
		memcpy_P(&oscil2, &sineTable[index2],2);
		memcpy_P(&oscil3, &sineTable[index3],2);
	}

	// SAWTOOTH WAVE
	// Just using the index for the oscillator produces a sawtooth shaped waveform
	else if(saw) {
		oscil1 = index1;
		oscil2 = index2;
		oscil3 = index3;
	}
	
	// SQUARE WAVE
	else if(square) {
		oscil1 = index1;
		oscil2 = index2;
		oscil3 = index3;
		oscil1 &= 0x0800;
		oscil1 ^= 0x0100;
		oscil2 &= 0x0800;
		oscil2 ^= 0x0100;
		oscil3 &= 0x0800;
		oscil3 ^= 0x0100;
	}
	
	// The DAC formatting routine below assumes the sample to be transmitted
	// is in the higher 12 bits of the 2 byte variable, so we shift the 
	// sample up 2 bits each which adds up to 4 bits.
	// The individual gains for each oscillator is added.
	sample = (oscil1 * gain1) << 2; 
	sample += (oscil2 * gain2) << 2; 
	sample += (oscil3 * gain3) << 2; 
	sample >>= 16;
	

	// AMPLIFICATION ENVELOPE
	// Amplification envelope is calculated here
	if(envelopeOn) {
		
		if(envStage == 1) {
			env += attack;
			if(MAX_ENV_GAIN < env) {
				env = MAX_ENV_GAIN;
				envStage = 2;
			}
		}
		else if(envStage == 2) {
			env -= decay;
			if(env < velSustain || MAX_ENV_GAIN < env) {
				env = velSustain;
				envStage = 3;
			}
		}
		else if (envStage == 3) {
			env = velSustain;
		}
		else if (envStage == 4) {
			env -= release;
			if(MAX_ENV_GAIN < env) {
				env = 0;
				envStage = 0;
			}
		}
		else if (envStage == 0) {
			env = 0;
			accumulator1 = 0;
			accumulator2 = 0;
			accumulator3 = 0;
		}
		
	} else {
		env = 65535;
	}

	// Adding the amplification envelope (16bit) we bring it back to the 16bit frame again afterwards.
	sample = (env * sample) >> 16;
	
	
	// Formatting the samples to be transfered to the MCP4921 DAC  
	dacSPI0 = sample >> 8;
	dacSPI0 >>= 4;
	dacSPI0 |= 0x30;
	dacSPI1 = sample >> 4;
	
	SPCR |= (1 << MSTR);
	
	// transmit value out the SPI port
	SPDR = dacSPI0;
	while (!(SPSR & (1<<SPIF)));  // Maybe this can be optimised
	SPDR = dacSPI1;
	while (!(SPSR & (1<<SPIF)));  // Maybe this can be optimised
	
	// Frame sync high
	PORTB |= (1<<2);
	
}

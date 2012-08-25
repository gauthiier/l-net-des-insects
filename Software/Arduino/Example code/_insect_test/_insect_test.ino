//#define CFO_MIDI_CHANNEL 4

#include <stdint.h>
#include <CFOMidi.h>
#include <CFOMusic.h>


void setup() {
  
  Music.init();  
  Midi.init();
  Midi.setChannel(16); // channel number is 1-16
  Music.setSaw();
  Music.setDetune(0.02);
  Music.enableEnvelope();
  
}

void loop() {
  
  Midi.checkMidi();
  
}


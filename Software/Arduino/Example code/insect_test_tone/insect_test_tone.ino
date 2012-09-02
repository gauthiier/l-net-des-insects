#define CFO_ID 0

#include <stdint.h>
#include <CFOMidi.h>
#include <CFOMusic.h>


void setup() {
  
  Music.init();  
  //Midi.init();
  //Midi.setID(CFO_ID);
  //Midi.setChannel(CFO_ID, 1); // channel number is 1-16
  Music.setSaw();
  //Music.setDetune(0.02);
  //Music.enableEnvelope();
  
}

void loop() {
  
  //Midi.checkMidi();
  
}


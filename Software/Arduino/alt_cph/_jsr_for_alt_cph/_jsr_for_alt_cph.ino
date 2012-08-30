
// This needs to be in all sketches at the moment
#include <stdint.h>

// The Music and Midi objects are automatically instantiated when the header file is included.
// Make calls to the Music and Midi objects with "Music.function(args)" and "Midi.function(args)"
// You still need to call Music.init() and Midi.init() in the setup() function below.
#include <CFOMusic.h>

// variables for this sketch
boolean noteIsOn = false;
int note = 48;

long time = 0;
long lastTime = 0;
int beatTime = 3000;
int originalBeatTime = beatTime;
int timeIncrement = -3;
int lowestBeatTime = 1500;
int highestBeatTime = 10000;

int notes[] = {0,5,7,9};
int octaveSpanL = -3;
int octaveSpanH = 1;
int numberOfNotes = 4;

int randomizeNotes = 100;

float detuning = 0.0;
float origDetuning = detuning;
float maxDetuning = 0.2;
float detuneIncrement = 0.001;

long lastDetune = millis();
int detuneDelay = 10;

int randomizeWave = 100;

int arpFrequency = 50;
int arpCounter = 0;
int arpLenght = 25;
boolean arpMode = false;
int arpDelay = 60;

void setup() {

  // We initialise the sound engine by calling Music.init() which outputs a tone
  Music.init();
  
  // enabling the envelope lets us define an gain envelope for the synth
  // without having to specify it in our loop() or physics code.
  Music.enableEnvelope();
  Music.setAttack(0);
  Music.setDecay(4);
  Music.setSustain(255);
  Music.setRelease(255);
  Music.setSaw();
  
  Serial.begin(9600);
}

void loop() {
  
  // This short routine loops note over and over again
  time = millis();
  if(time - lastTime > beatTime) {
    if(!noteIsOn) {
      
      if (random(randomizeNotes) == 0) {
      
        pickNewNote();
        
      }
      
      Music.noteOn(note);
      noteIsOn = true;
      
      
    } else {
      Music.noteOff();
      noteIsOn = false;
    }
    lastTime = time;
    beatTime = beatTime + timeIncrement;
    if (beatTime < lowestBeatTime || beatTime > highestBeatTime) {
      beatTime = originalBeatTime;
    }
    
    if (random(randomizeWave) == 0) {
      int nextWave = random(4);
      if (nextWave == 0) {
        Music.setSaw();
        Serial.println("now saw");
      } else if (nextWave == 1) {
        Music.setSine();
        Serial.println("now sine");
      } else if (nextWave == 2) {
        Music.setSquare();
        Serial.println("now square");
      }
    }
    
    if (random(arpFrequency) == 0) {
      arpMode = true;
      Serial.println("let the arp begin...");
    }
    

    
    if (lastDetune + detuneDelay < millis()) {
      Music.setDetune(detuning);
      detuning = detuning + detuneIncrement;
      if (detuning > maxDetuning) {
        detuning = origDetuning;
      }
      lastDetune = millis();
    }
    
  }
  
  if (arpMode) {
    pickNewNote();
    Music.noteOn(note+12);
    noteIsOn = true;
    delay(arpDelay);
    
    arpCounter ++;
    
    if (arpCounter > arpLenght) {
      
      arpMode = false;
      arpCounter = 0;
      Serial.println("arp stopped");
      
      pickNewNote();
      Music.noteOn(note-24);
      noteIsOn = true;
      
    }
    
  }

}

void pickNewNote () {
 int currentOctave = random(octaveSpanL, octaveSpanH);
 Serial.println(currentOctave);       
 note = 48+12*currentOctave+notes[random(numberOfNotes)];
}


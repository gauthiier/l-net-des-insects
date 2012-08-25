import rwmidi.*;
import processing.serial.*;

MidiInput input;
//MidiOutput output; //to be implemented later

Serial port0;

int defaultMidiChannel = 9;

void setup() {
  
  size(128,128);
  smooth();
  background(0);
    
  println("print MIDI input devices:");
  println(RWMidi.getInputDeviceNames());
  input = RWMidi.getInputDevices()[0].createInput(this);

  //println("print MIDI ouput devices:");
  //println(RWMidi.getOutputDeviceNames());
  //output = RWMidi.getOutputDevices()[0].createOutput(); // later
  
  println(Serial.list());
  port0 = new Serial(this, Serial.list()[0], 115200);
  
}


void draw() {
  
  if(port0.available() > 0) {
    //int val = port0.read();
    //float val = float(port0.read());
    int val = port0.read();
    println(val); 
  }
  
  
}

void keyPressed() {
  
  if(key=='b' || key=='B') {
    sendNoteOn(byte(defaultMidiChannel), (byte)36, (byte)0x7F); 
  }
  if(key=='s' || key=='S') {
    sendNoteOn(byte(defaultMidiChannel), (byte)38, (byte)0x7F); 
  }
  if(key=='i' || key=='I') {
    sendNoteOn(byte(defaultMidiChannel), (byte)42, (byte)0x7F); 
  }
  if(key=='o' || key=='O') {
    sendNoteOn(byte(defaultMidiChannel), (byte)44, (byte)0x7F); 
  }
  
}

void keyReleased() {

  if(key=='b' || key=='B') {
    sendNoteOff(byte(defaultMidiChannel), (byte)0x00, (byte)0x7F); 
  }
  if(key=='s' || key=='S') {
    sendNoteOff(byte(defaultMidiChannel), (byte)0x01, (byte)0x7F); 
  }
  if(key=='i' || key=='I') {
    sendNoteOff(byte(defaultMidiChannel), (byte)0x02, (byte)0x7F); 
  }
  if(key=='o' || key=='O') {
    sendNoteOff(byte(defaultMidiChannel), (byte)0x03, (byte)0x7F); 
  }
}

void noteOnReceived(Note note) {
  
  byte channel = byte(note.getChannel());
  byte pitch = byte(note.getPitch());
  byte velocity = byte(note.getVelocity());

  sendNoteOn(channel, pitch, velocity);
  
}

void noteOffReceived(Note note) {

  byte channel = byte(note.getChannel());
  byte pitch = byte(note.getPitch());
  byte velocity = byte(note.getVelocity());
  
  sendNoteOff(channel, pitch, velocity);
  
}

void controllerChangeReceived(Controller controller) {
  
  byte channel = byte(controller.getChannel());
  byte CC = byte(controller.getCC());
  byte value = byte(controller.getValue());
  
  sendControlChange(channel, CC, value);
}

void sendNoteOn(byte channel, byte pitch, byte velocity) {

  byte noteOn = byte(0x90 | channel);  
  byte endMessage = byte(0xFF); // not standard midi!!!!

  port0.write(noteOn);
  port0.write(pitch);
  port0.write(velocity);
  port0.write(endMessage);
  println('\n' + hex(noteOn) + " " + hex(pitch) + " " + hex(velocity) + " " + hex(endMessage));

}

void sendNoteOff(byte channel, byte pitch, byte velocity) {

  byte noteOff = byte(0x80 | channel);  
  byte endMessage = byte(0xFF); // not standard midi!!!!

  port0.write(noteOff);
  port0.write(pitch);
  port0.write(velocity);
  port0.write(endMessage);
  println('\n' + hex(noteOff) + " " + hex(pitch) + " " + hex(velocity) + " " + hex(endMessage));

}

void sendControlChange(byte channel, byte CC, byte value) {
  
  byte controlChange = byte(0xB0 | channel);
  byte endMessage = byte(0xFF); // Not standard MIDI!!!!
  
  port0.write(controlChange);
  port0.write(CC);
  port0.write(value);
  port0.write(endMessage);
  println('\n' + hex(controlChange) + " " + hex(CC) + " " + hex(value) + " " + hex(endMessage));
  
}   

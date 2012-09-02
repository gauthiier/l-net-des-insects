

#define R  8
#define G  A5
#define B  7
#define M 4

#define MASTER 0



void setup() {
  
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(M, OUTPUT);

  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);
  digitalWrite(M, LOW);
  
  Serial.begin(9600);
  
}

void loop() {
  
  if(MASTER==1) {
    Serial.print('1');
    digitalWrite(G, HIGH);
    digitalWrite(B, HIGH);
    delay(1000);
    Serial.print('c');
    digitalWrite(G, LOW);
    digitalWrite(B, LOW);
    delay(1000);
  } else {
    while(Serial.available()) {
      char c = (char)Serial.read();
      if(c == '1') {
        digitalWrite(G, HIGH);
        digitalWrite(B, HIGH);
      } else {
        digitalWrite(G, LOW);
        digitalWrite(B, LOW);
      }
    }
  }

}

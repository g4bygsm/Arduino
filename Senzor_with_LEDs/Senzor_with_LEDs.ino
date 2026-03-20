const int trigPin = 13;
const int echoPin = 12;
const int buzzerPin = 3;

const int red = 10;
const int green = 8;
const int yellow = 9;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // --- TREBUIE SA DECLARI PINII CA OUTPUT ---
  pinMode(buzzerPin, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  
  Serial.begin(9600); // Optional, pentru verificare in Serial Monitor
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long cm = microsecondsToCentimeters(duration);
  
  verify(cm);
  
  delay(100); 
}

void verify(int cm){
  // Folosim "else if" pentru a nu suprapune comenzile
  
  // ZONA 1: PERICOL (Mai mic de 10 cm)
  if(cm < 25){
    digitalWrite(red, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
    tone(buzzerPin, 1000); // Sunet ascutit
  }
  // ZONA 2: ATENTIE (Intre 10 si 20 cm)
  else if(cm < 50){
    digitalWrite(red, LOW);
    digitalWrite(yellow, HIGH);
    digitalWrite(green, LOW);
    tone(buzzerPin, 440); // Sunet mai gros
  }
  // ZONA 3: SIGURANTA (Mai mare de 20 cm)
  else {
    digitalWrite(red, LOW);
    digitalWrite(yellow, LOW);
    digitalWrite(green, HIGH);
    noTone(buzzerPin); // Oprim sunetul
  }
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}
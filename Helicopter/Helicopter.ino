#include <Servo.h>

int buttonPin = 2;
int buzzerPin = 4;
int ledPin = 8;
int servoPin = 12;
int del = 1000;

Servo myservo;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  myservo.attach(servoPin); 
  myservo.write(90); // 90 ar trebui să însemne STOP la pornire
}

void loop() {
  if(digitalRead(buttonPin) == LOW){ 
    // BUTON APĂSAT
    
    // Pornim motorul (180 = viteză maximă înainte, 0 = înapoi)
     
    
    digitalWrite(ledPin, HIGH);
    tone(buzzerPin, 1000, 100);
    delay(150);
    tone(buzzerPin, 1500, 200);
    delay(150);
    noTone(buzzerPin);
    for(int i=90; i<=180; i++){
      myservo.write(i);
      delay(100);
      if(i==180){
        delay(3000);
      for(int j=180;j>=90;j--){
        myservo.write(j);
        delay(100);
      }
      }
    }
    
  } else {
    // BUTON NEAPĂSAT
    
    // Oprim motorul
    myservo.write(90); 
    
    digitalWrite(ledPin, LOW);
  }
}
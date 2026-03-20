#include <Keypad.h>
#include <Servo.h>

// --- CONFIGURARE SENSORI ---
// Pentru "deget pe senzor", valoarea trebuie să fie mică.
// Dacă nu se aprinde când pui degetul, scade acest prag (ex: la 100).
int PRAG_INTUNERIC = 250; 
int PRAG_APA = 200;    

// --- PINI COMPONENTE ---
const int PIN_LDR = A0;
const int PIN_WATER = A1;
const int PIN_SERVO = 9;
const int PIN_BUZZER = 10;
const int LED_ROSU = 11;
const int LED_VERDE = 12;
const int LED_ALBASTRU = 13;
const int LED_GALBEN = A2; 

// --- CONFIGURARE KEYPAD (4x3) ---
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {8, 7, 6, 5}; 
byte colPins[COLS] = {4, 3, 2}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- VARIABILE STARE ---
Servo myServo;
String masterCode = "1234";
String inputCode = "";
bool isWaterOk = false;
bool isDarkOk = false;

void setup() {
  Serial.begin(9600);
  myServo.attach(PIN_SERVO);
  myServo.write(90); // Stop servo

  pinMode(LED_ROSU, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ALBASTRU, OUTPUT);
  pinMode(LED_GALBEN, OUTPUT); 
  pinMode(PIN_BUZZER, OUTPUT);
  
  // --- TEST HARDWARE LED-URI ---
  Serial.println("--- TEST INITIALIZARE ---");
  digitalWrite(LED_GALBEN, HIGH); 
  delay(1000); 
  digitalWrite(LED_GALBEN, LOW);
  
  Serial.println("Sistem pregatit. Pune degetul pe senzor si uda senzorul de apa!");
}

void loop() {
  checkPhysicalConditions();
  
  char key = keypad.getKey();
  
  // Ambele conditii (Apa + Intuneric) trebuie sa fie adevarate
  if (isWaterOk && isDarkOk) {
    if (key) {
      handleKey(key);
    }
  } else {
    if (inputCode.length() > 0) {
      inputCode = "";
      tone(PIN_BUZZER, 150, 200);
      Serial.println("Conditii pierdute (ai luat degetul sau senzorul s-a uscat).");
    }
  }
}

void checkPhysicalConditions() {
  int lightVal = analogRead(PIN_LDR);
  int waterVal = analogRead(PIN_WATER);

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    Serial.print("Lumina (mic=intuneric): "); Serial.print(lightVal);
    Serial.print(" | Apa: "); Serial.println(waterVal);
    lastPrint = millis();
  }

  // LOGICA INVERSATA: Conditia e adevarata cand e INTUNERIC (valoare mica)
  isDarkOk = (lightVal < PRAG_INTUNERIC);
  isWaterOk = (waterVal > PRAG_APA);

  digitalWrite(LED_GALBEN, isDarkOk ? HIGH : LOW);
  digitalWrite(LED_ALBASTRU, isWaterOk ? HIGH : LOW);
  
  if (!isWaterOk || !isDarkOk) {
    digitalWrite(LED_ROSU, HIGH);
    digitalWrite(LED_VERDE, LOW);
  } else {
    // Clipim LED-ul roșu pentru a indica faptul că sistemul așteaptă codul de la tastatură
    digitalWrite(LED_ROSU, (millis() % 500 < 250) ? HIGH : LOW);
  }
}

void handleKey(char key) {
  if (key == '#') {
    if (inputCode == masterCode) {
      unlockSafe();
    } else {
      failAlarm();
    }
    inputCode = "";
  } else if (key == '*') {
    inputCode = "";
    tone(PIN_BUZZER, 400, 100);
  } else {
    inputCode += key;
    tone(PIN_BUZZER, 1000, 50); 
    Serial.print("Cod: "); Serial.println(inputCode);
  }
}

void unlockSafe() {
  Serial.println("DESCHIS!");
  digitalWrite(LED_ROSU, LOW);
  digitalWrite(LED_VERDE, HIGH);
  tone(PIN_BUZZER, 2000, 500);

  myServo.write(100); 
  delay(1500);        
  myServo.write(90);  
  
  delay(5000); 
  
  myServo.write(80); 
  delay(1500);
  myServo.write(90);
}

void failAlarm() {
  Serial.println("Cod incorect!");
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_ROSU, LOW);
    tone(PIN_BUZZER, 200, 200);
    delay(100);
    digitalWrite(LED_ROSU, HIGH);
    delay(100);
  }
}
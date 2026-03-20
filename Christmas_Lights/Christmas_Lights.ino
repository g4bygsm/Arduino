#include <Arduino.h>

// --- 1. DEFINIREA NOTELOR ---
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

// --- 2. CONFIGURARE PINI ---
int leduri[] = {2, 3, 4, 5, 6, 7, 8, 9};
int nrLeduri = 8;
const int buzzerPin = 10; // Buzzer pe pin 10

// --- 3. MELODIA COMBINATĂ (Jingle Bells + We Wish You) ---
// Am unit cele doua melodii intr-un singur sir lung
int melody[] = {
  // Jingle Bells
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5,
  
  // Pauza scurta intre melodii (0 = liniste)
  0, 0,
  
  // We Wish You a Merry Christmas
  NOTE_B3, 
  NOTE_F4, NOTE_F4, NOTE_G4, NOTE_F4, NOTE_E4,
  NOTE_D4, NOTE_D4, NOTE_D4,
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_F4,
  NOTE_E4, NOTE_E4, NOTE_E4,
  NOTE_A4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4,
  NOTE_F4, NOTE_D4, NOTE_B3, NOTE_B3,
  NOTE_D4, NOTE_G4, NOTE_E4,
  NOTE_F4
};

int tempo[] = {
  // Jingle Bells
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4,
  
  // Pauza
  4, 4,
  
  // We Wish You
  4,
  4, 8, 8, 8, 8,
  4, 4, 4,
  4, 8, 8, 8, 8,
  4, 4, 4,
  4, 8, 8, 8, 8,
  4, 4, 8, 8,
  4, 4, 4,
  2
};

// --- VARIABILE PENTRU MULTITASKING (nu umbla aici) ---
// Muzica
unsigned long musicLastTime = 0;
int noteIndex = 0;
int noteDuration = 0;
boolean isNotePlaying = false;
int totalNotes;

// Leduri
unsigned long ledLastTime = 0;
int ledInterval = 100; // Viteza ledurilor (ms)
int ledState = 0;      // Pasul curent al modelului
int patternType = 0;   // Ce model ruleaza (0-3)
unsigned long patternChangeTime = 0; // Cand am schimbat ultima oara modelul

void setup() {
  pinMode(buzzerPin, OUTPUT);
  for (int i = 0; i < nrLeduri; i++) {
    pinMode(leduri[i], OUTPUT);
  }
  
  // Calculam cate note sunt in total
  totalNotes = sizeof(melody) / sizeof(int);
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Gestionam Muzica (fara delay)
  playMusic(currentMillis);

  // 2. Gestionam Ledurile (fara delay)
  updateLeds(currentMillis);
}

// --- LOGICA MUZICII ---
void playMusic(unsigned long currentMillis) {
  if (currentMillis - musicLastTime >= noteDuration * 1.30) {
    // E timpul pentru urmatoarea nota
    musicLastTime = currentMillis;

    if (noteIndex >= totalNotes) {
      noteIndex = 0; // Luam melodia de la capat
    }

    int note = melody[noteIndex];
    int duration = tempo[noteIndex];

    if (note != 0) {
      noteDuration = 1000 / duration;
      tone(buzzerPin, note, noteDuration);
    } else {
      noteDuration = 500; // Pauza
      noTone(buzzerPin);
    }
    
    noteIndex++;
  }
}

// --- LOGICA LEDURILOR ---
void updateLeds(unsigned long currentMillis) {
  
  // Schimbam modelul LED la fiecare 5 secunde (5000 ms)
  if (currentMillis - patternChangeTime > 5000) {
    patternType++;
    if (patternType > 3) patternType = 0;
    ledState = 0; // Resetam pasul
    stingeTot();
    patternChangeTime = currentMillis;
  }

  // Aici verificam daca e timpul sa facem un pas in animatia LED
  if (currentMillis - ledLastTime > ledInterval) {
    ledLastTime = currentMillis;
    
    // Executam modelul curent
    switch (patternType) {
      case 0: runKnightRider(); break;
      case 1: runAlternativ(); break;
      case 2: runUmplere(); break;
      case 3: runFlash(); break;
    }
  }
}

// --- IMPLEMENTAREA MODELELOR (RESCRISE FARA FOR LOOP) ---

// Model 1: Knight Rider
void runKnightRider() {
  stingeTot();
  // Folosim ledState ca pozitie (0..14)
  // 0-7: Dus, 8-14: Intors
  int pos = ledState;
  if (pos >= 14) { ledState = 0; pos = 0; }
  
  if (pos < 8) {
    digitalWrite(leduri[pos], HIGH);
  } else {
    // Formula pentru intors: 14 - 8 = 6 (adica ledul 6)
    digitalWrite(leduri[14 - pos], HIGH);
  }
  ledState++;
}

// Model 2: Alternativ
void runAlternativ() {
  ledState++;
  if (ledState > 1) ledState = 0;
  
  for (int i = 0; i < nrLeduri; i++) {
    if (ledState == 0) {
      digitalWrite(leduri[i], (i % 2 == 0) ? HIGH : LOW);
    } else {
      digitalWrite(leduri[i], (i % 2 != 0) ? HIGH : LOW);
    }
  }
}

// Model 3: Umplere
void runUmplere() {
  // 0-7: Umple, 8-15: Goleste
  if (ledState < 8) {
    digitalWrite(leduri[ledState], HIGH);
  } else if (ledState < 16) {
    digitalWrite(leduri[15 - ledState], LOW); // 15-8 = 7
  } else {
    ledState = -1; // Reset
  }
  ledState++;
}

// Model 4: Flash
void runFlash() {
  ledState++;
  if (ledState > 1) ledState = 0;
  
  for (int i = 0; i < nrLeduri; i++) {
    digitalWrite(leduri[i], (ledState == 0) ? HIGH : LOW);
  }
}

void stingeTot() {
  for (int i = 0; i < nrLeduri; i++) {
    digitalWrite(leduri[i], LOW);
  }
}
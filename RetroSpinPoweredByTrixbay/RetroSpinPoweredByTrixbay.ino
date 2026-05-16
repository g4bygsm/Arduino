#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

SoftwareSerial mySoftwareSerial(7, 8); 
DFRobotDFPlayerMini myDFPlayer;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// === PINI ===
const int buttonPausePin = 2;    
const int buttonNextPin = 3;     
const int buttonPrevPin = 4;     
const int motorPin = 5;          
const int busyPin = 6;           
const int buttonModePin = A0;    

// === VITEZA MOTOR ===
int customMotorSpeed = 170; 
bool motorEnabled = true;

// === STARI SISTEM (TrixBay OS) ===
enum SystemState { BOOT, BIOS, OS_MENU, RS_MENU, RS_PLAY, MS_MENU, MS_PLAY, APP_TIMER, APP_8BALL, APP_DICE, APP_ZEN, APP_SETTINGS, SHUTDOWN_SEQ };
SystemState sysState = BOOT;

// === SETARI CUSTOM (EEPROM simulation) ===
int customTextSpeed = 480; 
unsigned long customSleepDelay = 600000; // 10 min
int settingIndex = 1;

// === STATISTICI EEPROM & TIMERS ===
unsigned long totalUptimeMinutes = 0;
unsigned long lastMinuteTimer = 0;
unsigned long lastActivityTime = 0; 
int biosPage = 1;
const int totalBiosPages = 5;

// === VARIABILE RETRO/MODERN SPIN ===
int currentAlbum = 1;            
int currentMSAlbum = 1; // Album pentru Modern Spin
int albumStart = 1;              
int albumEnd = 49;               
int currentTrack = 1;
bool isPlaying = false;          
int currentVolume = 12;          
const int MAX_VOLUME = 20;       
unsigned long trackStartTime = 0; 
int rsDisplayMode = 0; // 0=Nume, 1=Vinyl, 2=Meniu SS, 3=Egalizator
bool ssMenuState = false;

// === VARIABILE APLICATII ===
int currentOSApp = 1;
const int totalOSApps = 8; 
bool appInit = false; 

// Timer App (Background)
int timerSeconds = 0;
bool timerRunning = false;
bool timerAlarm = false;
unsigned long timerLastMillis = 0;
unsigned long alarmPulseTime = 0;
bool alarmToggle = false;

// Zen App
unsigned long zenTimer = 0;
int zenStep = 0;

// Egalizator
int eqHeights[16] = {0};

// === VARIABILE TEXT CULISANT ===
char currentSongName[55]; // Marit pentru titluri lungi
unsigned long lastScrollTime = 0;
int scrollPos = 0;
bool isTextLong = false;

// === VARIABILE UI & VOLUM ===
bool isVolumeOnScreen = false;
unsigned long showVolumeTimer = 0;

// === VARIABILE BUTOANE ===
bool isPausePressed = false; unsigned long pausePressTime = 0; bool pauseHandled = false; bool menuFeedbackShown = false;
bool isNextPressed = false;  unsigned long nextPressTime = 0;  bool nextLongHandled = false;
bool isPrevPressed = false;  unsigned long prevPressTime = 0;  bool prevLongHandled = false;
bool isModePressed = false;  unsigned long modePressTime = 0;  bool modeLongHandled = false;

unsigned long lastVolChangeTime = 0; 
const int LONG_PRESS_TIME = 500;     
const int VOL_CHANGE_SPEED = 300;    

// === PIXEL ART ===
byte notaMuzicala[8] = { B00010, B00011, B00010, B00010, B01110, B11110, B01100, B00000 };
byte blockChar[8]    = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };
byte bar1[8]         = { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B11111 };
byte bar2[8]         = { B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111 };
byte bar3[8]         = { B00000, B00000, B00000, B11111, B11111, B11111, B11111, B11111 };

void startMotorSoft() {
  if (!motorEnabled) return;
  analogWrite(motorPin, 255); delay(150); analogWrite(motorPin, customMotorSpeed); 
}
void stopMotor() { analogWrite(motorPin, 0); }

// ==========================================
// DATE TEXT MELODII (ARTISTI/GENURI)
// ==========================================
void printAutor(int track) {
  if (track >= 1 && track <= 9) { lcd.print(F("> >- QUEEN -< <")); }
  else if (track >= 10 && track <= 20) { lcd.print(F("> > > ABBA < < <")); }
  else if (track >= 21 && track <= 28) { lcd.print(F(">Michael Jackson")); }
  else if (track == 29) { lcd.print(F("> > Nirvana < <")); }
  else if (track >= 30 && track <= 32) { lcd.print(F("> Guns 'n Roses <")); }
  else if (track >= 33 && track <= 35) { lcd.print(F("=>>> AC/DC <<<=")); }
  else if (track >= 36 && track <= 37) { lcd.print(F("=>\\ Bon Jovi \\<=")); }
  else if (track == 38) { lcd.print(F("  = The Police =")); }
  else if (track == 39) { lcd.print(F("= Cindi Lauper =")); }
  else if (track == 40) { lcd.print(F("  === a-ha ===")); }
  else if (track == 41) { lcd.print(F("> > Haddaway < <")); }
  else if (track == 42) { lcd.print(F("> Rick Astley <")); }
  else if (track == 43) { lcd.print(F("  >>> Wham! <<<")); }
  else if (track == 44) { lcd.print(F(">BackStreet Boys")); }
  else if (track == 45) { lcd.print(F(">>Cranberries<<")); }
  else if (track == 46) { lcd.print(F("> - > Toto < - <")); }
  else if (track == 47) { lcd.print(F("= Phill Colins =")); }
  else if (track == 48) { lcd.print(F("= Ben E. King =")); }
  else if (track == 49) { lcd.print(F(">George Michael")); }
  else if (track >= 53 && track <= 58) { lcd.print(F("> > > BLUES < < <")); }
  else if (track >= 59 && track <= 69) { lcd.print(F("> > > HOUSE < < <")); }
  else if (track >= 70 && track <= 93) { lcd.print(F("=>>> MANELE <<<<=")); }
  else if (track >= 94 && track <= 106) { lcd.print(F("  > > POP < <  ")); }
  else if (track >= 107 && track <= 117) { lcd.print(F(" > > POPCORN < <")); }
  else if (track >= 118 && track <= 125) { lcd.print(F("> > REGGAETON < <")); }
  else if (track >= 126 && track <= 131) { lcd.print(F(" > > SUMMER < < ")); }
}

void loadSongName(int track) {
  switch(track) {
    case 1: strcpy_P(currentSongName, PSTR("Bohemian Rhapsody")); break;
    case 2: strcpy_P(currentSongName, PSTR("I Want To Break Free")); break; 
    case 3: strcpy_P(currentSongName, PSTR("We Will Rock You")); break;
    case 4: strcpy_P(currentSongName, PSTR("Another One Bites The Dust")); break; 
    case 5: strcpy_P(currentSongName, PSTR("Don't Stop Me Now")); break; 
    case 6: strcpy_P(currentSongName, PSTR("Somebody To Love")); break;
    case 7: strcpy_P(currentSongName, PSTR("The Show Must Go On")); break; 
    case 8: strcpy_P(currentSongName, PSTR("Under Pressure")); break;
    case 9: strcpy_P(currentSongName, PSTR("We Are The Champions")); break;
    case 10: strcpy_P(currentSongName, PSTR("Gimme Gimme Gimme")); break;
    case 11: strcpy_P(currentSongName, PSTR("Dancing Queen")); break;
    case 12: strcpy_P(currentSongName, PSTR("Super Trouper")); break;
    case 13: strcpy_P(currentSongName, PSTR("Angeleyes")); break;
    case 14: strcpy_P(currentSongName, PSTR("The Winner Takes It All")); break;
    case 15: strcpy_P(currentSongName, PSTR("Take A Chance On Me")); break;
    case 16: strcpy_P(currentSongName, PSTR("Money Money Money")); break;
    case 17: strcpy_P(currentSongName, PSTR("Mamma Mia")); break;
    case 18: strcpy_P(currentSongName, PSTR("Fernando")); break;
    case 19: strcpy_P(currentSongName, PSTR("Chiquitita")); break;
    case 20: strcpy_P(currentSongName, PSTR("I Have A Dream")); break;
    case 21: strcpy_P(currentSongName, PSTR("Billie Jean")); break;
    case 22: strcpy_P(currentSongName, PSTR("Human Nature")); break;
    case 23: strcpy_P(currentSongName, PSTR("Beat It")); break;
    case 24: strcpy_P(currentSongName, PSTR("Earth Song")); break;
    case 25: strcpy_P(currentSongName, PSTR("Smooth Criminal")); break;
    case 26: strcpy_P(currentSongName, PSTR("Thriller")); break;
    case 27: strcpy_P(currentSongName, PSTR("They Don't Care About Us")); break;
    case 28: strcpy_P(currentSongName, PSTR("Black or White")); break;
    case 29: strcpy_P(currentSongName, PSTR("Smells Like Teen Spirit")); break;
    case 30: strcpy_P(currentSongName, PSTR("Sweet Child O' Mine")); break;
    case 31: strcpy_P(currentSongName, PSTR("Knockin' On Heaven's Door")); break;
    case 32: strcpy_P(currentSongName, PSTR("Welcome To The Jungle")); break;
    case 33: strcpy_P(currentSongName, PSTR("Back In Black")); break;
    case 34: strcpy_P(currentSongName, PSTR("Thunderstruck")); break;
    case 35: strcpy_P(currentSongName, PSTR("Highway To Hell")); break;
    case 36: strcpy_P(currentSongName, PSTR("It's My Life")); break;
    case 37: strcpy_P(currentSongName, PSTR("Livin' On A Prayer")); break;
    case 38: strcpy_P(currentSongName, PSTR("Every Breath You Take")); break;
    case 39: strcpy_P(currentSongName, PSTR("Girls Just Want to Have Fun")); break;
    case 40: strcpy_P(currentSongName, PSTR("Take On Me")); break;
    case 41: strcpy_P(currentSongName, PSTR("What Is Love")); break;
    case 42: strcpy_P(currentSongName, PSTR("Never Gonna Give You Up")); break;
    case 43: strcpy_P(currentSongName, PSTR("Wake Me Up Before You Go-Go")); break;
    case 44: strcpy_P(currentSongName, PSTR("I Want It That Way")); break;
    case 45: strcpy_P(currentSongName, PSTR("Zombie")); break;
    case 46: strcpy_P(currentSongName, PSTR("Africa")); break;
    case 47: strcpy_P(currentSongName, PSTR("Another Day In Paradise")); break;
    case 48: strcpy_P(currentSongName, PSTR("Stand By Me")); break;
    case 49: strcpy_P(currentSongName, PSTR("Careless Whisper")); break;
    
    // SFX: 50(Alarm), 51(Startup), 52(Shutdown) - ignorate de display

    case 53: strcpy_P(currentSongName, PSTR("A sky full of stars")); break;
    case 54: strcpy_P(currentSongName, PSTR("Lady Gaga X Bradley Cooper - Shallow")); break;
    case 55: strcpy_P(currentSongName, PSTR("Loreen - Tattoo")); break;
    case 56: strcpy_P(currentSongName, PSTR("The Urs - Ancora")); break;
    case 57: strcpy_P(currentSongName, PSTR("The Urs - Pamantul")); break;
    case 58: strcpy_P(currentSongName, PSTR("The Urs - Taramul Interzis")); break;
    case 59: strcpy_P(currentSongName, PSTR("AMI X Tata Vlad - Enigma")); break;
    case 60: strcpy_P(currentSongName, PSTR("Andia - De la Dela")); break;
    case 61: strcpy_P(currentSongName, PSTR("Dirty Nano - August")); break;
    case 62: strcpy_P(currentSongName, PSTR("Bakermat - Baiana")); break;
    case 63: strcpy_P(currentSongName, PSTR("Dirty Nano ADI - Anestezic")); break;
    case 64: strcpy_P(currentSongName, PSTR("Dirty Nano X EMAA - Zburatorul")); break;
    case 65: strcpy_P(currentSongName, PSTR("Dirty Nano X The Motans - Insula")); break;
    case 66: strcpy_P(currentSongName, PSTR("James Hype - Disconnected")); break;
    case 67: strcpy_P(currentSongName, PSTR("Raffa Guido - Famax")); break;
    case 68: strcpy_P(currentSongName, PSTR("Hugel - Morenita")); break;
    case 69: strcpy_P(currentSongName, PSTR("Ferreck Dawn - Life")); break;
    case 70: strcpy_P(currentSongName, PSTR("Alessio - Am doua blonde")); break;
    case 71: strcpy_P(currentSongName, PSTR("Alex Botea - 112 Arabii")); break;
    case 72: strcpy_P(currentSongName, PSTR("Alex Botea - Decibeli")); break;
    case 73: strcpy_P(currentSongName, PSTR("Florin Salam - Avioanele Americane")); break;
    case 74: strcpy_P(currentSongName, PSTR("Babasha - Marae")); break;
    case 75: strcpy_P(currentSongName, PSTR("Nicolae Guta - Baterie foc foc")); break;
    case 76: strcpy_P(currentSongName, PSTR("Costel Biju - Printesa din Dubai")); break;
    case 77: strcpy_P(currentSongName, PSTR("Florin Salam - Shikdum Shikdum")); break;
    case 78: strcpy_P(currentSongName, PSTR("Florin Salam X Florin Cercel - Bomba")); break;
    case 79: strcpy_P(currentSongName, PSTR("Lele - Peter Parker")); break;
    case 80: strcpy_P(currentSongName, PSTR("Luis Gabriel - Ramai talent")); break;
    case 81: strcpy_P(currentSongName, PSTR("Mariano - Iubire de o vara")); break;
    case 82: strcpy_P(currentSongName, PSTR("Tzanca Uraganu - 24 de karate")); break;
    case 83: strcpy_P(currentSongName, PSTR("Vali Vijelie X Jean - Cheia inima mea")); break;
    case 84: strcpy_P(currentSongName, PSTR("Carmen de la Salciua - Paraseste-ma")); break;
    case 85: strcpy_P(currentSongName, PSTR("Florin Salam X Claudia - Ce frumoasa e dragostea")); break;
    case 86: strcpy_P(currentSongName, PSTR("Leo de la Rosiori - Cum naiba sa nu te mai iubesc")); break;
    case 87: strcpy_P(currentSongName, PSTR("Nicolae Guta - Nemuritorul")); break;
    case 88: strcpy_P(currentSongName, PSTR("Nicolae Guta - Copacabana")); break;
    case 89: strcpy_P(currentSongName, PSTR("Nicolae Guta - Doua zile in calendar")); break;
    case 90: strcpy_P(currentSongName, PSTR("Nicolae Guta - Nu poti iubi")); break;
    case 91: strcpy_P(currentSongName, PSTR("Florin Salam - Pe o insula pustie")); break;
    case 92: strcpy_P(currentSongName, PSTR("Petrica Cercel - Daca ploua")); break;
    case 93: strcpy_P(currentSongName, PSTR("Nicolae Guta - Vin la tine puisor")); break;
    case 94: strcpy_P(currentSongName, PSTR("Clean Bandit - Rockabye")); break;
    case 95: strcpy_P(currentSongName, PSTR("Celia - Trag aer in piept")); break;
    case 96: strcpy_P(currentSongName, PSTR("Akon - Dangerous")); break;
    case 97: strcpy_P(currentSongName, PSTR("Madcom - Freaky Like Me")); break;
    case 98: strcpy_P(currentSongName, PSTR("Habibi Love")); break;
    case 99: strcpy_P(currentSongName, PSTR("Havana - Vita Bella")); break;
    case 100: strcpy_P(currentSongName, PSTR("Lykke Li - I Follow Rivers")); break;
    case 101: strcpy_P(currentSongName, PSTR("OneRepublic - Counting Stars")); break;
    case 102: strcpy_P(currentSongName, PSTR("R. City - Make Up")); break;
    case 103: strcpy_P(currentSongName, PSTR("Tinnie Tempah - Written in the stars")); break;
    case 104: strcpy_P(currentSongName, PSTR("Trumpet Lights")); break;
    case 105: strcpy_P(currentSongName, PSTR("Rihanna - We Found Love")); break;
    case 106: strcpy_P(currentSongName, PSTR("Whats UP X Andra - K la meteo")); break;
    case 107: strcpy_P(currentSongName, PSTR("Akcent - My Passion")); break;
    case 108: strcpy_P(currentSongName, PSTR("Alex Ferrari - Bara Bara Bere Bere")); break;
    case 109: strcpy_P(currentSongName, PSTR("Andreea Banica - Could U")); break;
    case 110: strcpy_P(currentSongName, PSTR("Connect-R - Ring The Alarm")); break;
    case 111: strcpy_P(currentSongName, PSTR("David Deejay feat Dony - Temptation")); break;
    case 112: strcpy_P(currentSongName, PSTR("DJ Project x AMI - 4 Camere")); break;
    case 113: strcpy_P(currentSongName, PSTR("DJ Antoine - Ma Cherie")); break;
    case 114: strcpy_P(currentSongName, PSTR("Edward Maya - This is my life")); break;
    case 115: strcpy_P(currentSongName, PSTR("Glance feat. Mandinga - Cinema")); break;
    case 116: strcpy_P(currentSongName, PSTR("Puya - Americandrim")); break;
    case 117: strcpy_P(currentSongName, PSTR("Radio Killer - Lonely Heart")); break;
    case 118: strcpy_P(currentSongName, PSTR("Baby Hello")); break;
    case 119: strcpy_P(currentSongName, PSTR("CNCO - Reggaeton Lento")); break;
    case 120: strcpy_P(currentSongName, PSTR("Enrique Iglesias - Taking Back My Love")); break;
    case 121: strcpy_P(currentSongName, PSTR("Enrique Iglesias - Duele el corazon")); break;
    case 122: strcpy_P(currentSongName, PSTR("Gente de Zona - La Gozadera")); break;
    case 123: strcpy_P(currentSongName, PSTR("Gusttavo Lima - Balada")); break;
    case 124: strcpy_P(currentSongName, PSTR("Michel Telo - Ai Se Eu Te Pego")); break;
    case 125: strcpy_P(currentSongName, PSTR("Dhurata Dora - Only you")); break;
    case 126: strcpy_P(currentSongName, PSTR("Calvin Harris - Summer")); break;
    case 127: strcpy_P(currentSongName, PSTR("Farruko - Pepas")); break;
    case 128: strcpy_P(currentSongName, PSTR("Fly Project - Get Wet")); break;
    case 129: strcpy_P(currentSongName, PSTR("Mr. Probz - Waves")); break;
    case 130: strcpy_P(currentSongName, PSTR("Robin Schulz - Prayer In C")); break;
    case 131: strcpy_P(currentSongName, PSTR("Robin Schulz - Sugar")); break;

    default: strcpy_P(currentSongName, PSTR("Playing Track...")); break;
  }
  
  if (strlen(currentSongName) > 16) { isTextLong = true; strcat(currentSongName, " *** "); } 
  else { isTextLong = false; }
  scrollPos = 0; 
}

// ==========================================
// SETUP & BOOT
// ==========================================
void setup() {
  Serial.begin(9600); randomSeed(analogRead(A1)); 
  pinMode(buttonPausePin, INPUT_PULLUP); pinMode(buttonNextPin, INPUT_PULLUP);
  pinMode(buttonPrevPin, INPUT_PULLUP); pinMode(buttonModePin, INPUT_PULLUP);
  pinMode(busyPin, INPUT); pinMode(motorPin, OUTPUT); analogWrite(motorPin, 0); 

  lcd.init(); lcd.backlight();
  lcd.createChar(1, notaMuzicala); lcd.createChar(2, blockChar);
  lcd.createChar(3, bar1); lcd.createChar(4, bar2); lcd.createChar(5, bar3); lcd.clear();
  
  long bootCount = EEPROM.read(0) + (EEPROM.read(1) << 8); if(bootCount < 0 || bootCount > 65000) bootCount = 0;
  bootCount++; EEPROM.write(0, bootCount & 0xFF); EEPROM.write(1, (bootCount >> 8) & 0xFF);
  EEPROM.get(2, totalUptimeMinutes); if(totalUptimeMinutes == 0xFFFFFFFF) totalUptimeMinutes = 0; 

  lastMinuteTimer = millis(); lastActivityTime = millis();
  bool enterBIOS = false; lcd.setCursor(1,0); lcd.print(F("  BUN VENIT!  ")); 
  for(int i=0; i<50; i++) { if(digitalRead(buttonModePin) == LOW) { enterBIOS = true; break; } delay(40); }

  if(enterBIOS) {
    sysState = BIOS; lcd.clear(); lcd.setCursor(0,0); lcd.print(F(">> ENTER BIOS <<")); delay(1000);
  } else {
    // --- PORNIRE MODUL AUDIO & STARTUP SOUND ---
    mySoftwareSerial.begin(9600);
    bool audioOk = myDFPlayer.begin(mySoftwareSerial, true, false);
    if(audioOk) {
      myDFPlayer.volume(currentVolume); 
      delay(100);
      myDFPlayer.playMp3Folder(51); // 0051.mp3 începe exact când apare animația TrixBay OS
    }

    // --- ANIMATIA DE BOOT TRIXBAY OS ---
    lcd.clear(); for(int i=0; i<3; i++) { lcd.noBacklight(); delay(100); lcd.backlight(); delay(100); }
    lcd.setCursor(3, 0); lcd.print(F("TrixBay OS")); lcd.setCursor(6, 1); lcd.print(F("v1.4"));
    for(int i=0; i<=15; i++) {
      lcd.setCursor(i, 0); if(i!=3 && i!=4 && i!=5 && i!=6 && i!=7 && i!=8 && i!=9 && i!=10 && i!=11 && i!=12) lcd.write(2);
      lcd.setCursor(15-i, 1); if((15-i)!=6 && (15-i)!=7 && (15-i)!=8 && (15-i)!=9) lcd.write(2);
      delay(80);
    }
    
    if(!audioOk) { lcd.clear(); lcd.print(F("AUDIO SYS ERROR!")); while(true); }
    
    delay(1000); 
    sysState = OS_MENU; appInit = false;
  }
}

// ==========================================
// LOOP & STATE MACHINE
// ==========================================
void loop() {
  checkGlobalButtons();

  if (timerRunning) {
    if (millis() - timerLastMillis >= 1000) {
      timerLastMillis = millis(); timerSeconds--; 
      if(sysState == APP_TIMER) appInit = false; 
      
      if (timerSeconds <= 0) { 
        timerRunning = false; timerAlarm = true; alarmPulseTime = millis(); 
        isPlaying = false; stopMotor(); 
        myDFPlayer.playMp3Folder(50);  
        sysState = APP_TIMER; appInit = false; 
      }
    }
  }

  if (millis() - lastMinuteTimer >= 60000) { lastMinuteTimer = millis(); totalUptimeMinutes++; EEPROM.put(2, totalUptimeMinutes); }
  if (!isPlaying && sysState != SHUTDOWN_SEQ && customSleepDelay > 0 && !timerRunning && !timerAlarm) {
    if (millis() - lastActivityTime > customSleepDelay) sysState = SHUTDOWN_SEQ;
  }

  if (isVolumeOnScreen && (millis() - showVolumeTimer > 800)) {
    isVolumeOnScreen = false;
    if (sysState == RS_PLAY || sysState == MS_PLAY) { 
      appInit = false; 
      if (rsDisplayMode == 0) { lcd.clear(); lcd.setCursor(0,0); printAutor(currentTrack); }
      else if (rsDisplayMode == 3) { lcd.clear(); } 
    } 
    else { appInit = false; }
  }

  if (!isVolumeOnScreen) {
    switch (sysState) {
      case BIOS: runBIOS(); break;
      case OS_MENU: runOSMenu(); break;
      case RS_MENU: runRetroSpinMenu(); break;
      case RS_PLAY: runSpinPlay(); break; 
      case MS_MENU: runModernSpinMenu(); break;
      case MS_PLAY: runSpinPlay(); break;
      case APP_TIMER: runTimerApp(); break;
      case APP_8BALL: run8BallApp(); break;
      case APP_DICE: runDiceApp(); break;
      case APP_ZEN: runZenApp(); break;
      case APP_SETTINGS: runSettingsApp(); break;
      case SHUTDOWN_SEQ: runShutdownAnimation(); break;
    }
  } else {
    if ((sysState == RS_PLAY || sysState == MS_PLAY) && isPlaying && (millis() - trackStartTime > 3500)) {
      if (digitalRead(busyPin) == HIGH) { delay(100); if (digitalRead(busyPin) == HIGH) {
          currentTrack++; if (currentTrack > albumEnd) currentTrack = albumStart; startPlayback(currentTrack); 
      }}
    }
  }
}

// ==========================================
// CONTROL GLOBAL BUTOANE
// ==========================================
void checkGlobalButtons() {
  int pState = digitalRead(buttonPausePin);
  if (pState == LOW && !isPausePressed) { isPausePressed = true; pausePressTime = millis(); pauseHandled = false; menuFeedbackShown = false; lastActivityTime = millis(); }
  if (pState == LOW && isPausePressed) {
    unsigned long holdTime = millis() - pausePressTime;
    if (holdTime >= 3000 && holdTime < 7000 && !menuFeedbackShown) {
      if(sysState == RS_PLAY || sysState == MS_PLAY) { lcd.clear(); lcd.setCursor(1,0); lcd.print(F("Elibereaza pt.")); lcd.setCursor(2,1); lcd.print(F("MENIU ALBUME")); menuFeedbackShown = true; }
    }
    if (holdTime >= 7000 && !pauseHandled) { sysState = SHUTDOWN_SEQ; pauseHandled = true; }
  }
  if (pState == HIGH && isPausePressed) {
    isPausePressed = false; unsigned long holdTime = millis() - pausePressTime;
    if (!pauseHandled) {
      if (holdTime >= 3000) {
        if(sysState == RS_PLAY) { myDFPlayer.stop(); stopMotor(); isPlaying = false; sysState = RS_MENU; appInit = false; rsDisplayMode = 0; }
        else if(sysState == MS_PLAY) { myDFPlayer.stop(); stopMotor(); isPlaying = false; sysState = MS_MENU; appInit = false; rsDisplayMode = 0; }
      } else if (holdTime > 50) { handlePauseClick(); }
    }
  }

  int mState = digitalRead(buttonModePin);
  if (mState == LOW && !isModePressed) { isModePressed = true; modePressTime = millis(); modeLongHandled = false; lastActivityTime = millis(); }
  if (mState == LOW && isModePressed) {
    if (millis() - modePressTime > 2000 && !modeLongHandled) {
      if(sysState != OS_MENU && sysState != BIOS && sysState != SHUTDOWN_SEQ) { exitToOS(); } modeLongHandled = true;
    }
  }
  if (mState == HIGH && isModePressed) {
    isModePressed = false; if (!modeLongHandled && (millis() - modePressTime > 50)) { handleModeClick(); }
  }

  int nState = digitalRead(buttonNextPin);
  if (nState == LOW && !isNextPressed) { isNextPressed = true; nextPressTime = millis(); nextLongHandled = false; lastActivityTime = millis(); }
  if (nState == LOW && isNextPressed && (millis() - nextPressTime > LONG_PRESS_TIME)) {
    if (millis() - lastVolChangeTime > VOL_CHANGE_SPEED && currentVolume < MAX_VOLUME) { currentVolume++; myDFPlayer.volume(currentVolume); drawVolume(); lastVolChangeTime = millis(); }
    nextLongHandled = true;
  }
  if (nState == HIGH && isNextPressed) { isNextPressed = false; if (!nextLongHandled && (millis() - nextPressTime > 50)) handleNextClick(); }

  int prState = digitalRead(buttonPrevPin);
  if (prState == LOW && !isPrevPressed) { isPrevPressed = true; prevPressTime = millis(); prevLongHandled = false; lastActivityTime = millis(); }
  if (prState == LOW && isPrevPressed && (millis() - prevPressTime > LONG_PRESS_TIME)) {
    if (millis() - lastVolChangeTime > VOL_CHANGE_SPEED && currentVolume > 0) { currentVolume--; myDFPlayer.volume(currentVolume); drawVolume(); lastVolChangeTime = millis(); }
    prevLongHandled = true;
  }
  if (prState == HIGH && isPrevPressed) { isPrevPressed = false; if (!prevLongHandled && (millis() - prevPressTime > 50)) handlePrevClick(); }
}

void exitToOS() {
  myDFPlayer.pause(); stopMotor(); isPlaying = false; appInit = false; sysState = OS_MENU; rsDisplayMode = 0;
  lcd.clear(); lcd.setCursor(3,0); lcd.print(F("EXITING...")); delay(800);
}

// ==========================================
// RUTARE CLICK-URI
// ==========================================
void handlePauseClick() {
  if (sysState == OS_MENU) {
    appInit = false;
    if(currentOSApp == 1) sysState = RS_MENU;
    else if(currentOSApp == 2) sysState = MS_MENU;
    else if(currentOSApp == 3) sysState = APP_TIMER;
    else if(currentOSApp == 4) sysState = APP_8BALL;
    else if(currentOSApp == 5) sysState = APP_DICE;
    else if(currentOSApp == 6) sysState = APP_ZEN;
    else if(currentOSApp == 7) sysState = APP_SETTINGS;
    else if(currentOSApp == 8) sysState = SHUTDOWN_SEQ;
  }
  else if (sysState == BIOS) { void(* resetFunc) (void) = 0; resetFunc(); }
  else if (sysState == RS_MENU) { 
    setAlbumLimits(); currentTrack = albumStart; 
    motorEnabled = true; // Activat by default la Retro
    lcd.clear(); lcd.setCursor(1,0); lcd.print(F("Pregatire Disc")); 
    delay(1500); sysState = RS_PLAY; rsDisplayMode = 0; startPlayback(currentTrack); 
  }
  else if (sysState == MS_MENU) { 
    setMSAlbumLimits(); currentTrack = albumStart; 
    motorEnabled = false; // Dezactivat by default la Modern
    lcd.clear(); lcd.setCursor(1,0); lcd.print(F("Pregatire Date")); 
    delay(1500); sysState = MS_PLAY; rsDisplayMode = 0; startPlayback(currentTrack); 
  }
  else if (sysState == RS_PLAY || sysState == MS_PLAY) {
    if (rsDisplayMode == 2) { 
      if (ssMenuState) { rsDisplayMode = 3; lcd.clear(); } else { rsDisplayMode = 0; lcd.clear(); lcd.setCursor(0,0); printAutor(currentTrack); } appInit = false;
    } else {
      if (isPlaying) { myDFPlayer.pause(); stopMotor(); isPlaying = false; rsDisplayMode = 0; lcd.clear(); lcd.setCursor(2,0); lcd.print(F("== PAUZA ==")); }
      else { myDFPlayer.start(); startMotorSoft(); isPlaying = true; appInit = false; if(rsDisplayMode == 0) { lcd.clear(); lcd.setCursor(0,0); printAutor(currentTrack); } }
    }
  }
  else if (sysState == APP_TIMER) {
    if(timerAlarm) { timerAlarm = false; timerRunning = false; timerSeconds = 0; appInit=false; lcd.backlight(); stopMotor(); myDFPlayer.stop(); }
    else if(timerSeconds > 0) { timerRunning = !timerRunning; appInit=false; }
  }
  else if (sysState == APP_SETTINGS) {
    if(settingIndex == 1) { customTextSpeed = (customTextSpeed == 480) ? 200 : 480; }
    else if(settingIndex == 2) { if(customSleepDelay == 600000) customSleepDelay = 1800000; else if(customSleepDelay == 1800000) customSleepDelay = 0; else customSleepDelay = 600000; }
    else if(settingIndex == 3) { if(customMotorSpeed == 170) customMotorSpeed = 220; else if(customMotorSpeed == 220) customMotorSpeed = 120; else customMotorSpeed = 170; }
    else if(settingIndex == 4) { EEPROM.write(0,0); EEPROM.write(1,0); EEPROM.put(2, (unsigned long)0); lcd.clear(); lcd.print(F("RESTARTING...")); delay(1000); void(* r) (void) = 0; r(); }
    appInit = false;
  }
}

void handleNextClick() {
  if (sysState == OS_MENU) { currentOSApp++; if(currentOSApp > totalOSApps) currentOSApp = 1; appInit=false; }
  else if (sysState == BIOS) { biosPage++; if(biosPage > totalBiosPages) biosPage = 1; appInit=false; }
  else if (sysState == RS_MENU) { currentAlbum++; if(currentAlbum > 5) currentAlbum = 1; appInit=false; }
  else if (sysState == MS_MENU) { currentMSAlbum++; if(currentMSAlbum > 7) currentMSAlbum = 1; appInit=false; }
  else if (sysState == RS_PLAY || sysState == MS_PLAY) {
    if (rsDisplayMode == 0 || rsDisplayMode == 3) { currentTrack++; if(currentTrack > albumEnd) currentTrack = albumStart; startPlayback(currentTrack); }
    else if (rsDisplayMode == 1) { motorEnabled = !motorEnabled; appInit=false; if(isPlaying) { if(motorEnabled) startMotorSoft(); else stopMotor(); } }
    else if (rsDisplayMode == 2) { ssMenuState = !ssMenuState; appInit = false; }
  }
  else if (sysState == APP_TIMER && !timerRunning && !timerAlarm) { timerSeconds += 5; appInit=false; }
  else if (sysState == APP_8BALL) {
    lcd.clear(); lcd.setCursor(4,0); lcd.print(F("GANDIND...")); delay(800); lcd.clear(); lcd.setCursor(0,0); lcd.print(F("Raspunsul tau:")); lcd.setCursor(0,1);
    int r = random(0, 5); if(r==0) lcd.print(F("> DA, CLAR! <")); else if(r==1) lcd.print(F("> ABSOLUT NU <")); else if(r==2) lcd.print(F("> POATE... <")); else if(r==3) lcd.print(F("> INTREABA IAR <")); else lcd.print(F("> E UN MISTER <"));
  }
  else if (sysState == APP_DICE) { lcd.clear(); lcd.print(F("Aruncare...")); delay(500); lcd.clear(); lcd.setCursor(0,0); lcd.print(F("Zarul a picat:")); lcd.setCursor(6,1); lcd.print(F("[ ")); lcd.print(random(1,7)); lcd.print(F(" ]")); }
  else if (sysState == APP_SETTINGS) { settingIndex++; if(settingIndex > 4) settingIndex = 1; appInit=false; }
}

void handlePrevClick() {
  if (sysState == OS_MENU) { currentOSApp--; if(currentOSApp < 1) currentOSApp = totalOSApps; appInit=false; }
  else if (sysState == BIOS) { biosPage--; if(biosPage < 1) biosPage = totalBiosPages; appInit=false; }
  else if (sysState == RS_MENU) { currentAlbum--; if(currentAlbum < 1) currentAlbum = 5; appInit=false; }
  else if (sysState == MS_MENU) { currentMSAlbum--; if(currentMSAlbum < 1) currentMSAlbum = 7; appInit=false; }
  else if (sysState == RS_PLAY || sysState == MS_PLAY) {
    if (rsDisplayMode == 0 || rsDisplayMode == 3) { currentTrack--; if(currentTrack < albumStart) currentTrack = albumEnd; startPlayback(currentTrack); }
    else if (rsDisplayMode == 1) { motorEnabled = !motorEnabled; appInit=false; if(isPlaying) { if(motorEnabled) startMotorSoft(); else stopMotor(); } }
    else if (rsDisplayMode == 2) { ssMenuState = !ssMenuState; appInit = false; }
  }
  else if (sysState == APP_TIMER && !timerRunning && !timerAlarm) { if(timerSeconds >= 5) timerSeconds -= 5; appInit=false; }
  else if (sysState == APP_SETTINGS) { settingIndex--; if(settingIndex < 1) settingIndex = 4; appInit=false; }
}

void handleModeClick() {
  if (sysState == RS_PLAY || sysState == MS_PLAY) { 
    if (rsDisplayMode == 3) { rsDisplayMode = 0; } else { rsDisplayMode++; if(rsDisplayMode > 2) rsDisplayMode = 0; }
    appInit = false; if(rsDisplayMode == 0) { lcd.clear(); lcd.setCursor(0,0); printAutor(currentTrack); scrollPos = 0; }
  }
}

// ==========================================
// APLICATII NOI & MENIURI
// ==========================================
void runOSMenu() {
  if(!appInit) {
    lcd.clear(); lcd.setCursor(0,0); lcd.print(F("== TrixBay OS ==")); lcd.setCursor(0,1);
    switch(currentOSApp) {
      case 1: lcd.print(F("> 1. RetroSpin <")); break;
      case 2: lcd.print(F("> 2. ModrnSpin <")); break;
      case 3: lcd.print(F("> 3. Timer     <")); break;
      case 4: lcd.print(F("> 4. 8-Ball    <")); break;
      case 5: lcd.print(F("> 5. Zar Magic <")); break;
      case 6: lcd.print(F("> 6. Zen Mode  <")); break;
      case 7: lcd.print(F("> 7. Settings  <")); break;
      case 8: lcd.print(F("> 8. SHUTDOWN  <")); break;
    }
    appInit = true;
  }
}

void runTimerApp() {
  if (timerAlarm) {
    if (millis() - alarmPulseTime > 300) {
      alarmPulseTime = millis(); alarmToggle = !alarmToggle;
      if(alarmToggle) { lcd.noBacklight(); analogWrite(motorPin, 255); } else { lcd.backlight(); analogWrite(motorPin, 0); }
      if(!appInit) { lcd.clear(); lcd.setCursor(1,0); lcd.print(F("!!! TIMPUL !!!")); lcd.setCursor(1,1); lcd.print(F("!!! A EXPIRAT!")); appInit=true; }
    }
  } else if (!timerRunning) {
    if(!appInit) { lcd.clear(); lcd.setCursor(0,0); lcd.print(F("Focus Timer")); lcd.setCursor(0,1); lcd.print(F("Set: ")); lcd.print(timerSeconds); lcd.print(F(" sec")); appInit=true; }
  } else {
    if(!appInit && !timerAlarm) { lcd.clear(); lcd.setCursor(0,0); lcd.print(F("-- RUNNING --")); lcd.setCursor(0,1); lcd.print(F("Timp: ")); lcd.print(timerSeconds); lcd.print(F(" s")); appInit=true; }
  }
}

void run8BallApp() { if(!appInit) { lcd.clear(); lcd.setCursor(0,0); lcd.print(F("Magic 8-Ball")); lcd.setCursor(0,1); lcd.print(F("Pune intrebare..")); appInit = true; } }
void runDiceApp() { if(!appInit) { lcd.clear(); lcd.setCursor(0,0); lcd.print(F("Zar Magic")); lcd.setCursor(0,1); lcd.print(F("Apasa NEXT pt ZAR")); appInit = true; } }
void runZenApp() {
  if(!appInit) { lcd.clear(); lcd.print(F("~ ZEN MODE ~")); appInit=true; zenTimer=millis(); }
  if(millis() - zenTimer > 2000) {
    zenTimer = millis(); zenStep++; if(zenStep > 3) zenStep = 0; lcd.setCursor(0,1);
    if(zenStep==0) lcd.print(F("   Inspirati... ")); else if(zenStep==1) lcd.print(F("   Tineti...    ")); else if(zenStep==2) lcd.print(F("   Expirati...  ")); else if(zenStep==3) lcd.print(F("   Relaxati...  "));
  }
}
void runSettingsApp() {
  if(!appInit) {
    lcd.clear(); lcd.setCursor(0,0); lcd.print(F("SETARI SISTEM:")); lcd.setCursor(0,1);
    if(settingIndex == 1) { lcd.print(F("Scrl: ")); if(customTextSpeed==480) lcd.print(F("Lent")); else lcd.print(F("Rapid")); }
    else if(settingIndex == 2) { lcd.print(F("Sleep: ")); if(customSleepDelay==600000) lcd.print(F("10m")); else if(customSleepDelay==1800000) lcd.print(F("30m")); else lcd.print(F("OFF")); }
    else if(settingIndex == 3) { lcd.print(F("Motor: ")); if(customMotorSpeed==120) lcd.print(F("Lent")); else if(customMotorSpeed==170) lcd.print(F("Normal")); else lcd.print(F("Rapid")); }
    else if(settingIndex == 4) { lcd.print(F("RESET BIOS DATA")); }
    appInit = true;
  }
}

// ==========================================
// RETRO / MODERN SPIN PLAY (COMBINAT)
// ==========================================
void runSpinPlay() {
  if (!isPlaying) return;

  if (rsDisplayMode == 0) {
    if (!isTextLong) { lcd.setCursor(0, 1); lcd.print(currentSongName); for(int i=strlen(currentSongName); i<16; i++) lcd.print(" "); } 
    else {
      if (millis() - lastScrollTime > customTextSpeed) {
        lastScrollTime = millis(); lcd.setCursor(0, 1); int len = strlen(currentSongName);
        for (int i = 0; i < 16; i++) lcd.print(currentSongName[(scrollPos + i) % len]);
        scrollPos++; if (scrollPos >= len) scrollPos = 0;
      }
    }
  } 
  else if (rsDisplayMode == 1) { if(!appInit) { lcd.clear(); lcd.setCursor(0,0); lcd.print(F("VINYL MOTOR:")); lcd.setCursor(0,1); if(motorEnabled) lcd.print(F("< ON >")); else lcd.print(F("< OFF >")); appInit=true; } } 
  else if (rsDisplayMode == 2) { if(!appInit) { lcd.clear(); lcd.setCursor(0,0); lcd.print(F("SCREEN SAVER:")); lcd.setCursor(0,1); if(ssMenuState) lcd.print(F("< ON >  (Pauza)")); else lcd.print(F("< OFF > (Pauza)")); appInit=true; } }
  else if (rsDisplayMode == 3) {
    if (millis() - lastScrollTime > 120) { 
      lastScrollTime = millis();
      for (int i = 0; i < 16; i++) {
        if (eqHeights[i] > 0) eqHeights[i]--;
        if (random(0, 100) > 75) { eqHeights[i] = random(3, 9); }
        int val = eqHeights[i];
        lcd.setCursor(i, 1);
        if(val == 0) lcd.print(F(" ")); else if(val == 1) lcd.write(3); else if(val == 2) lcd.write(4); else if(val == 3) lcd.write(5); else lcd.write(2); 
        lcd.setCursor(i, 0);
        if(val <= 4) lcd.print(F(" ")); else if(val == 5) lcd.write(3); else if(val == 6) lcd.write(4); else if(val == 7) lcd.write(5); else if(val >= 8) lcd.write(2); 
      }
    }
  }

  if (millis() - trackStartTime > 3500) {
    if (digitalRead(busyPin) == HIGH) { delay(100); if (digitalRead(busyPin) == HIGH) {
      currentTrack++; if (currentTrack > albumEnd) currentTrack = albumStart; startPlayback(currentTrack); 
    }}
  }
}

void startPlayback(int track) {
  myDFPlayer.stop(); delay(150); myDFPlayer.playMp3Folder(track); delay(50); 
  isPlaying = true; trackStartTime = millis(); loadSongName(track); appInit = false; 
  if(rsDisplayMode == 0) { lcd.clear(); lcd.setCursor(0,0); printAutor(currentTrack); }
  startMotorSoft(); 
}

void runRetroSpinMenu() {
  if(!appInit) {
    lcd.clear(); lcd.setCursor(0,0); lcd.print(F("[ ALEGE ALBUM  ]")); lcd.setCursor(0,1);
    switch (currentAlbum) { case 1: lcd.print(F("> Queen  Hits  <")); break; case 2: lcd.print(F(">  ABBA  GOLD  <")); break; case 3: lcd.print(F("> MJ  Thriller <")); break; case 4: lcd.print(F("> Rock 'n Roll <")); break; case 5: lcd.print(F(">  Retro Mix   <")); break; }
    appInit = true;
  }
}

void runModernSpinMenu() {
  if(!appInit) {
    lcd.clear(); lcd.setCursor(0,0); lcd.print(F("[ MODRN GENRE  ]")); lcd.setCursor(0,1);
    switch (currentMSAlbum) {
      case 1: lcd.print(F(">   Blues    <")); break;
      case 2: lcd.print(F(">   House    <")); break;
      case 3: lcd.print(F(">   Manele   <")); break;
      case 4: lcd.print(F(">    Pop     <")); break;
      case 5: lcd.print(F(">  Popcorn   <")); break;
      case 6: lcd.print(F("> Reggaeton  <")); break;
      case 7: lcd.print(F(">   Summer   <")); break;
    }
    appInit = true;
  }
}

void runBIOS() {
  if(!appInit) {
    lcd.clear();
    switch(biosPage) {
      case 1: lcd.setCursor(0,0); lcd.print(F("TrixBay OS v1.4")); lcd.setCursor(0,1); lcd.print(F("Bld: May 16 2026")); break;
      case 2: lcd.setCursor(0,0); lcd.print(F("CPU: ATmega328P")); lcd.setCursor(0,1); lcd.print(F("RAM: 2KB EE:1KB")); break;
      case 3: lcd.setCursor(0,0); lcd.print(F("Audio: YX5200 SD")); lcd.setCursor(0,1); lcd.print(F("Total Tracks: 131")); break;
      case 4: long bc = EEPROM.read(0) + (EEPROM.read(1) << 8); lcd.setCursor(0,0); lcd.print(F("System Boots:")); lcd.setCursor(0,1); lcd.print(bc); break;
      case 5: lcd.setCursor(0,0); lcd.print(F("Total Uptime:")); lcd.setCursor(0,1); lcd.print(totalUptimeMinutes); lcd.print(F(" Min")); break;
    }
    appInit = true;
  }
}

void setAlbumLimits() {
  switch (currentAlbum) { case 1: albumStart = 1; albumEnd = 9; break; case 2: albumStart = 10; albumEnd = 20; break; case 3: albumStart = 21; albumEnd = 28; break; case 4: albumStart = 29; albumEnd = 37; break; case 5: albumStart = 38; albumEnd = 49; break; }
}

void setMSAlbumLimits() {
  switch (currentMSAlbum) {
    case 1: albumStart = 53; albumEnd = 58; break;
    case 2: albumStart = 59; albumEnd = 69; break;
    case 3: albumStart = 70; albumEnd = 93; break;
    case 4: albumStart = 94; albumEnd = 106; break;
    case 5: albumStart = 107; albumEnd = 117; break;
    case 6: albumStart = 118; albumEnd = 125; break;
    case 7: albumStart = 126; albumEnd = 131; break;
  }
}

void drawVolume() {
  lcd.clear(); lcd.setCursor(0,0); lcd.print(F("=== VOLUM ===")); lcd.setCursor(5,1); lcd.print(currentVolume); lcd.print(F(" / 20"));
  isVolumeOnScreen = true; showVolumeTimer = millis();    
}

void runShutdownAnimation() {
  myDFPlayer.stop(); stopMotor(); lcd.clear(); lcd.setCursor(3,0); lcd.print(F("SHUTTING")); lcd.setCursor(5,1); lcd.print(F("DOWN")); 
  myDFPlayer.playMp3Folder(52); delay(1500); 
  for(int i=0; i<8; i++) { lcd.setCursor(i, 0); lcd.print(F(" ")); lcd.setCursor(15-i, 0); lcd.print(F(" ")); lcd.setCursor(i, 1); lcd.print(F(" ")); lcd.setCursor(15-i, 1); lcd.print(F(" ")); delay(100); }
  lcd.setCursor(7,0); lcd.print(F("-")); delay(200); lcd.setCursor(7,0); lcd.print(F(".")); delay(200); lcd.clear(); lcd.noBacklight(); delay(2000); 
  while(digitalRead(buttonPausePin) == HIGH && digitalRead(buttonModePin) == HIGH) { delay(100); } void(* resetFunc) (void) = 0; resetFunc();
}
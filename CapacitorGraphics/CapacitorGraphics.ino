// Definim pinii
const int chargePin = 8;     // Pinul care "pompează" curent sau îl trage înapoi
const int analogPin = A0;    // Pinul care măsoară nivelul de încărcare
const int redPin = 9;        // Pin PWM pentru culoarea Roșie
const int greenPin = 10;     // Pin PWM pentru culoarea Verde

// Variabile pentru starea circuitului
int voltageValue = 0;        // Va stoca valoarea citită (între 0 și 1023)
bool isCharging = true;      // Ține minte dacă suntem pe încărcare sau descărcare

void setup() {
  // Setăm pinii
  pinMode(chargePin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  
  // Pornim comunicarea serială cu PC-ul pentru "Osciloscop"
  Serial.begin(9600);
  
  // Începem prin a descărca complet capacitorul (pentru siguranță)
  digitalWrite(chargePin, LOW);
  delay(2000); 
}

void loop() {
  // 1. Decidem dacă încărcăm sau descărcăm
  if (isCharging) {
    digitalWrite(chargePin, HIGH); // Trimitem 5V spre capacitor
  } else {
    digitalWrite(chargePin, LOW);  // Facem pinul 0V ca să se descarce capacitorul în el
  }

  // 2. Măsurăm tensiunea de pe capacitor
  // Va fi o valoare de la 0 (descărcat, 0V) la 1023 (încărcat, 5V)
  voltageValue = analogRead(analogPin);

  // 3. Trimitem valoarea către PC (Aici se întâmplă magia pentru grafic!)
  Serial.println(voltageValue);

  // 4. Transformăm valoarea citită în culori pentru LED-ul RGB
  // Mapăm valoarea 0-1023 la 0-255 (cât suportă pinii PWM pentru LED)
  int redIntensity = map(voltageValue, 0, 1023, 255, 0);   // Scade pe măsură ce se încarcă
  int greenIntensity = map(voltageValue, 0, 1023, 0, 255); // Crește pe măsură ce se încarcă

  // Aplicăm culorile:
  // Descărcat = Roșu pur (255, 0)
  // La jumătate = Galben (amestec de roșu cu verde)
  // Încărcat = Verde pur (0, 255)
  analogWrite(redPin, redIntensity);
  analogWrite(greenPin, greenIntensity);

  // 5. Verificăm dacă a ajuns la capăt ca să schimbăm direcția
  if (voltageValue >= 1010 && isCharging) {
    // Dacă e aproape plin, oprește încărcarea și începe descărcarea
    isCharging = false;
  } 
  else if (voltageValue <= 10 && !isCharging) {
    // Dacă e aproape gol, începe să-l încarci din nou
    isCharging = true;
  }

  // O mică pauză ca să nu inundăm PC-ul cu date prea rapid
  delay(10); 
}
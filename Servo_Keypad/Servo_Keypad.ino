#include <Keypad.h>
#include <Servo.h>

Servo myservo;

const byte ROWS = 4; 
const byte COLS = 4;

char rowPins[ROWS] = {13, 12, 11, 10};
char colPins[COLS] = {9, 8, 7, 6};

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad k = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  myservo.attach(2);
  myservo.write(90); 
}

void loop() {
  char key = k.getKey();
  
  if(key){
    // Verificam daca e cifra
    if(key >= '0' && key <= '9'){
      
      // 1. Convertim caracterul in numar (ex: '5' devine 5)
      int numar = key - '0';
      
      // 2. Mapam (transformam) numarul in viteza pentru servo
      // Intrare: 0 la 9
      // Iesire: 90 (Stop) la 180 (Maxim)
      // Daca vrei invers, pune map(numar, 0, 9, 90, 0);
      
      int viteza = map(numar, 0, 9, 90, 0);
      
      myservo.write(viteza);
    }
    
    // Optional: Buton de urgenta (ex: tasta 'D' opreste tot)
    if(key == 'D'){
      myservo.write(90); // STOP
    }
    if
  }
}
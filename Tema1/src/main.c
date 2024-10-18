#include <Arduino.h>

int led1 = 10;  // L1 - 25%
int led2 = 9;   // L2 - 50%
int led3 = 8;   // L3 - 75%
int led4 = 7;   // L4 - 100%

int ledRGB_r = 6;  // LED RGB roșu
int ledRGB_g = 5;  // LED RGB verde
int ledRGB_b = 4;  // LED RGB albastru

int buttonStart = 3;  // Buton Start
int buttonStop = 2;   // Buton Stop

bool charging = false;
bool stationFree = true;

unsigned long lastMillis;
int currentStep = 0; // 0 = Stația liberă, 1 = 25%, 2 = 50%, 3 = 75%, 4 = 100%

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(ledRGB_r, OUTPUT);
  pinMode(ledRGB_g, OUTPUT);
  pinMode(ledRGB_b, OUTPUT);

  pinMode(buttonStart, INPUT_PULLUP);
  pinMode(buttonStop, INPUT_PULLUP);

  resetStation();
}

void loop() {
  // Apăsarea butonului de Start (Pin 3) când stația este liberă
  if (digitalRead(buttonStart) == LOW && stationFree) {
    delay(50); // debounce
    startCharging();
  }
  
  // Apăsarea lungă a butonului Stop (Pin 2) pentru oprire
  if (digitalRead(buttonStop) == LOW && !stationFree) {
    long pressDuration = 0;
    while (digitalRead(buttonStop) == LOW) {
      pressDuration++;
      delay(10);
      if (pressDuration > 100) {  // 1 sec apasare lungă
        stopCharging();
        break;
      }
    }
  }

  if (charging) {
    updateChargingProgress();
  }
}

void resetStation() {
  stationFree = true;
  charging = false;
  currentStep = 0;
  
  // Led-uri albastre stinse
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
  
  // Led RGB verde
  digitalWrite(ledRGB_r, LOW);
  digitalWrite(ledRGB_g, HIGH);  // Verde
  digitalWrite(ledRGB_b, LOW);
}

void startCharging() {
  stationFree = false;
  charging = true;
  
  // LED RGB roșu
  digitalWrite(ledRGB_r, HIGH); // Roșu
  digitalWrite(ledRGB_g, LOW);
  digitalWrite(ledRGB_b, LOW);

  lastMillis = millis();
  currentStep = 1;  // Începem de la 25%
}

void stopCharging() {
  charging = false;

  blinkAllLEDs();
  
  resetStation();
}

void updateChargingProgress() {
  if (millis() - lastMillis >= 3000) {
    lastMillis = millis();
    
    // Controlul fiecărui LED în funcție de procentaj
    if (currentStep == 1) {
      blinkLED(led1);
    } else if (currentStep == 2) {
      digitalWrite(led1, HIGH);  // L1 aprins continuu
      blinkLED(led2);
    } else if (currentStep == 3) {
      digitalWrite(led2, HIGH);  // L2 aprins continuu
      blinkLED(led3);
    } else if (currentStep == 4) {
      digitalWrite(led3, HIGH);  // L3 aprins continuu
      blinkLED(led4);
    } else if (currentStep == 5) {
      // Încărcare completă, toate ledurile aprinse
      digitalWrite(led4, HIGH);
      
      blinkAllLEDs();
      
      resetStation();
    }
    
    currentStep++;
  }
}

// Funcție pentru a clipi toate LED-urile de 3 ori
void blinkAllLEDs() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);
    delay(500);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    delay(500);
  }
}

void blinkLED(int ledPin) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
  digitalWrite(ledPin, HIGH); 
}

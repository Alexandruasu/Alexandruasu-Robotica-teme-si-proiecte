#include <Arduino.h>

// Pin definitions
const int led1 = 10;  // L1 - 25%
const int led2 = 9;   // L2 - 50%
const int led3 = 8;   // L3 - 75%
const int led4 = 7;   // L4 - 100%

const int ledRGB_r = 6;  // LED RGB roșu
const int ledRGB_g = 5;  // LED RGB verde
const int ledRGB_b = 4;  // LED RGB albastru

const int buttonStart = 3;  // Buton Start
const int buttonStop = 2;   // Buton Stop

const unsigned long CHARGING_STEP_INTERVAL = 3000;  // Timpul între pașii de încărcare (3 secunde)
const unsigned long BUTTON_HOLD_THRESHOLD = 1000;   // Prag pentru apăsare lungă (1 secundă)
const unsigned long LED_BLINK_DELAY = 500;          // Delay pentru clipirea LED-urilor (0.5 secunde)
const int BLINK_COUNT = 3;                          // Număr de clipiri pentru LED-uri

bool charging = false;
bool stationFree = true;

unsigned long lastMillis;
int currentStep = 0;  // 0 = Stația liberă, 1 = 25%, 2 = 50%, 3 = 75%, 4 = 100%

void setup() {
  // Setare pini pentru ieșire
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(ledRGB_r, OUTPUT);
  pinMode(ledRGB_g, OUTPUT);
  pinMode(ledRGB_b, OUTPUT);

  // Setare pini butoane cu rezistență internă de pull-up
  pinMode(buttonStart, INPUT_PULLUP);
  pinMode(buttonStop, INPUT_PULLUP);

  resetStation();
}

void loop() {
  // Apăsarea butonului de Start (Pin 3) când stația este liberă
  if (digitalRead(buttonStart) == LOW && stationFree) {
    delay(50); 
    startCharging();
  }

  // Apăsarea lungă a butonului Stop (Pin 2) pentru oprire
  if (digitalRead(buttonStop) == LOW && !stationFree) {
    long pressDuration = 0;
    while (digitalRead(buttonStop) == LOW) {
      pressDuration++;
      delay(10);
      if (pressDuration > BUTTON_HOLD_THRESHOLD / 10) {  // Apăsare lungă (1 sec)
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
  
  // Oprire LED-uri albastre
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
  
  // Pornire LED RGB verde
  digitalWrite(ledRGB_r, LOW);
  digitalWrite(ledRGB_g, HIGH);  // Verde
  digitalWrite(ledRGB_b, LOW);
}

void startCharging() {
  stationFree = false;
  charging = true;
  
  // LED RGB roșu activ
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
  if (millis() - lastMillis >= CHARGING_STEP_INTERVAL) {
    lastMillis = millis();
    
    // Controlul fiecărui LED în funcție de procentaj
    switch (currentStep) {
      case 1:
        blinkLED(led1);
        break;
      case 2:
        digitalWrite(led1, HIGH);  // L1 aprins continuu
        blinkLED(led2);
        break;
      case 3:
        digitalWrite(led2, HIGH);  // L2 aprins continuu
        blinkLED(led3);
        break;
      case 4:
        digitalWrite(led3, HIGH);  // L3 aprins continuu
        blinkLED(led4);
        break;
      case 5:
        // Încărcare completă, toate LED-urile aprinse
        digitalWrite(led4, HIGH);
        blinkAllLEDs();
        resetStation();
        break;
    }
    
    currentStep++;
  }
}

// Funcție pentru a clipi toate LED-urile de 3 ori
void blinkAllLEDs() {
  for (int i = 0; i < BLINK_COUNT; i++) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);
    delay(LED_BLINK_DELAY);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    delay(LED_BLINK_DELAY);
  }
}

void blinkLED(int ledPin) {
  for (int i = 0; i < BLINK_COUNT; i++) {
    digitalWrite(ledPin, HIGH);
    delay(LED_BLINK_DELAY);
    digitalWrite(ledPin, LOW);
    delay(LED_BLINK_DELAY);
  }
  digitalWrite(ledPin, HIGH); 
}

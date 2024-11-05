#include <Arduino.h>

// Pini LED-uri și Butoane
const int ledAlbastru = 4; // Canal albastru LED RGB
const int ledVerde = 5;    // Canal verde LED RGB
const int ledRosu = 6;     // Canal roșu LED RGB
const int butonDificultate = 2;
const int butonStartStop = 3;

// Timpuri de delay și limitări pentru joc
const unsigned long debounceDelay = 200;       // 200 ms pentru debouncing
const unsigned long durataRunda = 30000;       // 30 secunde pentru o rundă
const unsigned long numaratorInversaDelay = 500; // Delay de 500 ms pentru clipire LED
const int numaratorInversaSecunde = 3;         // Numărătoare inversă de 3 secunde

// Timpi pentru fiecare nivel de dificultate
const unsigned long timpUsor = 15000;
const unsigned long timpMediu = 10000;
const unsigned long timpGreu = 5000;

// Definire valoare zero
const int zero = 0;

const long baudRate = 9600;
const int eliminaUnCaracter = 1;

// Enum pentru dificultate
enum Dificultate { usor, mediu, greu };
Dificultate dificultateCurenta = usor;

// Variabile pentru stări și temporizare
bool jocActiv = false;
bool incepeNumaratoare = false; 
unsigned long startTimpRunda;
unsigned long timpPentruCuvant;
unsigned long timpUltimulCuvant;
volatile unsigned long ultimulTimpButonDificultate = zero;
unsigned long ultimulTimpButonStartStop = zero;
int cuvinteCorecte = zero;

String cuvinte[] = {
  "pisica", "soare", "luna", "casa", "floare", "verde", "masina", 
  "nor", "copac", "apa", "cer", "munte", "deal", "lac", "padure", 
  "stea", "vis", "zbor", "praf", "foc", "roua", "vulpe", "iarba", 
  "punct", "zambet", "vreme", "cale", "cifra", "glas", "lumina", 
  "nuca", "cetina", "speranta", "umbre", "ochi", "puncte", "masca"
}; // Dicționar de cuvinte

int indexCuvantCurent = zero;
String cuvantCorespunzator;
String textIntrodus;

// Prototipurile funcțiilor
void setStandby();
void schimbareDificultate(); 
void startStopJoc(); 
void gestioneazaCuvinte();
void verificaTimp();
void opresteJoc();
void pornesteNumaratoare();
void pornesteJoc();
void seteazaTimpPentruDificultate();
void seteazaCuvantNou();
void setVerde();
void setRosu();

void setup() {
  pinMode(ledAlbastru, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRosu, OUTPUT);
  pinMode(butonDificultate, INPUT_PULLUP);
  pinMode(butonStartStop, INPUT_PULLUP);

  Serial.begin(baudRate);
  
  randomSeed(analogRead(zero) + millis());
  setStandby();

  attachInterrupt(digitalPinToInterrupt(butonDificultate), schimbareDificultate, FALLING);
  attachInterrupt(digitalPinToInterrupt(butonStartStop), startStopJoc, FALLING);
}

void loop() {
  if (incepeNumaratoare) {
    pornesteNumaratoare();
    incepeNumaratoare = false;
  }

  if (jocActiv) {
    gestioneazaCuvinte();
    verificaTimp();
  }
}

// Funcții pentru controlul LED-urilor

void setStandby() {
  digitalWrite(ledAlbastru, HIGH); // Setează alb
  digitalWrite(ledVerde, HIGH);
  digitalWrite(ledRosu, HIGH);
}

void setVerde() {
  digitalWrite(ledAlbastru, LOW);
  digitalWrite(ledVerde, HIGH); // LED verde în timpul rundei
  digitalWrite(ledRosu, LOW);
}

void setRosu() {
  digitalWrite(ledAlbastru, LOW);
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledRosu, HIGH); // LED roșu la greșeală
}

// Funcție pentru întreruperea butonului de dificultate
void schimbareDificultate() {
  unsigned long timpCurent = millis();

  // Debouncing
  if (timpCurent - ultimulTimpButonDificultate >= debounceDelay && !jocActiv) {
    if (dificultateCurenta == usor) {
      dificultateCurenta = mediu;
      Serial.println("Medium mode on!");
    } else if (dificultateCurenta == mediu) {
      dificultateCurenta = greu;
      Serial.println("Hard mode on!");
    } else {
      dificultateCurenta = usor;
      Serial.println("Easy mode on!");
    }
    ultimulTimpButonDificultate = timpCurent;
  }
}

// Funcție pentru întreruperea butonului de start/stop
void startStopJoc() {
  unsigned long timpCurent = millis();

  // Debouncing
  if (timpCurent - ultimulTimpButonStartStop >= debounceDelay) {
    if (jocActiv) {
      opresteJoc();
    } else {
      incepeNumaratoare = true;
    }
    ultimulTimpButonStartStop = timpCurent;
  }
}

// Pornirea numărătorii inverse și jocului
void pornesteNumaratoare() {
  for (int i = numaratorInversaSecunde; i > zero; i--) {
    Serial.print("Începe în: ");
    Serial.println(i);
    
    // LED alb clipind
    digitalWrite(ledAlbastru, HIGH); 
    digitalWrite(ledVerde, HIGH);    
    digitalWrite(ledRosu, HIGH);     
    delay(numaratorInversaDelay);
    
    digitalWrite(ledAlbastru, LOW);
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledRosu, LOW);
    delay(numaratorInversaDelay);
  }
  
  pornesteJoc();
}

void pornesteJoc() {
  jocActiv = true;
  startTimpRunda = millis();
  cuvinteCorecte = zero;
  seteazaTimpPentruDificultate();
  seteazaCuvantNou();
}

void opresteJoc() {
  jocActiv = false;
  setStandby();
  Serial.print("Runda terminată. Cuvinte corecte: ");
  Serial.println(cuvinteCorecte);
}

// Setare timp pentru fiecare dificultate
void seteazaTimpPentruDificultate() {
  if (dificultateCurenta == usor) {
    timpPentruCuvant = timpUsor;
  } else if (dificultateCurenta == mediu) {
    timpPentruCuvant = timpMediu;
  } else {
    timpPentruCuvant = timpGreu;
  }
}

void seteazaCuvantNou() {
  indexCuvantCurent = random(zero, sizeof(cuvinte) / sizeof(cuvinte[zero]));
  cuvantCorespunzator = cuvinte[indexCuvantCurent];
  textIntrodus = "";
  timpUltimulCuvant = millis();
  Serial.print("Cuvânt: ");
  Serial.println(cuvantCorespunzator);
  setVerde();
}

void gestioneazaCuvinte() {
  if (Serial.available() > zero) {
    char caracter = Serial.read();
    
    if (caracter == '\b') {
      if (textIntrodus.length() == zero) {
        setVerde();
      } else {
        textIntrodus.remove(textIntrodus.length() - eliminaUnCaracter);
      }
    } else {
      textIntrodus += caracter;
    }
    
    if (textIntrodus == cuvantCorespunzator) {
      Serial.println("Corect!");
      cuvinteCorecte++;
      seteazaCuvantNou();
    } else if (cuvantCorespunzator.startsWith(textIntrodus)) {
      setVerde();
    } else {
      setRosu();
    }
  }
}

// Se verifică timpul pentru fiecare cuvânt și runda 
void verificaTimp() {
  if (millis() - startTimpRunda >= durataRunda) {
    opresteJoc();
  } else if (millis() - timpUltimulCuvant >= timpPentruCuvant) {
    seteazaCuvantNou();
  }
}

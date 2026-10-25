#include <Arduino.h>

const int ENABLE_PIN = 5;  // D1 - ON/OFF
const int PWM_PIN    = 2;  // D4 - PWM

// Wartosci dla analogWrite 0-255
// ok. 1,5 V / 2,0 V / 3,3 V
const int poziomy[] = {116, 155, 255};
const int liczbaPoziomow = sizeof(poziomy) / sizeof(poziomy[0]);

int indeks = 0;
bool wlaczony = false;

unsigned long ostatniaZmiana = 0;
const unsigned long INTERWAL_ZMIANY = 5000; // 5 s

void setup() {
  Serial.begin(115200);

  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);

  digitalWrite(ENABLE_PIN, LOW); // start: OFF
  analogWrite(PWM_PIN, poziomy[indeks]);

  Serial.println("Start: ON/OFF = OFF");
  Serial.print("PWM = ");
  Serial.print(poziomy[indeks]);
  Serial.println("/255");
}

void loop() {
  if (millis() - ostatniaZmiana >= INTERWAL_ZMIANY) {
    ostatniaZmiana = millis();

    // Przelacz ON/OFF
    wlaczony = !wlaczony;
    digitalWrite(ENABLE_PIN, wlaczony ? HIGH : LOW);

    // Zmien PWM
    indeks++;
    if (indeks >= liczbaPoziomow) {
      indeks = 0;
    }
    analogWrite(PWM_PIN, poziomy[indeks]);

    Serial.print("ON/OFF = ");
    Serial.print(wlaczony ? "ON" : "OFF");
    Serial.print(", PWM = ");
    Serial.print(poziomy[indeks]);
    Serial.println("/255");
  }
}
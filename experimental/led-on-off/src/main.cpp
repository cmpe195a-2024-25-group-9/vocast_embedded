#include <Arduino.h>

#define LED1 2

void setup() {
  pinMode(LED1,OUTPUT);
}

void loop() {
  delay(1000);
  digitalWrite(LED1,HIGH);
  delay(1000);
  digitalWrite(LED1,LOW);
}
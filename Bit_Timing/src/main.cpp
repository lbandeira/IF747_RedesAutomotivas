#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  // bit_timing_setup();
}

void loop() {
  Serial.println("Dale");
}
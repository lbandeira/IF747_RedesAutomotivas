#include <Arduino.h>
#include "frame_builder.h"
#include "bit_stuff_control.h"

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println("Teste");
  delay(1000);
}
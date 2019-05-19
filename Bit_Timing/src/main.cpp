#include "bit_timing.h"

void setup() {
  Serial.begin(9600);
  bit_timing_setup();
}

void loop() {
  cli();
  write_plot();
  bit_timing_sm();
  sei();
}
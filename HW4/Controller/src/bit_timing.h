#ifndef BIT_TIMING_H
#define BIT_TIMING_H

#include <Arduino.h>
#include "frame_control.h"

#ifdef __AVR__
#include "TimerOne.h"
#endif

//Variaveis do calculo do Timequanta
#define TQ_SYNC 1
#define TQ_PROP 1
#define TQ_SEG_1 8 //(PROP + SEG1)
#define TQ_SEG_2 7
#define TQ_QUANT_TOTAL 16
#define SJW 10
#define BIT_RATE 200000 // interrupcao a cada 1 segundo (tq a cada segundo)

//Variaveis da maquina de estados
#define SYNC 0
#define PROP_SEG 1
#define SEG_1 2
#define WINDOW_1 3
#define SEG_2 4
#define WINDOW_2 5

#define RESYNC_PIN 2
#define HARD_SYNC_PIN 3

extern bool writing_point;
extern bool sample_point;

void bit_timing_setup();
void write_plot();
void bit_timing_sm();
void detect_falling_edge(bool old_rx, bool rx);
void detect_falling_edge();

#endif
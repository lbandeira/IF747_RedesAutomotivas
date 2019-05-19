#include <Arduino.h>

//Variaveis do calculo do Timequanta
#define TQ_SYNC 1
#define TQ_PROP 1
#define TQ_SEG_1 8 //(PROP + SEG1)
#define TQ_SEG_2 7
#define TQ_QUANT_TOTAL 16
#define SJW 10
#define BIT_RATE 5 // Bitrate de 500bps, apenas para teste por enquanto

//Variaveis da maquina de estados
#define SYNC 0
#define PROP_SEG 1
#define SEG_1 2
#define WINDOW_1 3
#define SEG_2 4
#define WINDOW_2 5

#define RESYNC_PIN 2
#define HARD_SYNC_PIN 3

//Variaveis do Bit timing logic
extern bool writing_point;
extern bool sample_point;

//Variaveis da CAN
bool bus_idle;

void setupInterrupt() ;

int getTqFrequency(); 

void btl_setup();

void btl_state_machine() ;

void pins_setup() ;

void resync_isr() ;

void hard_sync_isr() ;

void plot_setup() ;

void update_plotter_values();

void write_plot();



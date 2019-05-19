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

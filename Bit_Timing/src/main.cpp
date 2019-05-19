#include <Arduino.h>
#include <TimerOne.h>


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

int state_new;
int state_old;
int tq_tot;
int tq_seg1;
int tq_seg2;
int phase_error;
int window_size;

volatile bool btlState = false; // boolean soh pra avisar que houve a interrupcao
volatile int tot = 0;
int time_t;
int time_new;

//Variaveis do Bit timing logic
bool writing_point;
bool sample_point;
bool resync;
bool resync_enable;
volatile bool new_tq;
bool hard_sync;

//Variaveis da CAN
bool bus_idle;

// variaveis utilizadas para plottar
int seg_plotter_state;
bool tq_plotter_state;

void interrupt_routine() {
  new_tq = true;
  tq_plotter_state = !tq_plotter_state;
}

// esse metodo eh responsavel por fazer o setup inicial dos regs que iram tratar na interrupcao
void setupInterrupt() {
   // desativando as interrupcoes
  noInterrupts();

  // resetando os registradores responsaveis pelo comportamento do timer 1, cada um deles tem 8 bits
  TCCR1A = 0;
  TCCR1B = 0;
  
  // resetando o timer counter do timer 1
  // esse carinha eh que eh incrementado de acordo com a frequencia
  TCNT1 = 0;

  // esse registrador aqui vai ficar o valor com o qual queremos comparar para que role a interrupcao
  // nos nao estamos usando o prescaler, portanto o contador devera incrementar a cada tick do clock do arduino
  // o que significa que ele incrementara 16 millhoes de vezes em 1 segundo, ou seja, dentro de 1 segundo, ocorrem 8000 tqs
  // como o tq dura 0.000125s, sua frequencia eh de 8000Hz
//  OCR1A = (F_CPU / getTqFrequency()) - 1;      // 16MHz/1/8000Hz --- 8000Hz = 1/((1/500bps)/16)
   OCR1A = 65535;      // 16MHz/1/8000Hz --- 8000Hz = 1/((1/500bps)/16)

  // setando o 4 bit menos significativo para 1, isso significa que queremos que haja uma interrupcao toda vez que o contador
  // atingir o valor que estah no OCR1A
  TCCR1B |= (1 << WGM12);
  // TCCR1A |= (1<<WGM01);
  // setando o bit menos significativo para 1, isso significa que nao vamos usar nenhum prescaler
  TCCR1B |= ((1 << CS11) | (1 << CS10));
  // habilitando a interrupcao por comparacao na mascara de interrupcoes
  TIMSK1 |= (1 << OCIE1A);

  // reabilitando interrupcoes
  interrupts();
}

int getTqFrequency() {
  double bitTime = 1.0 / BIT_RATE;
  double tq = bitTime / (TQ_SYNC + TQ_PROP + TQ_SEG_1 + TQ_SEG_2);

  return 1.0 / tq;
}

void btl_setup(){
  state_new = SYNC;
  tq_tot = 0;
  tq_seg1 = 0;
  tq_seg2 = 0;
  phase_error = 0;
  window_size = 0;
  writing_point = false;
  sample_point = false;
  resync = false;
  resync_enable = false;
  hard_sync = false;
}

void update_plotter_values() {
  switch (state_old) {
    case SYNC:
      seg_plotter_state = 1;
      break;

    case SEG_1:
      seg_plotter_state = 3;
      break;

    case WINDOW_1:
      seg_plotter_state = 5;
      break;

    case SEG_2:
      seg_plotter_state = 7;
      break;

    case WINDOW_2:
      seg_plotter_state = 9;
      break;

    default:
      break;
  }
}

void btl_state_machine() {
  if (new_tq) {
    new_tq = false;
    if (hard_sync) {

      state_new = SYNC;
      hard_sync = false;
    
    } 
    else if (resync) {

      if (resync_enable) {

        if (state_old == SEG_1) {
          window_size = min(SJW, phase_error);
          state_new = WINDOW_1;
          resync_enable = false;
        } 
        else if (state_old == SEG_2) {
          window_size = min(SJW, -phase_error);
          if (SJW > -phase_error) {
            tq_tot = 0;
            window_size = 0;
            phase_error = 0;
            state_new = SEG_1;
            resync_enable = false;
          } else {
            state_new = WINDOW_2;
          }
          resync_enable = false;
        }
      }
      resync = false;
    }

    //Core da maquina de estados
    /*
        State_new -> Proximo estado
        State_old -> Estado anterior
        tq_tot -> Contadot de TQs
    
    */ 
    switch (state_new){
      
      //SYNC: TQ_SYNC = 1
      case SYNC:    
        writing_point = true;
        resync_enable = true;
        tq_tot = 0;
        window_size = 0;
        phase_error = 0;
              
        state_old = SYNC;
        state_new = SEG_1;

        break;

      //SEG_1: TQ_SEG1 = 8 (PROP_SEG + SEG1)
      case SEG_1:
        tq_tot++;
        writing_point = false;
        if(state_old == SYNC){
          tq_seg1 = 0;
          state_old = SEG_1;
        }
        else if(state_old == SEG_1){
          tq_seg1++;
        }
        else{ //state_old == SEG_2 -> HardSync
          writing_point = true;
          tq_seg1 = 0;
          state_old = SEG_1;
        }

        phase_error = tq_tot;
        
        if(tq_seg1 >= TQ_SEG_1 -1){
          state_new = SEG_2;
        }
        break;

      //RESYNC
      //WINDOW_1: Incrementar o tempo de erro
      case WINDOW_1 :
        //Fica nesse estado ate o ajuste da janela ser feito
        if(state_old == SEG_1){
          state_old = WINDOW_1; 
        }

        tq_tot++;
        tq_seg1++;

        if(tq_seg1 >= TQ_SEG_1 - 1 + window_size){
          state_new = SEG_2;
        }
        break;

      //SEG_2: TQ_SEG2 = 7
      case SEG_2:
        tq_tot++;
        if ((state_old == SEG_1) || state_old == WINDOW_1){
          tq_seg2 = 0;
          sample_point = true;
          state_old = SEG_2;
        }
        else{
          sample_point = false;
          tq_seg2++;
        }      
        
        phase_error = tq_seg2 + TQ_SYNC + TQ_SEG_1  - TQ_QUANT_TOTAL;
        
        if(tq_seg2 >= TQ_SEG_2 - 1){
          state_new = SYNC;
        }
        break;
      
      //RESYNC
      //WINDOW_2: Decrementar o tempo de erro
      case WINDOW_2:
        if(state_old == SEG_2){
          state_old = WINDOW_2;
        }
        tq_tot++;
        tq_seg2++;
        if(tq_seg2 >= TQ_SEG_2 -1 -window_size){
          state_new = SYNC;
        }
        break; 
      
    }
    update_plotter_values();
  } 
}

void resync_isr() {
  resync = true;
}

void hard_sync_isr() {
  hard_sync = true;
}

void pins_setup() {
  pinMode(RESYNC_PIN, INPUT_PULLUP);
  pinMode(HARD_SYNC_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESYNC_PIN), resync_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HARD_SYNC_PIN), hard_sync_isr, CHANGE);
}

void plot_setup() {
  seg_plotter_state = -1;
  tq_plotter_state = false;
}

void write_plot() {
  Serial.print(seg_plotter_state + 2);
  Serial.print(" ");
  Serial.print(tq_plotter_state);
  Serial.print(" ");
  Serial.print(hard_sync - 1);
  Serial.print(" ");
  Serial.print(resync - 3);
  Serial.print(" ");
  Serial.print(writing_point - 5);
  Serial.print(" ");
  Serial.println(sample_point - 7);
}

void setup() {
  Serial.begin(9600);
  btl_setup();
  
  Timer1.initialize(BIT_RATE);
  //setupInterrupt();
  plot_setup();
  pins_setup();
  Timer1.attachInterrupt(interrupt_routine);
}

void loop() {
  write_plot();
  btl_state_machine();
}

#include "bit_timing.h"

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
bool is_falling_edge;

//Variaveis da CAN
bool bus_idle;

// variaveis utilizadas para plottar
int seg_plotter_state;
bool tq_plotter_state;

int getTqFrequency() {
  double bitTime = 1.0 / BIT_RATE;
  double tq = bitTime / (TQ_SYNC + TQ_PROP + TQ_SEG_1 + TQ_SEG_2);

  return 1.0 / tq;
}

void time_quanta_isr() {
  new_tq = true;
  tq_plotter_state = !tq_plotter_state;
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

void bit_timing_setup() {
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
  is_falling_edge = false;

  Timer1.initialize(BIT_RATE);
  plot_setup();
  pins_setup();
  Timer1.attachInterrupt(time_quanta_isr);
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

// Detecta se houve um falling edge no bus
void detect_falling_edge(bool rx) {
  is_falling_edge = !rx;
}

void bit_timing_sm() {
  // Toda vez que ocorrer um falling edge no bus
  if (is_falling_edge) {
    if (is_idle) {
      hard_sync = true;
      sample_point = false;
      // Reseta o controle de tqs
      // Timer1.restart();
    } else {
      resync = true;
    }
    is_falling_edge = false;
  }

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
        Serial.println("SYNC STATE");//tu descobriu mais alguma coisa?
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
        Serial.println("SEG_1 STATE");
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
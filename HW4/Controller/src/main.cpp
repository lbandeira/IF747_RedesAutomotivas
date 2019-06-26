#include <Arduino.h>
#include "frame_builder.h"
#include "bit_stuff_control.h"
#include "frame_control.h"
#include "ack_error_control.h"
#include "bit_error_control.h"
#include "form_error_control.h"
#include "crc_error_control.h"
#include "bit_stuff_insert.h"
#include "bit_timing.h"

bool rx[256] = { 0 };
bool bus[256] = { 0 };
bool unstuffed_frame[512] = { 0 };
int unstuffed_frame_size;
int bus_idx = 0;
int current_state;
int last_state;
bool last_bit_tx = true;
bool Rx;
bool last_writing_state = false;
bool first_falling_edge = true;
bool old_rx = true;
int state_count = 0;
bool lol = false;

const char bit_stuff_error_frame[] PROGMEM = "011001110010000100010101010101010101010101010101010101010101010101010101010101010100000000010100011111111111";
const char ack_error_frame[] PROGMEM = "0110011100100001000101010101010101010101010101010101010101010101010101010101010101000001000010100011111111111";
const char form_error_frame[] PROGMEM = "0100010010011111000001000001111100100001000101010101010101010101010101010101010101010101010101010101010101011110111110101011011111110";
const char crc_error_frame[] PROGMEM = "01100111001000010001010101010101010101010101010101010101010101010101010101010101010000010000111000110100000011111111";

void reset_all() {
  current_state = IDLE;
  last_state = IDLE;
  last_bit_tx = true;
  bit_stuff_flag = false;
  bit_error_flag = false;
  bit_stuff_error = false;
  form_error_flag = false;
  ack_error_flag = false;
  crc_error_flag = false;
  bus_idx = 0;
}

void test_frame(char *bit_string) {
  int frame_size = strlen(bit_string);
  string_to_bit_array(bit_string, rx);

  reset_all();

  for (int i = 0; i < frame_size; i++) {
    check_bit_stuff(rx[i]);
    frame_decoder(rx[i]);
    ack_error_control(rx[i]);
    // bit_error_control(rx[i]);
    form_error_control(rx[i]);
    crc_error_control(rx[i]);
  }

  // Printando o frame lido pelo bus
  print_frame(frame);

  // Printando as flags de erro
  print_error_flags(ack_error_flag, bit_stuff_error, bit_error_flag, form_error_flag, crc_error_flag);
}

void test_crc() {
  char bit_string[] = "01100111001000010001010101010101010101010101010101010101010101010101010101010101010";
  int bit_string_size = strlen(bit_string);
  bool bits_array[bit_string_size];
  string_to_bit_array(bit_string, bits_array);
  
  int crc = calculate_crc(bits_array, bit_string_size);
  Serial.print("CRC >>>>> ");
  Serial.println(crc, HEX);
}

int create_frame(bool *buffer, uint32_t id, uint64_t payload, bool is_extended, bool is_data, int dlc) {
  int frame_id_size = is_extended ? 29 : 11;
  int payload_size = dlc * 8;
  bool frame_id[frame_id_size];
  bool payload_array[payload_size];
  int frame_size;

  // Convertendo id para array de bits
  convert_to_bit_array(id, frame_id, frame_id_size);

  // Convertendo payload para array de bits
  convert_to_bit_array(payload, payload_array, payload_size);

  // Construindo o frame e transformando em um array de bits retornando o numero de bits
  frame_size = build_frame(frame_id, payload_array, buffer, is_extended, is_data, dlc);

  if (is_writing)
    return 0;

  send_frame(unstuffed_frame, frame_size);

  return frame_size;
}

// Caso retorne 0 indica que o bus esta ocupado
int prepare_transmission(bool *unstuffed_frame) {
  // Editar valores de acordo com o frame que vai ser transmitido
  uint32_t id = 0x1127007A;
  uint64_t payload = 0xAAAAAAAAAAAAAAAA;
  bool is_extended = true;
  bool is_data = true;
  int dlc = 8;

  // Criando o frame sem bit stuff
  return create_frame(unstuffed_frame, id, payload, is_extended, is_data, dlc);
}

void receive_frame() {
  check_bit_stuff(Rx);
  frame_decoder(Rx);
  ack_error_control(Rx);
  // bit_error_control(rx[i]);
  form_error_control(Rx);
  crc_error_control(Rx);
}

void debug_frame() {
  if (!(current_state == IDLE && Rx)) {
    if (last_state != current_state) {
      Serial.print(" ");
      state_count = 0;
      Serial.print(state_map[current_state]);
      Serial.print(": ");
      Serial.print(Rx);
    } else if (last_state == current_state) {
      Serial.print(Rx);
      state_count++;
    }
  } else if (current_state == IDLE) {
    Serial.println("BUS IDLE");
  }
}

void print_menu() {
  Serial.println("================================================");
  Serial.print("Do you want to transmit a package? (y/n) ");
  while (!Serial.available());
  Serial.println();
  Serial.println("================================================");
  if (Serial.available()) {
    char option = Serial.readString()[0];
    if (option == 'y' || option == 'Y') {
      // Essa parte so sera executada pela ECU que envia o pacote, no caso a ESP32
      unstuffed_frame_size = prepare_transmission(unstuffed_frame);
      // Setando o Rx = false para forçar um falling edge
      Rx = false;
    } else {
      Rx = true;
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Setando os pinos que serao o RX e TX
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);

  // Reseta todos os estados e variaveis
  reset_all();

  // Faz o setup dos estados e variaveis do modulo de bit timing
  bit_timing_setup();

  // Atribuindo uma interrupcao ao pino do RX
  // Toda vez que houver uma mudanca do nivel logico 1 para o nivel logico 0
  // havera uma interrupcao
  attachInterrupt(digitalPinToInterrupt(RX_PIN), detect_falling_edge, FALLING);
  
  Rx = true;
}

// Loop principal onde os modulos de encoder e decoder irao executar
// Cada TQ esta programado para durar 1 segundo
void loop() {
  if (Serial.available()) {
    Serial.read();
    // Essa parte so sera executada pela ECU que envia o pacote, no caso a ESP32
    unstuffed_frame_size = prepare_transmission(unstuffed_frame);
    // Setando o Rx = false para forçar um falling edge
    Rx = false;
    is_idle = false;
  }

  // Desativando interrupcoes
  cli();

  // Executando a maquina de estados do bit timing
  bit_timing_sm();

  // Caso seja o momento de escrever no bus
  if (writing_point) {
    // Pega um bit do frame e escreve no bus
    bit_stuff_insert();
    
    // Escrevendo no barramento
    digitalWrite(TX_PIN, Tx);

    writing_point = false;
    
    last_writing_state = is_writing;
  }

  // Caso seja o momento de ler do bus
  if (sample_point) {
    // Obtendo um bit do barramento
    Rx = digitalRead(RX_PIN);
    debug_frame();

    check_bit_stuff(Rx);
    frame_decoder(Rx);
    ack_error_control(Rx);
    bit_error_control(Rx, Tx);
    form_error_control(Rx);
    crc_error_control(Rx);

    if(form_error_flag || ack_error_flag || crc_error_flag || bit_stuff_error || bit_error_flag){
        print_error_flags(ack_error_flag, bit_stuff_flag, bit_error_flag, form_error_flag, crc_error_flag);
        current_state = ERR_FLAG;
    }

    sample_point = false;
  }

  old_rx = Rx;

  // Reativando interrupcoes
  sei();
}
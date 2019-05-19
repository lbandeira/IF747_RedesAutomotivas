#include <Arduino.h>
#include "frame_builder.h"
#include "bit_stuff_control.h"
#include "frame_control.h"
#include "ack_error_control.h"
#include "bit_error_control.h"
#include "form_error_control.h"
#include "crc_error_control.h"
#include "bit_stuff_insert.h"

bool rx[256] = { 0 };
bool bus[256] = { 0 };
int bus_idx = 0;
int current_state;
int last_state;
bool last_bit_tx = true;
bool Rx;

const char test_frames[][256] PROGMEM = {
  "0110011100100001000101010101010101010101010101010101010101010101010101010101010101000001000010100011011111111",
  "011001110010000011111001010101010101010101010101010101010101010101010101010101011001100111011011111111",
  "011001110010000011101010101010101010101010101010101010101010101010100001001001011101011111111",
  "0110011100100000110110101010101010101010101010101010101010101100111001001101011111111",
  "01100111001000001100101010101010101010101010101010101110111010100101011111111",
  "0110011100100000101110101010101010101010101001001011100000111011111111",
  "0110011100100000101010101010101010100110110111100111011111111",
  "01100111001000001001101010101100111000010101011111111",
  "011001110010000010000110010110101011011111111",
  "0110011100101000001000001000010100011011111111",
  "011001110010100000110000100100010011011111111",
  "0100010010011111000001000001111100100001000101010101010101010101010101010101010101010101010101010101010101011110111110101011011111111",
  "010001001001111100000100000111110010100100001010011111001101011111111"
};

const char bit_stuff_error_frame[] PROGMEM = "011001110010000100010101010101010101010101010101010101010101010101010101010101010100000000010100011111111111";
const char ack_error_frame[] PROGMEM = "0110011100100001000101010101010101010101010101010101010101010101010101010101010101000001000010100011111111111";
const char form_error_frame[] PROGMEM = "0100010010011111000001000001111100100001000101010101010101010101010101010101010101010101010101010101010101011110111110101011011111110";
const char crc_error_frame[] PROGMEM = "0110011100100001000101010101010101010101010101010101010101010101010101010101010101000001100010100011011111111";

void test_frame_control() {
  for (int f = 0; f < 13; f++) {
    int frame_size = strlen(test_frames[f]);
    current_state = IDLE;
    last_state = current_state;
    string_to_bit_array((char *)test_frames[f], rx);

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
}

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

  // Convertendo id para array de bits
  convert_to_bit_array(id, frame_id, frame_id_size);

  // Convertendo payload para array de bits
  convert_to_bit_array(payload, payload_array, payload_size);

  // Construindo o frame e transformando em um array de bits retornando o numero de bits
  return build_frame(frame_id, payload_array, buffer, is_extended, is_data, dlc);
}

int prepare_transmission(bool *unstuffed_frame) {
  // Editar valores de acordo com o frame que vai ser transmitido
  uint32_t id = 0x0672;
  uint64_t payload = 0xAAAAAAAAAAAAAAAA;
  bool is_extended = false;
  bool is_data = true;
  int dlc = 8;

  // Criando o frame sem bit stuff
  return create_frame(unstuffed_frame, id, payload, is_extended, is_data, dlc);
}

void transmit_frame(bool current_bit) {
  bit_stuff_insert(current_bit, last_bit_tx);
  last_bit_tx = Tx;
}

void receive_frame() {
  check_bit_stuff(Rx);
  frame_decoder(Rx);
  ack_error_control(Rx);
  // bit_error_control(rx[i]);
  form_error_control(Rx);
  crc_error_control(Rx);
}

void test_rx_tx() {
  // Array de bits que representa o frame sem bit stuff
  bool unstuffed_frame[128];
  // Tamanho do frame sem bit stuff
  int unstuffed_frame_size;

  // Montando o frame sem bit stuff
  unstuffed_frame_size = prepare_transmission(unstuffed_frame);

  // Loop que representa o envio de um bit no bus, assim como a leitura do mesmo
  for (int i = 0; i < unstuffed_frame_size; ) {
    // Simulando trasmissao no bus
    if (flag_stuff)
      transmit_frame(unstuffed_frame[i]);
    else
      transmit_frame(unstuffed_frame[i++]);
    bus[bus_idx++] = Tx;

    // Simulando recepcao no bus
    Rx = bus[--bus_idx];
    receive_frame();
  }

  // Printando o frame recebido e construido
  print_frame(frame);

  // Printando flags de erro
  print_error_flags(ack_error_flag, bit_stuff_error, bit_error_flag, form_error_flag, crc_error_flag);

  // Printando o frame em binario
  print_bit_array(frame.raw, frame_idx);
}

void setup() {
  Serial.begin(115200);

  reset_all();

  // test_frame_control();

  #if defined(__AVR__)
  // char buffer[256];
  // Testando frame com ack error
  // strcpy_P(buffer, ack_error_frame);
  // test_frame(buffer);
  // Testando frame com form error
  // strcpy_P(buffer, form_error_frame);
  // test_frame(buffer);
  // Testando frame com crc error
  // strcpy_P(buffer, crc_error_frame);
  // test_frame(buffer);

  test_rx_tx();
  #elif defined(ESP32)
  // Testando erro de bit stuffing
  // test_frame((char *)bit_stuff_error_frame);
  // Testando frame com ack error
  // test_frame((char *)ack_error_frame);
  // Testando frame com form error
  // test_frame((char *)form_error_frame);
  // Testando frame com crc error
  // test_frame((char *)crc_error_frame);

  // Testando encoder/decoder simulando o bus
  test_rx_tx();
  #endif

}

void loop() {
  
}
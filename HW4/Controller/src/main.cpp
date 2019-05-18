#include <Arduino.h>
#include "frame_builder.h"
#include "bit_stuff_control.h"
#include "frame_control.h"
#include "ack_error_control.h"
#include "bit_error_control.h"
#include "form_error_control.h"
#include "crc_error_control.h"

bool rx[128] = { 0 };
int current_state;
int last_state;

const char PROGMEM test_frames[][256] = {
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

const char PROGMEM ack_error_frame[] = "0110011100100001000101010101010101010101010101010101010101010101010101010101010101000001000010100011111111111";
const char PROGMEM form_error_frame[] = "0100010010011111000001000001111100100001000101010101010101010101010101010101010101010101010101010101010101011110111110101011011101111";
const char PROGMEM crc_error_frame[] = "0110011100100001000101010101010101010101010101010101010101010101010101010101010101000001100010100011011111111";

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
  bit_stuff_flag = false;
  bit_error_flag = false;
  bit_stuff_error = false;
  form_error_flag = false;
  ack_error_flag = false;
  crc_error_flag = false;
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

void setup() {
  Serial.begin(9600);

  // test_frame_control();

  // Testando frame com ack error
  test_frame((char *)ack_error_frame);
  // Testando frame com form error
  test_frame((char *)form_error_frame);
  // Testando frame com crc error
  test_frame((char *)crc_error_frame);

  // test_crc();
}

void loop() {
  
}
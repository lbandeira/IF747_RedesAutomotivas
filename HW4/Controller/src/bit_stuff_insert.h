#ifndef BIT_STUFF_INSERT_H
#define BIT_STUFF_INSERT_H

#include "utils.h"
#include "bit_error_control.h"
#include "crc_error_control.h"
#include "bit_timing.h"

extern bool Tx; // bit a ser escrito no bus
extern bool lost; // indica que perdeu a arbitracao
extern bool flag_stuff; // indica que o bit escrito no bus eh bit stuff
extern bool is_writing; // indica que o bus esta ocupado com alguem escrevendo

void bit_stuff_insert(); // funcao que escreve cada bit do frame no bus
void send_frame(bool *unstuffed_frame, int size); // funcao que obtem o frame a ser escrito no bus

#endif
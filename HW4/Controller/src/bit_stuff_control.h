#ifndef BIT_STUFF_CONTROL_H
#define BIT_STUFF_CONTROL_H

#include "utils.h"

extern bool bit_stuff_error;
extern bool bit_stuff_flag;
extern int current_state;

void check_bit_stuff(bool rx);

#endif
#ifndef BIT_STUFF_INSERT_H
#define BIT_STUFF_INSERT_H

#include "utils.h"
#include "bit_error_control.h"
#include "crc_error_control.h"

extern bool Tx;
extern bool writing_point;
extern bool lost;
extern bool flag_stuff;

void bit_stuff_insert(bool unstuffed_frame, bool last_bit);

#endif
#ifndef BIT_ERROR_CONTROL_H
#define BIT_ERROR_CONTROL_H

#include "utils.h"
#include "bit_stuff_control.h"

extern bool arbitration_flag;
extern bool bit_error_flag;

void bit_error_control(bool rx, bool tx);

#endif
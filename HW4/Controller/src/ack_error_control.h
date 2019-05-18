#ifndef ACK_ERROR_CONTROL_H
#define ACK_ERROR_CONTROL_H

#include "utils.h"
#include "bit_stuff_control.h"

extern bool ack_error_flag;

void ack_error_control(bool rx);

#endif
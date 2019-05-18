#ifndef CRC_ERROR_CONTROL_H
#define CRC_ERROR_CONTROL_H

#include "utils.h"
#include "bit_stuff_control.h"
#include "frame_control.h"

extern bool crc_error_flag;

void crc_error_control(bool rx);

#endif
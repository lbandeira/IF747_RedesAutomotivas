#ifndef CRC_ERROR_CONTROL_H
#define CRC_ERROR_CONTROL_H

#include "utils.h"
#include "bit_stuff_control.h"
#include "frame_control.h"

extern bool crc_error_flag;
extern bool valid_frame; // indica que o frame lido tem o crc correto, portanto um frame valido

void crc_error_control(bool rx);

#endif
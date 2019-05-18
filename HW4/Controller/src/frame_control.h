#ifndef FRAME_CONTROL_H
#define FRAME_CONTROL_H

#include "utils.h"
#include "bit_stuff_control.h"
#include "form_error_control.h"
#include "ack_error_control.h"
#include "crc_error_control.h"

extern CanFrame frame; // representa um can frame
extern int frame_idx; // representa o indice atual do frame

void frame_decoder(bool rx);

#endif
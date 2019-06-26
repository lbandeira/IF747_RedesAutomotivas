#ifndef FRAME_CONTROL_H
#define FRAME_CONTROL_H

#include "utils.h"
#include "bit_stuff_control.h"
#include "form_error_control.h"
#include "ack_error_control.h"
#include "crc_error_control.h"
#include "bit_error_control.h"

extern CanFrame frame; // representa um can frame
extern int frame_idx; // representa o indice atual do frame
extern bool is_idle; // booleano que indica se o bus esta em idle
extern bool is_frame_complete; // booleano que indica se o frame recebido chegou ao fim

void frame_decoder(bool rx);

#endif
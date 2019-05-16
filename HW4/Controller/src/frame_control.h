#ifndef FRAME_CONTROL_H
#define FRAME_CONTROL_H

#include "utils.h"
#include "bit_stuff_control.h"

extern CanFrame frame; // representa um can frame

void frame_decoder(bool rx, int current_state);

#endif
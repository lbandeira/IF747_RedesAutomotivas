#include "ack_error_control.h"

bool ack_error_flag = false;

void ack_error_control(bool rx){
    if (bit_stuff_flag) {
        ack_error_flag = false;
        return;
    }

    switch (last_state)
    {
        case ACK:
            if(rx){
                ack_error_flag = true;
            }
            break;
        
        default:
            break;
    }
}
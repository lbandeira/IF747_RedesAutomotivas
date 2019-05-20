#include "form_error_control.h"

bool form_error_flag = false;
int eof_idx = 1;

void form_error_control(bool rx){
    bool bit_rtr_a_srr = true;

    if (bit_stuff_flag) {
        form_error_flag = false;
        return;
    }

    switch(last_state){
        case RTR_A_SRR:
            eof_idx = 1;
            bit_rtr_a_srr = rx;
            break;
        
        case IDE:
            if(rx && (!bit_rtr_a_srr)){
                form_error_flag = true;
            }
            break;
        
        case CRC_DELIM:
            if(!rx){
                form_error_flag = true;
            }
            break;
        
        case ACK_DELIM: 
            if(!rx){
                form_error_flag = true;
            }
            break;
        
        case EOFRAME:
            if(!rx && eof_idx < 7){
                form_error_flag = true;
            }
            eof_idx++;
            break;

        case INTER:
            if((!rx) && (inter_count <= 1)){
                current_state = OVERLOAD_FLAG;
            }
            break;

        default:
            break;
    }
}
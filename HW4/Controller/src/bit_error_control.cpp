#include "bit_error_control.h"

bool arbitration_flag = false;
bool bit_error_flag = false;

void bit_error_control(bool rx, bool tx){
    
    // Esta verificacao de erro ocorre apenas pra quem esta escrevendo no bus
    if (is_writing) {
        switch (last_state){
            case ID_A:
                if(rx != tx){
                    arbitration_flag = true; 
                }
                break;

            case RTR_A_SRR:
                if(rx != tx){
                    arbitration_flag = true; 
                }
                break;
            
            case IDE:
                if(rx){
                    if(rx != tx){
                        arbitration_flag = true;
                    }
                }   
                else{
                    if(rx != tx){
                        bit_error_flag = true;
                    }    
                }
                break;

            case ID_B:
                if(rx != tx){
                    arbitration_flag = true; 
                }
                break;
            
            case RTR_B:
                if(rx != tx){
                    arbitration_flag = true; 
                }
                break;

            default:
                if(rx != tx){
                    bit_error_flag = true;
                }   
                break;
        }
    }
}
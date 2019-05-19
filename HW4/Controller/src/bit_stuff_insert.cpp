#include "bit_stuff_insert.h"
#include <Arduino.h>

bool flag_stuff = false;
int cont = 1;
int err_cont = 0; 
bool Tx = false;
bool writing_point = true;

void bit_stuff_insert(bool unstuffed_frame, bool last_bit){
  
    // if(current_state == bit_error_flag){
    //     if(err_cont <= 6)
    //         Tx = 0;
    //     else 
    //         Tx = 1;
    //     err_cont++;
    // }
    // else if (current_state == ERR_DELIM){
    //     err_cont = 0;
    //     Tx = 1;
    // }

    // else if(arbitration_flag){
    //     Tx = 1;
    //     cont = 1;
    //     flag_stuff = 0;
    // }

    if(flag_stuff){
        Tx = !last_bit;
        cont = 1;
        flag_stuff = false;
    }
    else if(writing_point){
        if(current_state < CRC_DELIM){
            //Conta as ocorrencias
            if(unstuffed_frame == last_bit) {
                cont++;
            }
            else {
                cont = 1;
            }

            if(cont == 5){
                //Insere o stuff
                flag_stuff = true;
            }  
        }
        else{
            cont = 1;
            flag_stuff = false;
        }
        Tx = unstuffed_frame;
    }
    if(current_state == IDLE){
        cont = 1;
        flag_stuff = 0;
    }
}
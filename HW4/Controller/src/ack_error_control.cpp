#include "ack_error_control.h"

bool ack_error_flag = false;

void ack_error_control(bool rx){
    if (ack_error_flag) {
        ack_error_flag = false;
    }

    if (last_state == ACK && rx) {
        // Serial.print("ACK >>>>> ");
        // Serial.println(rx);
        ack_error_flag = true;
        Serial.println(">>>>>> ACK ERROR <<<<<<");
    }
}
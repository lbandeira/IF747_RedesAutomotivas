#include "frame_control.h"

int frame_idx = 0; // representa o indice atual do frame
int state_idx = 0; // indice representando o estado atual do frame
int dlc = 0; // representa o tamanho (em bytes) do payload
int payload_size = 0; // representa o tamanho do payload em bits
bool is_extended = false; // booleano que indica que eh um frame extendido ou nao
bool is_idle = true; // booleano que indica se o bus esta em idle

int err_flag_count = 0;
int err_delim_count = 0;
int inter_count = 0;
int over_flag_count = 0;
int over_delim_count = 0;

CanFrame frame;

void frame_decoder(bool rx) {
    // Caso o bit atual seja um bit stuff, o decoder o ignora
    if (bit_stuff_flag) {
        return;
    }

    if(form_error_flag || ack_error_flag || crc_error_flag || arbitration_flag || bit_error_flag){
        current_state = ERR_FLAG;
    }

    if (current_state != IDLE) {
        if (last_state == IDLE)
            frame.raw[frame_idx++] = 0;
        last_state = current_state;
        frame.raw[frame_idx++] = rx;
    }

    switch(current_state) {
        // Estado de IDLE
        case IDLE:
            Serial.println("IDLE");
            if (rx == 0) {
                //ele entra aqui alguma vez??
                // E ele tinha que entrar no SOF. Nao ta entrando aqui
                // pois eh
                Serial.println("HEY!");
                current_state = ID_A;
                state_idx = 0;
                frame_idx = 0;
                is_idle = false; 
            } else {
                is_idle = true;
            }
            break; // ta rolando meio que um deadlock, eu acho, ei

        // Identificador do frame
        case ID_A:
            Serial.println("ID_A");
            frame.id_a[state_idx++] = rx;
            // recebeu os 11 bits do id
            if (state_idx == 11) {
                state_idx = 0;
                current_state = RTR_A_SRR;
            }
            break;

        case RTR_A_SRR:
            frame.rtr_a_srr = rx;
            current_state = IDE;
            break;

        case IDE:
            frame.ide = rx;
            // se o bit for dominante (0) entao o frame eh standard
            // caso contrario, o frame eh extended
            is_extended = rx;
            if (!is_extended)
                current_state = R0;
            else
                current_state = ID_B;
            break;
            
        case R0:
            frame.r0 = rx;
            current_state = DLC;
            break;
        
        case ID_B:
            frame.id_b[state_idx++] = rx;
            // recebeu os 18 bits do id_b
            if (state_idx == 18) {
                state_idx = 0;
                current_state = RTR_B;
            }
            break;

        case RTR_B:
            frame.rtr_b = rx;
            current_state = R1_R0;
            break;

        case R1_R0:
            frame.r1_r0[state_idx++] = rx;
            if (state_idx == 2) {
                state_idx = 0;
                current_state = DLC;
            }
            break;

        case DLC:
            frame.dlc[state_idx++] = rx;
            if (state_idx == 4) {
                state_idx = 0;

                // converte o tamanho do payload (em bytes) para inteiro
                dlc = min(8, (int)convert_bit_array_to_int(frame.dlc, 4));

                if ((!is_extended && frame.rtr_a_srr) || (is_extended && frame.rtr_b) || dlc == 0)
                    current_state = CRC; // frame remoto ou com payload de tamanho 0
                else
                    current_state = PAYLOAD; // frame de dados

                // obtem o tamanho do payload em bits
                payload_size = dlc * 8;
            }
            break;

        case PAYLOAD:
            frame.payload[state_idx++] = rx;
            // recebeu todos os bits do payload
            if (state_idx == payload_size) {
                state_idx = 0;
                current_state = CRC;
            }
            break;

        case CRC:
            frame.crc[state_idx++] = rx;
            // recebeu todos os bits do CRC
            if (state_idx == 15) {
                state_idx = 0;
                current_state = CRC_DELIM;
            }
            break;

        case CRC_DELIM:
            frame.crc_delimiter = rx;
            current_state = ACK;
            break;

        case ACK:
            frame.ack = rx;
            current_state = ACK_DELIM;
            break;

        case ACK_DELIM:
            frame.ack_delimiter = rx;
            current_state = EOFRAME;
            break;

        case EOFRAME:
            frame.eof[state_idx++] = rx;
            // recebeu os 7 bits do EOF
            if (state_idx == 7) {
                state_idx = 0;
            }
            break;
        
        case ERR_FLAG:
            if(rx == false){
                err_flag_count++;
                current_state = ERR_FLAG;
            }
            else if((rx == true) && (err_flag_count >= 5)){
                err_delim_count = 0;
                current_state = ERR_DELIM;
            }
            break;

        case ERR_DELIM:
            if((rx == true) && (err_delim_count <= 7)){
                err_delim_count++;
                current_state = ERR_DELIM;
            }
            else{
                err_flag_count = 0;
                err_delim_count = 0;
                current_state = INTER;
            }
            break;
        
        case OVERLOAD_FLAG:
            if(rx == false){
                over_flag_count++;
                current_state = ERR_FLAG;
            }
            else if((rx == true) && (over_flag_count >= 5)){
                over_delim_count = 0;
                current_state = OVERLOAD_DELIM;
            }
            break;
        
        case OVERLOAD_DELIM:
            if(!rx){
               current_state = ERR_FLAG; 
            }
            else if((rx == true) && (over_delim_count <= 7)){
                over_delim_count++;
                current_state = ERR_DELIM;
            }
            else{
                err_flag_count = 0;
                err_delim_count = 0;
                current_state = INTER;
            }
            break;


        case INTER:
            if((rx == true) && (inter_count < 2)){
                inter_count++;
                current_state = INTER;
            }
            else{
                inter_count = 0;
                current_state = IDLE;
            }
            break;

        default:
            break;
    }
}
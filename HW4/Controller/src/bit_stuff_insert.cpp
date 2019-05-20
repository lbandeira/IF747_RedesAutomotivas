#include "bit_stuff_insert.h"
#include <Arduino.h>

bool flag_stuff = false;
int bit_count = 1;
int err_count = 0; 
bool Tx = true;
bool *frame_to_send;
int frame_size;
int idx = 0;
bool is_writing = false;
bool wait_next_frame = false;
bool last_bit = true;

void bit_stuff_insert() {

    // Caso esteja em estado de erro
    if (current_state == ERR_FLAG || current_state == OVERLOAD_FLAG){
        if(err_count <= 5) 
            Tx = false; 
        else 
            Tx = true;

        err_count++;
        wait_next_frame = false;
        idx = 0;
    }
    // Estado de error delimiter ou overload delimite
    else if (current_state == ERR_DELIM || current_state == OVERLOAD_DELIM) {
        err_count = 0;
        Tx = true;      
    }
    // Estado de intermission
    else if(current_state == INTER){ 
        Tx = true;
    }
    
    // Caso tenha perdido a arbitracao, uma mensagem vai ficar sendo printada
    else if (wait_next_frame) { 
        Serial.println("Sem poder de escrita no bus");
        Tx = true;
    }
    // Caso tenha perdido a arbitracao do bus, as variaveis de controle sao resetadas
    else if (arbitration_flag) {
        Tx = true;
        flag_stuff = false;
        bit_count = 1;
        idx = 0;
        wait_next_frame = true;
    }
    else if (flag_stuff){ 
        // Serial.println("Opa, inseri um bit stuff");
        // Certo, agr ele tah inserindo o it stuff, mas ta colocando onde nao deve tbm
        // mas isso eh pq o current state nao ta sendo atualizado
        // debugar isso agr
        // Eu sou muito burrooooooooo --'
        // acontece, eu não olhei nada na main o if do sample point la na main ta sem nada dentro kkkkkkkk

        Tx = !last_bit;
        bit_count = 1;
        flag_stuff = false;
    }
    else if (is_writing){ 
        Tx = frame_to_send[idx++];

        if(current_state < CRC_DELIM){
            //Conta as ocorrencias
            if(Tx == last_bit) {
                bit_count++;
            }
            else {
                bit_count = 1;
            }
            
            // Se 5 bits iguais foram escritos no bus, o proximo deve ser um bit stuff
            if(bit_count == 5)
                flag_stuff = true;
        }
        else{
            bit_count = 1;
            flag_stuff = false;
        }
        
        // Verificando se chegou ao fim do frame
        if (idx == frame_size) {
            is_writing = false;
            idx = 0;
        }
    }
    // Se for estado de ack, deve-se escrever um bit dominante (0) caso a mensagem tenha chegado correta
    else if (current_state == ACK && !is_writing) {
        if (valid_frame) 
            Tx = false;
        else
            Tx = true;
    }
    // Qualquer outra situacao, escreve bit recessivo
    else {
        Tx = true;
    }

    // Atualizando o ultimo bit lido
    last_bit = Tx;

    if(current_state == IDLE){
        wait_next_frame = false;
        bit_count = 1;
        flag_stuff = false; 
    }
}

void send_frame(bool *unstuffed_frame, int size) {
    frame_to_send = unstuffed_frame;
    frame_size = size;
    is_writing = true;
    Serial.print("O seguinte frame vai ser enviado no bus: ");
    print_bit_array(frame_to_send, frame_size);
}
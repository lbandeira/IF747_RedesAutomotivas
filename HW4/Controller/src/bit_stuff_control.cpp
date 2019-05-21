#include "bit_stuff_control.h"

bool last_bit_rx; // ultimo bit recebido pelo bus
byte bits_count; // variavel de 8 bits para contar numero de bits iguais consecutivos
bool bit_stuff_error = false; // indica que houve um erro de bit stuffing
bool bit_stuff_flag = false; // indica que o bit atual lido do bus eh um bit stuff

// Funcao responsavel por checar se ha erros de bit stuffing
// Recebe como parametros o bit atual do rx e o estado atual
void check_bit_stuff(bool rx) {

    if (bit_stuff_error)
        bit_stuff_error = false;

    // se o estado atual for IDLE (start of frame)
    if (current_state == IDLE) {
        bits_count = 1;
        last_bit_rx = 0; // o bit do start of frame eh dominante (0)
    }
    // caso seja qualquer outro estado antes do crc delimiter
    // (o bit stuff so eh aplicado ate o crc)
    else if (current_state > IDLE && current_state <= CRC) {
        // reseta a variavel que indica se o bit lido eh bit stuff
        if (bit_stuff_flag)
            bit_stuff_flag = false;
        // caso haja mais de 5 bits consecutivos iguais, um erro de bit stuffing eh detectado
        if (bits_count == 6) {
            Serial.println(">>>>>> BIT STUFF ERROR <<<<<<");
            bit_stuff_error = true;
            bits_count = 1;
        } else if (bits_count == 5) {
            // o bit atual eh um bit stuff e deve ser ignorado pelo decoder, pois ja foram lidos 
            // 5 bits consecutivos iguais
            bit_stuff_flag = true;
        }
        // se o bit atual for igua ao anterior, incrementa o contador
        // caso contrario o contador eh resetado
        if (rx == last_bit_rx)
            bits_count++;
        else
            bits_count = 1;

        last_bit_rx = rx;
    } else {
        bits_count = 1;
        last_bit_rx = 0;
        bit_stuff_flag = false;
    }
}
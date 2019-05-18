#include <Arduino.h>
#include "frame_builder.h"

int build_frame(bool *frame_id, bool *payload, bool *frame, bool is_extended, bool is_data, int dlc) {
    int idx_crc;
    int idx_crc_delimiter;
    int idx_ack;
    int idx_ack_delimiter;
    int idx_eof;
    
    // Start of Frame sempre vai ser zero (BIT 0)
    frame[IDX_SOF] = false;

    // Preenchendo os 11 bits do id A (BIT 1 ate BIT 11)
    memcpy(frame + IDX_ID_A, frame_id, 11);

    // Caso o frame seja standard o BIT 12 eh o RTR, caso contrario eh o SRR
    if (!is_extended)
        // Se o frame for de dados o RTR eh dominante (0), caso contrario eh recessivo (1)
        frame[IDX_RTR_A] = !is_data;
    else
        // Obrigatoriamente recessivo caso o frame seja is_extended
        frame[IDX_SRR_B] = true;
    
    // O IDE vai ser dominante (0) caso seja um frame standard, caso contrario sera recessivo (1)
    frame[IDX_IDE] = is_extended;

    // Se o frame for standard o BIT 14 eh o R0, caso contrario eh o inicio do IDB
    if (!is_extended)
        // R0 precisa ser dominante (0)
        frame[IDX_R0_A] = false;
    else
        // Preenchendo o IDB caso o frame seja extended
        memcpy(frame + IDX_ID_B, frame_id + 11, 18);

    if (!is_extended) {
        // Caso o frame seja standard
        // Preenchendo o DLC
        bool dlc_array[4];
        convert_to_bit_array(dlc, dlc_array, 4);
        memcpy(frame + IDX_DLC_A, dlc_array, 4);
        
        // Caso seja frame de dados, o payload eh preenchido
        int payload_size = 0;
        if (is_data) {
            payload_size = dlc * 8;
            memcpy(frame + IDX_PAYLOAD_A, payload, payload_size);
        }

        idx_crc = IDX_PAYLOAD_A + payload_size;
    } else {
        // Caso o frame seja extended
        //Preencher RTR
        frame[IDX_RTR_B] = !is_data;

        //Preencher R1, R0
        frame[IDX_R1_R0] = false;
        frame[IDX_R1_R0 + 1] = false;

        //Preencher DLC
        bool dlc_array[4];
        convert_to_bit_array(dlc, dlc_array, 4);
        memcpy(frame + IDX_DLC_B, dlc_array, 4);

        //Preencher Payload
        int payload_size = 0;
        if (is_data) {
            payload_size = dlc * 8;
            memcpy(frame + IDX_PAYLOAD_B, payload, payload_size);
        }

        idx_crc = IDX_PAYLOAD_B + payload_size;
    }

    // Calculando CRC
    short crc = calculate_crc(frame, idx_crc);
    // Convertendo para array de bits
    bool crc_array[15];
    convert_to_bit_array(crc, crc_array, 15);
    // Preenchendo os 15 bits do frame destinados ao CRC
    memcpy(frame + idx_crc, crc_array, 15);

    // Preenchendo o CRC delimiter, deve ser recessivo (1)
    idx_crc_delimiter = idx_crc + 15;
    frame[idx_crc_delimiter] = true;

    // Preenchendo o ACK slot, transmissor envia recessivo (1)
    idx_ack = idx_crc_delimiter + 1;
    frame[idx_ack] = true;

    // Preenchendo o ACK delimiter, deve ser recessivo (1)
    idx_ack_delimiter = idx_ack + 1;
    frame[idx_ack_delimiter] = true;

    // Preenchendo o EOF, 7 ultimos bits recessivos (1)
    idx_eof = idx_ack_delimiter + 1;
    memset(frame + idx_eof, true, 7);

    return idx_eof + 7;
}
#include "utils.h"

// Funcao auxiliar que transforma numero em um array de bits
void convert_to_bit_array(int value, bool *bit_array, int size) {
    for (int i = 0; i < size; i++) {
        bit_array[i] = value & 0x01;
        value >>= 1;
    }
}

// Converte um array de bits para um array e retorna o mesmo
int convert_bit_array_to_int(bool *bit_array, int size) {
    int result = 0;

    for (int i = 0; i < size; i++) {
        result <<= 1;
        result |= bit_array[i];
    }

    return result;
}

// Funcao que efetua o calculo do CRC
short calculate_crc(bool *frame, int frame_size) {
    short crc_rg = 0;
    bool last_bit;
    bool crc_next;

    for (int i = 0; i < frame_size; i++) {
        last_bit = (crc_rg >> 14) & 0x01;
        crc_next = frame[i] ^ last_bit;
        crc_rg <<= 1;
        if (crc_next)
            crc_rg = crc_rg ^ 0x4599;
    }

    return crc_rg;
}
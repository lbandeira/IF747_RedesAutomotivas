#ifndef UTILS_H
#define UTILS_H

#define IDX_SOF 0
#define IDX_ID_A 1
#define IDX_RTR_A 12
#define IDX_SRR_B 12
#define IDX_IDE 13
#define IDX_ID_B 14
#define IDX_RTR_B 32
#define IDX_R1_R0 33
#define IDX_R0_A 14
#define IDX_DLC_A 15
#define IDX_DLC_B 35
#define IDX_PAYLOAD_A 19
#define IDX_PAYLOAD_B 39

// Funcao auxiliar que transforma numero em um array de bits
void convert_to_bit_array(int value, bool *bit_array, int size);

// Funcao que efetua o calculo do CRC
short calculate_crc(bool *frame, int frame_size);

#endif
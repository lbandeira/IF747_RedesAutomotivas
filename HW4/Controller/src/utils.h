#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <Arduino.h>

// Tipo de dados de 8 bits sem sinal (0 - 255)
typedef unsigned char byte;

// Indices utilizados para auxiliar a construcao do frame
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

// Enumeration que representa o estado atual do frame
enum FrameState {
    IDLE, ID_A, RTR_A_SRR, IDE, R0, ID_B,
    RTR_B, R1_R0, DLC, PAYLOAD, CRC, CRC_DELIM,
    ACK, ACK_DELIM, EOFRAME
};

// Struct que representa um frame can
typedef struct CanFrame
{
    bool id_a[11];
    bool rtr_a_srr;
    bool ide;
    bool r0;
    bool id_b[18];
    bool rtr_b;
    bool r1_r0[2];
    bool dlc[4];
    bool payload[64];
    bool crc[15];
    bool crc_delimiter;
    bool ack;
    bool ack_delimiter;
    bool eof[7];
} CanFrame;

// Funcao auxiliar que transforma numero em um array de bits
void convert_to_bit_array(int value, bool *bit_array, int size);

// Converte um array de bits para um array e retorna o mesmo
int convert_bit_array_to_int(bool *bit_array, int size);

// Printa a estrutura do frame
void print_frame(CanFrame frame);

// Convert uma string representando bits para um array de bits
void string_to_it_array(char *bit_string, bool *bit_array);

// Funcao que efetua o calculo do CRC
short calculate_crc(bool *frame, int frame_size);

#endif
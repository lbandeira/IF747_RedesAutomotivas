#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <Arduino.h>

// Tipo de dados de 8 bits sem sinal (0 - 255)
typedef unsigned char byte;

// Tipo de dados de 64 bits
typedef unsigned long long ull;

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


#if defined(__AVR__)
// Pinos para o TX e RX no Arduino
#define RX_PIN 2
#define TX_PIN 3

#elif defined(ESP32)
// Pinos para o TX e RX na ESP32
#define RX_PIN 18
#define TX_PIN 26

#endif

// Enumeration que representa o estado atual do frame
enum FrameState {
    IDLE, ID_A, RTR_A_SRR, IDE, R0, ID_B,
    RTR_B, R1_R0, DLC, PAYLOAD, CRC, CRC_DELIM,
    ACK, ACK_DELIM, EOFRAME, ERR_FLAG, ERR_DELIM, OVERLOAD_FLAG, OVERLOAD_DELIM, INTER
};

static const char *state_map[] = {
    "IDLE", "ID_A", "RTR_A_SRR", "IDE", "R0", "ID_B",
    "RTR_B", "R1_R0", "DLC", "PAYLOAD", "CRC", "CRC_DELIM",
    "ACK", "ACK_DELIM", "EOFRAME", "ERR_FLAG", "ERR_DELIM", "OVERLOAD_FLAG", "OVERLOAD_DELIM", "INTER"
};

extern int inter_count; // contador dos bits do interframe

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
    bool raw[128];
} CanFrame;

// Funcao auxiliar que transforma numero em um array de bits
void convert_to_bit_array(uint64_t value, bool *bit_array, int size);

// Converte um array de bits para um array e retorna o mesmo
long convert_bit_array_to_int(bool *bit_array, int size);

// Printa a estrutura do frame
void print_frame(CanFrame frame);

// Printa as flags de erro
void print_error_flags(bool ack_error_flag, bool bit_stuff_error, bool bit_error_flag, bool form_error_flag, bool crc_error_flag);

// Convert uma string representando bits para um array de bits
void string_to_bit_array(char *bit_string, bool *bit_array);

// Funcao que efetua o calculo do CRC
short calculate_crc(bool *frame, int frame_size);

// Funcao que printa um array de bits
void print_bit_array(bool *bit_array, int bit_array_size);

#endif
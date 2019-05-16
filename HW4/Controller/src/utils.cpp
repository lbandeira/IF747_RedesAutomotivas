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

// Printa a estrutura do frame
void print_frame(CanFrame frame) {
    int id_a = convert_bit_array_to_int(frame.id_a, 11);
    int id_b = convert_bit_array_to_int(frame.id_b, 18);
    int dlc = convert_bit_array_to_int(frame.dlc, 4);
    int crc = convert_bit_array_to_int(frame.crc, 15);
    int eof = convert_bit_array_to_int(frame.eof, 7);
    bool rtr;
    bool is_extended = frame.ide;

    Serial.println("====================================================");

    if (!is_extended) {
        Serial.print("STANDARD FRAME ");
        rtr = frame.rtr_a_srr;
    } else {
        Serial.print("EXTENDED FRAME ");
        rtr = frame.rtr_b;
    }

    if (!rtr)
        Serial.println("(DATA)");
    else
        Serial.println("(REMOTE)");

    Serial.println("====================================================");

    Serial.print("ID_A: ");
    Serial.println(id_a, HEX);

    if (!is_extended)
        Serial.print("RTR: ");
    else
        Serial.print("SRR: ");
    Serial.println(frame.rtr_a_srr, HEX);

    Serial.print("IDE: ");
    Serial.println(frame.ide, HEX);

    if (is_extended) {
        Serial.print("ID_B: ");
        Serial.println(id_b, HEX);

        Serial.print("RTR: ");
        Serial.println(frame.rtr_b, HEX);

        Serial.print("R1 R0: ");
        Serial.print(frame.r1_r0[0], HEX);
        Serial.print(" ");
        Serial.println(frame.r1_r0[1], HEX);
    } else {
        Serial.print("R0: ");
        Serial.println(frame.r0, HEX);
    }

    Serial.print("DLC: ");
    Serial.println(dlc, HEX);

    // Printando o payload
    Serial.print("Payload: ");
    if (!rtr) {
        for (int i = 0; i < dlc; i++) {
            byte value = (byte) convert_bit_array_to_int(frame.payload + i, 8);
            Serial.print(value, HEX);
        }   
    }
    Serial.println();

    Serial.print("CRC: ");
    Serial.println(crc, HEX);

    Serial.print("CRC Delimiter: ");
    Serial.println(frame.crc_delimiter, HEX);

    Serial.print("ACK: ");
    Serial.println(frame.ack, HEX);

    Serial.print("ACK Delimiter: ");
    Serial.println(frame.ack_delimiter, HEX);

    Serial.print("EOF: ");
    Serial.println(eof, HEX);
}

// Converte uma string representando bits para um array de bits
void string_to_bit_array(char *bit_string, bool *bit_array) {
    int bit_string_size = strlen(bit_string);

    for (int i = 0; i < bit_string_size; i++) {
        bit_array[i] = bit_string[i] - '0';
    }
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
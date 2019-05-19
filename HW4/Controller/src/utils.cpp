#include "utils.h"

// Funcao auxiliar que transforma numero em um array de bits
void convert_to_bit_array(uint64_t value, bool *bit_array, int size) {
    while (size--) {
        bit_array[size] = value & 0x01;
        value >>= 1;
    }
}

// Converte um array de bits para um array e retorna o mesmo
long convert_bit_array_to_int(bool *bit_array, int size) {
    long result = 0;

    for (int i = 0; i < size; i++) {
        result <<= 1;
        result |= bit_array[i];
    }

    return result;
}

// Printa a estrutura do frame
void print_frame(CanFrame frame) {
    uint32_t id_a = convert_bit_array_to_int(frame.id_a, 11);
    uint32_t id_b = convert_bit_array_to_int(frame.id_b, 18);
    uint32_t dlc = convert_bit_array_to_int(frame.dlc, 4);
    uint32_t crc = convert_bit_array_to_int(frame.crc, 15);
    uint32_t eof = convert_bit_array_to_int(frame.eof, 7);
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
            bool *pointer = frame.payload + (i * 8);
            byte value = (byte) convert_bit_array_to_int(pointer, 8);
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

// Printa as flags de erro
void print_error_flags(bool ack_error_flag, bool bit_stuff_error, bool bit_error_flag, bool form_error_flag, bool crc_error_flag) {
    Serial.println("====================================================");
    Serial.println("ERROR FLAGS");
    Serial.println("====================================================");
    Serial.print("BIT ERROR: ");
    Serial.println(bit_error_flag);
    Serial.print("STUFF ERROR: ");
    Serial.println(bit_stuff_error);
    // Serial.print("CRC ERROR: ");
    // Serial.println(crc_error_flag);
    Serial.print("FORM ERROR: ");
    Serial.println(form_error_flag);
    Serial.print("ACK ERROR: ");
    Serial.println(ack_error_flag);
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
        crc_rg = (crc_rg << 1) & 0x7FFF;
        if (crc_next)
            crc_rg = crc_rg ^ 0x4599;
    }

    return crc_rg;
}

// Funcao que printa um array de bits
void print_bit_array(bool *bit_array, int bit_array_size) {
    for (int i = 0; i < bit_array_size; i++) {
        Serial.print(bit_array[i]);
    }
    Serial.println();
}
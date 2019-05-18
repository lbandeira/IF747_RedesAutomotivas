#include "crc_error_control.h"

bool crc_error_flag = false;
bool crc_bit_array[15];
int crc_idx = 0;

void crc_error_control(bool rx) {
    // Se o bit for um bit stuff, ignora
    if (bit_stuff_flag) {
        crc_error_flag = false;
        return;
    }

    // Resetando a flag de erro caso ela esteja setada como true
    if (crc_error_flag) {
        crc_error_flag = false;
        crc_idx = 0;   
    }

    // Caso o estado atual seja de crc, entao deve-se verificar se o mesmo esta correto
    if (last_state == CRC) {
        crc_bit_array[crc_idx++] = rx;
        // Ja foram lidos todos os bits do crc
        if (crc_idx == 15) {
            short calculated_crc = calculate_crc(frame.raw, frame_idx - crc_idx);
            short frame_crc = (short)convert_bit_array_to_int(frame.crc, 15);
            Serial.println("========= CRC ==========");
            Serial.print("Calculated CRC: ");
            Serial.println(calculated_crc, HEX);
            Serial.print("Frame CRC: ");
            Serial.println(frame_crc, HEX);
            Serial.println("========================");
            if (calculated_crc != frame_crc) {
                crc_error_flag = true;
                Serial.print("CRC ERROR: ");
                Serial.println(crc_error_flag);
            }
        }
    } else {
        crc_idx = 0;
        crc_error_flag = 0;
    }
}
#ifndef FRAME_BUILDER_H
#define FRAME_BUILDER_H

#include "utils.h"

// Funcao responsavel por criar o frame e retornar o tamanho do mesmo
int build_frame(bool *frame_id, bool *payload, bool *frame, bool is_extended, bool is_data, int dlc);

#endif
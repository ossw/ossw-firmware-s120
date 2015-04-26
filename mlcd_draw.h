#ifndef MLCD_DRAW_H
#define MLCD_DRAW_H

#include <stdbool.h>
#include <stdint.h>
#include "mlcd.h"

void mlcd_draw_digit(uint_fast8_t digit, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness);

#endif /* MLCD_DRAW_H */

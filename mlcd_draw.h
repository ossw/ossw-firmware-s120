#ifndef MLCD_DRAW_H
#define MLCD_DRAW_H

#include <stdbool.h>
#include <stdint.h>
#include "mlcd.h"

#define FONT_SMALL_REGULAR    0
#define FONT_SMALL_BOLD       1
#define FONT_OPTION_NORMAL    0x10
#define FONT_OPTION_BIG       0x11

#define ALIGN_LEFT  					0x0
#define ALIGN_CENTER					0x40
#define ALIGN_RIGHT						0x80

void mlcd_draw_digit(uint_fast8_t digit, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness);

void mlcd_draw_simple_progress(uint_fast8_t value, uint_fast8_t max, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height);

void mlcd_clear_rect(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height);

void mlcd_draw_rect(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height);

void mlcd_draw_arrow_up(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness);

void mlcd_draw_arrow_down(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness);

void mlcd_draw_rect_border(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness);

uint_fast8_t mlcd_draw_text(const char *text, uint_fast8_t x, uint_fast8_t y, uint_fast8_t width, uint_fast8_t height, uint_fast8_t font_type);

#endif /* MLCD_DRAW_H */

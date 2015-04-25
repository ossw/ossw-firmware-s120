#ifndef MLCD_H
#define MLCD_H

#include <stdbool.h>
#include <stdint.h>
#include "bsp/boards.h"

// Commands
#define MLCD_WR 0x80 //MLCD write line command
#define MLCD_CM 0x20 //MLCD clear memory command
#define MLCD_NO 0x00 //MLCD NOP command (used to switch VCOM)
//LCD resolution
#define MLCD_XRES 144 //pixels per horizontal line
#define MLCD_YRES 168 //pixels per vertical line
#define MLCD_LINE_BYTES (MLCD_XRES >> 3) //number of bytes in a line

//defines the VCOM bit in the command word that goes to the LCD
#define VCOM_HI 0x40
#define VCOM_LO 0x00

void mlcd_init(void);

void mlcd_power_on(void);

void mlcd_power_off(void);

void mlcd_display_on(void);

void mlcd_display_off(void);

void mlcd_backlight_on(void);

void mlcd_backlight_off(void);

void mlcd_set_screen_with_func(uint_fast8_t (*f)(uint_fast8_t, uint_fast8_t));

void mlcd_set_lines_with_func(uint_fast8_t (*f)(uint_fast8_t, uint_fast8_t), uint_fast8_t first_line, uint_fast8_t line_number);

void mlcd_fb_draw_with_func(uint_fast8_t (*f)(uint_fast8_t, uint_fast8_t), uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height);

void mlcd_fb_flush(void);

void mlcd_fb_clear(void);

#endif /* MLCD_H */

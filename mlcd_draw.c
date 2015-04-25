#include "mlcd_draw.h"
#include "mlcd.h"

static uint8_t digits[] = {0x3F, 0x6, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x7, 0x7F, 0x6F};

static uint8_t digit_width;
static uint8_t digit_height;
static uint8_t digit_thickness;
static uint8_t digit_digit;

static uint_fast8_t digit_draw_func(uint_fast8_t x, uint_fast8_t y) {
		uint8_t mask = digits[digit_digit];
		
		if(mask & 0x8) {
				if(y < digit_thickness) {
						return 1;
				}
				
		}
		if(mask & 0x10) {
				if(x >= digit_width-digit_thickness && y < digit_height/2) {
						return 1;
				}
		}
		if(mask & 0x20) {
				if(x >= digit_width-digit_thickness && y >= digit_height/2) {
						return 1;
				}
		}
		if(mask & 0x1) {
				if(y >  digit_height - digit_thickness) {
						return 1;
				}
		}
		if(mask & 0x2) {
				if(x < digit_thickness && y >= digit_height/2) {
						return 1;
				}
		}
		if(mask & 0x4) {
				if(x < digit_thickness && y < digit_height/2) {
						return 1;
				}
		}
		if(mask & 0x40) {
				if(y >  (digit_height - digit_thickness)/2 && y <= (digit_height + digit_thickness)/2) {
						return 1;
				}
		}
		return 0;
}

void mlcd_draw_digit(uint_fast8_t digit, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  digit_width = width;
	  digit_height = height;
	  digit_thickness = thickness;
	  digit_digit = digit;
	  mlcd_fb_draw_with_func(digit_draw_func, x_pos, y_pos, width, height);
}

#include "mlcd_draw.h"
#include "mlcd.h"

static uint8_t digits[] = {0x3F, 0x6, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x7, 0x7F, 0x6F};

static uint8_t draw_width;
static uint8_t draw_height;
static uint8_t draw_thickness;
static uint8_t draw_param;

static uint_fast8_t draw_digit_func(uint_fast8_t x, uint_fast8_t y) {
		uint8_t mask = digits[draw_param];
		
		if(mask & 0x1) {
				if (y < draw_thickness) {
						return 1;
				}	
		}
		if(mask & 0x2) {
				if(x >= draw_width-draw_thickness && y < draw_height/2) {
						return 1;
				}
		}
		if(mask & 0x4) {
				if(x >= draw_width-draw_thickness && y >= draw_height/2) {
						return 1;
				}
		}
		if(mask & 0x8) {
				if(y >= draw_height - draw_thickness) {
						return 1;
				}
		}
		if(mask & 0x10) {
				if(x < draw_thickness && y >= draw_height/2) {
						return 1;
				}
		}
		if(mask & 0x20) {
				if(x < draw_thickness && y < draw_height/2) {
						return 1;
				}
		}
		if(mask & 0x40) {
				if(y >= (draw_height - draw_thickness)/2 && y < (draw_height + draw_thickness)/2) {
						return 1;
				}
		}
		return 0;
}

static uint_fast8_t draw_horizontal_progress_func(uint_fast8_t x, uint_fast8_t y) {
	  return x < draw_param ? 1 : 0;
}

static uint_fast8_t draw_rect_func(uint_fast8_t x, uint_fast8_t y) {
	  return 1;
}

static uint_fast8_t draw_rect_border_func(uint_fast8_t x, uint_fast8_t y) {
  	return (x < draw_thickness || y < draw_thickness || x >= draw_width - draw_thickness || y >= draw_height - draw_thickness) ? 1 : 0;
}

static uint_fast8_t draw_arrow_up_func(uint_fast8_t x, uint_fast8_t y) {
	  return (
             (  (y < draw_height * x * 2 / draw_width) && (y < draw_height * (draw_width-x) * 2 / draw_width))
                &&
                (  (y + draw_thickness >= draw_height * x * 2 / draw_width) || (y + draw_thickness >= draw_height * (draw_width-x) * 2 / draw_width))
            
          ) ? 1 : 0;
}

static uint_fast8_t draw_arrow_down_func(uint_fast8_t x, uint_fast8_t y) {
	y = draw_height - y;
	return (
             (  (y < draw_height * x * 2 / draw_width) && (y < draw_height * (draw_width-x) * 2 / draw_width))
                &&
                (  (y + draw_thickness >= draw_height * x * 2 / draw_width) || (y + draw_thickness >= draw_height * (draw_width-x) * 2 / draw_width))
            
          ) ? 1 : 0;
}

void mlcd_draw_digit(uint_fast8_t digit, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  draw_width = width;
	  draw_height = height;
	  draw_thickness = thickness;
	  draw_param = digit;
	  mlcd_fb_draw_with_func(draw_digit_func, x_pos, y_pos, width, height);
}

void mlcd_draw_simple_progress(uint_fast8_t value, uint_fast8_t max, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height) {
		draw_width = width;
	  draw_height = height;
	  draw_param = value*width/max;
	  mlcd_fb_draw_with_func(draw_horizontal_progress_func, x_pos, y_pos, width, height);
}

void mlcd_draw_rect(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height) {
	  draw_width = width;
	  draw_height = height;
	  mlcd_fb_draw_with_func(draw_rect_func, x_pos, y_pos, width, height);
}
void mlcd_draw_rect_border(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  draw_width = width;
	  draw_height = height;
	  draw_thickness = thickness;
	  mlcd_fb_draw_with_func(draw_rect_border_func, x_pos, y_pos, width, height);
}

void mlcd_draw_arrow_up(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  draw_width = width;
	  draw_height = height;
	  draw_thickness = thickness;
	  mlcd_fb_draw_with_func(draw_arrow_up_func, x_pos, y_pos, width, height);
}

void mlcd_draw_arrow_down(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  draw_width = width;
	  draw_height = height;
	  draw_thickness = thickness;
	  mlcd_fb_draw_with_func(draw_arrow_down_func, x_pos, y_pos, width, height);
}



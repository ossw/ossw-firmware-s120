#include "scr_controls.h"
#include "mlcd_draw.h"
#include "string.h"

static void limit_int_range(uint32_t* value, uint32_t max_value) {
	  if(*value > max_value) {
			  *value = max_value;
		}
}

static void draw_int_value(uint32_t value, uint32_t old_value, uint8_t digits_no, uint8_t x, uint8_t y, uint8_t digit_width, uint8_t digit_height, uint8_t thickness, uint8_t digit_space, bool force) {
		/*if (force || value/10 != cfg->data->last_value/10) {
								mlcd_draw_digit(value/10, cfg->x, cfg->y, digit_width, digit_height, thickness);
						}
						if (force || value%10 != cfg->data->last_value%10) {
								mlcd_draw_digit(value%10, cfg->x + digit_dist + digit_width, cfg->y, digit_width, digit_height, thickness);
						}*/
	  int current_x = x;
		int div = 1;
		for (int i = 1; i < digits_no; i++) {
				div *= 10;
		}
		for (int i = 0; i < digits_no; i++) {
		    uint8_t old_digit = (old_value / div)%10;
		    uint8_t new_digit = (value / div)%10;
			  if (force || old_digit != new_digit) {
			      mlcd_draw_digit(new_digit, current_x, y, digit_width, digit_height, thickness);
				}
				div = div/10;
				current_x += digit_width + digit_space;
		}
}

static void scr_controls_draw_number_control(SCR_CONTROL_NUMBER_CONFIG* cfg, bool force) {
	  uint32_t value = cfg->data_handle(cfg->data_handle_param);
		
		if (!force && cfg->data->last_value == value) {
				return;
		}
						
    uint8_t digit_width = cfg->style >> 8 & 0xFF;
    uint8_t digit_height = cfg->style & 0xFF;
    uint8_t thickness = (cfg->style>>16) & 0x3F;
    uint8_t digit_dist = (cfg->style>>22) & 0x1F;
						
	  switch(cfg->range) {
			  case NUMBER_RANGE_0__9:
					  limit_int_range(&value, 9);
					  draw_int_value(value, cfg->data->last_value, 1, cfg->x, cfg->y, digit_width, digit_height, thickness, digit_dist, force);
				    break;
			  case NUMBER_RANGE_0__99:
					  limit_int_range(&value, 99);
					  draw_int_value(value, cfg->data->last_value, 2, cfg->x, cfg->y, digit_width, digit_height, thickness, digit_dist, force);
				    break;
			  case NUMBER_RANGE_0__999:
					  limit_int_range(&value, 999);
					  draw_int_value(value, cfg->data->last_value, 3, cfg->x, cfg->y, digit_width, digit_height, thickness, digit_dist, force);
				    break;
			  case NUMBER_RANGE_0__9999:
					  limit_int_range(&value, 9999);
					  draw_int_value(value, cfg->data->last_value, 4, cfg->x, cfg->y, digit_width, digit_height, thickness, digit_dist, force);
				    break;
			  case NUMBER_RANGE_0__99999:
					  limit_int_range(&value, 99999);
					  draw_int_value(value, cfg->data->last_value, 5, cfg->x, cfg->y, digit_width, digit_height, thickness, digit_dist, force);
				    break;
		}
		
		cfg->data->last_value = value;
}

static void scr_controls_draw_text_control(SCR_CONTROL_TEXT_CONFIG* cfg, bool force) {
	  char* value = (char*)cfg->data_handle(cfg->data_handle_param);
	
		if (force) {
			  mlcd_draw_text(value, cfg->x, cfg->y,cfg->width, cfg->height, FONT_OPTION_NORMAL);
		} else {
			  if (strcmp(value, cfg->data->last_value)!=0) {
				    mlcd_clear_rect(cfg->x, cfg->y,cfg->width, cfg->height);
			      mlcd_draw_text(value, cfg->x, cfg->y,cfg->width, cfg->height, FONT_OPTION_NORMAL);
				} else {
					  return;
				} 
		}
		strcpy((void*)cfg->data->last_value, (void*)value);
}

static void scr_controls_draw_horizontal_progress_bar_control(SCR_CONTROL_PROGRESS_BAR_CONFIG* cfg, bool force) {
	  uint32_t value = cfg->data_handle();
	
	  if (force || cfg->data->last_value != value) {
				if (cfg->border > 0) {
						uint8_t delta = cfg->border + 1;
					  if (force) {
								mlcd_draw_rect_border(cfg->x, cfg->y, cfg->width, cfg->height, cfg->border);
					  }
						mlcd_draw_simple_progress(value, cfg->max, cfg->x + delta, cfg->y + delta, cfg->width - 2 * delta, cfg->height - 2 * delta);
				} else {
				    mlcd_draw_simple_progress(value, cfg->max, cfg->x, cfg->y, cfg->width, cfg->height);
				}
		}
		
		cfg->data->last_value = value;
}

static void scr_controls_draw_static_rect(SCR_CONTROL_STATIC_RECT_CONFIG* cfg, bool force) {
		if (force) {
				mlcd_draw_rect(cfg->x, cfg->y, cfg->width, cfg->height);
		}
}

static void scr_controls_draw_internal(const SCR_CONTROLS_DEFINITION* ctrls_def, bool force) {
	  for (int i=0; i < ctrls_def->controls_no; i++) {
			  const SCR_CONTROL_DEFINITION* ctrl_def = &ctrls_def->controls[i];
			
			  switch(ctrl_def->type) {
					  case SCR_CONTROL_NUMBER:
						    scr_controls_draw_number_control((SCR_CONTROL_NUMBER_CONFIG*)ctrl_def->config, force);
					      break;
					  case SCR_CONTROL_TEXT:
						    scr_controls_draw_text_control((SCR_CONTROL_TEXT_CONFIG*)ctrl_def->config, force);
					      break;
					  case SCR_CONTROL_HORIZONTAL_PROGRESS_BAR:
						    scr_controls_draw_horizontal_progress_bar_control((SCR_CONTROL_PROGRESS_BAR_CONFIG*)ctrl_def->config, force);
					      break;
						case SCR_CONTROL_STATIC_RECT:
							  scr_controls_draw_static_rect((SCR_CONTROL_STATIC_RECT_CONFIG*)ctrl_def->config, force);
					      break;
				}
		}
}

void scr_controls_draw(const SCR_CONTROLS_DEFINITION* ctrls_def) {
	  scr_controls_draw_internal(ctrls_def, true);
}

void scr_controls_redraw(const SCR_CONTROLS_DEFINITION* ctrls_def) {
	  scr_controls_draw_internal(ctrls_def, false);
}

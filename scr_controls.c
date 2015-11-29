#include "scr_controls.h"
#include "mlcd_draw.h"
#include "string.h"
#include "fs.h"

static void limit_int_range(uint32_t* value, uint32_t max_value) {
	  if(*value > max_value) {
			  *value = max_value;
		}
}

static uint32_t pow(uint32_t x, uint8_t n) {
	  uint32_t result = 1;
	  for(uint32_t i = 0; i < n; i++) {
			  result *= x;
		}
		return result;
}

static void draw_int_value(uint32_t value, uint32_t old_value, uint8_t digits_no, uint8_t decimal_size, uint8_t x, uint8_t y, uint8_t digit_width, uint8_t digit_height, uint8_t thickness, uint8_t digit_space, bool left_padded, bool force) {
	  int current_x = x;
		int div = pow(10, digits_no-1);
		for (int i = 0; i < digits_no; i++) {
			
			  if (decimal_size > 0 && digits_no - i == decimal_size) {
					  if (force){
					      mlcd_draw_rect(current_x, y + digit_height - thickness, thickness, thickness);
						}
					  current_x += thickness + digit_space;
				}
			
        uint32_t scaled_old_val = old_value / div;
		    uint8_t old_digit = scaled_old_val%10;
        uint32_t scaled_val = value / div;
		    uint8_t new_digit = scaled_val % 10;
				
				bool draw_zero_value = left_padded || (digits_no - i - 1 <= decimal_size);
				
			  if (force || old_digit != new_digit || (scaled_val > 9)!=(scaled_old_val > 9)) {
					  if (scaled_val > 0 || draw_zero_value) {
								mlcd_draw_digit(new_digit, current_x, y, digit_width, digit_height, thickness);
						} else {
							  mlcd_clear_rect(current_x, y, digit_width, digit_height);
						}
				}
				div = div/10;
				current_x += digit_width + digit_space;
		}
}

static void draw_1X_int_value(uint32_t value, uint32_t old_value, uint8_t digits_no, uint8_t decimal_size, uint8_t x, uint8_t y, uint8_t digit_width, uint8_t digit_height, uint8_t thickness, uint8_t digit_space, bool force) {
		int div = pow(10, digits_no-1);
		uint8_t old_digit = (old_value / div)%10;
		uint8_t new_digit = (value / div)%10;
		
		if (force || old_digit != new_digit) {
			  if (new_digit) {
					  mlcd_draw_rect(x, y, thickness, digit_height);
				} else {
					  mlcd_clear_rect(x, y, thickness, digit_height);
				}
		}
		draw_int_value(value, old_value, digits_no - 1, decimal_size, x + thickness + digit_space, y, digit_width, digit_height, thickness, digit_space, false, force);
}

static void draw_int_img_value(uint32_t value, uint32_t old_value, uint8_t digits_no, uint8_t decimal_size, uint8_t x, uint8_t y, spiffs_file file, uint8_t digit_space, bool left_padded, bool force) {
	  int current_x = x;
		int div = pow(10, digits_no-1);
		uint8_t header[4];
		SPIFFS_read(&fs, file, header, 4);
		uint32_t base_address = SPIFFS_lseek(&fs, file, 0, SPIFFS_SEEK_CUR);
		uint8_t digit_width = header[2];
		uint8_t digit_height = header[3];
	
		for (int i = 0; i < digits_no; i++) {
			
			  if (decimal_size > 0 && digits_no - i == decimal_size) {
					  if (force){
					      //mlcd_draw_rect(current_x, y + digit_height - thickness, thickness, thickness);
						}
					  current_x += /*thickness +*/ digit_space;
				}
			
        uint32_t scaled_old_val = old_value / div;
		    uint8_t old_digit = scaled_old_val%10;
        uint32_t scaled_val = value / div;
		    uint8_t new_digit = scaled_val % 10;
				
				bool draw_zero_value = left_padded || (digits_no - i - 1 <= decimal_size);
				
			  if (force || old_digit != new_digit || (scaled_val > 9)!=(scaled_old_val > 9)) {
					  if (scaled_val > 0 || draw_zero_value) {
							
								SPIFFS_lseek(&fs, file, base_address + (((digit_width+7)/8)*digit_height*new_digit), SPIFFS_SEEK_SET);
								mlcd_fb_draw_bitmap_from_file(file, current_x, y, digit_width, digit_height, digit_width);
						} else {
							  mlcd_clear_rect(current_x, y, digit_width, digit_height);
						}
				}
				div = div/10;
				current_x += digit_width + digit_space;
		}
}

void scr_controls_draw_number_control(SCR_CONTROL_NUMBER_CONFIG* cfg, bool force) {
	  uint8_t decimal_size = cfg->range&0xF;
	  uint32_t value = cfg->data_handle(cfg->data_handle_param, decimal_size);
		
		if (!force && cfg->data->last_value == value) {
				return;
		}
						
		uint8_t type = cfg->style >> 30;
		uint8_t digit_dist = (cfg->style>>24) & 0x1F;
		bool leftPadded = cfg->style & 0x20000000;
					
		uint8_t digit_no = (cfg->range>>4)/2 + 1 + decimal_size;
		uint8_t is_1X_format = (cfg->range>>4)%2 == 0;
		
		switch(type) {
			case 0:
			{
					uint8_t digit_width = cfg->style >> 8 & 0xFF;
					uint8_t digit_height = cfg->style & 0xFF;
					uint8_t thickness = (cfg->style>>16) & 0x3F;
					
					if (is_1X_format) {
							 limit_int_range(&value, 2 * pow(10, digit_no-1) - 1);
							 draw_1X_int_value(value, cfg->data->last_value, digit_no, decimal_size, cfg->x, cfg->y, digit_width, digit_height, thickness, digit_dist, force);		
					} else {
							 limit_int_range(&value, pow(10, digit_no) - 1);
							 draw_int_value(value, cfg->data->last_value, digit_no, decimal_size, cfg->x, cfg->y, digit_width, digit_height, thickness, digit_dist, leftPadded, force);	   
					}
			}
			break;
			
			case 1:
			{
					spiffs_file file = cfg->style&0xFFFF;
					limit_int_range(&value, pow(10, digit_no) - 1);
					draw_int_img_value(value, cfg->data->last_value, digit_no, decimal_size, cfg->x, cfg->y, file, digit_dist, leftPadded, force);
			}
			break;
			
		}
		
		cfg->data->last_value = value;
}

void scr_controls_draw_static_image_control(SCR_CONTROL_STATIC_IMAGE_CONFIG* cfg, bool force) {

		if (!force) {
				return;
		}
		
		uint8_t header[4];
		SPIFFS_read(&fs, cfg->file, header, 4);
		
		mlcd_fb_draw_bitmap_from_file(cfg->file, cfg->x, cfg->y, 
				cfg->width != 0 ? cfg->width:header[2], 
				cfg->height != 0 ? cfg->height:header[3],
				header[2]);
}

void scr_controls_draw_image_from_set_control(SCR_CONTROL_IMAGE_FROM_SET_CONFIG* cfg, bool force) {

	  uint32_t value = cfg->data_handle(cfg->data_handle_param, 0);
	
		if (!force && cfg->data->last_value == value) {
				return;
		}
		
		uint8_t header[6];
		SPIFFS_read(&fs, cfg->file, header, 6);
		
    uint8_t firstImageId = header[2];
    uint8_t numberOfImages = header[3];

    if (value < firstImageId || value >= firstImageId + numberOfImages) {
        return;
    }
		
    uint8_t imageWidth = header[4];
    uint8_t imageHeight = header[5];
				
    uint8_t imageNo = value - firstImageId;
    uint32_t offset = (((imageWidth + 7) / 8) * imageHeight * imageNo);
		SPIFFS_lseek(&fs, cfg->file, offset, SPIFFS_SEEK_CUR);
		mlcd_fb_draw_bitmap_from_file(cfg->file, cfg->x, cfg->y, cfg->width, cfg->height, imageWidth);
		cfg->data->last_value = value;
}

void scr_controls_draw_text_control(SCR_CONTROL_TEXT_CONFIG* cfg, bool force) {
	  char* value = (char*)cfg->data_handle(cfg->data_handle_param);
	
	  uint8_t font_type = (cfg->style >> 24) & 0xFF;
	  uint8_t alignment = (cfg->style >> 16) & 0xFF;
	
		if (force) {
			  mlcd_draw_text(value, cfg->x, cfg->y,cfg->width, cfg->height, font_type, alignment);
		} else {
			  if (strcmp(value, cfg->data->last_value)!=0) {
				    mlcd_clear_rect(cfg->x, cfg->y,cfg->width, cfg->height);
			      mlcd_draw_text(value, cfg->x, cfg->y,cfg->width, cfg->height, font_type, alignment);
				} else {
					  return;
				} 
		}
		strcpy((void*)cfg->data->last_value, (void*)value);
}

void scr_controls_draw_progress_bar_control(SCR_CONTROL_PROGRESS_BAR_CONFIG* cfg, bool force) {
	  uint32_t value = cfg->data_handle(cfg->data_handle_param, 0);

	  bool horizontal = !((cfg->style>>24)&0x20);
	  uint8_t border = (cfg->style>>16)&0xFF;
	  
	  if (force || cfg->data->last_value != value) {
				if (border > 0) {
						uint8_t delta = border + 1;
					  if (force) {
								mlcd_draw_rect_border(cfg->x, cfg->y, cfg->width, cfg->height, border);
					  }
						mlcd_draw_simple_progress(value, cfg->max, cfg->x + delta, cfg->y + delta, cfg->width - 2 * delta, cfg->height - 2 * delta, horizontal);
				} else {
				    mlcd_draw_simple_progress(value, cfg->max, cfg->x, cfg->y, cfg->width, cfg->height, horizontal);
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
					  case SCR_CONTROL_PROGRESS_BAR:
						    scr_controls_draw_progress_bar_control((SCR_CONTROL_PROGRESS_BAR_CONFIG*)ctrl_def->config, force);
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

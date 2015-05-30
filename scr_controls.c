#include "scr_controls.h"
#include "mlcd_draw.h"

static void scr_controls_draw_number_control(SCR_CONTROL_NUMBER_CONFIG* cfg, bool force) {
	  uint32_t value = cfg->data_handle();
	
	  switch(cfg->format & 0xF) {
			  case NUMBER_FORMAT_0_99:
				{
					  if(value > 99) {value = 99;}
					
					  if (!force && cfg->data->last_value == value) {
							  return;
						}
					
					  uint8_t digit_dist = (cfg->width >> 5)+1;
					  uint8_t digit_width = (cfg->width - digit_dist) / 2;
						
						if (force || value/10 != cfg->data->last_value/10) {
								mlcd_draw_digit(value/10, cfg->x, cfg->y, digit_width, cfg->height, cfg->thickness);
						}
						if (force || value%10 != cfg->data->last_value%10) {
								mlcd_draw_digit(value%10, cfg->x + digit_dist + digit_width, cfg->y, digit_width, cfg->height, cfg->thickness);
						}
				}
						break;
		}
		
		cfg->data->last_value = value;
}

static void scr_controls_draw_horizontal_progress_bar_control(SCR_CONTROL_PROGRESS_BAR_CONFIG* cfg, bool force) {
	  uint32_t value = cfg->data_handle();
	
	  if(force || cfg->data->last_value != value) {
			  mlcd_draw_simple_progress(value, cfg->max, cfg->x, cfg->y, cfg->width, cfg->height);
		}
		
		cfg->data->last_value = value;
}

static void scr_controls_draw_internal(const SCR_CONTROLS_DEFINITION* ctrls_def, bool force) {
	  for (int i=0; i < ctrls_def->controls_no; i++) {
			  const SCR_CONTROL_DEFINITION* ctrl_def = &ctrls_def->controls[i];
			
			  switch(ctrl_def->type) {
					  case SCR_CONTROL_NUMBER:
						    scr_controls_draw_number_control((SCR_CONTROL_NUMBER_CONFIG*)ctrl_def->config, force);
					      break;
					  case SCR_CONTROL_HORIZONTAL_PROGRESS_BAR:
						    scr_controls_draw_horizontal_progress_bar_control((SCR_CONTROL_PROGRESS_BAR_CONFIG*)ctrl_def->config, force);
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

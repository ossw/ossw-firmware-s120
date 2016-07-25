#include "scr_dark_hours.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../i18n/i18n.h"
#include "../ext_ram.h"
#include "../graph.h"
#include "../alarm.h"

#define TIME_Y_POS    		80
#define ARROW_HEIGHT			8
#define MODE_HOUR1     		0
#define MODE_HOUR2	  		1

static uint8_t 						change_mode;
static int8_t 						light_hour1;
static int8_t 						light_hour2;

static void scr_hours_draw_hour1() {
	  mlcd_draw_digit(light_hour1/10, 4, TIME_Y_POS, 28, 40, 4);
	  mlcd_draw_digit(light_hour1%10, 36, TIME_Y_POS, 28, 40, 4);
}

static void scr_hours_draw_hour2() {
	  mlcd_draw_digit(light_hour2/10, 80, TIME_Y_POS, 28, 40, 4);
	  mlcd_draw_digit(light_hour2%10, 112, TIME_Y_POS, 28, 40, 4);
}

static void scr_changetime_draw_all() {
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_BACKLIGHT_HOURS), 10, 10, MLCD_XRES-10, 40, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_LEFT | MULTILINE);
	  fillRectangle(67, TIME_Y_POS + 18, 10, 4, DRAW_WHITE);
	
	  if (change_mode == MODE_HOUR1) {
			  fillDown(34, TIME_Y_POS + 52, ARROW_HEIGHT, DRAW_WHITE);
			  fillUp(34, TIME_Y_POS - 13, ARROW_HEIGHT, DRAW_WHITE);
		} else if (change_mode == MODE_HOUR2) {
			  fillDown(110, TIME_Y_POS + 52, ARROW_HEIGHT, DRAW_WHITE);
			  fillUp(110, TIME_Y_POS - 13, ARROW_HEIGHT, DRAW_WHITE);
		}
		scr_hours_draw_hour1();
	  scr_hours_draw_hour2();
}

static void scr_changetime_handle_button_up(void) {
		if (change_mode == MODE_HOUR1) {
			  if (++light_hour1 > 23) {
					  light_hour1 = 0;
				}
				scr_hours_draw_hour1();
		}	else if (change_mode == MODE_HOUR2) {
			  if (++light_hour2 > 24) {
					  light_hour2 = 0;
				}
				scr_hours_draw_hour2();
		} 
	  mlcd_fb_flush();
}

static void scr_changetime_handle_button_down(void) {
	  if (change_mode == MODE_HOUR1) {
			  if(--light_hour1 < 0) {
					  light_hour1 = 23;
				}
				scr_hours_draw_hour1();
		}	else if (change_mode == MODE_HOUR2) {
			  if(--light_hour2 < 0) {
					  light_hour2 = 24;
				}
				scr_hours_draw_hour2();
		}
	  mlcd_fb_flush();
}

static void scr_changetime_handle_button_select(void) {
	  change_mode++;
		change_mode %= 2;
		mlcd_fb_clear();
	  scr_changetime_draw_all();
    mlcd_fb_flush();
}

static void scr_changetime_handle_button_back(void) {
		put_ext_ram_byte(EXT_RAM_DARK_HOURS, light_hour1);
		put_ext_ram_byte(EXT_RAM_DARK_HOURS + 1, light_hour2);
    scr_mngr_show_screen(SCR_SETTINGS);
}

static bool scr_changetime_handle_button_pressed(uint32_t event_type, uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  scr_changetime_handle_button_up();
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  scr_changetime_handle_button_down();
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
						scr_changetime_handle_button_select();
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_changetime_handle_button_back();
				    return true;
		}
		return false;
}

static void scr_changetime_init() {
		light_hour1 = get_ext_ram_byte(EXT_RAM_DARK_HOURS);
		light_hour2 = get_ext_ram_byte(EXT_RAM_DARK_HOURS + 1);
	  change_mode = MODE_HOUR1;
}

bool scr_set_dark_hours_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_changetime_init();
				    return true;
			  case SCR_EVENT_DRAW_SCREEN:
						scr_changetime_draw_all();
				    return true;
			  case SCR_EVENT_BUTTON_PRESSED:
			  case SCR_EVENT_BUTTON_LONG_PRESSED:
				    return scr_changetime_handle_button_pressed(event_type, event_param);
		}
		return false;
}

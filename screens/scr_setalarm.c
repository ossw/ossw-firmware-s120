#include "scr_setalarm.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../i18n/i18n.h"
#include "../ext_ram.h"
#include "../graph.h"
#include "../alarm.h"

#define TIME_Y_POS    		60
#define ARROW_HEIGHT			8
#define DAY_HEIGHT				5
#define MODE_HOUR     		0x01
#define MODE_MINUTE	  		0x02

static uint8_t 						change_mode;
static uint8_t						alarm_options;
static int8_t 						alarm_hour;
static int8_t 						alarm_minute;

static void scr_alarm_draw_hour() {
	  mlcd_draw_digit(alarm_hour/10, 4, TIME_Y_POS, 28, 40, 4);
	  mlcd_draw_digit(alarm_hour%10, 36, TIME_Y_POS, 28, 40, 4);
}

static void scr_alarm_draw_minutes() {
	  mlcd_draw_digit(alarm_minute/10, 80, TIME_Y_POS, 28, 40, 4);
	  mlcd_draw_digit(alarm_minute%10, 112, TIME_Y_POS, 28, 40, 4);
}

static void scr_alarm_toggle_day(uint8_t day) {
		fillRectangle(4+day*20, 128, 17, 20);
}

static void scr_alarm_draw_day(uint8_t day) {
		uint8_t day_mask = 1<<day;
		uint8_t day_x = 4+day*20;
		mlcd_draw_text(I18N_TRANSLATE(MESSAGE_SUNDAY_1+day), day_x, 130, 18, 18, FONT_BIG_REGULAR, HORIZONTAL_ALIGN_CENTER);
		if (day == rtc_get_current_day_of_week()-1)
				fillRectangle(day_x+1, 145, 15, 2);
		if (alarm_options & day_mask)
				scr_alarm_toggle_day(day);
}

static void scr_alarm_draw_days() {
		for (uint8_t day = 0; day < 7; day++) {
				scr_alarm_draw_day(day);
		}
		if (change_mode > MODE_MINUTE) {
				uint8_t day = change_mode - MODE_MINUTE - 1;
			  fillUp(12 + day*20, 121, DAY_HEIGHT);
			  fillDown(12 + day*20, 154, DAY_HEIGHT);
		}
}

static void scr_changetime_draw_all() {
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_ALARM_CLOCK), 10, 5, 80, NULL, FONT_OPTION_BIG, HORIZONTAL_ALIGN_LEFT);
//	  mlcd_draw_rect(0, 25, MLCD_XRES, 2);
	
	  mlcd_draw_rect(69, TIME_Y_POS + 24, 5, 5);
	  mlcd_draw_rect(69, TIME_Y_POS + 10, 5, 5);
	
	  if (change_mode == MODE_HOUR) {
			  fillDown(34, TIME_Y_POS + 52, ARROW_HEIGHT);
			  fillUp(34, TIME_Y_POS - 13, ARROW_HEIGHT);
		} else if (change_mode == MODE_MINUTE) {
			  fillDown(110, TIME_Y_POS + 52, ARROW_HEIGHT);
			  fillUp(110, TIME_Y_POS - 13, ARROW_HEIGHT);
		}
		
	  scr_alarm_draw_hour();
	  scr_alarm_draw_minutes();
	  scr_alarm_draw_days();
}

static void scr_changetime_handle_button_up(void) {
		if (change_mode == MODE_HOUR) {
			  if (++alarm_hour > 23) {
					  alarm_hour = 0;
				}
				scr_alarm_draw_hour();
		}	else if (change_mode == MODE_MINUTE) {
			  if (++alarm_minute > 59) {
					  alarm_minute = 0;
				}
				scr_alarm_draw_minutes();
		} else {
				uint8_t day = change_mode - MODE_MINUTE - 1;
				uint8_t day_mask = 1<<day;
				alarm_options ^= day_mask;
				scr_alarm_toggle_day(day);
		}		
	  mlcd_fb_flush();
}

static void scr_changetime_handle_button_down(void) {
	  if (change_mode == MODE_HOUR) {
			  if(--alarm_hour < 0) {
					  alarm_hour = 23;
				}
				scr_alarm_draw_hour();
		}	else if (change_mode == MODE_MINUTE) {
			  if(--alarm_minute < 0) {
					  alarm_minute = 59;
				}
				scr_alarm_draw_minutes();
		} else {
				uint8_t day = change_mode - MODE_MINUTE - 1;
				uint8_t day_mask = 1<<day;
				alarm_options ^= day_mask;
				scr_alarm_toggle_day(day);
		}
	  mlcd_fb_flush();
}

static void scr_changetime_handle_button_select(void) {
		if (change_mode >= MODE_MINUTE + 7) {
				store_alarm_clock(alarm_options, alarm_hour, alarm_minute);
				scr_mngr_show_screen(SCR_SETTINGS);
		} else {
			  change_mode++;
				mlcd_fb_clear();
			  scr_changetime_draw_all();
	      mlcd_fb_flush();
		}
}

static void scr_changetime_handle_button_back(void) {
	  if (change_mode == MODE_HOUR) {
		    scr_mngr_show_screen(SCR_SETTINGS);
	  } else {
			  change_mode--;
				mlcd_fb_clear();
			  scr_changetime_draw_all();
	      mlcd_fb_flush();
		}
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
						if (event_type == SCR_EVENT_BUTTON_LONG_PRESSED)
								change_mode = MODE_MINUTE + 7;
						scr_changetime_handle_button_select();
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_BACK:
						if (event_type == SCR_EVENT_BUTTON_LONG_PRESSED)
								change_mode = MODE_HOUR;
					  scr_changetime_handle_button_back();
				    return true;
		}
		return false;
}

static void scr_changetime_init() {
		load_alarm_clock(&alarm_options, &alarm_hour, &alarm_minute);
	  change_mode = MODE_HOUR;
}

bool scr_set_alarm_handle_event(uint32_t event_type, uint32_t event_param) {
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

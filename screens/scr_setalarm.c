#include "scr_setalarm.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../i18n/i18n.h"
#include "time.h"
#include "../ext_ram.h"
#include "../vibration.h"
#include "../graph.h"

#define TIME_Y_POS    		60
#define MODE_ACTIVATE			0x00
#define MODE_HOUR     		0x01
#define MODE_MINUTES  		0x02
#define ALARM_VIBRATION		0x0060AB6E

static app_timer_id_t			alarm_clock_id;
static uint8_t 						change_mode;
static uint8_t						alarm_options;
static int8_t 						alarm_hour;
static int8_t 						alarm_minute;

void load_alarm_clock(void) {
	  uint8_t buffer[3];
		ext_ram_read_data(EXT_RAM_DATA_ALARM, buffer, 3);
		alarm_options = buffer[0];
		alarm_minute = buffer[1];
	  if (0 > alarm_minute || alarm_minute > 59) {
			  alarm_minute = 0;
		}
		alarm_hour = buffer[2];
		if (0 > alarm_hour || alarm_hour > 23) {
				alarm_hour = 0;
		}
}

void store_alarm_clock(void) {
		uint8_t buffer[3];
		buffer[0] = alarm_options;
		buffer[1] = alarm_minute;
		buffer[2] = alarm_hour;
		ext_ram_write_data(EXT_RAM_DATA_ALARM, buffer, 3);
}

void alarm_clock_reschedule(void) {
		app_timer_stop(alarm_clock_id);
		if ((alarm_options & 0x80) == 0)
				return;
		int32_t interval = 3600 * alarm_hour + 60 * alarm_minute - rtc_get_current_time_in_seconds();
		while (interval <= 0)
				interval += DAY_IN_SECONDS;
    uint32_t err_code = app_timer_start(alarm_clock_id, APP_TIMER_TICKS(1000*interval, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
}

static void alarm_clock_handler(void * p_context) {
		vibration_vibrate(ALARM_VIBRATION, 30000);
		alarm_clock_reschedule();
}

void alarm_clock_init(void) {
    uint32_t err_code = app_timer_create(&alarm_clock_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                alarm_clock_handler);
    APP_ERROR_CHECK(err_code);
		alarm_clock_reschedule();
}

static void scr_alarm_draw_hour() {
	  mlcd_draw_digit(alarm_hour/10, 4, TIME_Y_POS, 28, 40, 4);
	  mlcd_draw_digit(alarm_hour%10, 36, TIME_Y_POS, 28, 40, 4);
}

static void scr_alarm_draw_minutes() {
	  mlcd_draw_digit(alarm_minute/10, 80, TIME_Y_POS, 28, 40, 4);
	  mlcd_draw_digit(alarm_minute%10, 112, TIME_Y_POS, 28, 40, 4);
}

static void scr_alarm_draw_options() {
		mlcd_clear_rect(90, 10, 46, 21);
		if ((alarm_options & 0x80) == 0) {
				circle(100, 20, 7);
				mlcd_draw_rect(109, 18, 16, 5);
		} else {
				fillCircle(125, 20, 7);
				mlcd_draw_rect(100, 18, 17, 5);
		}
}

static void scr_changetime_draw_all() {
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_ALARM_CLOCK), 5, 10, 80, NULL, FONT_OPTION_BIG, HORIZONTAL_ALIGN_LEFT);
//	  mlcd_draw_rect(0, 25, MLCD_XRES, 2);
	
	  mlcd_draw_rect(69, TIME_Y_POS + 24, 5, 5);
	  mlcd_draw_rect(69, TIME_Y_POS + 10, 5, 5);
	
	  if (change_mode == MODE_HOUR) {
//			  mlcd_draw_rect_border(1, TIME_Y_POS - 3, 66, 44, 1);
			  mlcd_draw_arrow_up(24, TIME_Y_POS + 45, 20, 10, 4);
			  mlcd_draw_arrow_down(24, TIME_Y_POS - 15, 20, 10, 4);
		} else if (change_mode == MODE_MINUTES) {
//			  mlcd_draw_rect_border(77, TIME_Y_POS - 3, 66, 44, 1);
			  mlcd_draw_arrow_up(100, TIME_Y_POS + 45, 20, 10, 4);
			  mlcd_draw_arrow_down(100, TIME_Y_POS - 15, 20, 10, 4);
		}
		
	  scr_alarm_draw_hour();
	  scr_alarm_draw_minutes();
	  scr_alarm_draw_options();
}

static void scr_changetime_handle_button_up(void) {
	  if (change_mode == MODE_ACTIVATE) {
				alarm_options ^= 0x80;
				scr_alarm_draw_options();
		} else if (change_mode == MODE_HOUR) {
			  if (++alarm_hour > 23) {
					  alarm_hour = 0;
				}
				scr_alarm_draw_hour();
		}	else if (change_mode == MODE_MINUTES) {
			  if (++alarm_minute > 59) {
					  alarm_minute = 0;
				}
				scr_alarm_draw_minutes();
		}
	  mlcd_fb_flush();
}

static void scr_changetime_handle_button_down(void) {
	  if (change_mode == MODE_ACTIVATE) {
				alarm_options ^= 0x80;
				scr_alarm_draw_options();
		} else if (change_mode == MODE_HOUR) {
			  if(--alarm_hour < 0) {
					  alarm_hour = 23;
				}
				scr_alarm_draw_hour();
		}	else if (change_mode == MODE_MINUTES) {
			  if(--alarm_minute < 0) {
					  alarm_minute = 59;
				}
				scr_alarm_draw_minutes();
		}
	  mlcd_fb_flush();
}

static void scr_confirm(void) {
		store_alarm_clock();
		alarm_clock_reschedule();
		scr_mngr_show_screen(SCR_SETTINGS);
}

static void scr_changetime_handle_button_select(void) {
	  if (change_mode == MODE_ACTIVATE) {
				change_mode = MODE_HOUR;
			  scr_changetime_draw_all();
	      mlcd_fb_flush();
	  } else if (change_mode == MODE_HOUR) {
			  change_mode = MODE_MINUTES;
				mlcd_fb_clear();
			  scr_changetime_draw_all();
	      mlcd_fb_flush();
		} else if (change_mode == MODE_MINUTES) {
				scr_confirm();
		}
}

static void scr_changetime_handle_button_back(void) {
	  if (change_mode == MODE_ACTIVATE) {
		    scr_mngr_show_screen(SCR_SETTINGS);
	  } else if (change_mode == MODE_HOUR) {
			  change_mode = MODE_ACTIVATE;
				mlcd_fb_clear();
			  scr_changetime_draw_all();
	      mlcd_fb_flush();
		} else if (change_mode == MODE_MINUTES) {
			  change_mode = MODE_HOUR;
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
								scr_confirm();
						else
								scr_changetime_handle_button_select();
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_changetime_handle_button_back();
				    return true;
		}
		return false;
}

static void scr_changetime_init() {
		load_alarm_clock();
	  change_mode = MODE_ACTIVATE;
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
				    return scr_changetime_handle_button_pressed(event_type, event_param);
		}
		return false;
}


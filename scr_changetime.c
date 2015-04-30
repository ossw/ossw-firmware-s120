#include "scr_changetime.h"
#include "scr_mngr.h"
#include "mlcd_draw.h"
#include "rtc.h"

static int8_t hour;
static int8_t minutes;

#define TIME_Y_POS     60
#define MODE_HOUR      0x01
#define MODE_MINUTES   0x02

static uint8_t change_mode;

static void scr_changetime_draw_hour() {
	  mlcd_draw_digit(hour/10, 112, TIME_Y_POS, 28, 38, 4);
	  mlcd_draw_digit(hour%10, 80, TIME_Y_POS, 28, 38, 4);
}

static void scr_changetime_draw_minutes() {
	  mlcd_draw_digit(minutes/10, 36, TIME_Y_POS, 28, 38, 4);
	  mlcd_draw_digit(minutes%10, 4, TIME_Y_POS, 28, 38, 4);
}

static void scr_changetime_draw_all() {
	  mlcd_fb_clear();
	  mlcd_draw_rect(69, TIME_Y_POS + 22, 5, 5);
	  mlcd_draw_rect(69, TIME_Y_POS + 10, 5, 5);
	
	  if (change_mode == MODE_HOUR) {
			  mlcd_draw_rect_border(77, TIME_Y_POS - 3, 66, 44, 1);
			  mlcd_draw_arrow_up(90, TIME_Y_POS + 48, 40, 20, 6);
			  mlcd_draw_arrow_down(90, TIME_Y_POS - 30, 40, 20, 6);
		} else if (change_mode == MODE_MINUTES) {
			  mlcd_draw_rect_border(1, TIME_Y_POS - 3, 66, 44, 1);
			  mlcd_draw_arrow_up(14, TIME_Y_POS + 48, 40, 20, 6);
			  mlcd_draw_arrow_down(14, TIME_Y_POS - 30, 40, 20, 6);
		}
		
	  scr_changetime_draw_hour();
	  scr_changetime_draw_minutes();
}

static void scr_changetime_handle_button_up(void) {
	  if(change_mode == MODE_HOUR) {
			  if(++hour > 23) {
					  hour = 0;
				}
				scr_changetime_draw_hour();
		}	  
		if(change_mode == MODE_MINUTES) {
			  if(++minutes > 59) {
					  minutes = 0;
				}
				scr_changetime_draw_minutes();
		}
	  mlcd_fb_flush();
}

static void scr_changetime_handle_button_down(void) {
		if(change_mode == MODE_HOUR) {
			  if(--hour < 0) {
					  hour = 23;
				}
				scr_changetime_draw_hour();
		}	  
		if(change_mode == MODE_MINUTES) {
			  if(--minutes < 0) {
					  minutes = 59;
				}
				scr_changetime_draw_minutes();
		}
	  mlcd_fb_flush();
}

static void scr_changetime_handle_button_select(void) {
	  if (change_mode == MODE_HOUR) {
			  change_mode = MODE_MINUTES;
			  scr_changetime_draw_all();
	      mlcd_fb_flush();
		} else if (change_mode == MODE_MINUTES) {
			  uint32_t time = (rtc_current_time()/86400 ) * 86400 + hour * 3600 + minutes * 60;
			  rtc_set_current_time(time);
			  scr_mngr_show_screen(SCR_WATCHFACE);
		}
}


static void scr_changetime_handle_button_back(void) {
	  
	  if (change_mode == MODE_HOUR) {
		    scr_mngr_show_screen(SCR_WATCHFACE);
		} else if (change_mode == MODE_MINUTES) {
			  change_mode = MODE_HOUR;
			  scr_changetime_draw_all();
	      mlcd_fb_flush();
		}
}

static void scr_changetime_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  scr_changetime_handle_button_up();
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  scr_changetime_handle_button_down();
				    break;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  scr_changetime_handle_button_select();
				    break;
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_changetime_handle_button_back();
				    break;
		}
}

static void scr_changetime_init() {
	  uint32_t current_time = rtc_current_time();
	  hour = (current_time / 3600) % 24;
	  minutes = (current_time / 60) % 60;
	
	  change_mode = MODE_HOUR;
	  scr_changetime_draw_all();
	  mlcd_fb_flush();
}

void scr_changetime_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_changetime_init();
				    break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_changetime_handle_button_pressed(event_param);
				    break;
		}
}

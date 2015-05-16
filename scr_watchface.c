#include <string.h>
#include "scr_watchface.h"
#include "scr_mngr.h"
#include "mlcd_draw.h"
#include "rtc.h"
#include "nrf_delay.h"
#include "mlcd.h"
#include "pawn/amxutil.h"

//static uint32_t last_time;

static bool backlight_on = false;

static void scr_watchface_redraw_time(uint32_t current_time) {
	  uint32_t hour = (current_time / 3600) % 24;
	  uint32_t minutes = (current_time / 60) % 60;
	  uint32_t seconds = current_time % 60;
	
	//  last_time = current_time;
	
	  mlcd_draw_digit(hour/10, 75, 87, 64, 76, 8);
	  mlcd_draw_digit(hour%10, 5, 87, 64, 76, 8);
	  mlcd_draw_digit(minutes/10, 75, 5, 64, 76, 6);
	  mlcd_draw_digit(minutes%10, 5, 5, 64, 76, 6);
	  mlcd_draw_simple_progress(seconds, 60, 0, 0, MLCD_XRES, 2);
	  mlcd_fb_flush();
}

extern bool runTestScript;

static void scr_watchface_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
				 		scr_mngr_show_screen(SCR_CHANGE_TIME);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				 		scr_mngr_show_screen(SCR_CHANGE_DATE);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_UP:
						runTestScript = true;
				    break;
		}
}

static void scr_watchface_handle_button_long_pressed(uint32_t button_id) {
		switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
				    backlight_on = !backlight_on;
				    if (backlight_on) {
							  mlcd_backlight_on();
						} else {
							  mlcd_backlight_off();
						}
				    break;
		}
}

static void scr_watchface_init() {
	  mlcd_fb_clear();
	  
//	  last_time = 0;
	  scr_watchface_redraw_time(rtc_current_time());
}

void scr_watchface_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_watchface_init();
				    break;
			  case SCR_EVENT_RTC_TIME_CHANGED:
				    scr_watchface_redraw_time(event_param);
				    break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_watchface_handle_button_pressed(event_param);
				    break;
			  case SCR_EVENT_BUTTON_LONG_PRESSED:
				    scr_watchface_handle_button_long_pressed(event_param);
				    break;
		}
}

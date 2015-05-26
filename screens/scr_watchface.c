#include <string.h>
#include "scr_watchface.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "nrf_delay.h"

static uint32_t lastTime = 0;

static uint8_t lastMinutes = 0xFF;
static uint8_t lastHour = 0xFF;
static uint8_t lastSeconds = 0xFF;


static void scr_watchface_draw_time(uint32_t current_time) {
    uint32_t hour = (current_time / 3600) % 24;
    uint32_t minutes = (current_time / 60) % 60;
    uint32_t seconds = current_time % 60;
  
	  if(lastHour == 0xFF || hour/10 != lastHour/10) {
				mlcd_draw_digit(hour/10, 5, 5, 64, 76, 8);
		}
		if(lastHour == 0xFF || hour%10 != lastHour%10) {
				mlcd_draw_digit(hour%10, 75, 5, 64, 76, 8);
		}
	  if(lastMinutes == 0xFF || minutes/10 != lastMinutes/10) {
				mlcd_draw_digit(minutes/10, 5, 87, 64, 76, 6);
    }
	  if(lastMinutes == 0xFF || minutes%10 != lastMinutes%10) {
			  mlcd_draw_digit(minutes%10, 75, 87, 64, 76, 6);
		}
	  if(lastSeconds == 0xFF || seconds != lastSeconds) {
			  mlcd_draw_simple_progress(seconds, 60, 0, MLCD_YRES - 3, MLCD_XRES, 2);
		}
    mlcd_fb_flush();
		lastTime = current_time;
		lastHour = hour;
		lastMinutes = minutes;
		lastSeconds = seconds;
}

static void scr_watchface_refresh_time() {
	  uint32_t currentTime = rtc_current_time();
    if( lastTime == currentTime) {
			  return;
		}
		scr_watchface_draw_time(currentTime);
}

static void scr_watchface_handle_button_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_SELECT:
            scr_mngr_show_screen(SCR_CHANGE_TIME);
            break;
        case SCR_EVENT_PARAM_BUTTON_DOWN:
            scr_mngr_show_screen(SCR_CHANGE_DATE);
            break;
        case SCR_EVENT_PARAM_BUTTON_UP:
            scr_mngr_show_screen(SCR_TEST);
            break;
        case SCR_EVENT_PARAM_BUTTON_BACK:
            NVIC_SystemReset();
            break;
    }
}

static void scr_watchface_handle_button_long_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_SELECT:
            scr_mngr_show_screen(SCR_SETTINGS);
            break;
    }
}

static void scr_watchface_init() {
    mlcd_fb_clear();
	
    lastMinutes = 0xFF;
    lastHour = 0xFF;
    lastSeconds = 0xFF;
    scr_watchface_draw_time(rtc_current_time());
}

void scr_watchface_handle_event(uint32_t event_type, uint32_t event_param) {
    switch(event_type) {
        case SCR_EVENT_INIT_SCREEN:
            scr_watchface_init();
            break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_watchface_refresh_time();
            break;
   //     case SCR_EVENT_RTC_TIME_CHANGED:
   //         scr_watchface_redraw_time(event_param);
   //         break;
        case SCR_EVENT_BUTTON_PRESSED:
            scr_watchface_handle_button_pressed(event_param);
            break;
        case SCR_EVENT_BUTTON_LONG_PRESSED:
            scr_watchface_handle_button_long_pressed(event_param);
            break;
    }
}

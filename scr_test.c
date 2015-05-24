#include <string.h>
#include "scr_test.h"
#include "scr_mngr.h"
#include "mlcd_draw.h"
#include "rtc.h"
#include "nrf_delay.h"
#include "mlcd.h"
#include "utf8.h"
#include "pawn/amxutil.h"

#define DIGITS_Y_POS 80

static uint32_t lastValue = 0;
//static uint32_t lastValue2 = 0;

static void scr_test_handle_button_back(void) {
	  scr_mngr_show_screen(SCR_WATCHFACE);
}

static void scr_test_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_test_handle_button_back();
				    break;
		}
}

extern uint8_t testValue;

static void scr_test_refresh_value1() {
	  if(testValue == lastValue) {
			  return;
		}
		if(testValue < 100) {
			  mlcd_clear_rect(5, DIGITS_Y_POS, 40, 58);
		} else {
				mlcd_draw_digit(testValue/100, 5, DIGITS_Y_POS, 40, 58, 5);
		}
    mlcd_draw_digit((testValue%100)/10, 49, DIGITS_Y_POS, 40, 58, 5);
    mlcd_draw_digit(testValue%10, 93, DIGITS_Y_POS, 40, 58, 5);
	
    lastValue = testValue;
}

static void scr_watchface_refresh_screen() {
	  scr_test_refresh_value1();
	  mlcd_fb_flush();
}

static void scr_test_init() {
	  mlcd_fb_clear();
	                 
	  mlcd_draw_text("Heart rate", 5, 13, FONT_SELECT_REGULAR);
	  mlcd_draw_rect(0, 50, MLCD_XRES, 2);
//	  mlcd_draw_text("123 reg test {}%!", 15, 40, FONT_SMALL_REGULAR);
//	  mlcd_draw_text("123 bold test {}%!", 5, 70, FONT_SMALL_BOLD);
	
    scr_watchface_refresh_screen();
}

void scr_test_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_test_init();
				    break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_watchface_refresh_screen();
            break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_test_handle_button_pressed(event_param);
				    break;
		}
}

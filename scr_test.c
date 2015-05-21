#include <string.h>
#include "scr_test.h"
#include "scr_mngr.h"
#include "mlcd_draw.h"
#include "rtc.h"
#include "nrf_delay.h"
#include "mlcd.h"
#include "utf8.h"
#include "pawn/amxutil.h"

static uint32_t lastValue = 0;

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

static void scr_test_refresh() {
    mlcd_draw_digit(testValue/10, 5, 87, 64, 76, 6);
    mlcd_draw_digit(testValue%10, 75, 87, 64, 76, 6);
	  mlcd_fb_flush();
	
    lastValue = testValue;
}

static void scr_test_init() {
	  mlcd_fb_clear();
	  
	  mlcd_draw_text("123 reg test {}%!", 15, 40, FONT_SMALL_REGULAR);
	  mlcd_draw_text("123 bold test {}%!", 5, 70, FONT_SMALL_BOLD);
	
    scr_test_refresh();
}

static void scr_watchface_refresh_screen() {
	  if(testValue == lastValue) {
			  return;
		}
	  scr_test_refresh();
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

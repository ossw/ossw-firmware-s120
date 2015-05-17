#include <string.h>
#include "scr_test.h"
#include "scr_mngr.h"
#include "mlcd_draw.h"
#include "rtc.h"
#include "nrf_delay.h"
#include "mlcd.h"
#include "utf8.h"
#include "pawn/amxutil.h"

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

static void scr_test_init() {
	  mlcd_fb_clear();
	  
	  mlcd_draw_text("123 reg test {}%!", 15, 40, FONT_SMALL_REGULAR);
	  mlcd_draw_text("123 bold test {}%!", 5, 70, FONT_SMALL_BOLD);
	
	  mlcd_fb_flush();
}

void scr_test_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_test_init();
				    break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_test_handle_button_pressed(event_param);
				    break;
		}
}

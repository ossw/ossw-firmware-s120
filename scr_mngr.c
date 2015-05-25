#include "scr_mngr.h"
#include "screens/scr_choosemode.h"
#include "screens/scr_watchface.h"
#include "screens/scr_changetime.h"
#include "screens/scr_changedate.h"
#include "screens/scr_test.h"
#include "mlcd.h"

static uint32_t current_screen;

bool initialized = false;

void scr_mngr_init(void) {
	  initialized = true;
	  scr_mngr_show_screen(SCR_CHOOSE_MODE);
}

static void scr_mngr_default_handle_button_long_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_BACK:
            mlcd_backlight_toggle();
            break;
    }
}

void scr_mngr_default_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_RTC_TIME_CHANGED:
				    mlcd_switch_vcom();
				    break;
        case SCR_EVENT_BUTTON_LONG_PRESSED:
            scr_mngr_default_handle_button_long_pressed(event_param);
            break;
		}
}

void scr_mngr_handle_event(uint32_t event_type, uint32_t event_param) {
	  if(!initialized) {
			  return;
		}
		scr_mngr_default_handle_event(event_type, event_param);
	  switch(current_screen) {
			  case SCR_CHOOSE_MODE:
				    scr_choosemode_handle_event(event_type, event_param);
				    break;
			  case SCR_WATCHFACE:
				    scr_watchface_handle_event(event_type, event_param);
				    break;
			  case SCR_CHANGE_DATE:
				    scr_changedate_handle_event(event_type, event_param);
				    break;
			  case SCR_CHANGE_TIME:
				    scr_changetime_handle_event(event_type, event_param);
				    break;
			  case SCR_TEST:
				    scr_test_handle_event(event_type, event_param);
				    break;
		}
}

void scr_mngr_show_screen(uint32_t screen_id) {
	  current_screen = screen_id;
	  scr_mngr_handle_event(SCR_EVENT_INIT_SCREEN, 0);
}

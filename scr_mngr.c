#include "scr_mngr.h"
#include "scr_watchface.h"
#include "scr_changetime.h"
#include "scr_changedate.h"
#include "mlcd.h"

static uint32_t current_screen;

bool initialized = false;

void scr_mngr_init(void) {
	  initialized = true;
	  scr_mngr_show_screen(SCR_WATCHFACE);
}

void scr_mngr_default_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_RTC_TIME_CHANGED:
				    mlcd_switch_vcom();
				    break;
		}
}

void scr_mngr_handle_event(uint32_t event_type, uint32_t event_param) {
	  if(!initialized) {
			  return;
		}
		scr_mngr_default_handle_event(event_type, event_param);
	  switch(current_screen) {
			  case SCR_WATCHFACE:
				    scr_watchface_handle_event(event_type, event_param);
				    break;
			  case SCR_CHANGE_DATE:
				    scr_changedate_handle_event(event_type, event_param);
				    break;
			  case SCR_CHANGE_TIME:
				    scr_changetime_handle_event(event_type, event_param);
				    break;
		}
}

void scr_mngr_show_screen(uint32_t screen_id) {
	  current_screen = screen_id;
	  scr_mngr_handle_event(SCR_EVENT_INIT_SCREEN, 0);
}

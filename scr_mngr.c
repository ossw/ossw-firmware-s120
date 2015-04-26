#include "scr_mngr.h"
#include "scr_watchface.h"
#include "scr_changetime.h"
#include "scr_changedate.h"

static uint32_t current_screen;

bool initialized = false;

void scr_mngr_init(void) {
	  scr_mngr_show_screen(SCR_WATCHFACE);
	  initialized = true;
}

void scr_mngr_handle_event(uint32_t event_type, uint32_t event_param) {
	  if(!initialized) {
			  return;
		}
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

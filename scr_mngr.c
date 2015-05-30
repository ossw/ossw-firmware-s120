#include "scr_mngr.h"
#include "scr_controls.h"
#include "rtc.h"
#include "screens/scr_choosemode.h"
#include "screens/scr_watchface.h"
#include "screens/scr_changetime.h"
#include "screens/scr_changedate.h"
#include "screens/scr_settings.h"
#include "screens/scr_test.h"
#include "mlcd.h"

bool initScreen = false;

static uint32_t current_screen;

static bool initialized = false;
		
static CONTROL_DATA hour_ctrl_data;
		
static const SCR_CONTROL_NUMBER_CONFIG hour_config = {
		NUMBER_FORMAT_0_99 | NUMBER_FORMAT_FLAG_ZERO_PADDED,
	  1,
	  1,
	  20,
	  15,
	  2,
	  rtc_get_current_hour,
    &hour_ctrl_data
};

static CONTROL_DATA minutes_ctrl_data;

static const SCR_CONTROL_NUMBER_CONFIG minutes_config = {
		NUMBER_FORMAT_0_99 | NUMBER_FORMAT_FLAG_ZERO_PADDED,
	  25,
	  1,
	  20,
	  15,
	  2,
	  rtc_get_current_minutes,
    &minutes_ctrl_data
};

static const SCR_CONTROL_STATIC_RECT_CONFIG vert_separator_config = {
	  0,
	  17,
	  MLCD_XRES,
	  1
};

static const SCR_CONTROL_DEFINITION notification_bar_controls[] = {
	  {SCR_CONTROL_NUMBER, (void*)&hour_config},
		{SCR_CONTROL_NUMBER, (void*)&minutes_config},
		{SCR_CONTROL_STATIC_RECT, (void*)&vert_separator_config}
};

static const SCR_CONTROLS_DEFINITION notification_bar_controls_definition = {
	  sizeof(notification_bar_controls)/sizeof(SCR_CONTROL_DEFINITION),
	  notification_bar_controls
};

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
			  case SCR_SETTINGS:
				    scr_settings_handle_event(event_type, event_param);
				    break;
			  case SCR_TEST:
				    scr_test_handle_event(event_type, event_param);
				    break;
		}
}

void scr_mngr_show_screen(uint32_t screen_id) {
	  current_screen = screen_id;
	  initScreen = true;
}

void scr_mngr_draw_notification_bar() {
	  scr_controls_draw(&notification_bar_controls_definition);
}

void scr_mngr_redraw_notification_bar() {
	  scr_controls_redraw(&notification_bar_controls_definition);
}

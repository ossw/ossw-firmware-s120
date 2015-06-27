#include "scr_mngr.h"
#include "scr_controls.h"
#include "rtc.h"
#include "battery.h"
#include "screens/scr_choosemode.h"
#include "screens/scr_watchface.h"
#include "screens/scr_changetime.h"
#include "screens/scr_changedate.h"
#include "screens/scr_settings.h"
#include "screens/scr_watchset.h"
#include "mlcd.h"

static uint16_t switch_to_screen = SCR_NOT_SET;

static uint16_t current_screen = SCR_NOT_SET;
		
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
	  26,
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

static const SCR_CONTROL_STATIC_RECT_CONFIG hour_sep_dot_1_config = {
	  22,
	  4,
	  2,
	  2
};

static const SCR_CONTROL_STATIC_RECT_CONFIG hour_sep_dot_2_config = {
	  22,
	  11,
	  2,
	  2
};

static CONTROL_DATA battery_level_ctrl_data;

static const SCR_CONTROL_PROGRESS_BAR_CONFIG battery_level_config = {
	  119,
	  3,
	  23,
	  11,
	  100,
		1,
	  battery_get_level,
    &battery_level_ctrl_data
};

static const SCR_CONTROL_DEFINITION notification_bar_controls[] = {
	  {SCR_CONTROL_NUMBER, (void*)&hour_config},
		{SCR_CONTROL_NUMBER, (void*)&minutes_config},
		{SCR_CONTROL_HORIZONTAL_PROGRESS_BAR, (void*)&battery_level_config},
		{SCR_CONTROL_STATIC_RECT, (void*)&vert_separator_config},
		{SCR_CONTROL_STATIC_RECT, (void*)&hour_sep_dot_1_config},
		{SCR_CONTROL_STATIC_RECT, (void*)&hour_sep_dot_2_config}
};

static const SCR_CONTROLS_DEFINITION notification_bar_controls_definition = {
	  sizeof(notification_bar_controls)/sizeof(SCR_CONTROL_DEFINITION),
	  (SCR_CONTROL_DEFINITION*)notification_bar_controls
};

void scr_mngr_init(void) {
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

void static scr_mngr_handle_event_internal(uint16_t screen_id, uint32_t event_type, uint32_t event_param) {
		bool allowDefaultHandler = true;
		
	  switch (screen_id) {
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
			  case SCR_WATCH_SET:
					  allowDefaultHandler = false;
				    scr_watch_set_handle_event(event_type, event_param);
				    break;
				case SCR_NOT_SET:
					  return;
		}
		if (allowDefaultHandler) {
			  scr_mngr_default_handle_event(event_type, event_param);
		}
}

void scr_mngr_handle_event(uint32_t event_type, uint32_t event_param) {
	  scr_mngr_handle_event_internal(current_screen, event_type, event_param);
}
	
void scr_mngr_show_screen(uint16_t screen_id) {
	  switch_to_screen = screen_id;
}

void scr_mngr_draw_notification_bar() {
	  scr_controls_draw(&notification_bar_controls_definition);
}

void scr_mngr_redraw_notification_bar() {
	  scr_controls_redraw(&notification_bar_controls_definition);
}

void scr_mngr_draw_screen(void) {
		if (switch_to_screen != SCR_NOT_SET) {
				uint16_t old_screen = current_screen;
			  // disable events
			  current_screen = SCR_NOT_SET;
			  // release memory used by old screen
				scr_mngr_handle_event_internal(old_screen, SCR_EVENT_DESTROY_SCREEN, 0);
			  // initilize new screen
				scr_mngr_handle_event_internal(switch_to_screen, SCR_EVENT_INIT_SCREEN, NULL);
			  // set new screen
			  current_screen = switch_to_screen;
			  switch_to_screen = SCR_NOT_SET;
		} else {
				scr_mngr_handle_event(SCR_EVENT_REFRESH_SCREEN, NULL);
		}
}

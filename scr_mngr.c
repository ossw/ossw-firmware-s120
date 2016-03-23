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
#include "screens/scr_watchset_list.h"
#include "screens/scr_notifications.h"
#include "screens/scr_alert_notification.h"
#include "screens/scr_about.h"
#include "screens/scr_status.h"
#include "screens/scr_watchface_analog.h"
#include "screens/scr_setalarm.h"
#include "screens/dialog_option_text.h"
#include "screens/dialog_select.h"
#include "mlcd.h"
#include "stopwatch.h"
#include "config.h"
#include "watchset.h"

static NUMBER_CONTROL_DATA hour_ctrl_data;
static uint32_t switch_to_screen_param = 0;
static uint32_t scr_alert_notification_address = 0;
static uint8_t switch_to_screen = SCR_NOT_SET;
static uint8_t current_screen = SCR_NOT_SET;
static uint8_t previous_screen = SCR_NOT_SET;

static uint8_t scr_notifications_state = SCR_NOTIFICATIONS_STATE_NONE;
static uint8_t scr_alert_notification_state = SCR_ALERT_NOTIFICATION_STATE_NONE;

static bool redraw = true;
static bool modal_dialog = false;

void set_modal_dialog(bool state) {
		modal_dialog = state;
		if (!state) {
				// temporal fix, need a queue to save all the history of screens
				switch_to_screen = previous_screen;
				scr_mngr_draw_ctx draw_ctx;
				draw_ctx.force_colors = false;
				scr_mngr_handle_event(SCR_EVENT_DRAW_SCREEN, (uint32_t)&draw_ctx);
		}
}
		
static const SCR_CONTROL_NUMBER_CONFIG hour_config = {
		NUMBER_RANGE_0__99,
	  2,
	  2,
	  NUMBER_FORMAT_FLAG_ZERO_PADDED | 2 << 24 | 2 << 16 | 8 << 8 | 13,
	  (uint32_t (*)(uint32_t, uint8_t, uint8_t*, bool*))rtc_get_current_hour_24,
	  0,
    &hour_ctrl_data
};

static NUMBER_CONTROL_DATA minutes_ctrl_data;

static const SCR_CONTROL_NUMBER_CONFIG minutes_config = {
		NUMBER_RANGE_0__99,
	  26,
	  2,
	  NUMBER_FORMAT_FLAG_ZERO_PADDED | 2 << 24 | 2 << 16 | 8 << 8 | 13,
	  (uint32_t (*)(uint32_t, uint8_t, uint8_t*, bool*))rtc_get_current_minutes,
	  0,
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

static NUMBER_CONTROL_DATA battery_level_ctrl_data;

static const SCR_CONTROL_PROGRESS_BAR_CONFIG battery_level_config = {
	  119,
	  3,
	  23,
	  11,
	  100,
		1<<16,
	  (uint32_t (*)(uint32_t, uint8_t, uint8_t*, bool*))battery_get_level,
	  0,
    &battery_level_ctrl_data
};

static const SCR_CONTROL_DEFINITION notification_bar_controls[] = {
	  {SCR_CONTROL_NUMBER, (void*)&hour_config},
		{SCR_CONTROL_NUMBER, (void*)&minutes_config},
		{SCR_CONTROL_PROGRESS_BAR, (void*)&battery_level_config},
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

bool scr_mngr_default_handle_event(uint32_t event_type, uint32_t event_param) {
		if (SCR_EVENT_RTC_TIME_CHANGED == event_type) {
				mlcd_switch_vcom();
				return true;
		}
		
		int idx = config_get_handler_index_from_event(event_type, event_param);
		if (idx < 0) {
				return false;
		}
		default_action action = config_get_default_global_actions()[idx];
		if (action.action_id != 0) {
				watchset_invoke_internal_function(action.action_id, action.parameter);
				return true;
		}
		return false;
}

void static scr_mngr_handle_event_internal(uint16_t screen_id, uint32_t event_type, uint32_t event_param) {
		bool handled = false;
	  switch (screen_id) {
			  case SCR_CHOOSE_MODE:
				    handled = scr_choosemode_handle_event(event_type, event_param);
				    break;
			  case SCR_WATCHFACE:
				    handled = scr_watchface_handle_event(event_type, event_param);
				    break;
			  case SCR_CHANGE_DATE:
				    handled = scr_changedate_handle_event(event_type, event_param);
				    break;
			  case SCR_CHANGE_TIME:
				    handled = scr_changetime_handle_event(event_type, event_param);
				    break;
			  case SCR_SETTINGS:
				    handled = scr_settings_handle_event(event_type, event_param);
				    break;
			  case SCR_STATUS:
				    handled = scr_status_handle_event(event_type, event_param);
				    break;
			  case SCR_ABOUT:
				    handled = scr_about_handle_event(event_type, event_param);
				    break;
			  case SCR_ALERT_NOTIFICATION:
				    handled = scr_alert_notification_handle_event(event_type, event_param);
				    break;
			  case SCR_NOTIFICATIONS:
				    handled = scr_notifications_handle_event(event_type, event_param);
				    break;
			  case SCR_WATCH_SET:
				    handled = scr_watch_set_handle_event(event_type, event_param);
				    break;
			  case SCR_WATCH_SET_LIST:
				    handled = scr_watchset_list_handle_event(event_type, event_param);
				    break;
			  case SCR_WATCHFACE_ANALOG:
				    handled = scr_watchface_analog_handle_event(event_type, event_param);
				    break;
			  case SCR_SET_ALARM:
				    handled = scr_set_alarm_handle_event(event_type, event_param);
				    break;
			  case SCR_DIALOG_OPTION:
				    handled = dialog_option_text_handle_event(event_type, event_param);
				    break;
			  case SCR_DIALOG_SELECT:
				    handled = dialog_select_handle_event(event_type, event_param);
				    break;
				case SCR_NOT_SET:
					  return;
		}
		
		if (!handled) {
				scr_mngr_default_handle_event(event_type, event_param);
		}
		
		if (event_type >= 0x10) {
				if (event_type == SCR_EVENT_RTC_TIME_CHANGED && stopwatch_get_state() == STOPWATCH_STATE_RUNNING) {
						// skip that refresh because stapwatch is refreshing the screen
						return;
				}
					
				scr_mngr_redraw();
		}
}

void scr_mngr_handle_event(uint32_t event_type, uint32_t event_param) {
		if (event_type == SCR_EVENT_BUTTON_PRESSED || event_type == SCR_EVENT_BUTTON_LONG_PRESSED) {
				mlcd_backlight_extend();
		}
		if (!modal_dialog) {
	  if (scr_alert_notification_state != SCR_ALERT_NOTIFICATION_STATE_NONE) {
		    if (scr_alert_notification_state == SCR_ALERT_NOTIFICATION_STATE_SHOW) {
				    scr_mngr_handle_event_internal(SCR_ALERT_NOTIFICATION, event_type, event_param);
		    }
				return;
	  }
	  if (scr_notifications_state != SCR_NOTIFICATIONS_STATE_NONE) {
		    if (scr_notifications_state == SCR_NOTIFICATIONS_STATE_SHOW) {
				   scr_mngr_handle_event_internal(SCR_NOTIFICATIONS, event_type, event_param);
		    }
				return;
	  }
		}
	  scr_mngr_handle_event_internal(current_screen, event_type, event_param);
}
	
void scr_mngr_show_screen_with_param(uint16_t screen_id, uint32_t param) {
	  switch_to_screen = screen_id;
	  switch_to_screen_param = param;
}

void scr_mngr_show_screen(uint16_t screen_id) {
		if (screen_id == current_screen)
				return;
	  switch_to_screen = screen_id;
	  switch_to_screen_param = NULL;
}

void scr_mngr_draw_notification_bar() {
	  scr_controls_draw(&notification_bar_controls_definition);
}

void scr_mngr_redraw_notification_bar() {
	  scr_controls_redraw(&notification_bar_controls_definition);
}

void scr_mngr_draw_screen(void) {
		scr_mngr_draw_ctx draw_ctx;
		draw_ctx.force_colors = false;
	
	  if (!modal_dialog && scr_alert_notification_state != SCR_ALERT_NOTIFICATION_STATE_NONE) {
				if (scr_alert_notification_state == SCR_ALERT_NOTIFICATION_STATE_INIT) {
						scr_mngr_handle_event_internal(SCR_ALERT_NOTIFICATION, SCR_EVENT_INIT_SCREEN, scr_alert_notification_address);
					  // draw alert notification screen
						mlcd_fb_clear();
						scr_mngr_handle_event_internal(SCR_ALERT_NOTIFICATION, SCR_EVENT_DRAW_SCREEN, (uint32_t)&draw_ctx);
						scr_alert_notification_state = SCR_ALERT_NOTIFICATION_STATE_SHOW;
				} else if (scr_alert_notification_state == SCR_ALERT_NOTIFICATION_STATE_SHOW) {
						scr_mngr_handle_event_internal(SCR_ALERT_NOTIFICATION, SCR_EVENT_REFRESH_SCREEN, (uint32_t)&draw_ctx);
				} else if (scr_alert_notification_state == SCR_ALERT_NOTIFICATION_STATE_CLOSE) {
						scr_mngr_handle_event_internal(SCR_ALERT_NOTIFICATION, SCR_EVENT_DESTROY_SCREEN, NULL);
						scr_alert_notification_state = SCR_ALERT_NOTIFICATION_STATE_NONE;
					  // draw sceen
						mlcd_fb_clear();
						scr_mngr_handle_event(SCR_EVENT_DRAW_SCREEN, (uint32_t)&draw_ctx);
				}
		} else if (!modal_dialog && scr_notifications_state != SCR_NOTIFICATIONS_STATE_NONE) {
				if (scr_notifications_state == SCR_NOTIFICATIONS_STATE_INIT) {
						scr_mngr_handle_event_internal(SCR_NOTIFICATIONS, SCR_EVENT_INIT_SCREEN, NULL);
					  // draw alert notification screen
						mlcd_fb_clear();
						scr_mngr_handle_event_internal(SCR_NOTIFICATIONS, SCR_EVENT_DRAW_SCREEN, (uint32_t)&draw_ctx);
						scr_notifications_state = SCR_NOTIFICATIONS_STATE_SHOW;
				} else if (scr_notifications_state == SCR_NOTIFICATIONS_STATE_SHOW) {
						scr_mngr_handle_event_internal(SCR_NOTIFICATIONS, SCR_EVENT_REFRESH_SCREEN, (uint32_t)&draw_ctx);
				} else if (scr_notifications_state == SCR_NOTIFICATIONS_STATE_REDRAW) {
						mlcd_fb_clear();
						scr_mngr_handle_event_internal(SCR_NOTIFICATIONS, SCR_EVENT_DRAW_SCREEN, (uint32_t)&draw_ctx);
						scr_notifications_state = SCR_NOTIFICATIONS_STATE_SHOW;
				} else if (scr_notifications_state == SCR_NOTIFICATIONS_STATE_CLOSE) {
						scr_mngr_handle_event_internal(SCR_NOTIFICATIONS, SCR_EVENT_DESTROY_SCREEN, NULL);
						scr_notifications_state = SCR_NOTIFICATIONS_STATE_NONE;
					  // draw sceen
						mlcd_fb_clear();
						scr_mngr_handle_event(SCR_EVENT_DRAW_SCREEN, (uint32_t)&draw_ctx);
				}
		} else {
				if (switch_to_screen != SCR_NOT_SET) {
						previous_screen = current_screen;
						uint8_t old_screen = current_screen;
						uint16_t new_screen;
						// disable events
						current_screen = SCR_NOT_SET;
						do {
							// release memory used by old screen
							scr_mngr_handle_event_internal(old_screen, SCR_EVENT_DESTROY_SCREEN, NULL);
							new_screen = switch_to_screen;
							// initilize new screen
							scr_mngr_handle_event_internal(switch_to_screen, SCR_EVENT_INIT_SCREEN, switch_to_screen_param);
							old_screen = new_screen;
							// handle case when init redirects to other screen
						} while (new_screen != switch_to_screen);
						
						// draw screen
						mlcd_fb_clear();
					
						#ifdef OSSW_DEBUG
								uint32_t start_draw_ticks;
								app_timer_cnt_get(&start_draw_ticks);
						#endif
						scr_mngr_handle_event_internal(switch_to_screen, SCR_EVENT_DRAW_SCREEN, (uint32_t)&draw_ctx);
						#ifdef OSSW_DEBUG
								uint32_t end_draw_ticks;
								uint32_t total_diff;
								app_timer_cnt_get(&end_draw_ticks);
								app_timer_cnt_diff_compute(end_draw_ticks, start_draw_ticks, &total_diff);
								printf("DRAW: 0x%08x\r\n", total_diff);
						#endif
						// set new screen
						current_screen = switch_to_screen;
						switch_to_screen = SCR_NOT_SET;
				} else {
						if (redraw == true) {
							
								#ifdef OSSW_DEBUG
										uint32_t start_draw_ticks;
										app_timer_cnt_get(&start_draw_ticks);
								#endif
								scr_mngr_handle_event(SCR_EVENT_REFRESH_SCREEN, (uint32_t)&draw_ctx);
								#ifdef OSSW_DEBUG
										uint32_t end_draw_ticks;
										uint32_t total_diff;
										app_timer_cnt_get(&end_draw_ticks);
										app_timer_cnt_diff_compute(end_draw_ticks, start_draw_ticks, &total_diff);
										printf("REDRAW: 0x%08x\r\n", total_diff);
								#endif
								redraw = false;
						}
				}
		}
    mlcd_fb_flush_with_param(draw_ctx.force_colors);
}

void scr_mngr_show_alert_notification(uint32_t address) {
		scr_alert_notification_address = address;
	  scr_alert_notification_state = SCR_ALERT_NOTIFICATION_STATE_INIT;
}

void scr_mngr_close_alert_notification() {
	  scr_alert_notification_state = SCR_ALERT_NOTIFICATION_STATE_CLOSE;
}

void scr_mngr_show_notifications() {
		if (scr_notifications_state != SCR_NOTIFICATIONS_STATE_SHOW) {
				scr_notifications_state = SCR_NOTIFICATIONS_STATE_INIT;
		} else {
				scr_notifications_state = SCR_NOTIFICATIONS_STATE_REDRAW;
		}
}

void scr_mngr_close_notifications() {
	  scr_notifications_state = SCR_NOTIFICATIONS_STATE_CLOSE;
}

uint8_t scr_mngr_current_screen(void) {
		return current_screen;
}

void scr_mngr_redraw(void) {
		redraw = true;
}

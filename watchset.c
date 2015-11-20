#include "watchset.h"
#include "fs.h"
#include "scr_mngr.h"
#include "mlcd.h"
#include "battery.h"
#include "rtc.h"
#include "stopwatch.h"
#include "ble/ble_central.h"

static spiffs_file default_watch_face_fd = -1;

static uint32_t (* const internal_data_source_handles[])(void) = {
		/* 0 */ battery_get_level,
		/* 1 */ rtc_get_current_hour_24,
		/* 2 */ rtc_get_current_hour_12,
		/* 3 */ rtc_get_current_hour_12_designator,
		/* 4 */ rtc_get_current_minutes,
		/* 5 */ rtc_get_current_seconds,
		/* 6 */ rtc_get_current_day_of_week,
		/* 7 */ rtc_get_current_day_of_month,
		/* 8 */ rtc_get_current_day_of_year,
		/* 9 */ rtc_get_current_month,
		/* 10 */rtc_get_current_year,
		/* 11 */stopwatch_get_total_time,
		/* 12 */stopwatch_get_current_lap_number,
		/* 13 */stopwatch_get_current_lap_time,
		/* 14 */stopwatch_get_current_lap_split,
		/* 15 */stopwatch_get_last_lap_time,
		/* 16 */stopwatch_get_recall_lap_number,
		/* 17 */stopwatch_get_recall_lap_time,
		/* 18 */stopwatch_get_recall_lap_split
};

static uint32_t (* const sensor_data_source_handles[])(void) = {
		/* 0 */ ble_central_heart_rate
};

static uint32_t watchset_converter_ms_to_hours(uint32_t v) {
		return v/3600000;
}

static uint32_t watchset_converter_ms_to_minutes_remainder(uint32_t v) {
		return (v%3600000)/60000;
}

static uint32_t watchset_converter_ms_to_seconds_remainder(uint32_t v) {
		return (v%60000)/1000;
}

static uint32_t watchset_converter_ms_to_cs_remainder(uint32_t v) {
		return (v%1000)/10;
}

static uint32_t watchset_converter_ms_remainder(uint32_t v) {
		return v%1000;
}

static uint32_t (* const data_converters[])(uint32_t) = {
		/* 0 */ NULL,
		/* 1 */ watchset_converter_ms_to_hours,
		/* 2 */ watchset_converter_ms_to_minutes_remainder,
		/* 3 */ watchset_converter_ms_to_seconds_remainder,
		/* 4 */ watchset_converter_ms_to_cs_remainder,
		/* 5 */ watchset_converter_ms_remainder
};

void* watchset_get_converter(uint8_t key) {
		if (key > sizeof(data_converters)/sizeof(void*)) {
				return NULL;
		}
		return data_converters[key];
}

void watchset_set_default_watch_face(struct spiffs_dirent* entry) {
		SPIFFS_close(&fs, default_watch_face_fd); 
		default_watch_face_fd = SPIFFS_open_by_dirent(&fs, entry, SPIFFS_RDONLY, 0);
}

spiffs_file watchset_get_dafault_watch_face_fd(void) {
		return default_watch_face_fd;
}

static void watchset_default_watch_face_handle_button_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_SELECT:
            scr_mngr_show_screen_with_param(SCR_WATCH_SET_LIST, WATCH_SET_TYPE_APPLICATION);
            break;
    }
}

static void watchset_default_watch_face_handle_button_long_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_BACK:
            mlcd_backlight_toggle();
            break;
        case SCR_EVENT_PARAM_BUTTON_SELECT:
            scr_mngr_show_screen(SCR_SETTINGS);
            break;
        case SCR_EVENT_PARAM_BUTTON_UP:
            scr_mngr_show_screen_with_param(SCR_WATCH_SET_LIST, WATCH_SET_TYPE_WATCH_FACE);
            break;
        case SCR_EVENT_PARAM_BUTTON_DOWN:
            scr_mngr_show_screen_with_param(SCR_WATCH_SET_LIST, WATCH_SET_TYPE_UTILITY);
            break;
    }
}

void watchset_default_watch_face_handle_event(uint32_t event_type, uint32_t event_param) {
		switch(event_type) {
        case SCR_EVENT_BUTTON_PRESSED:
            watchset_default_watch_face_handle_button_pressed(event_param);
            break;
        case SCR_EVENT_BUTTON_LONG_PRESSED:
            watchset_default_watch_face_handle_button_long_pressed(event_param);
            break;
		}
}

void watchset_invoke_internal_function(uint8_t function_id, uint16_t param) {
	  switch(function_id) {
			  case WATCH_SET_FUNC_TOGGLE_BACKLIGHT:
				    mlcd_backlight_toggle();
			      break;
			  case WATCH_SET_FUNC_TOGGLE_COLORS:
				    mlcd_colors_toggle();
			      break;
			  case WATCH_SET_FUNC_SHOW_SETTINGS:
				    scr_mngr_show_screen(SCR_SETTINGS);
			      break;
				case WATCH_SET_FUNC_CLOSE:
					  scr_mngr_show_screen(SCR_WATCHFACE);
					  break;
			  case WATCH_SET_FUNC_STOPWATCH_START:
						stopwatch_fn_start();
			      break;
			  case WATCH_SET_FUNC_STOPWATCH_RESET:
						stopwatch_fn_reset();
			      break;
			  case WATCH_SET_FUNC_STOPWATCH_STOP:
						stopwatch_fn_stop();
			      break;
			  case WATCH_SET_FUNC_STOPWATCH_RECALL_FIRST_LAP:
						stopwatch_fn_recall_first_lap();
						break;
			  case WATCH_SET_FUNC_STOPWATCH_RECALL_LAST_LAP:
						stopwatch_fn_recall_last_lap();
						break;
			  case WATCH_SET_FUNC_STOPWATCH_RECALL_PREV_LAP:
						stopwatch_fn_recall_prev_lap();
						break;
			  case WATCH_SET_FUNC_STOPWATCH_RECALL_NEXT_LAP:
						stopwatch_fn_recall_next_lap();
						break;
			  case WATCH_SET_FUNC_STOPWATCH_START_STOP:
						stopwatch_fn_start_stop();
			      break;
			  case WATCH_SET_FUNC_STOPWATCH_NEXT_LAP:
						stopwatch_fn_next_lap();
			      break;
		}
}

static uint32_t pow(uint32_t x, uint8_t n) {
	  uint32_t result = 1;
	  for(uint32_t i = 0; i < n; i++) {
			  result *= x;
		}
		return result;
}

uint32_t watchset_internal_data_source_get_value(uint32_t data_source_id, uint8_t expected_range) {
		uint32_t multiplier = pow(10, expected_range&0xF);
	  return multiplier * internal_data_source_handles[data_source_id]();
}

uint32_t watchset_sensor_data_source_get_value(uint32_t data_source_id, uint8_t expected_range) {
		uint32_t multiplier = pow(10, expected_range&0xF);
	  return multiplier * sensor_data_source_handles[data_source_id]();
}

uint32_t watchset_static_data_source_get_value(uint32_t data_source_id, uint8_t expected_range) {
		return data_source_id;
}

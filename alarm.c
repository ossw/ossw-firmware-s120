#include "alarm.h"
#include "vibration.h"
#include "ext_ram.h"
#include "i18n\i18n.h"
#include "rtc.h"
#include "mlcd_draw.h"
#include "scr_mngr.h"
#include "screens\dialog_option_text.h"

#define ALARM_VIBRATION		0x0060E738

static app_timer_id_t			alarm_clock_id;

void load_alarm_clock(uint8_t * alarm_options, int8_t * alarm_hour, int8_t * alarm_minute) {
	  uint8_t buffer[3];
		ext_ram_read_data(EXT_RAM_DATA_ALARM, buffer, 3);
		*alarm_options = buffer[0];
		*alarm_minute = buffer[1];
	  if (0 > *alarm_minute || *alarm_minute > 59) {
			  *alarm_minute = 0;
		}
		*alarm_hour = buffer[2];
		if (0 > *alarm_hour || *alarm_hour > 23) {
				*alarm_hour = 0;
		}
}

void store_alarm_clock(uint8_t alarm_options, int8_t alarm_hour, int8_t alarm_minute) {
		uint8_t buffer[3];
		buffer[0] = alarm_options;
		buffer[1] = alarm_minute;
		buffer[2] = alarm_hour;
		ext_ram_write_data(EXT_RAM_DATA_ALARM, buffer, 3);
}

void alarm_clock_reschedule(uint8_t alarm_options, int8_t alarm_hour, int8_t alarm_minute) {
		uint32_t err_code = app_timer_stop(alarm_clock_id);
    APP_ERROR_CHECK(err_code);
		if ((alarm_options & 0x80) == 0 || (alarm_options & 0x7F) == 0)
				return;
		uint8_t day = rtc_get_current_day_of_week()-1;
		uint8_t mask = 1<<day;
		int32_t interval = 3600 * alarm_hour + 60 * alarm_minute - rtc_get_current_time_in_seconds();
		while (interval <= 0 || (alarm_options & mask) == 0) {
				interval += DAY_IN_SECONDS;
				day = (day+1) % 7;
				mask = 1<<day;
		}
    err_code = app_timer_start(alarm_clock_id, APP_TIMER_TICKS(1000*interval, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
}

void alarm_snooze() {
		uint32_t err_code = app_timer_stop(alarm_clock_id);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(alarm_clock_id, APP_TIMER_TICKS(1000*90, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
}

bool alarm_button_handler(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
						vibration_stop();
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
						vibration_stop();
						// schedule next alarm
						uint8_t alarm_options;
						int8_t 	alarm_hour, alarm_minute;
						load_alarm_clock(&alarm_options, &alarm_hour, &alarm_minute);
						alarm_clock_reschedule(alarm_options, alarm_hour, alarm_minute);
				    return true;
		}
		return false;
}

static void alarm_clock_handler(void * p_context) {
		vibration_vibrate(ALARM_VIBRATION, 20000);
		pack_dialog_option(&alarm_button_handler, FONT_BIG_REGULAR, I18N_TRANSLATE(MESSAGE_ALARM_CLOCK),
				I18N_TRANSLATE(MESSAGE_ALARM_SNOOZE), I18N_TRANSLATE(MESSAGE_ALARM_DISMISS), "\0", "\0");
		scr_mngr_show_screen_with_param(SCR_DIALOG_OPTION, EXT_RAM_DATA_NOTIFICATION_UPLOAD_ADDRESS);
		alarm_snooze();
}

void alarm_clock_init(void) {
    uint32_t err_code = app_timer_create(&alarm_clock_id, APP_TIMER_MODE_SINGLE_SHOT,
                                alarm_clock_handler);
    APP_ERROR_CHECK(err_code);
		uint8_t alarm_options;
		int8_t 	alarm_hour, alarm_minute;
		load_alarm_clock(&alarm_options, &alarm_hour, &alarm_minute);
		alarm_clock_reschedule(alarm_options, alarm_hour, alarm_minute);
}

#include "alarm.h"
#include "vibration.h"
#include "ext_ram.h"
#include "rtc.h"
#include "mlcd_draw.h"
#include "scr_mngr.h"
#include "app_scheduler.h"
#include "i18n\i18n.h"
#include "screens\dialog_option_text.h"

#define ALARM_VIBRATION		0x0060E738
#define SNOOZE_MINUTES		2

static int8_t snooze_count = -1;

bool is_alarm_running() {
		return snooze_count > -1;
}

bool is_alarm_active() {
		uint8_t alarm_options;
		int8_t 	alarm_hour, alarm_minute;
		load_alarm_clock(&alarm_options, &alarm_hour, &alarm_minute);
		return (alarm_options & 0x80) != 0;
}

void alarm_toggle() {
		uint8_t alarm_options;
		int8_t 	alarm_hour, alarm_minute;
		load_alarm_clock(&alarm_options, &alarm_hour, &alarm_minute);
		alarm_options ^= 0x80;
		store_alarm_clock(alarm_options, alarm_hour, alarm_minute);
}

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

bool alarm_button_handler(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
						vibration_stop();
						snooze_count = SNOOZE_MINUTES;
						set_modal_dialog(false);
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
						vibration_stop();
						snooze_count = -1;
						set_modal_dialog(false);
				    return true;
		}
		return false;
}

void fire_alarm() {
		set_modal_dialog(true);
		vibration_vibrate(ALARM_VIBRATION, 40*1000, true);
		pack_dialog_option(&alarm_button_handler, FONT_OPTION_BIG, I18N_TRANSLATE(MESSAGE_ALARM_CLOCK),
				I18N_TRANSLATE(MESSAGE_ALARM_SNOOZE), I18N_TRANSLATE(MESSAGE_ALARM_DISMISS), "\0", "\0");
		scr_mngr_show_screen(SCR_DIALOG_OPTION);
		snooze_count = SNOOZE_MINUTES;
}

void alarm_clock_handle() {
		if (snooze_count == -1) {
				uint8_t day = rtc_get_current_day_of_week()-1;
				uint8_t mask = 1<<day;
				uint8_t alarm_options;
				int8_t 	alarm_hour, alarm_minute;
				load_alarm_clock(&alarm_options, &alarm_hour, &alarm_minute);
				if ((alarm_options & 0x80) == 0 || (alarm_options & mask) == 0)
						return;
				if (alarm_hour == rtc_get_current_hour_24() && alarm_minute == rtc_get_current_minutes())
						fire_alarm();
		} else if (--snooze_count == 0) {
				fire_alarm();
		}
}

#include "alarm.h"
#include "vibration.h"
#include "ext_ram.h"
#include "i18n\i18n.h"
#include "rtc.h"
#include "mlcd_draw.h"
#include "scr_mngr.h"

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

bool button_handler(uint32_t button_id) {
		return true;
}

void pack_dialog_option(bool (*dialog_callback)(uint32_t), uint8_t font,
		const char * title, const char *op1, const char *op2, const char *op3, const char *op4) {
		uint16_t len_title = strlen(title)+1;
		uint16_t len_op1 = strlen(op1)+1;
		uint16_t len_op2 = strlen(op2)+1;
		uint16_t len_op3 = strlen(op3)+1;
		uint16_t len_op4 = strlen(op4)+1;
		uint16_t offset = sizeof(dialog_callback) + sizeof(font) + 5 * sizeof(len_title);
		uint16_t buffer_size = offset;
		if (len_title > 1)
				buffer_size += len_title;
		if (len_op1 > 1)
				buffer_size += len_op1;
		if (len_op2 > 1)
				buffer_size += len_op2;
		if (len_op3 > 1)
				buffer_size += len_op3;
		if (len_op4 > 1)
				buffer_size += len_op4;
		uint8_t buffer[buffer_size];
		memset(buffer, 0, buffer_size);
		memcpy(buffer, &dialog_callback, sizeof(dialog_callback));
		buffer[4] = font;
		if (len_title > 1) {
				memcpy(&buffer[5], &offset, sizeof(offset));
				memcpy(&buffer[offset], title, len_title);
				offset+=len_title;
		}
		if (len_op1 > 1) {
				memcpy(&buffer[7], &offset, sizeof(offset));
				memcpy(&buffer[offset], op1, len_op1);
				offset+=len_op1;
		}
		if (len_op2 > 1) {
				memcpy(&buffer[9], &offset, sizeof(offset));
				memcpy(&buffer[offset], op2, len_op2);
				offset+=len_op2;
		}
		if (len_op3 > 1) {
				memcpy(&buffer[11], &offset, sizeof(offset));
				memcpy(&buffer[offset], op3, len_op3);
				offset+=len_op3;
		}
		if (len_op4 > 1) {
				memcpy(&buffer[13], &offset, sizeof(offset));
				memcpy(&buffer[offset], op4, len_op4);
		}
		
		ext_ram_write_data(EXT_RAM_DATA_NOTIFICATION_UPLOAD_ADDRESS, buffer, buffer_size);
}

static void alarm_clock_handler(void * p_context) {
		vibration_vibrate(ALARM_VIBRATION, 20000);
		pack_dialog_option(&button_handler, FONT_NORMAL_REGULAR, I18N_TRANSLATE(MESSAGE_ALARM_CLOCK),
				I18N_TRANSLATE(MESSAGE_ALARM_SNOOZE), I18N_TRANSLATE(MESSAGE_ALARM_DISMISS), "\0", "\0");
		scr_mngr_show_screen_with_param(SCR_DIALOG_OPTION, EXT_RAM_DATA_NOTIFICATION_UPLOAD_ADDRESS);
		// schedule next alarm
		uint8_t alarm_options;
		int8_t 	alarm_hour, alarm_minute;
		load_alarm_clock(&alarm_options, &alarm_hour, &alarm_minute);
		alarm_clock_reschedule(alarm_options, alarm_hour, alarm_minute);
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

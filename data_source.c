#include "data_source.h"
#include "string.h"
#include "rtc.h"
#include "battery.h"

static uint32_t (* const data_source_handles[])(void) = {
		/* 0 */ rtc_get_current_hour,
		/* 1 */ rtc_get_current_minutes,
		/* 2 */ rtc_get_current_seconds,
		/* 3 */ rtc_get_current_day_of_month,
		/* 4 */ rtc_get_current_month,
		/* 5 */ rtc_get_current_year,
		/* 6 */ battery_get_level
};

uint16_t data_source_get_value(uint16_t data_source_id) {
	  return data_source_handles[data_source_id]();
}

uint32_t (*(data_source_get_handle)(uint16_t data_source_id))(void) {
	  return data_source_handles[data_source_id];
}


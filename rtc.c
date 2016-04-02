#include "rtc.h"
#include "scr_mngr.h"
#include "nordic_common.h"
#include "time.h"
#include "ext_ram.h"
#include "app_scheduler.h"
#include "alarm.h"
#include "config.h"
#include "BLE\ble_peripheral.h"

static app_timer_id_t		m_rtc_timer_id;
static uint32_t					current_time;
static uint16_t 				rtc_refresh_interval;
static uint16_t					interval;
static bool							store_time = false;

void rtc_restart_event(void * p_event_data, uint16_t event_size) {
    uint32_t err_code = app_timer_stop(m_rtc_timer_id);
    APP_ERROR_CHECK(err_code);
		if (rtc_refresh_interval == RTC_INTERVAL_SECOND) {
				interval = RTC_INTERVAL_SECOND;
		} else if (rtc_refresh_interval == RTC_INTERVAL_MINUTE) {
				interval = RTC_INTERVAL_MINUTE - rtc_get_current_seconds();
				if (interval == 0)
						interval = RTC_INTERVAL_MINUTE;
		}
		err_code = app_timer_start(m_rtc_timer_id, APP_TIMER_TICKS(1000*interval, APP_TIMER_PRESCALER), NULL);
		APP_ERROR_CHECK(err_code);
}

void rtc_tick_event(void * p_event_data, uint16_t event_size)
{
    current_time += interval;
    store_time = true;
		scr_mngr_handle_event(SCR_EVENT_RTC_TIME_CHANGED, current_time);
		if (interval != rtc_refresh_interval || (rtc_refresh_interval == RTC_INTERVAL_MINUTE && rtc_get_current_seconds() != 0)) {
				rtc_restart_event(NULL, 0);
		}
		if (rtc_get_current_seconds() == 0) {
				alarm_clock_handle();
				if (rtc_get_current_minutes()%10 == 0)
						battery_level_update();
		}
}

static void rtc_timeout_handler(void * p_context) {
		uint32_t err_code = app_sched_event_put(NULL, 0, rtc_tick_event);
		APP_ERROR_CHECK(err_code);
}

static uint32_t rtc_load_time(void) {
	  uint8_t buffer[4];
		ext_ram_read_data(EXT_RAM_DATA_RTC, buffer, 4);
		return (uint32_t)(((uint32_t)buffer[3] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[1] << 8) | buffer[0]);
}

void rtc_toggle_refresh_interval() {
		if (rtc_refresh_interval == RTC_INTERVAL_MINUTE)
				rtc_set_refresh_interval(RTC_INTERVAL_SECOND);
		else
				rtc_set_refresh_interval(RTC_INTERVAL_MINUTE);
}

void rtc_timer_init(void) {
	  current_time = rtc_load_time();	
		if (current_time == 0) {
			  // set initial time
			  current_time = 1430141820;
		}
    uint32_t err_code = app_timer_create(&m_rtc_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                rtc_timeout_handler);
    APP_ERROR_CHECK(err_code);
		if (get_settings(CONFIG_SLOW_REFRESH))
				rtc_set_refresh_interval(RTC_INTERVAL_MINUTE);
		else
				rtc_set_refresh_interval(RTC_INTERVAL_SECOND);
}

uint16_t rtc_get_refresh_interval() {
		return rtc_refresh_interval;
}

void rtc_set_refresh_interval(uint16_t new_interval) {
		if (new_interval == rtc_refresh_interval)
				return;
		rtc_refresh_interval = new_interval;
		rtc_restart_event(NULL, 0);
}

uint32_t rtc_get_current_time(void) {
	  return current_time;
}

uint32_t rtc_get_current_time_in_seconds(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_hour*3600 + time_struct->tm_min*60 + time_struct->tm_sec;
}

uint32_t rtc_get_current_hour_24(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_hour;
}

uint32_t rtc_get_current_hour_12(void) {
    struct tm* time_struct = localtime(&current_time);
	  uint32_t h = time_struct->tm_hour % 12;
		return h == 0 ? 12 : h;
}

uint32_t rtc_get_current_hour_12_designator(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_hour / 12;
}

uint32_t rtc_get_current_minutes(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_min;
}

uint32_t rtc_get_current_seconds(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_sec;
}

uint32_t rtc_get_current_day_of_week(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_wday + 1;
}

uint32_t rtc_get_current_day_of_month(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_mday;
}

uint32_t rtc_get_current_day_of_year(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_yday + 1;
}

uint32_t rtc_get_current_month(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_mon + 1;
}

uint32_t rtc_get_current_year(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_year + 1900;
}

void rtc_set_current_time(uint32_t new_time) {
		if (new_time != current_time) {
				current_time = new_time;
				store_time = true;
		}
}

void rtc_store_current_time(void) {
		uint8_t buffer[4];
		buffer[0] = current_time;
		buffer[1] = current_time >>  8;
		buffer[2] = current_time >> 16;
		buffer[3] = current_time >> 24;
		ext_ram_write_data(EXT_RAM_DATA_RTC, buffer, 4);
	  store_time = false;
}

bool rtc_should_store_current_time(void) {
	  return store_time;
}

_ARMABI time_t time(time_t * tp) {
	  if( tp != 0 )
    {
        *tp = current_time; 
    }
    return current_time; 
}

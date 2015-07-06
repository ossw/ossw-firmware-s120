#include "rtc.h"
#include "scr_mngr.h"
#include "nordic_common.h"
#include "time.h"
#include "ext_ram.h"

static app_timer_id_t      m_rtc_timer_id;

static uint32_t current_time;

static bool store_time = false;

static void rtc_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);

		current_time++;
	  store_time = true;
	
		scr_mngr_handle_event(SCR_EVENT_RTC_TIME_CHANGED, current_time);

}

static uint32_t rtc_load_time(void) {
	   uint8_t buffer[4];
		 ext_ram_read_data(EXT_RAM_DATA_RTC, buffer, 4);
		 return (uint32_t)(((uint32_t)buffer[3] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[1] << 8) | buffer[0]);
}

void rtc_timer_init(void) {
    uint32_t err_code;	 
		
	  current_time = rtc_load_time();	
		if (current_time == 0){
			  // set initial time
			  current_time = 1430141820;
		}
		
    err_code = app_timer_create(&m_rtc_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                rtc_timeout_handler);
    APP_ERROR_CHECK(err_code);
	
    err_code = app_timer_start(m_rtc_timer_id, RTC_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

uint32_t rtc_get_current_time(void) {
	  return current_time;
}

uint32_t rtc_get_current_hour(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_hour;
}
uint32_t rtc_get_current_minutes(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_min;
}

uint32_t rtc_get_current_seconds(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_sec;
}

uint32_t rtc_get_current_day_of_month(void) {
    struct tm* time_struct = localtime(&current_time);
	  return time_struct->tm_mday;
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
	  current_time = new_time;
	  store_time = true;
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

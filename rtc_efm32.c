#include "rtc.h"
#include "scr_mngr.h"
#include "time.h"
#include "timer.h"

static timer_id_t      m_rtc_timer_id;

static uint32_t current_time;
static bool store_time = false;

static void rtc_timeout_handler(void * p_context) {
    current_time++;
    store_time = true;
    scr_mngr_handle_event(SCR_EVENT_RTC_TIME_CHANGED, current_time);
}

static uint32_t rtc_load_time(void) {
}

void rtc_timer_init(void) {
	  current_time = rtc_load_time();	
		if (current_time == 0){
			  // set initial time
			  current_time = 1430141820;
		}
		
		timer_create(&m_rtc_timer_id,
                                TIMER_TYPE_PERIODIC,
                                rtc_timeout_handler);
	
    timer_start(m_rtc_timer_id, 1000);
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

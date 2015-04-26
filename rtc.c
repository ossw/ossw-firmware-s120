#include "rtc.h"
#include "scr_mngr.h"
#include "nordic_common.h"

static app_timer_id_t      m_rtc_timer_id;
static uint32_t current_time;

static void rtc_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
	  current_time++;
	  scr_mngr_handle_event(SCR_EVENT_RTC_TIME_CHANGED, current_time);
/*	  */
}

void rtc_timer_init(void) {
    uint32_t err_code;
	  // temporary for tests
	  current_time = 1430063060;
	
    err_code = app_timer_create(&m_rtc_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                rtc_timeout_handler);
    APP_ERROR_CHECK(err_code);
	
    err_code = app_timer_start(m_rtc_timer_id, RTC_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

uint32_t rtc_current_time(void) {
	   return current_time;
}

void rtc_set_current_time(uint32_t time) {
	  current_time = time;
}

#include "rtc.h"
#include "scr_mngr.h"
#include "nordic_common.h"
#include "time.h"
#include "ext_ram.h"

static app_timer_id_t      m_rtc_timer_id;

static uint32_t current_time;

static void rtc_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);

		current_time++;
		rtc_set_current_time(current_time);
		scr_mngr_handle_event(SCR_EVENT_RTC_TIME_CHANGED, current_time);

}

void rtc_timer_init(void) {
    uint32_t err_code;	 
		
	current_time = rtc_load_time();	
		if (current_time==0){
			 // set initial time
			current_time = 1430141820;
			rtc_set_current_time(current_time);
		}
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

uint32_t rtc_load_time(void) {
	   uint8_t buffer[4];
		 ext_ram_read_data(EXT_RAM_DATA_RTC, buffer, 4);
		 return (uint32_t)(((uint32_t)buffer[3] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[1] << 8) | buffer[0]);
}


void rtc_set_current_time(uint32_t time) {
	  //save current_time
		current_time = time;
		uint8_t buffer[4];
		buffer[0] = time;
		buffer[1] = time >>  8;
		buffer[2] = time >> 16;
		buffer[3] = time >> 24;
		ext_ram_write_data(EXT_RAM_DATA_RTC,buffer,4);
}


_ARMABI time_t time(time_t * tp) {
	  if( tp != 0 )
    {
        *tp = current_time; 
    }
    return current_time ; 
}

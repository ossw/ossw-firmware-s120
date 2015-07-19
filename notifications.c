#include "notifications.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "scr_mngr.h"
#include "vibration.h"
#include "ble/ble_peripheral.h"

static app_timer_id_t      m_notifications_timer_id;
static uint16_t m_current_notification_id = 0;

static void notifications_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
	  vibration_stop();
	  scr_mngr_close_alert_notification();
		m_current_notification_id = 0;
}

void notifications_init(void) {
	  
    uint32_t err_code;	 
		
    err_code = app_timer_create(&m_notifications_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                notifications_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

void notifications_notify(uint16_t notification_id, uint16_t address, uint16_t timeout, uint32_t vibration_pattern) {
		m_current_notification_id = notification_id;
	  scr_mngr_show_alert_notification(address);
    vibration_vibrate(vibration_pattern);
	  
    uint32_t err_code;	 
    err_code = app_timer_start(m_notifications_timer_id, APP_TIMER_TICKS(timeout, 0), NULL);
    APP_ERROR_CHECK(err_code);
}

void notifications_extend(uint16_t notification_id, uint16_t timeout) {
	  if (m_current_notification_id == notification_id) {
				uint32_t err_code;	 
				err_code = app_timer_stop(m_notifications_timer_id);
				APP_ERROR_CHECK(err_code);
				 
				err_code = app_timer_start(m_notifications_timer_id, APP_TIMER_TICKS(timeout, 0), NULL);
				APP_ERROR_CHECK(err_code);
		}
}

void notifications_stop(uint16_t notification_id) {
	  if (m_current_notification_id == notification_id) {
				uint32_t err_code;	 
				err_code = app_timer_stop(m_notifications_timer_id);
				APP_ERROR_CHECK(err_code);
			
				vibration_stop();
				scr_mngr_close_alert_notification();
				m_current_notification_id = 0;
		}
}

void notifications_invoke_function(uint8_t function_id) {
	  ble_peripheral_invoke_notification_function(function_id);
}

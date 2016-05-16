#include "notifications.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "scr_mngr.h"
#include "vibration.h"
#include "ble/ble_peripheral.h"
#include "ext_ram.h"
#include "nrf_soc.h"
#include "board.h"
#include "mlcd.h"
#include "config.h"
#include "screens/dialog_select.h"

static app_timer_id_t      m_notifications_alert_timer_id;
static uint16_t m_current_alert_notification_id = 0;

void notification_timeout_event(void * p_event_data, uint16_t event_size) {
	  vibration_stop();
	  scr_mngr_close_alert_notification();
		m_current_alert_notification_id = 0;
}

static void notifications_alert_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
		uint32_t err_code = app_sched_event_put(NULL, NULL, notification_timeout_event);
		APP_ERROR_CHECK(err_code);
}

void notifications_init(void) {
    uint32_t err_code;	 
    err_code = app_timer_create(&m_notifications_alert_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                notifications_alert_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

void copy_notification_info_data(uint16_t address_from, uint16_t address_to, uint16_t size) {
	uint8_t buffer[16];
	uint16_t current_read_address = address_from;
	uint16_t current_write_address = address_to;
	uint16_t data_to_copy = size > 1024 ? 1024 : size;
	uint8_t part_size;
	while(data_to_copy > 0) {
		part_size = data_to_copy > 16 ? 16 : data_to_copy;
		ext_ram_read_data(current_read_address, buffer, part_size);
		ext_ram_write_data(current_write_address, buffer, part_size);
		current_read_address += part_size;
		current_write_address += part_size;
		data_to_copy -= part_size;
	}
}

static void send_select_result(uint8_t token, uint8_t buttons, uint8_t item) {
	uint8_t data[2] = {buttons, item};
	if (token == 0)
		ble_peripheral_invoke_notification_function_with_data(DIALOG_RESULT, data, sizeof(data));
	else
		ble_peripheral_invoke_notification_function_with_data(token, data, sizeof(data));
}

void notifications_handle_data(uint16_t address, uint16_t size) {
		uint8_t notification_type = get_next_byte(&address);
	 	switch (notification_type) {
				case NOTIFICATIONS_TYPE_ALERT:
				{
						uint16_t notification_id = get_next_short(&address);
						uint32_t vibration_pattern = get_next_int(&address);
						uint16_t timeout = get_next_short(&address);
						notifications_alert_notify(notification_id, address, timeout, vibration_pattern);
						}
						break;
				case NOTIFICATIONS_TYPE_INFO:
				{	
						uint32_t vibration_pattern = get_next_int(&address);
						uint16_t time = get_next_short(&address);
						copy_notification_info_data(address, EXT_RAM_DATA_NOTIFICATION_INFO_ADDRESS, size - 7);
						notifications_info_notify(time, vibration_pattern);
						}
						break;
				case NOTIFICATIONS_TYPE_UPDATE:
						if (size == 1) {
							  notifications_info_clear_all();
						} else {
								copy_notification_info_data(address, EXT_RAM_DATA_NOTIFICATION_INFO_ADDRESS, size - 1);
							  notifications_info_update();
						}
						break;
				case NOTIFICATIONS_TYPE_DIALOG_SELECT:
						dialog_select_init(send_select_result);
						copy_notification_info_data(address, EXT_RAM_DATA_NOTIFICATION_INFO_ADDRESS, size - 1);
						set_modal_dialog(true);
						scr_mngr_show_screen_with_param(SCR_DIALOG_SELECT, EXT_RAM_DATA_NOTIFICATION_INFO_ADDRESS);
						break;
				case NOTIFICATIONS_TYPE_DIALOG_CLOSE:
						set_modal_dialog(false);
						break;
		}
}

void notifications_info_notify(uint16_t time, uint32_t vibration_pattern) {
		//m_current_notification_id = notification_id;
	  scr_mngr_show_notifications();
	
	  if (m_current_alert_notification_id == 0) {
				vibration_vibrate(vibration_pattern, time, false);
		}
}

void notifications_info_update() {
	  scr_mngr_show_notifications();
}

void notifications_info_clear_all() {
	  scr_mngr_close_notifications();
}

void notifications_alert_notify(uint16_t notification_id, uint16_t address, uint16_t timeout, uint32_t vibration_pattern) {
		bool update = m_current_alert_notification_id == notification_id;

		m_current_alert_notification_id = notification_id;
	  scr_mngr_show_alert_notification(address);
		if (get_settings(CONFIG_NOTIFICATION_LIGHT))
				mlcd_backlight_short();
		if (!update) {
				vibration_vibrate(vibration_pattern, 60*1000, false);
	  
				uint32_t err_code;	 
				err_code = app_timer_start(m_notifications_alert_timer_id, APP_TIMER_TICKS(timeout, APP_TIMER_PRESCALER), NULL);
				APP_ERROR_CHECK(err_code);
		}
}

void notifications_alert_extend(uint16_t notification_id, uint16_t timeout) {
	
		#ifdef OSSW_DEBUG
				sd_nvic_critical_region_enter(0);
				printf("EXTEND: 0x%04x 0x%04x 0x%04x\r\n", notification_id, m_current_alert_notification_id, timeout);
				sd_nvic_critical_region_exit(0);
		#endif
	  if (m_current_alert_notification_id == notification_id) {
				uint32_t err_code;	 
				err_code = app_timer_stop(m_notifications_alert_timer_id);
				APP_ERROR_CHECK(err_code);
				 
				err_code = app_timer_start(m_notifications_alert_timer_id, APP_TIMER_TICKS(timeout, APP_TIMER_PRESCALER), NULL);
				APP_ERROR_CHECK(err_code);
		}
		if (get_settings(CONFIG_NOTIFICATION_LIGHT))
				mlcd_backlight_short();
}

void notifications_alert_stop(uint16_t notification_id) {
	  if (m_current_alert_notification_id == notification_id) {
				uint32_t err_code;	 
				err_code = app_timer_stop(m_notifications_alert_timer_id);
				APP_ERROR_CHECK(err_code);
			
				vibration_stop();
				scr_mngr_close_alert_notification();
				m_current_alert_notification_id = 0;
		}
}

void notifications_invoke_function(uint8_t function_id) {
	  ble_peripheral_invoke_notification_function(function_id);
}

void notifications_open(uint16_t notification_id) {
		uint8_t data[2] = {notification_id >> 8, notification_id&0xFF};
		ble_peripheral_invoke_notification_function_with_data(NOTIFICATIONS_OPEN, data, 2);
}

void notifications_prev_part(uint16_t notification_id, uint8_t current_part_no) {
		uint8_t data[3] = {notification_id >> 8, notification_id&0xFF, current_part_no};
		ble_peripheral_invoke_notification_function_with_data(NOTIFICATIONS_PREV_PART, data, 3);
}

void notifications_next_part(uint16_t notification_id, uint8_t current_part_no) {
		uint8_t data[3] = {notification_id >> 8, notification_id&0xFF, current_part_no};
		ble_peripheral_invoke_notification_function_with_data(NOTIFICATIONS_NEXT_PART, data, 3);
}

void notifications_next(uint16_t notification_id) {
		uint8_t data[2] = {notification_id >> 8, notification_id&0xFF};
		ble_peripheral_invoke_notification_function_with_data(NOTIFICATIONS_NEXT, data, 2);
}

uint16_t notifications_get_current_data(void) {
		return EXT_RAM_DATA_NOTIFICATION_INFO_ADDRESS;
}

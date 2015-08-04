#include "notifications.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "scr_mngr.h"
#include "vibration.h"
#include "ble/ble_peripheral.h"
#include "ext_ram.h"

#define NOTIFICATION_INFO_ADDRESS 0x1800

static app_timer_id_t      m_notifications_alert_timer_id;
static uint16_t m_current_alert_notification_id = 0;
static bool handle_data = false;

static void notifications_alert_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
	  vibration_stop();
	  scr_mngr_close_alert_notification();
		m_current_alert_notification_id = 0;
}

void notifications_init(void) {
	  
    uint32_t err_code;	 
		
    err_code = app_timer_create(&m_notifications_alert_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                notifications_alert_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

static uint8_t get_next_byte(uint16_t *ptr) {
    uint8_t data;
	  ext_ram_read_data(*ptr, &data, 1);
	  //ext_flash_read_data(*ptr, &data, 1);
	  (*ptr)++;
	  return data;
}

static uint16_t get_next_short(uint16_t *ptr) {
    uint8_t data[2];
	  ext_ram_read_data(*ptr, data, 2);
	  //ext_flash_read_data(*ptr, &data, 1);
	  (*ptr)+=2;		
	  return data[0] << 8 | data[1];
}

static uint32_t get_next_int(uint16_t *ptr) {
    uint8_t data[4];
	  ext_ram_read_data(*ptr, data, 4);
	  //ext_flash_read_data(*ptr, &data, 1);
	  (*ptr)+=4;
	  return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}


void notifications_process(void) {
		handle_data = false;
}

void copy_notification_info_data(uint16_t address, uint16_t size) {
		uint8_t buffer[16];
		uint16_t current_read_address = address;
	  uint16_t current_write_address = NOTIFICATION_INFO_ADDRESS;
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

void notifications_handle_data(uint16_t address, uint16_t size) {
		uint8_t notification_type = get_next_byte(&address);
	 	switch (notification_type) {
				case NOTIFICATIONS_TYPE_ALERT:
				{
						uint16_t notification_id = get_next_short(&address);
						uint32_t vibration_pattern = get_next_int(&address);
						uint16_t timeout = get_next_short(&address);
						notifications_alert_notify(notification_id, address, timeout, vibration_pattern);
						// mark data as handled after rendering notification alert (no need to rerender so data can be lost)
						handle_data = true;	
						}
						break;
				case NOTIFICATIONS_TYPE_INFO:
				{	
						uint32_t vibration_pattern = get_next_int(&address);
						uint16_t time = get_next_short(&address);
						copy_notification_info_data(address, size - 7);
						handle_data = true;	
						notifications_info_notify(NOTIFICATION_INFO_ADDRESS, time, vibration_pattern);
						}
						break;
				case NOTIFICATIONS_TYPE_UPDATE:
						if (size == 1) {
							  notifications_info_clear_all();
						} else {
								copy_notification_info_data(address, size - 1);
								handle_data = true;	
							  notifications_info_update(NOTIFICATION_INFO_ADDRESS);
						}
						break;
				default:
						handle_data = true;	
		}
}

bool notifications_is_data_handled(void) {
		return !handle_data;
}

void notifications_info_notify(uint16_t address, uint16_t time, uint32_t vibration_pattern) {
		//m_current_notification_id = notification_id;
	  scr_mngr_show_notifications(address);
	
	  if (m_current_alert_notification_id == 0) {
				vibration_vibrate(vibration_pattern, time);
		}
}

void notifications_info_update(uint16_t address) {
	  scr_mngr_show_notifications(address);
}

void notifications_info_clear_all() {
	  scr_mngr_close_notifications();
}

void notifications_alert_notify(uint16_t notification_id, uint16_t address, uint16_t timeout, uint32_t vibration_pattern) {
		m_current_alert_notification_id = notification_id;
	  scr_mngr_show_alert_notification(address);
    vibration_vibrate(vibration_pattern, 0);
	  
    uint32_t err_code;	 
    err_code = app_timer_start(m_notifications_alert_timer_id, APP_TIMER_TICKS(timeout, 0), NULL);
    APP_ERROR_CHECK(err_code);
}

void notifications_alert_extend(uint16_t notification_id, uint16_t timeout) {
	  if (m_current_alert_notification_id == notification_id) {
				uint32_t err_code;	 
				err_code = app_timer_stop(m_notifications_alert_timer_id);
				APP_ERROR_CHECK(err_code);
				 
				err_code = app_timer_start(m_notifications_alert_timer_id, APP_TIMER_TICKS(timeout, 0), NULL);
				APP_ERROR_CHECK(err_code);
		}
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

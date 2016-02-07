#include "notifications.h"
#include "timer.h"
#include "scr_mngr.h"
#include "vibration.h"
#include "ble/ble_peripheral.h"
#include "ext_ram.h"

static timer_id_t      m_notifications_alert_timer_id;
static uint16_t m_current_alert_notification_id = 0;

static void notifications_alert_timeout_handler(void * p_context) {
	  vibration_stop();
	  scr_mngr_close_alert_notification();
		m_current_alert_notification_id = 0;
}

void notifications_init(void) {
	  
    timer_create(&m_notifications_alert_timer_id,
                                TIMER_TYPE_SINGLE_SHOT,
                                notifications_alert_timeout_handler);
}

static uint8_t get_next_byte(uint16_t *ptr) {
    uint8_t data;
	  ext_ram_read_data(*ptr, &data, 1);
	  (*ptr)++;
	  return data;
}

static uint16_t get_next_short(uint16_t *ptr) {
    uint8_t data[2];
	  ext_ram_read_data(*ptr, data, 2);
	  (*ptr)+=2;		
	  return data[0] << 8 | data[1];
}

static uint32_t get_next_int(uint16_t *ptr) {
    uint8_t data[4];
	  ext_ram_read_data(*ptr, data, 4);
	  (*ptr)+=4;
	  return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

void copy_notification_info_data(uint16_t address, uint16_t size) {
		uint8_t buffer[16];
		uint16_t current_read_address = address;
	  uint16_t current_write_address = EXT_RAM_DATA_NOTIFICATION_INFO_ADDRESS;
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
						}
						break;
				case NOTIFICATIONS_TYPE_INFO:
				{	
						uint32_t vibration_pattern = get_next_int(&address);
						uint16_t time = get_next_short(&address);
						copy_notification_info_data(address, size - 7);
						notifications_info_notify(time, vibration_pattern);
						}
						break;
				case NOTIFICATIONS_TYPE_UPDATE:
						if (size == 1) {
							  notifications_info_clear_all();
						} else {
								copy_notification_info_data(address, size - 1);
							  notifications_info_update();
						}
						break;
		}
}

void notifications_info_notify(uint16_t time, uint32_t vibration_pattern) {
		//m_current_notification_id = notification_id;
	  scr_mngr_show_notifications();
	
	  if (m_current_alert_notification_id == 0) {
				vibration_vibrate(vibration_pattern, time);
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
		if (!update) {
				vibration_vibrate(vibration_pattern, 0);
	  
				timer_start(m_notifications_alert_timer_id, timeout);
		}
}

void notifications_alert_extend(uint16_t notification_id, uint16_t timeout) {
	
		#ifdef OSSW_DEBUG
				sd_nvic_critical_region_enter(0);
				printf("EXTEND: 0x%04x 0x%04x 0x%04x\r\n", notification_id, m_current_alert_notification_id, timeout);
				sd_nvic_critical_region_exit(0);
		#endif
	  if (m_current_alert_notification_id == notification_id) {
				timer_stop(m_notifications_alert_timer_id);
				timer_start(m_notifications_alert_timer_id, timeout);
		}
}

void notifications_alert_stop(uint16_t notification_id) {
	  if (m_current_alert_notification_id == notification_id) {
				timer_stop(m_notifications_alert_timer_id);
			
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

uint32_t notifications_get_current_data(void) {
		return EXT_RAM_DATA_NOTIFICATION_INFO_ADDRESS;
}

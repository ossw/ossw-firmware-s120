#include <string.h>
#include "scr_notifications.h"
#include "nrf_delay.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../notifications.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../ext_ram.h"
#include "../utf8.h"
#include "../i18n/i18n.h"
#include "../ble/ble_peripheral.h"
#include <stdlib.h> 

static uint8_t get_next_byte(uint32_t *ptr) {
    uint8_t data;
	  ext_ram_read_data(*ptr, &data, 1);
	  (*ptr)++;
	  return data;
}

static uint16_t get_next_short(uint32_t *ptr) {
    uint8_t data[2];
	  ext_ram_read_data(*ptr, data, 2);
	  (*ptr)+=2;		
	  return data[0] << 8 | data[1];
}

static void scr_notifications_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
						scr_mngr_close_notifications();
				    break;
			  case SCR_EVENT_PARAM_BUTTON_UP:
				{
						uint32_t read_address = notifications_get_current_data();
						uint8_t notification_type = get_next_byte(&read_address);
	
						if (notification_type != NOTIFICATIONS_CATEGORY_SUMMARY) {
								uint16_t notification_id = get_next_short(&read_address);
								uint8_t page = get_next_byte(&read_address);
								if (page > 0) {
										notifications_prev_part(notification_id, page);
								}
						}
				}
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				{
					
						uint32_t read_address = notifications_get_current_data();
						uint8_t notification_type = get_next_byte(&read_address);
	
						if (notification_type == NOTIFICATIONS_CATEGORY_SUMMARY) {
								//show first
								notifications_invoke_function(NOTIFICATIONS_SHOW_FIRST);
						} else {
								uint16_t notification_id = get_next_short(&read_address);
								uint8_t page = get_next_byte(&read_address);
								uint8_t font = get_next_byte(&read_address);
								bool has_more = get_next_byte(&read_address);
								if (has_more) {
										notifications_next_part(notification_id, page);
								} else {
										notifications_next(notification_id);
								}
						}
				}
				    break;
		}
}

static void scr_notifications_handle_button_long_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				{
						uint32_t read_address = notifications_get_current_data();
						uint8_t notification_type = get_next_byte(&read_address);
						if (notification_type != NOTIFICATIONS_CATEGORY_SUMMARY) {
								uint16_t notification_id = get_next_short(&read_address);
								notifications_next(notification_id);
						}				
				}
				    break;
		}
}

static void scr_notifications_init() {
}

static void scr_notifications_draw_screen() {
		uint32_t read_address = notifications_get_current_data();
    uint8_t notification_type = get_next_byte(&read_address);
	
		switch(notification_type) {
				case NOTIFICATIONS_CATEGORY_MESSAGE:
				case NOTIFICATIONS_CATEGORY_EMAIL:
				case NOTIFICATIONS_CATEGORY_SOCIAL:
				case NOTIFICATIONS_CATEGORY_ALARM:
				case NOTIFICATIONS_CATEGORY_INCOMING_CALL:
				case NOTIFICATIONS_CATEGORY_OTHER:
				{
						uint16_t notification_id = get_next_short(&read_address);
						uint8_t page = get_next_byte(&read_address);
						uint8_t font = get_next_byte(&read_address);
						bool has_more = get_next_byte(&read_address);
					
						char* data_ptr = (char*)(0x80000000 + read_address);
						mlcd_draw_text(data_ptr, 3, 3,  MLCD_XRES - 6, MLCD_YRES - 6, font, HORIZONTAL_ALIGN_LEFT | MULTILINE);
				}
						break;
				case NOTIFICATIONS_CATEGORY_SUMMARY:
				{
						uint8_t notification_count = get_next_byte(&read_address);
				
						if (notification_count>9) {
								notification_count = 9;
						}
						mlcd_draw_digit(notification_count, 20, 20, MLCD_XRES-40, MLCD_YRES-40, 11);
				}
						break;
		}
}

static void scr_notifications_refresh_screen() {
}

void scr_notifications_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_notifications_init();
				    break;
        case SCR_EVENT_DRAW_SCREEN:
            scr_notifications_draw_screen();
            break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_notifications_refresh_screen();
            break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_notifications_handle_button_pressed(event_param);
				    break;
			  case SCR_EVENT_BUTTON_LONG_PRESSED:
				    scr_notifications_handle_button_long_pressed(event_param);
				    break;
			  case SCR_EVENT_DESTROY_SCREEN:
				    break;
		}
}

#include <string.h>
#include "scr_notifications.h"
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

#define MARGIN_SUMMARY 20
#define SIZE_SUMMARY1_X MLCD_XRES-2*MARGIN_SUMMARY
#define SIZE_SUMMARY1_Y MLCD_YRES-2*MARGIN_SUMMARY
#define SIZE_SUMMARY2_X MLCD_XRES/2-2*MARGIN_SUMMARY
#define SIZE_SUMMARY2_Y MLCD_YRES/2-2*MARGIN_SUMMARY
#define SUMMARY2_Y MLCD_YRES/4+MARGIN_SUMMARY

static bool scr_notifications_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
						scr_mngr_close_notifications();
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_UP:
				{
						uint16_t read_address = notifications_get_current_data();
						uint8_t notification_type = get_next_byte(&read_address);
	
						if (notification_type != NOTIFICATIONS_CATEGORY_SUMMARY) {
								uint16_t notification_id = get_next_short(&read_address);
								uint8_t page = get_next_byte(&read_address);
								if (page > 0) {
										notifications_prev_part(notification_id, page);
								}
						}
				}
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				{
					
						uint16_t read_address = notifications_get_current_data();
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
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
				{
						uint16_t read_address = notifications_get_current_data();
						uint8_t notification_type = get_next_byte(&read_address);
	
						if (notification_type != NOTIFICATIONS_CATEGORY_SUMMARY) {
								uint16_t notification_id = get_next_short(&read_address);
								notifications_open(notification_id);
						}
				}
				    return true;
		}
		return false;
}

static bool scr_notifications_handle_button_long_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				{
						uint16_t read_address = notifications_get_current_data();
						uint8_t notification_type = get_next_byte(&read_address);
						if (notification_type != NOTIFICATIONS_CATEGORY_SUMMARY) {
								uint16_t notification_id = get_next_short(&read_address);
								notifications_next(notification_id);
						}				
				}
				    return true;
		}
		return false;
}

static void scr_notifications_init() {
}

static void scr_notifications_draw_screen() {
		uint16_t read_address = notifications_get_current_data();
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
						mlcd_backlight_temp_on();
				}
						break;
				case NOTIFICATIONS_CATEGORY_SUMMARY:
				{
						uint8_t notification_count = get_next_byte(&read_address);

						if (notification_count < 10) {
								mlcd_draw_digit(notification_count, MARGIN_SUMMARY, MARGIN_SUMMARY, SIZE_SUMMARY1_X, SIZE_SUMMARY1_Y, 11);
						} else {
								if (notification_count > 99) {
										notification_count = 99;
								}
								mlcd_draw_digit(notification_count / 10U, MARGIN_SUMMARY, SUMMARY2_Y, SIZE_SUMMARY2_X, SIZE_SUMMARY2_Y, 11);
								mlcd_draw_digit(notification_count % 10U, (MLCD_XRES>>1)+MARGIN_SUMMARY, SUMMARY2_Y, SIZE_SUMMARY2_X, SIZE_SUMMARY2_Y, 11);
								mlcd_backlight_temp_on();
						}
				}
						break;
		}
}

static void scr_notifications_refresh_screen() {
}

bool scr_notifications_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_notifications_init();
				    return true;
        case SCR_EVENT_DRAW_SCREEN:
            scr_notifications_draw_screen();
				    return true;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_notifications_refresh_screen();
				    return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    return scr_notifications_handle_button_pressed(event_param);
			  case SCR_EVENT_BUTTON_LONG_PRESSED:
				    return scr_notifications_handle_button_long_pressed(event_param);
		}
		return false;
}

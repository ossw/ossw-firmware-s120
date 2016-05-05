#include <string.h>
#include "scr_alert_notification.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../notifications.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../ext_ram.h"
#include "../config.h"
#include "../utf8.h"
#include "../pawn/amxutil.h"
#include "../i18n/i18n.h"
#include "../ble/ble_peripheral.h"
#include <stdlib.h> 

static uint16_t m_address;

static bool scr_alert_notification_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
						notifications_invoke_function(NOTIFICATIONS_FUNCTION_ALERT_DISMISS);
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_UP:
						notifications_invoke_function(NOTIFICATIONS_FUNCTION_ALERT_OPTION_1);
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
						notifications_invoke_function(NOTIFICATIONS_FUNCTION_ALERT_OPTION_2);
				    return true;
		}
		return false;
}

static void scr_alert_notification_init(uint32_t address) {
	m_address = address;
	if (get_settings(CONFIG_NOTIFICATION_LIGHT))
		mlcd_backlight_short();
}
	/*
static void draw_incmonig_call_notification() {
	  uint32_t read_address = m_address + 1;
    uint16_t param_1_offset = get_next_short(&read_address);
    uint16_t param_2_offset = get_next_short(&read_address);
	
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_INCOMING_CALL), 0, 5, MLCD_XRES, 20, FONT_OPTION_NORMAL, ALIGN_CENTER);
	
	  uint8_t data[32];
		read_address = m_address + param_1_offset;
	  ext_ram_read_data(read_address, data, 32);
	  mlcd_draw_text((char*)data, 0, 60, MLCD_XRES, 20, FONT_OPTION_NORMAL, ALIGN_CENTER);
	
		read_address = m_address + param_2_offset;
	  ext_ram_read_data(read_address, data, 32);
	  mlcd_draw_text((char*)data, 0, 90, MLCD_XRES, 20, FONT_OPTION_NORMAL, ALIGN_CENTER);
}
*/
static void draw_default_notification() {
	  uint16_t read_address = m_address + 1;
    uint16_t text_offset = get_next_short(&read_address);
    uint8_t font = get_next_byte(&read_address);
    uint8_t operationsNo = get_next_byte(&read_address);
		uint16_t op1_name_offset = operationsNo>0?get_next_short(&read_address):0;
	  uint16_t op2_name_offset = operationsNo>1?get_next_short(&read_address):0;
	
		char* data_ptr = (char*)(0x80000000 + m_address + text_offset);
	  mlcd_draw_text(data_ptr, 3, 30, MLCD_XRES - 6, MLCD_YRES-60, font, HORIZONTAL_ALIGN_CENTER | MULTILINE | VERTICAL_ALIGN_CENTER);

		if (op1_name_offset != 0) {
				data_ptr = (char*)(0x80000000 + m_address + op1_name_offset);
				mlcd_draw_text(data_ptr, 3, 0, MLCD_XRES-6, 30, font, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
		}
		
		if (op2_name_offset != 0) {
				data_ptr = (char*)(0x80000000 + m_address + op2_name_offset);
				mlcd_draw_text(data_ptr, 3, MLCD_YRES-30, MLCD_XRES-6, 30, font, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
		}
}

static void scr_alert_notification_draw_screen() {
		draw_default_notification();
}

static void scr_alert_notification_refresh_screen() {
}

bool scr_alert_notification_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_alert_notification_init(event_param);
				    return true;
        case SCR_EVENT_DRAW_SCREEN:
            scr_alert_notification_draw_screen();
				    return true;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_alert_notification_refresh_screen();
				    return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    return scr_alert_notification_handle_button_pressed(event_param);
		}
		return false;
}

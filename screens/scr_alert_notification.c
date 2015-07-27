#include <string.h>
#include "scr_alert_notification.h"
#include "nrf_delay.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../notifications.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../ext_ram.h"
#include "../utf8.h"
#include "../pawn/amxutil.h"
#include "../i18n/i18n.h"
#include "../ble/ble_peripheral.h"
#include <stdlib.h> 

static uint32_t m_address;
static uint8_t m_notification_type;

static void scr_alert_notification_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
						//notifications_invoke_function(ALERT_NOTIFICATION_FUNCTION_);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_UP:
						notifications_invoke_function(NOTIFICATIONS_FUNCTION_ALERT_OPTION_1);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
						notifications_invoke_function(NOTIFICATIONS_FUNCTION_ALERT_OPTION_2);
				    break;
		}
}

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

static void scr_alert_notification_init(uint32_t address) {
	  m_address = address;
}
	
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

static void draw_default_notification() {
	  uint32_t read_address = m_address + 1;
    uint16_t param_1_offset = get_next_short(&read_address);
    uint16_t param_2_offset = get_next_short(&read_address);
	
	  uint8_t data[32];
		read_address = m_address + param_1_offset;
	  ext_ram_read_data(read_address, data, 32);
	  mlcd_draw_text((char*)data, 0, 60, MLCD_XRES, 20, FONT_OPTION_NORMAL, ALIGN_CENTER);
	
		read_address = m_address + param_2_offset;
	  ext_ram_read_data(read_address, data, 32);
	  mlcd_draw_text((char*)data, 0, 90, MLCD_XRES, 20, FONT_OPTION_NORMAL, ALIGN_CENTER);
}

static void scr_alert_notification_draw_screen() {
	  uint32_t read_address = m_address;
    m_notification_type = get_next_byte(&read_address);
	
	  switch(m_notification_type) {
			case NOTIFICATIONS_CATEGORY_INCOMING_CALL:
				  draw_incmonig_call_notification();
					break;
			default:
				  draw_default_notification();
		}
}

static void scr_alert_notification_refresh_screen() {
}

void scr_alert_notification_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_alert_notification_init(event_param);
				    break;
        case SCR_EVENT_DRAW_SCREEN:
            scr_alert_notification_draw_screen();
            break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_alert_notification_refresh_screen();
            break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_alert_notification_handle_button_pressed(event_param);
				    break;
			  case SCR_EVENT_DESTROY_SCREEN:
				    break;
		}
}

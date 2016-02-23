#include <string.h>
#include "dialog_option_text.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../notifications.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../ext_ram.h"
#include "../utf8.h"
#include <stdlib.h> 

static bool (*dialog_callback)(uint32_t button_id);

static void scr_dialog_draw_screen() {
	  uint16_t read_address = EXT_RAM_DATA_NOTIFICATION_UPLOAD_ADDRESS+1;
		dialog_callback = (bool (*)(uint32_t))get_next_int(&read_address);
    uint8_t font = get_next_byte(&read_address);
    uint16_t text_offset = get_next_short(&read_address);
		uint16_t text_up = get_next_short(&read_address);
	  uint16_t text_down = get_next_short(&read_address);
	  uint16_t text_select = get_next_short(&read_address);
	  uint16_t text_back = get_next_short(&read_address);
		
		uint16_t m_address = EXT_RAM_DATA_NOTIFICATION_UPLOAD_ADDRESS;
		char* data_ptr;
		if (text_offset != 0) {
				data_ptr = (char*)(0x80000000 + m_address + text_offset);
				mlcd_draw_text(data_ptr, 3, 30, MLCD_XRES - 6, 60, font, HORIZONTAL_ALIGN_CENTER | MULTILINE | VERTICAL_ALIGN_CENTER);
		}
		if (text_up != 0) {
				data_ptr = (char*)(0x80000000 + m_address + text_up);
				mlcd_draw_text(data_ptr, 3, 0, MLCD_XRES-6, 30, font, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
		}
		if (text_down != 0) {
				data_ptr = (char*)(0x80000000 + m_address + text_down);
				mlcd_draw_text(data_ptr, MLCD_XRES/2, MLCD_YRES-30, MLCD_XRES/2, 30, font, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
		}
		if (text_select != 0) {
				data_ptr = (char*)(0x80000000 + m_address + text_select);
				mlcd_draw_text(data_ptr, 3, MLCD_YRES/2-15, MLCD_XRES-6, 30, font, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
		}
		if (text_back != 0) {
				data_ptr = (char*)(0x80000000 + m_address + text_back);
				mlcd_draw_text(data_ptr, 3, MLCD_YRES-30, MLCD_XRES/2, 30, font, HORIZONTAL_ALIGN_LEFT | VERTICAL_ALIGN_CENTER);
		}
}

bool dialog_option_text_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    return true;
        case SCR_EVENT_DRAW_SCREEN:
            scr_dialog_draw_screen();
				    return true;
        case SCR_EVENT_REFRESH_SCREEN:
            // TODO
				    return true;
			  case SCR_EVENT_BUTTON_PRESSED:
						if (dialog_callback(event_param)) {
								scr_mngr_show_screen(SCR_WATCH_SET);
								return true;
						}
		}
		return false;
}

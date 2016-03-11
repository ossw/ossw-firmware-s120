#include <string.h>
#include <stdlib.h> 
#include "dialog_option_text.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../notifications.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../ext_ram.h"
#include "../utf8.h"

#define MARGIN 2

static bool (*dialog_callback)(uint32_t);

static void scr_dialog_draw_screen() {
	  uint16_t read_address = EXT_RAM_DATA_DIALOG_TEXT;
		dialog_callback = (bool (*)(uint32_t))get_next_int(&read_address);
    uint8_t font = get_next_byte(&read_address);
    uint8_t text_offset = get_next_byte(&read_address);
		uint8_t text_up = get_next_byte(&read_address);
	  uint8_t text_down = get_next_byte(&read_address);
	  uint8_t text_select = get_next_byte(&read_address);
	  uint8_t text_back = get_next_byte(&read_address);
		
		uint32_t m_address = 0x80000000 + EXT_RAM_DATA_DIALOG_TEXT;
		char* data_ptr;
		if (text_offset != 0) {
				data_ptr = (char*)(m_address + text_offset);
				mlcd_draw_text(data_ptr, MARGIN, 30, MLCD_XRES-2*MARGIN, 60, font, HORIZONTAL_ALIGN_LEFT | MULTILINE | VERTICAL_ALIGN_CENTER);
		}
		if (text_up != 0) {
				data_ptr = (char*)(m_address + text_up);
				mlcd_draw_text(data_ptr, MARGIN, 0, MLCD_XRES-2*MARGIN, 30, font, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
		}
		if (text_down != 0) {
				data_ptr = (char*)(m_address + text_down);
				int x = MARGIN;
				if (text_back != 0)
						x = MLCD_XRES >> 1;
				mlcd_draw_text(data_ptr, x, MLCD_YRES-30, MLCD_XRES-MARGIN-x, 30, font, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
		}
		if (text_select != 0) {
				data_ptr = (char*)(m_address + text_select);
				mlcd_draw_text(data_ptr, MARGIN, MLCD_YRES/2-15, MLCD_XRES-2*MARGIN, 30, font, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
		}
		if (text_back != 0) {
				int x = MARGIN;
				if (text_down != 0)
						x = MLCD_XRES >> 1;
				data_ptr = (char*)(m_address + text_back);
				mlcd_draw_text(data_ptr, MARGIN, MLCD_YRES-30, MLCD_XRES-MARGIN-x, 30, font, HORIZONTAL_ALIGN_LEFT | VERTICAL_ALIGN_CENTER);
		}
}

void pack_dialog_option(bool (*d_callback)(uint32_t), uint8_t font,
		const char * title, const char *op1, const char *op2, const char *op3, const char *op4) {
		uint8_t len_title = strlen(title)+1;
		uint8_t len_op1 = strlen(op1)+1;
		uint8_t len_op2 = strlen(op2)+1;
		uint8_t len_op3 = strlen(op3)+1;
		uint8_t len_op4 = strlen(op4)+1;
		uint8_t offset = 4 + sizeof(font) + 5 * sizeof(len_title);
		uint8_t buffer_size = offset;
		if (len_title > 1)
				buffer_size += len_title;
		if (len_op1 > 1)
				buffer_size += len_op1;
		if (len_op2 > 1)
				buffer_size += len_op2;
		if (len_op3 > 1)
				buffer_size += len_op3;
		if (len_op4 > 1)
				buffer_size += len_op4;
		uint8_t buffer[buffer_size];
		memset(buffer, 0, buffer_size);
		uint8_t bindex = 0;
		//memcpy(buffer, &d_callback, sizeof(d_callback));
		uint32_t addr = (uint32_t)d_callback;
		buffer[0] = addr >> 24 & 0xFF;
		buffer[1] = addr >> 16 & 0xFF;
		buffer[2] = addr >>  8 & 0xFF;
		buffer[3] = addr & 0xFF;
		bindex += sizeof(addr);
		buffer[bindex] = font;
		bindex += sizeof(font);
		if (len_title > 1) {
				buffer[bindex] = offset;
				bindex += sizeof(offset);
				memcpy(&buffer[offset], title, len_title);
				offset+=len_title;
		}
		if (len_op1 > 1) {
				buffer[bindex] = offset;
				bindex += sizeof(offset);
				memcpy(&buffer[offset], op1, len_op1);
				offset+=len_op1;
		}
		if (len_op2 > 1) {
				buffer[bindex] = offset;
				bindex += sizeof(offset);
				memcpy(&buffer[offset], op2, len_op2);
				offset+=len_op2;
		}
		if (len_op3 > 1) {
				buffer[bindex] = offset;
				bindex += sizeof(offset);
				memcpy(&buffer[offset], op3, len_op3);
				offset+=len_op3;
		}
		if (len_op4 > 1) {
				buffer[bindex] = offset;
				bindex += sizeof(offset);
				memcpy(&buffer[offset], op4, len_op4);
		}
		
		ext_ram_write_data(EXT_RAM_DATA_DIALOG_TEXT, buffer, buffer_size);
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

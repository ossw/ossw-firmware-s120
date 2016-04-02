#include <string.h>
#include <stdlib.h>
#include "dialog_select.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../notifications.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../ext_ram.h"
#include "../utf8.h"
#include "../graph.h"

#define MARGIN					2
#define SCROLL_HEIGHT		6

static void (*dialog_callback)(uint8_t);
static uint16_t dialog_input;
static bool redraw = false;
static uint8_t items_per_page;

static void skip_string_ext_ram(uint8_t no, uint16_t* address_ptr) {
		while (no-- > 0) {
				while (get_next_byte(address_ptr) != '\0');
		}
}

static void dialog_select_draw_screen() {
		mlcd_clear_rect(0, 0, MLCD_XRES, MLCD_YRES);
		uint16_t read_address = dialog_input;
		uint8_t item = get_next_byte(&read_address);
		uint8_t list_size = get_next_byte(&read_address);
    uint8_t font = get_next_byte(&read_address);
		const FONT_INFO* font_info = mlcd_resolve_font(font);
		uint8_t item_height = font_info->height;
		uint8_t title_height = item_height+2;

		uint32_t m_address = 0x80000000;
		char* data_ptr;
		data_ptr = (char*)(m_address + read_address);
		mlcd_draw_text(data_ptr, 0, 0, MLCD_XRES, item_height, font, HORIZONTAL_ALIGN_CENTER);
		fillRectangle(MARGIN, item_height, MLCD_XRES-2*MARGIN, 2);
		skip_string_ext_ram(1, &read_address);

		items_per_page = (MLCD_YRES-MARGIN-title_height)/item_height;
		uint8_t page_no = item / items_per_page;
		uint8_t start_item = page_no * items_per_page;
		skip_string_ext_ram(start_item, &read_address);
		uint8_t items_no;
		if (list_size - start_item < items_per_page)
				items_no = list_size - start_item;
		else
				items_no = items_per_page;
		// draw the page with selected item
		uint8_t list_top = title_height + ((MLCD_YRES-title_height-item_height*items_per_page)>>1);
		uint8_t y = list_top;
		for (int i = 0; i < items_no; i++) {
				data_ptr = (char*)(m_address + read_address);
				mlcd_draw_text(data_ptr, MARGIN, y, MLCD_XRES-2*MARGIN, item_height, font, HORIZONTAL_ALIGN_CENTER | VERTICAL_ALIGN_CENTER);
				y += item_height;
				skip_string_ext_ram(1, &read_address);
		}
		fillRectangle(MARGIN, list_top+(item-start_item)*item_height, MLCD_XRES-2*MARGIN, item_height);
		if (page_no > 0)
				fillUp(MLCD_XRES-SCROLL_HEIGHT-MARGIN, MARGIN, SCROLL_HEIGHT);
		if (page_no + 1 < CEIL(list_size, items_per_page))
				fillDown(MLCD_XRES-SCROLL_HEIGHT-MARGIN, MLCD_YRES-MARGIN, SCROLL_HEIGHT);
}

void dialog_select_init(void (*d_callback)(uint8_t)) {
		dialog_callback = d_callback;
}

void pack_dialog_select(uint8_t init, void (*d_callback)(uint8_t), uint8_t font, const char *title, uint8_t list_size, const char *list) {
		dialog_callback = d_callback;
		uint8_t len_title = strlen(title);
		uint8_t buffer_size = sizeof(init) + 4 + sizeof(font) + len_title+1 + sizeof(list_size);
		uint16_t shift = 0;
		for (int i=0; i<list_size; i++) {
				shift += strlen(list+shift) + 1;
		}
		buffer_size += shift;
		uint8_t buffer[buffer_size];

		uint8_t bindex = 0;
		buffer[bindex] = init;
		bindex += sizeof(init);
		buffer[bindex] = list_size;
		bindex += sizeof(list_size);
		buffer[bindex] = font;
		bindex += sizeof(font);
		memcpy(buffer+bindex, title, ++len_title);
		bindex += len_title;
		memcpy(buffer+bindex, list, shift);
		ext_ram_write_data(EXT_RAM_DATA_DIALOG_TEXT, buffer, buffer_size);
}

static bool dialog_select_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK: {
						dialog_callback(CANCEL);
					  set_modal_dialog(false);
				    return true;
				}
			  case SCR_EVENT_PARAM_BUTTON_UP: {
						uint16_t read_address = dialog_input;
						uint8_t item = get_next_byte(&read_address);
					  if (item > 0) {
								item--;
								uint8_t list_size = get_next_byte(&read_address);
								uint8_t font = get_next_byte(&read_address);
								const FONT_INFO* font_info = mlcd_resolve_font(font);
								uint8_t item_height = font_info->height;
								items_per_page = (MLCD_YRES-MARGIN-item_height-2)/item_height;
								if (item/items_per_page == (item+1)/items_per_page) {
										// same page, move selection only
										uint8_t list_top = item_height + 2 + ((MLCD_YRES-item_height*(items_per_page+1)-2)>>1);
										fillRectangle(MARGIN, list_top+(item % items_per_page)*item_height, MLCD_XRES-2*MARGIN, item_height<<1);
								} else
										redraw = true;
								ext_ram_write_data(dialog_input, &item, sizeof(item));
						}
				    return true;
				}
			  case SCR_EVENT_PARAM_BUTTON_DOWN: {
						uint16_t read_address = dialog_input;
						uint8_t item = get_next_byte(&read_address);
						uint8_t last = get_next_byte(&read_address)-1;
					  if (item < last) {
								uint8_t font = get_next_byte(&read_address);
								const FONT_INFO* font_info = mlcd_resolve_font(font);
								uint8_t item_height = font_info->height;
								items_per_page = (MLCD_YRES-MARGIN-item_height-2)/item_height;
								if (item/items_per_page == (item+1)/items_per_page) {
										// same page, move selection only
										uint8_t list_top = item_height + 2 + ((MLCD_YRES-item_height*(items_per_page+1)-2)>>1);
										fillRectangle(MARGIN, list_top+(item % items_per_page)*item_height, MLCD_XRES-2*MARGIN, item_height<<1);
								} else
										redraw = true;
								item++;
								ext_ram_write_data(dialog_input, &item, sizeof(item));
						}
				    return true;
				}
			  case SCR_EVENT_PARAM_BUTTON_SELECT: {
						uint16_t read_address = dialog_input;
						uint8_t item = get_next_byte(&read_address);
					  dialog_callback(item);
					  set_modal_dialog(false);
				    return true;
				}
		}
		return false;
}

static bool dialog_select_button_long_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP: {
						uint16_t read_address = dialog_input;
						uint8_t item = get_next_byte(&read_address);
					  if (item > 0) {
								if (item < items_per_page)
										item = 0;
								else
										item -= items_per_page;
								ext_ram_write_data(dialog_input, &item, sizeof(item));
								redraw = true;
						}
				    return true;
					}
			  case SCR_EVENT_PARAM_BUTTON_DOWN: {
						uint16_t read_address = dialog_input;
						uint8_t item = get_next_byte(&read_address);
						uint8_t last = get_next_byte(&read_address)-1;
					  if (item < last) {
								if (item + items_per_page > last)
										item = last;
								else
										item += items_per_page;
								ext_ram_write_data(dialog_input, &item, sizeof(item));
								redraw = true;
						}
				    return true;
					}
			  case SCR_EVENT_PARAM_BUTTON_SELECT: {
						uint16_t read_address = dialog_input;
						uint8_t item = get_next_byte(&read_address);
					  dialog_callback(item);
						return true;
				}
		}
		return false;
}

static void dialog_select_refresh_screen() {
		if (!redraw)
				return;
		dialog_select_draw_screen();
		redraw = false;
}

bool dialog_select_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
						dialog_input = event_param;
				    return true;
        case SCR_EVENT_DRAW_SCREEN:
            dialog_select_draw_screen();
				    return true;
        case SCR_EVENT_REFRESH_SCREEN:
            dialog_select_refresh_screen();
				    return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    return dialog_select_button_pressed(event_param);
			  case SCR_EVENT_BUTTON_LONG_PRESSED:
						return dialog_select_button_long_pressed(event_param);
		}
		return false;
}

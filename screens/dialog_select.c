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
#define RADIO_RADIUS		7
#define CHECK_BOX_SIZE	14
#define WIDGET_PADDING	24

static void (*d_select_callback)(uint8_t);
static uint16_t dialog_input;
static bool redraw = false;
static uint8_t items_per_page;

void dialog_select_init(void (*d_callback)(uint8_t)) {
	d_select_callback = d_callback;
}

static void skip_string_ext_ram(uint8_t no, uint16_t* address_ptr) {
	while (no-- > 0)
		while (get_next_byte(address_ptr) != '\0');
}

static void dialog_select_draw_screen() {
		fillRectangle(0, 0, MLCD_XRES, MLCD_YRES, DRAW_BLACK);
		uint16_t read_address = dialog_input;
		uint8_t item = get_next_byte(&read_address);
		uint8_t list_size = get_next_byte(&read_address);
    uint8_t font = get_next_byte(&read_address);
		uint8_t style = get_next_byte(&read_address);
		uint8_t bitset_size = CEIL(list_size, 8);
		uint8_t bitset[bitset_size];
		uint8_t font_style = VERTICAL_ALIGN_CENTER;
		uint8_t text_x = MARGIN;
		if (style > 0) {
			text_x = WIDGET_PADDING;
			font_style |= HORIZONTAL_ALIGN_LEFT;
			for (int i = 0; i < bitset_size; i++)
				bitset[i] = get_next_byte(&read_address);
		} else
			font_style |= HORIZONTAL_ALIGN_CENTER;
		const FONT_INFO* font_info = mlcd_resolve_font(font);
		uint8_t item_height = font_info->height;
		uint8_t title_height = item_height+2;

		uint32_t m_address = 0x80000000;
		char* data_ptr;
		data_ptr = (char*)(m_address + read_address);
		mlcd_draw_text(data_ptr, 0, 0, MLCD_XRES, item_height, font, HORIZONTAL_ALIGN_CENTER);
		fillRectangle(MARGIN, item_height, MLCD_XRES-2*MARGIN, 2, DRAW_XOR);
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
				uint8_t marked = bitset[(i + start_item) >> 3] & (1 << ((i + start_item) & 7));
				if (marked)
					mlcd_draw_text(data_ptr, text_x, y, MLCD_XRES-MARGIN-text_x, item_height, font, font_style | STYLE_LINE_THROUGH);
				else
					mlcd_draw_text(data_ptr, text_x, y, MLCD_XRES-MARGIN-text_x, item_height, font, font_style);
				switch (style) {
					case SELECT_RADIO: {
						uint8_t cx = MARGIN + RADIO_RADIUS;
						uint8_t cy = y + (item_height >> 1);
						circle(cx, cy, RADIO_RADIUS, DRAW_XOR);
						if (marked) {
							fillCircle(cx, cy, RADIO_RADIUS-2, DRAW_XOR);
						}
						break;
					}
					case SELECT_CHECK: {
						uint8_t cx = MARGIN;
						uint8_t cy = y + ((item_height - CHECK_BOX_SIZE) >> 1);
						rectangle(cx, cy, CHECK_BOX_SIZE, CHECK_BOX_SIZE, DRAW_WHITE);
						if (marked) {
							fillRectangle(cx+2, cy+2, CHECK_BOX_SIZE-4, CHECK_BOX_SIZE-4, DRAW_XOR);
						}
						break;
					}
				}
				y += item_height;
				skip_string_ext_ram(1, &read_address);
		}
		fillRectangle(text_x-MARGIN, list_top+(item-start_item)*item_height, MLCD_XRES+MARGIN-text_x, item_height, DRAW_XOR);
		if (page_no > 0)
				fillUp(MLCD_XRES-SCROLL_HEIGHT-MARGIN, MARGIN, SCROLL_HEIGHT, DRAW_XOR);
		if (page_no + 1 < CEIL(list_size, items_per_page))
				fillDown(MLCD_XRES-SCROLL_HEIGHT-MARGIN, MLCD_YRES-MARGIN, SCROLL_HEIGHT, DRAW_XOR);
}

static bool dialog_select_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK: {
					  set_modal_dialog(false);
						d_select_callback(CANCEL);
				    return true;
				}
			  case SCR_EVENT_PARAM_BUTTON_UP: {
						uint16_t read_address = dialog_input;
						uint8_t item = get_next_byte(&read_address);
					  if (item > 0) {
								item--;
								uint8_t list_size = get_next_byte(&read_address);
								uint8_t font = get_next_byte(&read_address);
								uint8_t style = get_next_byte(&read_address);
								uint8_t select_x = 0;
								if (style > 0)
									select_x = WIDGET_PADDING - MARGIN;
								const FONT_INFO* font_info = mlcd_resolve_font(font);
								uint8_t item_height = font_info->height;
								items_per_page = (MLCD_YRES-MARGIN-item_height-2)/item_height;
								if (item/items_per_page == (item+1)/items_per_page) {
										// same page, move selection only
										uint8_t list_top = item_height + 2 + ((MLCD_YRES-item_height*(items_per_page+1)-2)>>1);
										fillRectangle(select_x, list_top+(item % items_per_page)*item_height, MLCD_XRES-select_x, item_height<<1, DRAW_XOR);
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
								uint8_t style = get_next_byte(&read_address);
								uint8_t select_x = 0;
								if (style > 0)
									select_x = WIDGET_PADDING - MARGIN;
								const FONT_INFO* font_info = mlcd_resolve_font(font);
								uint8_t item_height = font_info->height;
								items_per_page = (MLCD_YRES-MARGIN-item_height-2)/item_height;
								if (item/items_per_page == (item+1)/items_per_page) {
										// same page, move selection only
										uint8_t list_top = item_height + 2 + ((MLCD_YRES-item_height*(items_per_page+1)-2)>>1);
										fillRectangle(select_x, list_top+(item % items_per_page)*item_height, MLCD_XRES-select_x, item_height<<1, DRAW_XOR);
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
						uint8_t list_size = get_next_byte(&read_address);
						read_address++;
						uint8_t style = get_next_byte(&read_address);
						uint8_t bitset_size = CEIL(list_size, 8);
						uint8_t bitset[bitset_size];
						if (style > 0) {
							uint8_t item_mask = 1 << (item & 7);
							if (style >= SELECT_CHECK) {
								for (int i = 0; i < bitset_size; i++)
									bitset[i] = get_next_byte(&read_address);
								bitset[item >> 3] ^= item_mask;
							} else {
								memset(bitset, 0, bitset_size);
								bitset[item >> 3] |= item_mask;
							}
							ext_ram_write_data(dialog_input+4, bitset, bitset_size);
							// TODO: can be optimized for less refresh
							dialog_select_draw_screen();
						} else
							d_select_callback(item);
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
					  d_select_callback(item);
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

void pack_dialog_select(uint8_t init, void (*d_callback)(uint8_t), uint8_t font, uint8_t st, const char *title, uint8_t list_size, const char *list) {
		d_select_callback = d_callback;
		uint8_t len_title = strlen(title);
		uint8_t bitset_size = CEIL(list_size, 8);
		uint8_t buffer_size = sizeof(init) + 4 + sizeof(font) + len_title+1 + sizeof(list_size);
		if (st > 0)
			buffer_size += bitset_size;
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
		buffer[bindex] = st;
		bindex += sizeof(st);
		if (st > 0)
			bindex += bitset_size;
		memcpy(buffer+bindex, title, ++len_title);
		bindex += len_title;
		memcpy(buffer+bindex, list, shift);
		ext_ram_write_data(EXT_RAM_DATA_DIALOG_TEXT, buffer, buffer_size);
}

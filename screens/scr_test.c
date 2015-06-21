#include <string.h>
#include "scr_test.h"
#include "nrf_delay.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../data_source.h"
#include "../ext_ram.h"
#include "../utf8.h"
#include "../pawn/amxutil.h"
#include "../i18n/i18n.h"

#define DIGITS_Y_POS 80

//static uint32_t lastValue = 0;

uint8_t testValue;
uint32_t screens_section_address;

static void scr_test_handle_button_back(void) {
	  scr_mngr_show_screen(SCR_WATCHFACE);
}

static void scr_test_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_test_handle_button_back();
				    break;
		}
}
/*
static void scr_test_refresh_value1() {
	  if (testValue == lastValue) {
			  return;
		}
		if (testValue < 100) {
			  mlcd_clear_rect(5, DIGITS_Y_POS, 40, 58);
		} else {
				mlcd_draw_digit(testValue/100, 5, DIGITS_Y_POS, 40, 58, 5);
		}
    mlcd_draw_digit((testValue%100)/10, 49, DIGITS_Y_POS, 40, 58, 5);
    mlcd_draw_digit(testValue%10, 93, DIGITS_Y_POS, 40, 58, 5);
	
    lastValue = testValue;
}
*/
static void scr_test_refresh_screen() {
	 // scr_test_refresh_value1();
	  mlcd_fb_flush();
}

static uint8_t get_next_byte(uint32_t *ptr) {
    uint8_t data;
	  ext_ram_read_data(*ptr, &data, 1);
	  (*ptr)++;
	  return data;
}


static uint32_t (*(parse_data_source)(uint32_t *read_address))(void) {
    uint8_t type = get_next_byte(read_address);
    uint8_t property = get_next_byte(read_address);
	  // TODO
	  return data_source_get_handle(property);
}

static bool parse_screen_control_number(uint32_t *read_address) {
    uint8_t format = get_next_byte(read_address);
    uint8_t x = get_next_byte(read_address);
    uint8_t y = get_next_byte(read_address);
    uint8_t width = get_next_byte(read_address);
    uint8_t height = get_next_byte(read_address);
    uint8_t style1 = get_next_byte(read_address);
    uint8_t style2 = get_next_byte(read_address);
    uint8_t thickness = style1 & 0x3F;
	
	  uint32_t (*data_source)(void) = parse_data_source(read_address);
	
	  CONTROL_DATA data;
	  data.last_value = 0;
	
	  SCR_CONTROL_NUMBER_CONFIG config = {
				NUMBER_FORMAT_0_99 | NUMBER_FORMAT_FLAG_ZERO_PADDED,
				x,
				y,
				width,
				height,
				thickness,
				data_source,
				&data
		};
		
		SCR_CONTROL_DEFINITION controls[] = {
				{SCR_CONTROL_NUMBER, (void*)&config}
		};

		SCR_CONTROLS_DEFINITION controls_definition = {
				sizeof(controls)/sizeof(SCR_CONTROL_DEFINITION),
				controls
		};
	
	  scr_controls_draw(&controls_definition);
		return true;
}

static bool parse_controls(uint32_t *read_address) {
	  uint8_t controls_no = get_next_byte(read_address);
		for (int i = 0; i < controls_no; i++) {
			  uint8_t control_type = get_next_byte(read_address);
			  switch (control_type) {
					case SCR_CONTROL_NUMBER:
						  parse_screen_control_number(read_address);
					    break;
				}
		}
		return true;
}

static bool parse_screen(uint32_t read_address) {
	  uint8_t section;
	  do {
				section = get_next_byte(&read_address);
				switch (section) {
					case WATCH_SET_SCREEN_SECTION_CONTROLS:
						  parse_controls(&read_address);
							break;
					case WATCH_SET_SCREEN_SECTION_ACTIONS:
							break;
				}
	  } while (section != WATCH_SET_END_OF_DATA);
		return true;
}

static bool init_screen(uint8_t screen_id) {
	  uint32_t read_address = screens_section_address;
	  uint8_t screens_no = get_next_byte(&read_address);
	
	  if (screen_id >= screens_no) {
		    return false;
	  }
		// jump to screen offset
		read_address += 2 * screen_id;
		uint16_t screen_offset = get_next_byte(&read_address) << 8;
		screen_offset |= get_next_byte(&read_address);
		return parse_screen(screens_section_address + screen_offset);
}

static void scr_test_init() {
	  uint32_t read_address = 0x1000;
	
	  mlcd_fb_clear();
	
	  uint8_t section;
	  while ((section = get_next_byte(&read_address))!= WATCH_SET_END_OF_DATA){
				switch (section) {
						case WATCH_SET_SECTION_SCREENS:
								screens_section_address = read_address + 2;
		
								break;
						case WATCH_SET_SECTION_STATIC_CONTENT:
								break;
				}
				uint16_t section_size = get_next_byte(&read_address) << 8;
				section_size |= get_next_byte(&read_address);
				read_address+=section_size;
	  };
		init_screen(0);
		
//	  uint8_t screens_no = data[0];
	
	                 
	//  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_HEART_RATE), 5, 13, NULL, NULL, FONT_OPTION_BIG);
	//  mlcd_draw_rect(0, 50, MLCD_XRES, 2);
	
  //  scr_test_refresh_screen();
}

static void scr_test_cleanup() {
	  
}

void scr_test_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_test_init();
				    break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_test_refresh_screen();
            break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_test_handle_button_pressed(event_param);
				    break;
			  case SCR_EVENT_DESTROY_SCREEN:
				    scr_test_cleanup();
				    break;
		}
}

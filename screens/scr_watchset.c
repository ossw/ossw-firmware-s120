#include <string.h>
#include "scr_watchset.h"
#include "nrf_delay.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../ext_ram.h"
#include "../ext_flash.h"
#include "../utf8.h"
#include "../pawn/amxutil.h"
#include "../i18n/i18n.h"
#include "../ble/ble_peripheral.h"
#include "../battery.h"
#include <stdlib.h> 

uint32_t screens_section_address;
uint32_t external_properties_section_address;
uint8_t external_properties_no = 0;
uint8_t* external_properties_data = NULL;
SCR_CONTROLS_DEFINITION controls;

FUNCTION action_handlers[8];

static void scr_watch_set_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  scr_watch_set_invoke_function(&action_handlers[0]);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  scr_watch_set_invoke_function(&action_handlers[1]);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  scr_watch_set_invoke_function(&action_handlers[2]);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_watch_set_invoke_function(&action_handlers[3]);
				    break;
		}
}

static void scr_watch_set_handle_button_long_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  scr_watch_set_invoke_function(&action_handlers[4]);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  scr_watch_set_invoke_function(&action_handlers[5]);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  scr_watch_set_invoke_function(&action_handlers[6]);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_watch_set_invoke_function(&action_handlers[7]);
				    break;
		}
}

static void scr_watch_set_refresh_screen() {
	  scr_controls_redraw(&controls);
	  mlcd_fb_flush();
}

static uint8_t get_next_byte(uint32_t *ptr) {
    uint8_t data;
	  ext_ram_read_data(*ptr, &data, 1);
	  //ext_flash_read_data(*ptr, &data, 1);
	  (*ptr)++;
	  return data;
}

static uint16_t get_next_short(uint32_t *ptr) {
    uint8_t data[2];
	  ext_ram_read_data(*ptr, data, 2);
	  //ext_flash_read_data(*ptr, &data, 1);
	  (*ptr)+=2;
	  return data[0] << 8 | data[1];
}

static uint32_t (* const data_source_handles[])(void) = {
		/* 0 */ rtc_get_current_hour,
		/* 1 */ rtc_get_current_minutes,
		/* 2 */ rtc_get_current_seconds,
		/* 3 */ rtc_get_current_day_of_month,
		/* 4 */ rtc_get_current_month,
		/* 5 */ rtc_get_current_year,
		/* 6 */ battery_get_level
};

static uint32_t internal_data_source_get_value(uint32_t data_source_id) {
	  return data_source_handles[data_source_id]();
}

static uint8_t calc_ext_property_size(uint8_t type, uint8_t range) {
	  switch(type) {
			  case WATCH_SET_EXT_PROP_TYPE_NUMBER:
						if (range == NUMBER_RANGE_0__9 || range == NUMBER_RANGE_0__19 || range == NUMBER_RANGE_0__99 || range == NUMBER_RANGE_0__199) {
								return 1;
						} else if (range == NUMBER_RANGE_0__999 || range == NUMBER_RANGE_0__1999 || range == NUMBER_RANGE_0__9999 || range == NUMBER_RANGE_0__1999) {
								return 2;
						}
						return 0;
				case WATCH_SET_EXT_PROP_TYPE_ENUM:
					  return 1;
				case WATCH_SET_EXT_PROP_TYPE_STRING:
					  return range + 1;
		}
		return 0;
}

static uint32_t external_data_source_get_property_value(uint32_t property_id) {	  
	  if (external_properties_data == NULL) {
			  return NULL;
		}
		if (property_id >= external_properties_no) {
			  return NULL;
		}
		uint8_t type = external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE];
		uint8_t range = external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+1];
		uint16_t offset = external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+2] << 8;
		offset |= external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+3];
		
		uint8_t number_size;
		switch(type) {
			  case WATCH_SET_EXT_PROP_TYPE_NUMBER:
						number_size = calc_ext_property_size(type, range);
				    switch (number_size) {
							case 1:
								  return external_properties_data[offset];
							case 2:
								  return external_properties_data[offset] << 8 | external_properties_data[offset+1];
							case 3:
								  return external_properties_data[offset] << 16 | 
										  external_properties_data[offset+1] << 8 | 
									    external_properties_data[offset+2];
						}
		        return NULL;
			  case WATCH_SET_EXT_PROP_TYPE_STRING:
		        return (uint32_t)&external_properties_data[offset];
			  default:
				    return NULL;
		}
}

static void parse_data_source(uint32_t *read_address, uint32_t (**data_source)(uint32_t), uint32_t* data_source_param) {
    uint8_t type = get_next_byte(read_address);
    uint8_t property = get_next_byte(read_address);
	  switch (type) {
			  case DATA_SOURCE_INTERNAL:
			      *data_source = internal_data_source_get_value;
				    *data_source_param = property;
				    break;
			  case DATA_SOURCE_EXTERNAL:
			      *data_source = external_data_source_get_property_value;
				    *data_source_param = property;
				    break;
		}
}

void set_external_property_data(uint8_t property_id, uint8_t* data_ptr, uint8_t size) {
	  if (external_properties_data == NULL) {
			  return;
		}
		if (property_id >= external_properties_no) {
			  return;
		}
		uint16_t offset = external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+2] << 8;
		offset |= external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+3];
		memcpy(&external_properties_data[offset], data_ptr, size);
		uint8_t type = external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE];
		if (type == WATCH_SET_EXT_PROP_TYPE_STRING) {
			  // end of string char
			  external_properties_data[offset + size] = 0;
		}
}

static void* parse_screen_control_number(uint32_t *read_address) {
    uint8_t range = get_next_byte(read_address);
    uint8_t x = get_next_byte(read_address);
    uint8_t y = get_next_byte(read_address);
    uint8_t width = get_next_byte(read_address);
    uint8_t height = get_next_byte(read_address);
    uint16_t style = get_next_short(read_address);
	
	
	  NUMBER_CONTROL_DATA* data = malloc(sizeof(NUMBER_CONTROL_DATA));
	  
	  SCR_CONTROL_NUMBER_CONFIG* config = malloc(sizeof(SCR_CONTROL_NUMBER_CONFIG));
	
		config->range = range;
		config->x = x;
		config->y = y;
		config->width = width;
		config->height = height;
		config->style = style;
		config->data = data;
	  parse_data_source(read_address, &config->data_handle, &config->data_handle_param);
		
		return config;
}

static void* parse_screen_control_text(uint32_t *read_address) {
    uint8_t format = get_next_byte(read_address);
    uint8_t x = get_next_byte(read_address);
    uint8_t y = get_next_byte(read_address);
    uint8_t width = get_next_byte(read_address);
    uint8_t height = get_next_byte(read_address);
	
	  TEXT_CONTROL_DATA* data = malloc(sizeof(TEXT_CONTROL_DATA));
	  
	  SCR_CONTROL_TEXT_CONFIG* config = malloc(sizeof(SCR_CONTROL_TEXT_CONFIG));
	
		config->format = 0;
		config->x = x;
		config->y = y;
		config->width = width;
		config->height = height;
		config->data = data;
	  parse_data_source(read_address, &config->data_handle, &config->data_handle_param);
		
		return config;
}

static bool parse_controls(uint32_t *read_address) {
	  uint8_t controls_no = get_next_byte(read_address);
	
	  //alloc memory for controls
	  controls.controls_no = controls_no;
	  controls.controls = malloc(sizeof(SCR_CONTROL_DEFINITION)*controls_no);
	
		for (int i = 0; i < controls_no; i++) {
			  uint8_t control_type = get_next_byte(read_address);
			  controls.controls[i].type = control_type;
			  switch (control_type) {
					case SCR_CONTROL_NUMBER:
						  controls.controls[i].config = parse_screen_control_number(read_address);
					    break;
					case SCR_CONTROL_TEXT:
						  controls.controls[i].config = parse_screen_control_text(read_address);
					    break;
				}
		}
		return true;
}

static void parse_actions(uint32_t *read_address) {
	  uint8_t actions_no = get_next_byte(read_address);
	
	  for(int i=0; i<actions_no; i++) {
			  uint8_t event = get_next_byte(read_address);
			  action_handlers[event].id = get_next_byte(read_address);
			  action_handlers[event].parameter = get_next_short(read_address);
		}
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
						  parse_actions(&read_address);
							break;
				}
	  } while (section != WATCH_SET_END_OF_DATA);
		return true;
}

static bool init_screen(uint8_t screen_id) {
	  uint32_t read_address = screens_section_address;
	  uint8_t screens_no = get_next_byte(&read_address);
	
	  if (read_address == 0) {
				return false;
		}
	
	  if (screen_id >= screens_no) {
		    return false;
	  }
		
		// reset action handlers
		memset(action_handlers, 0, 8 * sizeof(FUNCTION));
		
		// jump to screen offset
		read_address += 2 * screen_id;
		uint16_t screen_offset = get_next_short(&read_address);
		return parse_screen(screens_section_address + screen_offset);
}

static void parse_external_properties() {
	  uint32_t read_address = external_properties_section_address;
	  external_properties_no = get_next_byte(&read_address);
	  uint16_t ptr = external_properties_no*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE;
	  uint8_t ptr_array[external_properties_no*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE];
	  for (int i=0; i<external_properties_no; i++) {
			  uint8_t type = get_next_byte(&read_address);
			  uint8_t range = get_next_byte(&read_address);
			  
			  ptr_array[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE] = type;
			  ptr_array[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+1] = range;
			  ptr_array[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+2] = ptr >> 8;
			  ptr_array[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+3] = ptr & 0xFF;
			
			  ptr+=calc_ext_property_size(type, range);
		}
			
	  external_properties_data = malloc(ptr);
		memcpy(external_properties_data, ptr_array, external_properties_no*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE);
}

static void scr_watch_set_init() {
	  uint32_t read_address = 0x1C00;
	
	  mlcd_fb_clear();
	
	  uint8_t section;
	  while ((section = get_next_byte(&read_address))!= WATCH_SET_END_OF_DATA){
				uint16_t section_size = get_next_short(&read_address);
				switch (section) {
						case WATCH_SET_SECTION_SCREENS:
								screens_section_address = read_address;
								break;
						
						case WATCH_SET_SECTION_EXTERNAL_PROPERTIES:
							  external_properties_section_address = read_address;
								break;
						
						case WATCH_SET_SECTION_STATIC_CONTENT:
								break;
				}
				read_address+=section_size;
	  };
		parse_external_properties();
		init_screen(0);
		scr_controls_draw(&controls);
}

static void scr_watch_set_clear_screen_data() {
	  if (controls.controls_no == 0 || controls.controls == NULL) {
			  return;
		}
		
	  for (int i=0; i<controls.controls_no; i++) {
			  switch (controls.controls[i].type){
					  case SCR_CONTROL_NUMBER:
							  free(((SCR_CONTROL_NUMBER_CONFIG *)controls.controls[i].config)->data);
						    break;
				}
		    free(controls.controls[i].config);
		}
		free(controls.controls);
		controls.controls_no = 0;
		controls.controls = NULL;
}

void scr_watch_set_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_watch_set_init();
				    break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_watch_set_refresh_screen();
            break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_watch_set_handle_button_pressed(event_param);
				    break;
			  case SCR_EVENT_BUTTON_LONG_PRESSED:
				    scr_watch_set_handle_button_long_pressed(event_param);
				    break;
			  case SCR_EVENT_DESTROY_SCREEN:
				    scr_watch_set_clear_screen_data();
				    if (external_properties_data != NULL) {
								free(external_properties_data);
							  external_properties_data = NULL;
						}
				    break;
		}
}

void scr_watch_set_invoke_function(const FUNCTION* function) {
	  if (function->id == WATCH_SET_FUNC_EXTENSION) {
			  scr_watch_set_invoke_external_function(function->parameter);
	  } else {
		    scr_watch_set_invoke_internal_function(function->id, function->parameter);
	  }
}

void scr_watch_set_invoke_internal_function(uint8_t function_id, uint16_t param) {
	  switch(function_id) {
			  case WATCH_SET_FUNC_TOGGLE_BACKLIGHT:
				    mlcd_backlight_toggle();
			      break;
			  case WATCH_SET_FUNC_TOGGLE_COLORS:
				    mlcd_colors_toggle();
			      break;
			  case WATCH_SET_FUNC_SHOW_SETTINGS:
				    scr_mngr_show_screen(SCR_SETTINGS);
			      break;
				case WATCH_SET_FUNC_CLOSE:
					  scr_mngr_show_screen(SCR_WATCHFACE);
					  break;
			  case WATCH_SET_FUNC_CHANGE_SCREEN:
					  scr_watch_set_clear_screen_data();
	          mlcd_fb_clear();
				    init_screen(param);
						scr_controls_draw(&controls);
			      break;
		}
}

void scr_watch_set_invoke_external_function(uint8_t function_id) {
	  ble_peripheral_invoke_external_function(function_id);
}

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
#include "../ble/ble_central.h"
#include "../battery.h"
#include "../spiffs/spiffs.h"
#include <stdlib.h> 

uint32_t screens_section_address;
uint32_t current_screen_controls_address;
uint32_t external_properties_section_address;
uint32_t resources_section_address;
uint8_t external_properties_no = 0;
uint8_t* external_properties_data = NULL;
//SCR_CONTROLS_DEFINITION controls;
uint8_t* screen_data_buffer = NULL;
uint32_t checkpoint;

uint8_t current_subscreen = 0;
uint8_t switch_to_subscreen = 0;

static spiffs_file watchset_fd;
extern spiffs fs;

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

static uint8_t get_next_byte() {
    uint8_t data;
		SPIFFS_read(&fs, watchset_fd, &data, 1);
	  return data;
}

static uint16_t get_next_short() {
    uint8_t data[2];
		SPIFFS_read(&fs, watchset_fd, data, 2);
	  return data[0] << 8 | data[1];
}

static uint32_t get_next_int() {
    uint8_t data[4];
		SPIFFS_read(&fs, watchset_fd, data, 4);
	  return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

static uint32_t (* const internal_data_source_handles[])(void) = {
		/* 0 */ battery_get_level,
		/* 1 */ rtc_get_current_hour_24,
		/* 2 */ rtc_get_current_hour_12,
		/* 3 */ rtc_get_current_hour_12_designator,
		/* 4 */ rtc_get_current_minutes,
		/* 5 */ rtc_get_current_seconds,
		/* 6 */ rtc_get_current_day_of_week,
		/* 7 */ rtc_get_current_day_of_month,
		/* 8 */ rtc_get_current_day_of_year,
		/* 9 */ rtc_get_current_month,
		/* 10 */ rtc_get_current_year,
};

static uint32_t (* const sensor_data_source_handles[])(void) = {
		/* 0 */ ble_central_heart_rate
};

static uint32_t internal_data_source_get_value(uint32_t data_source_id) {
	  return internal_data_source_handles[data_source_id]();
}

static uint32_t sensor_data_source_get_value(uint32_t data_source_id) {
	  return sensor_data_source_handles[data_source_id]();
}

static uint8_t calc_ext_property_size(uint8_t type, uint8_t range) {
	  switch(type) {
			  case WATCH_SET_EXT_PROP_TYPE_NUMBER:
					  return range >> 5;
				case WATCH_SET_EXT_PROP_TYPE_STRING:
					  return range + 1;
		}
		return 0;
}

static uint32_t pow(uint32_t x, uint8_t n) {
	  uint32_t result = 1;
	  for(uint32_t i = 0; i < n; i++) {
			  result *= x;
		}
		return result;
}

static uint32_t external_data_source_get_property_value(uint32_t property_id, uint8_t expected_range) {	  
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
				{
						number_size = range>>5;
				    uint32_t result;
				    switch (number_size) {
							case 1:
								  result = external_properties_data[offset];
									break;
							case 2:
								  result = external_properties_data[offset] << 8 | 
										external_properties_data[offset+1];
									break;
							case 3:
								  result = (external_properties_data[offset] << 16 | 
										  external_properties_data[offset+1] << 8 | 
									    external_properties_data[offset+2]);
									break;
						  default:
										result = 0;
						}
						uint8_t decimal_size = range&0x1F;
						uint8_t expected_decimal_size = expected_range&0xF;
						if (expected_decimal_size > decimal_size) {
							  return result * pow(10, expected_decimal_size - decimal_size);
						} else if (expected_decimal_size < decimal_size) {
							  uint32_t divider = pow(10, decimal_size - expected_decimal_size);
							  // divide and round half up
							  return (result + (divider>>1)) / divider;
						}
						return result;
				}
			  case WATCH_SET_EXT_PROP_TYPE_STRING:
		        return (uint32_t)&external_properties_data[offset];
			  default:
				    return NULL;
		}
}

static void parse_data_source(void **data_source, uint32_t* data_source_param) {
    uint8_t type = get_next_byte();
    uint8_t property = get_next_byte();
	  switch (type) {
			  case DATA_SOURCE_INTERNAL:
			      *data_source = internal_data_source_get_value;
				    *data_source_param = property;
				    break;
			  case DATA_SOURCE_SENSOR:
			      *data_source = sensor_data_source_get_value;
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

static void parse_screen_control_number(SCR_CONTROL_NUMBER_CONFIG* config) {
    uint8_t range = get_next_byte();
    uint8_t x = get_next_byte();
    uint8_t y = get_next_byte();
    uint32_t style = get_next_int();
	
	
	  //NUMBER_CONTROL_DATA* data = malloc(sizeof(NUMBER_CONTROL_DATA));
	  //memset(data, 0, sizeof(NUMBER_CONTROL_DATA));
	  
	  //SCR_CONTROL_NUMBER_CONFIG* config = malloc(sizeof(SCR_CONTROL_NUMBER_CONFIG));
	
		config->range = range;
		config->x = x;
		config->y = y;
	
	  parse_data_source((void **)&config->data_handle, &config->data_handle_param);
		uint16_t dataPtr = get_next_short();
		config->data = (NUMBER_CONTROL_DATA*)(screen_data_buffer + dataPtr);
	
		checkpoint = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
				
		if ((style>>30)==1) {
				//uint8_t res_type = style >> 8 & 0xFF;
				uint8_t res_id = style & 0xFF;
				config->style = (style & 0xFF000000) | watchset_fd;
			
				SPIFFS_lseek(&fs, watchset_fd, resources_section_address+1+(3*res_id), SPIFFS_SEEK_SET);
				uint8_t offset_t[3];
				SPIFFS_read(&fs, watchset_fd, offset_t, 3);
				uint32_t offset = offset_t[0]<<16 | offset_t[1]<<8 | offset_t[2];
				SPIFFS_lseek(&fs, watchset_fd, resources_section_address+offset, SPIFFS_SEEK_SET);
		} else {
				config->style = style;
		}
}

static void parse_screen_control_static_image(SCR_CONTROL_STATIC_IMAGE_CONFIG* config) {
    uint8_t x = get_next_byte();
    uint8_t y = get_next_byte();
    uint8_t width = get_next_byte();
    uint8_t height = get_next_byte();
    uint8_t res_source = get_next_byte();
    uint8_t res_id = get_next_byte();
	
		checkpoint = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
	
		SPIFFS_lseek(&fs, watchset_fd, resources_section_address+1+(3*res_id), SPIFFS_SEEK_SET);
		uint8_t offset_t[3];
		SPIFFS_read(&fs, watchset_fd, offset_t, 3);
		uint32_t offset = offset_t[0]<<16 | offset_t[1]<<8 | offset_t[2];
		SPIFFS_lseek(&fs, watchset_fd, resources_section_address+offset, SPIFFS_SEEK_SET);
	
		config->x = x;
		config->y = y;
		config->width = width;
		config->height = height;
		config->file = watchset_fd;
}

static void parse_screen_control_image_from_set(SCR_CONTROL_IMAGE_FROM_SET_CONFIG* config) {
    uint8_t x = get_next_byte();
    uint8_t y = get_next_byte();
    uint8_t width = get_next_byte();
    uint8_t height = get_next_byte();
    uint8_t res_source = get_next_byte();
    uint8_t res_id = get_next_byte();
	
	  parse_data_source((void **)&config->data_handle, &config->data_handle_param);
		uint16_t dataPtr = get_next_short();
		config->data = (NUMBER_CONTROL_DATA*)(screen_data_buffer + dataPtr);
	
		checkpoint = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
	
		SPIFFS_lseek(&fs, watchset_fd, resources_section_address+1+(3*res_id), SPIFFS_SEEK_SET);
		uint8_t offset_t[3];
		SPIFFS_read(&fs, watchset_fd, offset_t, 3);
		uint32_t offset = offset_t[0]<<16 | offset_t[1]<<8 | offset_t[2];
		SPIFFS_lseek(&fs, watchset_fd, resources_section_address+offset, SPIFFS_SEEK_SET);
	
		config->x = x;
		config->y = y;
		config->width = width;
		config->height = height;
		config->file = watchset_fd;
}

static void* parse_screen_control_text(SCR_CONTROL_TEXT_CONFIG* config) {
    uint8_t x = get_next_byte();
    uint8_t y = get_next_byte();
    uint8_t width = get_next_byte();
    uint8_t height = get_next_byte();
    uint32_t style = get_next_int();
	
	  //TEXT_CONTROL_DATA* data = malloc(sizeof(TEXT_CONTROL_DATA));
	  //memset(data, 0, sizeof(TEXT_CONTROL_DATA));
	  
	  //SCR_CONTROL_TEXT_CONFIG* config = malloc(sizeof(SCR_CONTROL_TEXT_CONFIG));
	
		config->x = x;
		config->y = y;
		config->width = width;
		config->height = height;
		config->style = style;
		//config->data = data;
	  parse_data_source((void **)&config->data_handle, &config->data_handle_param);
		uint16_t dataPtr = get_next_short();
		config->data->last_value = (char*)(screen_data_buffer + dataPtr);
		return config;
}

static void parse_screen_control_progress(SCR_CONTROL_PROGRESS_BAR_CONFIG* config) {
    uint32_t max_value = get_next_int();
    uint8_t x = get_next_byte();
    uint8_t y = get_next_byte();
    uint8_t width = get_next_byte();
    uint8_t height = get_next_byte();
    uint32_t style = get_next_int();
	
	  //NUMBER_CONTROL_DATA* data = malloc(sizeof(NUMBER_CONTROL_DATA));
	  //memset(data, 0, sizeof(NUMBER_CONTROL_DATA));
	  
	  //SCR_CONTROL_PROGRESS_BAR_CONFIG* config = malloc(sizeof(SCR_CONTROL_PROGRESS_BAR_CONFIG));
	
	  config->max = max_value;
		config->x = x;
		config->y = y;
		config->width = width;
		config->height = height;
		config->style = style;
		//config->data = data;
	  parse_data_source((void **)&config->data_handle, &config->data_handle_param);
		uint16_t dataPtr = get_next_short();
		config->data = (NUMBER_CONTROL_DATA*)(screen_data_buffer + dataPtr);
}

static bool draw_screen_controls(bool force) {
	  uint8_t controls_no = get_next_byte();

		for (int i = 0; i < controls_no; i++) {
			  uint8_t control_type = get_next_byte();
			  switch (control_type) {
					case SCR_CONTROL_NUMBER:
					{
							SCR_CONTROL_NUMBER_CONFIG config;
						  parse_screen_control_number(&config);
							scr_controls_draw_number_control(&config, force);
							SPIFFS_lseek(&fs, watchset_fd, checkpoint, SPIFFS_SEEK_SET);
					}
					    break;
					case SCR_CONTROL_TEXT:
					{
							SCR_CONTROL_TEXT_CONFIG config;
							TEXT_CONTROL_DATA data;
							config.data = &data;
						  parse_screen_control_text(&config);
							scr_controls_draw_text_control(&config, force);
					}
					    break;
					case SCR_CONTROL_PROGRESS_BAR:
					{
							SCR_CONTROL_PROGRESS_BAR_CONFIG config;
						  parse_screen_control_progress(&config);
							scr_controls_draw_progress_bar_control(&config, force);
					}
					    break;
					case SCR_CONTROL_STATIC_IMAGE:
					{
							SCR_CONTROL_STATIC_IMAGE_CONFIG config;
						  parse_screen_control_static_image(&config);
							scr_controls_draw_static_image_control(&config, force);
							SPIFFS_lseek(&fs, watchset_fd, checkpoint, SPIFFS_SEEK_SET);
					}
					    break;
					case SCR_CONTROL_IMAGE_FROM_SET:
					{
							SCR_CONTROL_IMAGE_FROM_SET_CONFIG config;
						  parse_screen_control_image_from_set(&config);
							scr_controls_draw_image_from_set_control(&config, force);
							SPIFFS_lseek(&fs, watchset_fd, checkpoint, SPIFFS_SEEK_SET);
					}
					    break;
				}
		}
		return true;
}

static void parse_actions() {
	  uint8_t actions_no = get_next_byte();
	
	  for(int i=0; i<actions_no; i++) {
			  uint8_t event = get_next_byte();
			  action_handlers[event].id = get_next_byte();
			  action_handlers[event].parameter = get_next_short();
		}
}

static bool parse_screen() {
	  uint8_t section;
	  do {
				section = get_next_byte();
				switch (section) {
					case WATCH_SET_SCREEN_SECTION_CONTROLS:
					{
							uint32_t size = get_next_short();
							current_screen_controls_address = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
							SPIFFS_lseek(&fs, watchset_fd, size, SPIFFS_SEEK_CUR);
					}
						  //parse_controls(true);
							break;
					case WATCH_SET_SCREEN_SECTION_ACTIONS:
						  parse_actions();
							break;
					case WATCH_SET_SCREEN_SECTION_MEMORY:
					{
						  uint16_t buffer_size = get_next_short();
							screen_data_buffer = malloc(buffer_size);
					}
							break;
				}
	  } while (section != WATCH_SET_END_OF_DATA);
		return true;
}

static bool init_subscreen(uint8_t screen_id) {
	
	  if (screens_section_address == 0) {
				return false;
		}
		
		SPIFFS_lseek(&fs, watchset_fd, screens_section_address, SPIFFS_SEEK_SET);
	  //uint32_t read_address = screens_section_address;
	  uint8_t screens_no = get_next_byte();
	
	  if (screen_id >= screens_no) {
		    return false;
	  }
		
		// reset action handlers
		memset(action_handlers, 0, 8 * sizeof(FUNCTION));
		
		// jump to screen offset
		SPIFFS_lseek(&fs, watchset_fd, 2 * screen_id, SPIFFS_SEEK_CUR);
		uint16_t screen_offset = get_next_short();
		
		SPIFFS_lseek(&fs, watchset_fd, screens_section_address + screen_offset, SPIFFS_SEEK_SET);
		return parse_screen();
}

static void parse_external_properties() {
		SPIFFS_lseek(&fs, watchset_fd, external_properties_section_address, SPIFFS_SEEK_SET);
	  external_properties_no = get_next_byte();
	  uint16_t ptr = external_properties_no*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE;
	  uint8_t ptr_array[external_properties_no*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE];
	  for (int i=0; i<external_properties_no; i++) {
			  uint8_t type = get_next_byte();
			  uint8_t range = get_next_byte();
			  
			  ptr_array[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE] = type;
			  ptr_array[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+1] = range;
			  ptr_array[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+2] = ptr >> 8;
			  ptr_array[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+3] = ptr & 0xFF;
			
			  ptr+=calc_ext_property_size(type, range);
		}
			
	  external_properties_data = malloc(ptr);
		memset(external_properties_data, 0, ptr);
		memcpy(external_properties_data, ptr_array, external_properties_no*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE);
}

static void scr_watch_set_init() {
    watchset_fd = SPIFFS_open(&fs, "watchset", SPIFFS_RDONLY, 0);
		if (watchset_fd < 0) {
			  screens_section_address = NULL;
			  external_properties_section_address = NULL;
			  scr_mngr_show_screen(SCR_WATCHFACE);
			  return;
		}
	
	  current_subscreen = 0;
	  switch_to_subscreen = 0;
	
	  uint32_t watchset_id = get_next_int();
	
	  uint8_t section;
	  while ((section = get_next_byte())!= WATCH_SET_END_OF_DATA){
				uint16_t section_size = get_next_short();
				switch (section) {
						case WATCH_SET_SECTION_SCREENS:
								screens_section_address = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
								break;
						
						case WATCH_SET_SECTION_EXTERNAL_PROPERTIES:
							  external_properties_section_address = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
								break;
						
						case WATCH_SET_SECTION_STATIC_CONTENT:
							  resources_section_address = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
								break;
				}
				SPIFFS_lseek(&fs, watchset_fd, section_size, SPIFFS_SEEK_CUR);
	  };
		parse_external_properties();
		
		// send info to watch to send property values
		ble_peripheral_set_watch_set_id(watchset_id);
		
		init_subscreen(current_subscreen);
}

static void scr_watch_set_draw_screen() {
		SPIFFS_lseek(&fs, watchset_fd, current_screen_controls_address, SPIFFS_SEEK_SET);
		draw_screen_controls(true);
}

static void clear_subscreen_data() {
	 /* if (controls.controls_no == 0 || controls.controls == NULL) {
			  return;
		}
		
	  for (int i=0; i<controls.controls_no; i++) {
			  switch (controls.controls[i].type){
					  case SCR_CONTROL_NUMBER:
							  free(((SCR_CONTROL_NUMBER_CONFIG *)controls.controls[i].config)->data);
						    break;
					  case SCR_CONTROL_TEXT:
							  free(((SCR_CONTROL_TEXT_CONFIG *)controls.controls[i].config)->data);
						    break;
					  case SCR_CONTROL_HORIZONTAL_PROGRESS_BAR:
							  free(((SCR_CONTROL_PROGRESS_BAR_CONFIG *)controls.controls[i].config)->data);
						    break;
				}
		    free(controls.controls[i].config);
		}
		free(controls.controls);
		controls.controls_no = 0;
		controls.controls = NULL;*/
		if (screen_data_buffer != NULL) {
				free(screen_data_buffer);
		}
		screen_data_buffer = NULL;
}

static void scr_watch_set_refresh_screen() {
	  if (current_subscreen != switch_to_subscreen) {
				clear_subscreen_data();
	      mlcd_fb_clear();
			  init_subscreen(switch_to_subscreen);
			
				SPIFFS_lseek(&fs, watchset_fd, current_screen_controls_address, SPIFFS_SEEK_SET);
				draw_screen_controls(true);
			
			  current_subscreen = switch_to_subscreen;
		} else {
				SPIFFS_lseek(&fs, watchset_fd, current_screen_controls_address, SPIFFS_SEEK_SET);
				draw_screen_controls(false);
	  }
}

void scr_watch_set_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_watch_set_init();
				    break;
			  case SCR_EVENT_DRAW_SCREEN:
				    scr_watch_set_draw_screen();
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
						ble_peripheral_set_watch_set_id(0);
				    clear_subscreen_data();		
						SPIFFS_close(&fs, watchset_fd); 
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
					  switch_to_subscreen = param;
			      break;
		}
}

void scr_watch_set_invoke_external_function(uint8_t function_id) {
	  ble_peripheral_invoke_external_function(function_id);
}

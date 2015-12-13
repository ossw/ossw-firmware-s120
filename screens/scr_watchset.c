#include <string.h>
#include "scr_watchset.h"
#include "nrf_delay.h"
#include "time.h"
#include "../scr_mngr.h"
#include "../scr_controls.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../ext_ram.h"
#include "../ext_flash.h"
#include "../utf8.h"
#include "../pawn/amxutil.h"
#include "../i18n/i18n.h"
#include "../ble/ble_peripheral.h"
#include "../spiffs/spiffs.h"
#include "../fs.h"
#include "../watchset.h"
#include "../config.h"
#include <stdlib.h> 

struct model_property {
		int16_t value;
		int16_t min;
		int16_t max;
		bool overflow;
};

//struct scr_watchset_data {
		uint32_t screens_section_address;
		uint32_t external_properties_section_address;
		uint32_t resources_section_address;
		uint32_t ws_data_ptr;
		uint8_t external_properties_no = 0;
		uint8_t* external_properties_data = NULL;
		uint8_t* screen_data_buffer = NULL;
		uint8_t* actions_data_buffer = NULL;
		struct model_property* model_data_buffer = NULL;
		uint32_t checkpoint;

		uint8_t current_subscreen = 0;

		spiffs_file watchset_fd;
		uint16_t action_handlers[9];
		uint32_t watchset_id;
		bool (* base_actions_handler)(uint32_t event_type, uint32_t event_param);
		bool force_colors = false;
		uint8_t show_screen_param = 0xFF;
		bool lock_actions = false;
		uint16_t ext_prop_offset;
//}

static bool parse_screen_controls(bool force);

static void clear_subscreen_data() {
		if (screen_data_buffer != NULL) {
				free(screen_data_buffer);
				screen_data_buffer = NULL;
		}
		if (actions_data_buffer != NULL) {
				free(actions_data_buffer);
				actions_data_buffer = NULL;
		}
		if (model_data_buffer != NULL) {
				free(model_data_buffer);
				model_data_buffer = NULL;
		}
}

static void clean_before_exit() {
		watchset_id = NULL;
		ble_peripheral_set_watch_set_id(NULL);
	
		SPIFFS_close(&fs, watchset_fd); 

	  clear_subscreen_data();		
		if (external_properties_data != NULL) {
				free(external_properties_data);
			  external_properties_data = NULL;
		}
}

void close(void) {
		clean_before_exit();
		scr_mngr_show_screen(SCR_WATCHFACE);
}

void handle_error(void) {
		if (watchset_is_watch_face()) {
				config_clear_dafault_watch_face();
		}
		close();
}

static uint8_t get_next_byte(void) {
    uint8_t data;
		SPIFFS_read(&fs, watchset_fd, &data, 1);
	  return data;
}

static uint16_t get_next_short(void) {
    uint8_t data[2];
		SPIFFS_read(&fs, watchset_fd, data, 2);
	  return data[0] << 8 | data[1];
}

static uint32_t get_next_int(void) {
    uint8_t data[4];
		SPIFFS_read(&fs, watchset_fd, data, 4);
	  return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

static uint8_t ws_data_get_next_byte(void) {
    uint8_t data;
		ext_ram_read_data(ws_data_ptr, &data, 1);
		ws_data_ptr++;
	  return data;
}

static uint16_t ws_data_get_next_short() {
    uint8_t data[2];
		ext_ram_read_data(ws_data_ptr, data, 2);
		ws_data_ptr+=2;
	  return data[0] << 8 | data[1];
}

static uint32_t ws_data_get_next_int() {
    uint8_t data[4];
		ext_ram_read_data(ws_data_ptr, data, 4);
		ws_data_ptr+=4;
	  return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

static inline void ws_data_read(uint8_t *data, uint32_t size) {
		ext_ram_read_data(ws_data_ptr, data, size);
		ws_data_ptr += size;
}

static inline void ws_data_skip(uint32_t size) {
		ws_data_ptr += size;
}

static uint32_t pow(uint32_t x, uint8_t n) {
	  uint32_t result = 1;
	  for(uint32_t i = 0; i < n; i++) {
			  result *= x;
		}
		return result;
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

static uint32_t model_data_source_get_property_value(uint32_t property_id, uint8_t expected_range) {	 
		return model_data_buffer[property_id].value;
}

static uint32_t pass_argument_data_source_get_property_value(uint32_t value, uint8_t expected_range) {	 
		return value;
}

static uint32_t external_data_source_get_property_value(uint32_t property_id, uint8_t expected_range, uint8_t* data, bool* has_changed) {	  
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
						uint8_t buffer[4];
						ext_ram_read_data(ext_prop_offset + offset, buffer, number_size);
				    switch (number_size) {
							case 1:
								  result = buffer[0];
									break;
							case 2:
								  result = buffer[0] << 8 | 
										buffer[1];
									break;
							case 3:
								  result = (buffer[0] << 16 | 
										  buffer[1] << 8 | 
									    buffer[2]);
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
			  case WATCH_SET_EXT_PROP_TYPE_STRING: {
						ext_ram_read_text(ext_prop_offset + offset, data, range, has_changed);
		        return NULL;
				}
			  default:
				    return NULL;
		}
}

static void parse_data_source(void **data_source, uint32_t* data_source_param, uint8_t* data_cache) {
    uint8_t type = ws_data_get_next_byte();
    uint8_t property = ws_data_get_next_byte();
		  switch (type&0x3F) {
			  case DATA_SOURCE_STATIC:
			      *data_source = watchset_static_data_source_get_value;
						// property = static data length
						ws_data_read(data_cache, property);
						data_cache[property] = 0;
				    *data_source_param = (uint32_t)data_cache;
				    break;
			  case DATA_SOURCE_INTERNAL:
			      *data_source = watchset_internal_data_source_get_value;
				    *data_source_param = property;
				    break;
			  case DATA_SOURCE_SENSOR:
			      *data_source = watchset_sensor_data_source_get_value;
				    *data_source_param = property;
				    break;
			  case DATA_SOURCE_EXTERNAL:
			      *data_source = external_data_source_get_property_value;
						*data_source_param = property;
			      //*data_source = pass_argument_data_source_get_property_value;
				    //*data_source_param = external_data_source_get_property_value(property, 0, data_cache);
				    break;
			  case DATA_SOURCE_MODEL:
			      *data_source = model_data_source_get_property_value;
				    *data_source_param = property;
				    break;
		}
		if (type & 0x40) {
				//parse index
				uint32_t (* data_handle)(uint32_t);
				uint32_t data_handle_param;
				uint32_t static_data;
				parse_data_source((void **)&data_handle, &data_handle_param, (void *)&static_data);
				int index = data_handle(data_handle_param);
				*data_source_param = (*data_source_param) | index<<8;
		} 
		if (type & 0x80) {
				uint8_t converter_no = ws_data_get_next_byte();
			  uint32_t value = ((uint32_t (*)(uint32_t, uint8_t))*data_source)(*data_source_param, 0);
				for (int i = 0; i < converter_no; i++) {
						uint8_t converter_id = ws_data_get_next_byte();
						value = ((uint32_t (*)(uint32_t))watchset_get_converter(converter_id))(value);
				}
			  *data_source = pass_argument_data_source_get_property_value;
				*data_source_param = value;
		}
}

static int parse_data_source_value() {
    uint8_t type = get_next_byte();
    uint8_t property = get_next_byte();
	
		int value = 0;
	  switch (type&0x3F) {
			  case DATA_SOURCE_STATIC:
						//get_next_byte();
			      value = get_next_int();
				    break;
			  case DATA_SOURCE_INTERNAL:
			      value = watchset_internal_data_source_get_value(property, 0, NULL, NULL);
				    break;
			  case DATA_SOURCE_SENSOR:
			      value = watchset_sensor_data_source_get_value(property, 0);
				    break;
			 /* case DATA_SOURCE_EXTERNAL:
			      value = external_data_source_get_property_value(property, 0);
				    break;*/
			  case DATA_SOURCE_MODEL:
			      value = model_data_source_get_property_value(property, 0);
				    break;
		}
		if (type & 0x40) {
					parse_data_source_value();
		}
		if (type & 0x80) {
				uint8_t converter_no = get_next_byte();
				for (int i = 0; i < converter_no; i++) {
						uint8_t converter_id = get_next_byte();
						value = ((uint32_t (*)(uint32_t))watchset_get_converter(converter_id))(value);
				}
		}
		return value;
}

static uint32_t get_next_byte_from_array(uint8_t** data) {
		return *((*data)++);
}

static void parse_data_source_value_from_array(uint8_t** data, void* buf) {
    uint8_t type = get_next_byte_from_array(data);
    uint8_t property = get_next_byte_from_array(data);
	
	  switch (type&0x3F) {
			  case DATA_SOURCE_STATIC:
			      *((uint32_t*)buf) = get_next_byte_from_array(data)<<24|get_next_byte_from_array(data)<<16|get_next_byte_from_array(data)<<8|get_next_byte_from_array(data);
				    break;
			  case DATA_SOURCE_INTERNAL:
			     *((uint32_t*)buf) = watchset_internal_data_source_get_value(property, 0, buf, NULL);
				    break;
			  case DATA_SOURCE_SENSOR:
			      *((uint32_t*)buf) = watchset_sensor_data_source_get_value(property, 0);
				    break;
		/*	  case DATA_SOURCE_EXTERNAL:
			      *((uint32_t*)buf) = external_data_source_get_property_value(property, 0);
				    break;*/
			  case DATA_SOURCE_MODEL:
			      *((uint32_t*)buf) = model_data_source_get_property_value(property, 0);
				    break;
		}
		if (type & 0x40) {
				// index
				uint32_t value;
				parse_data_source_value_from_array(data, &value);
		}
		if (type & 0x80) {
				// converter
				uint8_t converter_no = get_next_byte_from_array(data);
				for (int i = 0; i < converter_no; i++) {
						uint8_t converter_id = get_next_byte_from_array(data);
						*((uint32_t*)buf) = ((uint32_t (*)(uint32_t))watchset_get_converter(converter_id))(*((uint32_t*)buf));
				}
		}
}
/*
static void parse_data_source_text_value_from_array(uint8_t** data, void* buf) {
    uint8_t type = get_next_byte_from_array(data);
    uint8_t property = get_next_byte_from_array(data);
	
	  switch (type&0x3F) {
			  case DATA_SOURCE_STATIC:
						memcpy(buf, *data, property);
						((uint8_t*)buf)[property] = 0;
			      break;
		}
		if (type & 0x40) {
				// index
				uint32_t value;
				parse_data_source_value_from_array(data, &value);
				// skip index
		}
		if (type & 0x80) {
				// converter
				uint8_t converter_no = get_next_byte_from_array(data);
				for (int i = 0; i < converter_no; i++) {
						uint8_t converter_id = get_next_byte_from_array(data);
						// skip converter
				}
		}
}
*/
void set_external_property_data(uint8_t property_id, uint8_t* data_ptr, uint8_t size) {
	  if (external_properties_data == NULL) {
			  return;
		}
		if (property_id >= external_properties_no) {
			  return;
		}
		uint16_t offset = external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+2] << 8;
		uint8_t type = external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE];
		offset |= external_properties_data[property_id*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+3];
		
		ext_ram_write_data(ext_prop_offset + offset, data_ptr, size);
		if (type == WATCH_SET_EXT_PROP_TYPE_STRING) {
			  // end of string char
				uint8_t zero = 0;
			  ext_ram_write_data(ext_prop_offset + offset + size, &zero, 1);
		}
/*		memcpy(&external_properties_data[offset], data_ptr, size);
		if (type == WATCH_SET_EXT_PROP_TYPE_STRING) {
			  // end of string char
			  external_properties_data[offset + size] = 0;
		}*/
}

static bool parse_and_draw_screen_control_number(bool force) {
	
    uint8_t data[9];
		ws_data_read(data, sizeof(data));
	
		SCR_CONTROL_NUMBER_CONFIG config;
		config.range = data[0];
		config.x = data[1];
		config.y = data[2];
    uint32_t style = data[3]<<24|data[4]<<16|data[5]<<8|data[6];
		uint16_t dataPtr = data[7] << 8 | data[8];
		config.data = (NUMBER_CONTROL_DATA*)(screen_data_buffer + dataPtr);
		
	  parse_data_source((void **)&config.data_handle, &config.data_handle_param, NULL);

	
		uint8_t decimal_size = config.range&0xF;
	  uint32_t value = config.data_handle(config.data_handle_param, decimal_size, NULL, NULL);
		
		if (!force && config.data->last_value == value) {
				return false;
		}
				
		if ((style>>30)==1) {
				checkpoint = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
			
				//uint8_t res_type = style >> 8 & 0xFF;
				uint8_t res_id = style & 0xFF;
				config.style = (style & 0xFF000000) | watchset_fd;
			
				SPIFFS_lseek(&fs, watchset_fd, resources_section_address+1+(3*res_id), SPIFFS_SEEK_SET);
				uint8_t offset_t[3];
				SPIFFS_read(&fs, watchset_fd, offset_t, 3);
				uint32_t offset = offset_t[0]<<16 | offset_t[1]<<8 | offset_t[2];
				SPIFFS_lseek(&fs, watchset_fd, resources_section_address+offset, SPIFFS_SEEK_SET);
		} else {
				config.style = style;
		}
		
		scr_controls_draw_number_control(&config, force);
		
		
		if ((style>>30)==1) {
				SPIFFS_lseek(&fs, watchset_fd, checkpoint, SPIFFS_SEEK_SET);
		}
		return false;
}

static bool parse_and_draw_screen_control_static_image(bool force) {
	
    uint8_t data[6];
		ws_data_read(data, sizeof(data));
	
		if (!force) {
				return false;
		}
	
		SCR_CONTROL_STATIC_IMAGE_CONFIG config;
    uint8_t x = data[0];
    uint8_t y = data[1];
    uint8_t width = data[2];
    uint8_t height = data[3];
    //uint8_t res_source = data[4];
    uint8_t res_id = data[5];
	
		checkpoint = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
	
		SPIFFS_lseek(&fs, watchset_fd, resources_section_address+1+(3*res_id), SPIFFS_SEEK_SET);
		uint8_t offset_t[3];
		SPIFFS_read(&fs, watchset_fd, offset_t, 3);
		uint32_t offset = offset_t[0]<<16 | offset_t[1]<<8 | offset_t[2];
		SPIFFS_lseek(&fs, watchset_fd, resources_section_address+offset, SPIFFS_SEEK_SET);
	
		config.x = x;
		config.y = y;
		config.width = width;
		config.height = height;
		config.file = watchset_fd;
		
		scr_controls_draw_static_image_control(&config, force);
		SPIFFS_lseek(&fs, watchset_fd, checkpoint, SPIFFS_SEEK_SET);
		return false;
}

static bool parse_and_draw_screen_control_image_from_set(bool force) {
	
    uint8_t data[8];
		ws_data_read(data, sizeof(data));
	
		SCR_CONTROL_IMAGE_FROM_SET_CONFIG config;
	
    uint8_t x = data[0];
    uint8_t y = data[1];
    uint8_t width = data[2];
    uint8_t height = data[3];
    //uint8_t res_source = data[4];
    uint8_t res_id = data[5];
		uint16_t dataPtr = data[6]<<8 | data[7];
		config.data = (NUMBER_CONTROL_DATA*)(screen_data_buffer + dataPtr);
	  parse_data_source((void **)&config.data_handle, &config.data_handle_param, NULL);
		
	
	  uint32_t value = config.data_handle(config.data_handle_param, 0, NULL, NULL);
	
		if (!force && config.data->last_value == value) {
				return false;
		}
		
		checkpoint = SPIFFS_lseek(&fs, watchset_fd, 0, SPIFFS_SEEK_CUR);
	
		SPIFFS_lseek(&fs, watchset_fd, resources_section_address+1+(3*res_id), SPIFFS_SEEK_SET);
		uint8_t offset_t[3];
		SPIFFS_read(&fs, watchset_fd, offset_t, 3);
		uint32_t offset = offset_t[0]<<16 | offset_t[1]<<8 | offset_t[2];
		SPIFFS_lseek(&fs, watchset_fd, resources_section_address+offset, SPIFFS_SEEK_SET);
	
		config.x = x;
		config.y = y;
		config.width = width;
		config.height = height;
		config.file = watchset_fd;
		
		scr_controls_draw_image_from_set_control(&config, force);
		SPIFFS_lseek(&fs, watchset_fd, checkpoint, SPIFFS_SEEK_SET);
		return false;
}

static bool parse_and_draw_choose_control(bool force) {
		
	  uint32_t (* data_handle)(uint32_t, uint8_t);
	  uint32_t data_handle_param;
		uint32_t static_data;
		parse_data_source((void **)&data_handle, &data_handle_param, (void *)&static_data);
		int data_offset = ws_data_get_next_short();
		uint8_t* data_ptr = (uint8_t*)(screen_data_buffer + data_offset);
		uint32_t value = data_handle(data_handle_param, 0);
		
		if (!force && *data_ptr != value) {
				return true;
		}
		
		uint8_t options_no = ws_data_get_next_byte();
		bool handled = false;
		for (int i=0; i<options_no; i++) {
				uint8_t number_of_values = ws_data_get_next_byte();
			
				bool matched = false;
				for( int v = 0; v<number_of_values; v++) {
						uint8_t expected_value = ws_data_get_next_byte();
						if (!matched && value == expected_value) {
								matched = true;
						}
				}
				int block_size = ws_data_get_next_short();
				if (matched && !handled) {
						if (parse_screen_controls(force)) {
								return true;
						}
						handled = true;
				} else {
						ws_data_skip(block_size);
				}
		}
		int otherwise_block_size = ws_data_get_next_short();
		if (otherwise_block_size != 0) {
				if (handled) {
								ws_data_skip(otherwise_block_size);
				} else {
						if (parse_screen_controls(force)) {
								return true;
						}
				}
		}
		*data_ptr = value;
		return false;
}

static bool parse_and_draw_screen_control_text(bool force) {
	
		SCR_CONTROL_TEXT_CONFIG config;
		TEXT_CONTROL_DATA data;
		config.data = &data;
	
		config.x = ws_data_get_next_byte();
		config.y = ws_data_get_next_byte();
		config.width = ws_data_get_next_byte();
		config.height = ws_data_get_next_byte();
		config.style = ws_data_get_next_int();
		uint16_t dataPtr = ws_data_get_next_short();
		config.data->last_value = (char*)(screen_data_buffer + dataPtr);
	  parse_data_source((void **)&config.data_handle, &config.data_handle_param, (void *)config.data->last_value);
	
		scr_controls_draw_text_control(&config, force);
		return false;
}

static bool parse_and_draw_screen_control_progress(bool force) {
	
		SCR_CONTROL_PROGRESS_BAR_CONFIG config;

	  config.max = ws_data_get_next_int();
		config.x = ws_data_get_next_byte();
		config.y = ws_data_get_next_byte();
		config.width = ws_data_get_next_byte();
		config.height = ws_data_get_next_byte();
		config.style = ws_data_get_next_int();
		uint16_t dataPtr = ws_data_get_next_short();
		config.data = (NUMBER_CONTROL_DATA*)(screen_data_buffer + dataPtr);
	  parse_data_source((void **)&config.data_handle, &config.data_handle_param, NULL);
		
		scr_controls_draw_progress_bar_control(&config, force);
		return false;
}

static bool parse_screen_controls(bool force) {
	  uint8_t controls_no = ws_data_get_next_byte();
	
		for (int i = 0; i < controls_no; i++) {
			  uint8_t control_type = ws_data_get_next_byte();
			
	/*	#ifdef OSSW_DEBUG
				uint32_t start_control_ticks;
				app_timer_cnt_get(&start_control_ticks);
		#endif*/
			
			  switch (control_type) {
					case SCR_CONTROL_NUMBER:
						  if (parse_and_draw_screen_control_number(force)) {
									return true;
							}
					    break;
					case SCR_CONTROL_TEXT:
					{
						  if (parse_and_draw_screen_control_text(force)){
									return true;
							}
					}
					    break;
					case SCR_CONTROL_PROGRESS_BAR:
					{
						  if (parse_and_draw_screen_control_progress(force)) {
									return true;
							}
					}
					    break;
					case SCR_CONTROL_STATIC_IMAGE:
					{
						  if (parse_and_draw_screen_control_static_image(force)) {
									return true;
							}
					}
					    break;
					case SCR_CONTROL_IMAGE_FROM_SET:
					{
						  if (parse_and_draw_screen_control_image_from_set(force)) {
									return true;
							}
					}
					    break;
					case 0xF0: //choose
					{
						  if (parse_and_draw_choose_control(force)) {
									return true;
							}
					}
					    break;
				}
				
/*				#ifdef OSSW_DEBUG
						uint32_t end_control_ticks;
						uint32_t control_diff;
						app_timer_cnt_get(&end_control_ticks);
						app_timer_cnt_diff_compute(end_control_ticks, start_control_ticks, &control_diff);
						printf("CTRL 0x%02x: 0x%08x\r\n", control_type, control_diff);
				#endif*/
		}
		return false;
}

static bool draw_screen_controls(bool force, scr_mngr_draw_ctx* ctx) {
		ctx->force_colors = force_colors;
		ws_data_ptr = EXT_RAM_DATA_CURRENT_SCREEN_CACHE;
		return parse_screen_controls(force);
}

static bool parse_actions() {
		uint16_t size = get_next_short();
		uint8_t events_no = get_next_byte();
		size -= events_no*3+1;
		actions_data_buffer = malloc(size);
		if (actions_data_buffer == NULL) {
				handle_error();
				return false;
		}
	
		for (int i=0; i<events_no; i++) {
				 uint8_t event = get_next_byte();
				 uint16_t offset = get_next_short();
				if (event < 9) {
						action_handlers[event] = offset;
				} else if (event == 0xF0) {
						// init screen action
			/*			FUNCTION action;
						action.id = get_next_byte();
						action.parameter = get_next_short();
					  scr_watch_set_invoke_function(&action);*/
				}
			
		}
		SPIFFS_read(&fs, watchset_fd, actions_data_buffer, size);
	/*
	  uint8_t actions_no = get_next_byte();
	
	  for(int i=0; i<actions_no; i++) {
			
		}*/
		return true;
}

static void parse_base_actions() {
		uint8_t base_actions_id = get_next_byte();
		switch(base_actions_id) {
				case WATCH_SET_BASE_ACTIONS_WATCH_FACE:
						base_actions_handler = &watchset_default_watch_face_handle_event;
		}
}

static bool parse_model() {
		uint32_t size_left = get_next_short();
		uint8_t variables_no = get_next_byte();
	
		model_data_buffer = malloc(sizeof(struct model_property)*variables_no);
		if (model_data_buffer == NULL) {
				handle_error();
				return false;
		}
	
		for (int i=0; i<variables_no; i++) {
				uint8_t type = get_next_byte();//type
				uint8_t flags = get_next_byte();//flags

				if (flags & 0x80) {
						//read init value
						int v = parse_data_source_value();
						model_data_buffer[i].value = v;
				}
				model_data_buffer[i].overflow = (flags & 0x40) != 0;
				if (flags & 0x20) {
						model_data_buffer[i].max = parse_data_source_value();
				} else {
						model_data_buffer[i].max = 32000;
				}

				if (flags & 0x10) {
						model_data_buffer[i].min = parse_data_source_value();
				} else {
						model_data_buffer[i].min = -32000;
				}
		}
		return true;
}

static void parse_settings(void) {
		uint16_t size = get_next_short();
		uint8_t settings_no = get_next_byte();
		for (int i=0; i<settings_no; i++) {
				uint8_t setting_id = get_next_byte();
				uint8_t setting_value = get_next_byte();
				switch(setting_id) {
						case WATCH_SET_SETTING_INVERTIBLE:
								force_colors = !setting_value;
				}
		}
}

static bool parse_screen() {
	  uint8_t section;
		force_colors = false;
	  do {
				section = get_next_byte();
				switch (section) {
					case WATCH_SET_SCREEN_SECTION_MODEL:
					{
						  if (!parse_model()) {
									return false;
							}
					}
							break;
					case WATCH_SET_SCREEN_SECTION_CONTROLS:
					{
							uint32_t size_left = get_next_short();
							uint32_t dest_addr = EXT_RAM_DATA_CURRENT_SCREEN_CACHE;
						
							uint8_t buff[128];
							while (size_left > 0) {
									uint32_t chunk_size = size_left > 128? 128 : size_left;
									SPIFFS_read(&fs, watchset_fd, buff, chunk_size);
									ext_ram_write_data(dest_addr, buff, chunk_size);
									size_left -= chunk_size;
									dest_addr += chunk_size;
							}
					}
							break;
					case WATCH_SET_SCREEN_SECTION_ACTIONS:
					{
						  if (!parse_actions()) {
									return false;
							}
					}
							break;
					case WATCH_SET_SCREEN_SECTION_MEMORY:
					{
						  uint16_t buffer_size = get_next_short();
							screen_data_buffer = calloc(buffer_size, 1);
							if (screen_data_buffer == NULL) {
									handle_error();
									return false;
							}
					}
							break;
					case WATCH_SET_SCREEN_SECTION_BASE_ACTIONS:
						  parse_base_actions();
							break;
					case WATCH_SET_SCREEN_SECTION_SETTINGS:
							parse_settings();
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
		memset(action_handlers, 0xFF, 9 * sizeof(uint16_t));
		base_actions_handler = 0;
		
		// jump to screen offset
		SPIFFS_lseek(&fs, watchset_fd, 2 * screen_id, SPIFFS_SEEK_CUR);
		uint16_t screen_offset = get_next_short();
		
		SPIFFS_lseek(&fs, watchset_fd, screens_section_address + screen_offset, SPIFFS_SEEK_SET);
		return parse_screen();
}

static bool parse_external_properties() {
		SPIFFS_lseek(&fs, watchset_fd, external_properties_section_address, SPIFFS_SEEK_SET);
	  external_properties_no = get_next_byte();
	
	  uint16_t header_size = external_properties_no*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE;
	  external_properties_data = malloc(header_size);
		if (external_properties_data == NULL) {
				return false;
		}
		
		uint16_t ptr = 0;//header_size;
	  for (int i=0; i<external_properties_no; i++) {
			  uint8_t type = get_next_byte();
			  uint8_t range = get_next_byte();
			  
			  external_properties_data[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE] = type;
			  external_properties_data[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+1] = range;
			  external_properties_data[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+2] = ptr >> 8;
			  external_properties_data[i*WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE+3] = ptr & 0xFF;
			
			  ptr+=calc_ext_property_size(type, range);
		}
		ext_prop_offset = EXT_RAM_DATA_CURRENT_SCREEN_CACHE + EXT_RAM_DATA_CURRENT_SCREEN_CACHE_SIZE - ptr;
		ext_ram_fill(ext_prop_offset, 0, ptr);
/*	  void* after_realloc = realloc(external_properties_data, ptr);
		if (after_realloc == NULL) {
				return false;
		}
		external_properties_data = after_realloc; 
		memset(external_properties_data + header_size, 0, ptr - header_size);*/
		return true;
}

static void scr_watch_set_init(uint32_t param) {
		lock_actions = true;
		uint8_t mode = (param>>24)&0xF;
		
		watchset_set_watch_face(param & (1<<28));
		if (mode == 1) {
				struct spiffs_dirent entry;
				ext_ram_read_data(param&0xFFFFFF, (uint8_t*)&entry, sizeof(struct spiffs_dirent));
				watchset_fd = SPIFFS_open_by_dirent(&fs, &entry, SPIFFS_RDONLY, 0);
		} else if (mode == 2) {
				watchset_fd = param & 0xFFFF;
		}
		if (watchset_fd < 0) {
			  handle_error();
			  return;
		}
	
	  current_subscreen = 0;
	
	  int magic_number = get_next_short();
		if (magic_number != 0x0553) {
			  handle_error();
			  return;
		}
	  int api_version = get_next_short();
		if (api_version != 1) {
			  handle_error();
			  return;
		}
	  watchset_id = get_next_int();
	
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
		if (!parse_external_properties()){
				handle_error();
			  return;
		}
		
		// send info to watch to send property values
		ble_peripheral_set_watch_set_id(watchset_id);
		
		init_subscreen(current_subscreen);
		lock_actions = false;
}

static void scr_watch_set_draw_screen(scr_mngr_draw_ctx* ctx) {
//		SPIFFS_lseek(&fs, watchset_fd, current_screen_controls_address, SPIFFS_SEEK_SET);
		draw_screen_controls(true, ctx);
}

static void scr_watch_set_refresh_screen(scr_mngr_draw_ctx* ctx) {
	  if (show_screen_param != 0xFF && current_subscreen != show_screen_param) {
				lock_actions = true;
				clear_subscreen_data();
	      mlcd_fb_clear();
			  init_subscreen(show_screen_param);
			
				draw_screen_controls(true, ctx);
			  current_subscreen = show_screen_param;
			
				show_screen_param = 0xFF;
				lock_actions = false;
		} else {
				bool forceRedraw = draw_screen_controls(false, ctx);
			
				if (forceRedraw) {
						mlcd_fb_clear();
						draw_screen_controls(true, ctx);
				}
	  }
}

static void scr_watch_set_parse_actions(uint8_t** data);
	
void scr_watch_set_parse_choose_actions(uint8_t** data) {
		int value;
		parse_data_source_value_from_array(data, &value);
		uint8_t options_no = *((*data)++);
		for (int i=0; i<options_no; i++) {
				uint8_t expected_value = *((*data)++);
				if (value == expected_value) {
						*data+=2;
						scr_watch_set_parse_actions(data);
				} else {
						int size = *((*data)++)<<8 | *((*data)++);
						*data+=size;
				}
		}
}
				
static void trim_model_property(uint8_t property_id) {
		if (model_data_buffer[property_id].value > model_data_buffer[property_id].max) {
				if (model_data_buffer[property_id].overflow) {
						model_data_buffer[property_id].value = model_data_buffer[property_id].min;
				} else {
						model_data_buffer[property_id].value = model_data_buffer[property_id].max;	
				}
		} else if (model_data_buffer[property_id].value < model_data_buffer[property_id].min) {
				if (model_data_buffer[property_id].overflow) {
						model_data_buffer[property_id].value = model_data_buffer[property_id].max;
				} else {
						model_data_buffer[property_id].value = model_data_buffer[property_id].min;
				}
		}
}

static void scr_watch_set_parse_actions(uint8_t** data) {
		uint8_t actions_no = *((*data)++);
		for(int i=0; i<actions_no; i++) {
				uint8_t action_id = *((*data)++);
				if (action_id == WATCH_SET_FUNC_CHOOSE) {
						scr_watch_set_parse_choose_actions(data);
				} else if (action_id == WATCH_SET_FUNC_EXTENSION) {
						uint16_t param = *((*data)++)<<8 | *((*data)++);
						scr_watch_set_invoke_external_function(param);
				} else if (action_id == WATCH_SET_FUNC_CHANGE_SCREEN) {
						uint16_t param = *((*data)++)<<8 | *((*data)++);
						show_screen_param = param&0xFF;
				} else if (action_id == WATCH_SET_FUNC_SET_TIME) {
						time_t t;
						time(&t);
						struct tm* time_struct = localtime(&t);

						uint8_t field_no = *((*data)++);
						for (int f = 0; f < field_no; f++) {
								uint8_t field_id = (*((*data)++))&0xFF;
								int32_t field_value;
								parse_data_source_value_from_array(data, &field_value);
								switch(field_id) {
									case 0:
										time_struct->tm_year = field_value - 1900;
										break;
									case 1:
										time_struct->tm_mon = field_value - 1;
										break;
									case 2:
										time_struct->tm_mday = field_value;
										break;
									case 3:
										time_struct->tm_hour = field_value;
										break;
									case 4:
										time_struct->tm_min = field_value;
										break;
									case 5:
										time_struct->tm_sec = field_value;
										break;
								}
						}
						watchset_invoke_internal_function(action_id, mktime(time_struct));
				} else if (action_id == WATCH_SET_FUNC_MODEL_SET) {
						uint8_t property_id = *((*data)++);
						int value;
						parse_data_source_value_from_array(data, &value);
						model_data_buffer[property_id].value = value;
						trim_model_property(property_id);
				} else if (action_id == WATCH_SET_FUNC_MODEL_ADD) {
						uint8_t property_id = *((*data)++);
						int value;
						parse_data_source_value_from_array(data, &value);
						model_data_buffer[property_id].value += value;
						trim_model_property(property_id);
				} else if (action_id == WATCH_SET_FUNC_MODEL_SUBTRACT) {
						uint8_t property_id = *((*data)++);
						int value;
						parse_data_source_value_from_array(data, &value);
						model_data_buffer[property_id].value -= value;
						trim_model_property(property_id);
				} else if (action_id == WATCH_SET_FUNC_MODEL_INCREMENT) {
						uint8_t property_id = *((*data)++);
						model_data_buffer[property_id].value++;
						trim_model_property(property_id);
				} else if (action_id == WATCH_SET_FUNC_MODEL_DECREMENT) {
						uint8_t property_id = *((*data)++);
						model_data_buffer[property_id].value--;
						trim_model_property(property_id);
				} else if (action_id == WATCH_SET_FUNC_SHOW_APPLICATION || action_id == WATCH_SET_FUNC_SHOW_UTILITY) {
						watchset_invoke_internal_function(action_id,  (uint32_t)*data);
				} else {
						watchset_invoke_internal_function(action_id, 0);
				}
		}
}

bool scr_watch_set_invoke_function(int idx) {
		if (action_handlers[idx] == 0xFFFF) {
				// handler not set
				return false;
		}
		uint8_t* data = actions_data_buffer + action_handlers[idx];
		scr_watch_set_parse_actions(&data);
		return true;
}

void scr_watch_set_invoke_external_function(uint8_t function_id) {
	  ble_peripheral_invoke_external_function(function_id);
}

static bool scr_watch_set_handle_button_pressed(uint32_t button_id) {
		if (lock_actions) {
				return true;
		}
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  return scr_watch_set_invoke_function(0);
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  return scr_watch_set_invoke_function(1);
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  return scr_watch_set_invoke_function(2);
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  return scr_watch_set_invoke_function(3);
		}
		return false;
}

static bool scr_watch_set_handle_button_long_pressed(uint32_t button_id) {
		if (lock_actions) {
				return true;
		}
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  return scr_watch_set_invoke_function(4);
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  return scr_watch_set_invoke_function(5);
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  return scr_watch_set_invoke_function(6);
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  return scr_watch_set_invoke_function(7);
		}
		return false;
}

bool scr_watch_set_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_watch_set_init(event_param);
				    return true;
			  case SCR_EVENT_DRAW_SCREEN:
				    scr_watch_set_draw_screen((scr_mngr_draw_ctx*)event_param);
				    return true;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_watch_set_refresh_screen((scr_mngr_draw_ctx*)event_param);
            return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    if (scr_watch_set_handle_button_pressed(event_param)) {
								return true;
						}
						break;
			  case SCR_EVENT_BUTTON_LONG_PRESSED:
				    if (scr_watch_set_handle_button_long_pressed(event_param)) {
								return true;
						}
						break;
				case SCR_EVENT_WRIST_SHAKE:
						return scr_watch_set_invoke_function(8);
				case SCR_EVENT_APP_CONNECTION_CONFIRMED:
						ble_peripheral_set_watch_set_id(watchset_id);
						return true;
			  case SCR_EVENT_DESTROY_SCREEN:
						clean_before_exit();
				    return true;
		}
		if (base_actions_handler != NULL) {
				return base_actions_handler(event_type, event_param);
		}
		return false;
}

#ifndef WATCHSET_H
#define WATCHSET_H

#include <stdbool.h>
#include <stdint.h>
#include "spiffs/spiffs.h"

#define WATCH_SET_SECTION_SCREENS 1
#define WATCH_SET_SECTION_EXTERNAL_PROPERTIES 2
#define WATCH_SET_SECTION_STATIC_CONTENT 3

#define WATCH_SET_SCREEN_SECTION_CONTROLS 1
#define WATCH_SET_SCREEN_SECTION_ACTIONS 2
#define WATCH_SET_SCREEN_SECTION_MEMORY 3
#define WATCH_SET_SCREEN_SECTION_BASE_ACTIONS 4
#define WATCH_SET_SCREEN_SECTION_MODEL 5
#define WATCH_SET_SCREEN_SECTION_SETTINGS 6

#define WATCH_SET_SETTING_INVERTIBLE	1

#define WATCH_SET_END_OF_DATA 0xFF

#define WATCH_SET_BASE_ACTIONS_WATCH_FACE 1

#define WATCH_SET_FUNC_TOGGLE_BACKLIGHT 			0x1
#define WATCH_SET_FUNC_TOGGLE_COLORS    			0x2
#define WATCH_SET_FUNC_TEMPORATY_BACKLIGHT 		0x3
#define WATCH_SET_FUNC_SET_TEMPORARY_BACKLIGHT_TIMEOUT 0x4
#define WATCH_SET_FUNC_SET_TIME					 			0x5
#define WATCH_SET_FUNC_FORMAT_DATA 						0xE
#define WATCH_SET_FUNC_RESTART			 					0xF

#define WATCH_SET_FUNC_STOPWATCH_START        0x10
#define WATCH_SET_FUNC_STOPWATCH_RESET        0x11
#define WATCH_SET_FUNC_STOPWATCH_STOP         0x12
#define WATCH_SET_FUNC_STOPWATCH_START_STOP   0x13
#define WATCH_SET_FUNC_STOPWATCH_NEXT_LAP     0x14

#define WATCH_SET_FUNC_MODEL_SET 0xA0
#define WATCH_SET_FUNC_MODEL_ADD 0xA1
#define WATCH_SET_FUNC_MODEL_SUBTRACT 0xA2
#define WATCH_SET_FUNC_MODEL_INCREMENT 0xA3
#define WATCH_SET_FUNC_MODEL_DECREMENT 0xA4
		
#define WATCH_SET_FUNC_CHANGE_SCREEN 				0xE0
#define WATCH_SET_FUNC_SHOW_SETTINGS 				0xE1
#define WATCH_SET_FUNC_SHOW_STATUS	 				0xE2
#define WATCH_SET_FUNC_SHOW_NOTIFICATIONS 	0xE3
#define WATCH_SET_FUNC_SHOW_WATCH_FACES			0xE4
#define WATCH_SET_FUNC_SHOW_APPLICATIONS		0xE5
#define WATCH_SET_FUNC_SHOW_UTILS						0xE6
#define WATCH_SET_FUNC_SHOW_NEXT_WATCH_FACE	0xE7
#define WATCH_SET_FUNC_SHOW_APPLICATION			0xE8
#define WATCH_SET_FUNC_SHOW_UTILITY					0xE9

#define WATCH_SET_FUNC_CLOSE 0xEE
#define WATCH_SET_FUNC_EXTENSION 0xEF

#define WATCH_SET_FUNC_CHOOSE 0xF0

#define WATCH_SET_EXT_PROP_TYPE_NUMBER 1
#define WATCH_SET_EXT_PROP_TYPE_STRING 2
#define WATCH_SET_EXT_PROP_TYPE_ENUM 3

#define WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE 4

#define WATCH_SET_TYPE_WATCH_FACE		1
#define WATCH_SET_TYPE_APPLICATION	2
#define WATCH_SET_TYPE_UTILITY			3

#define WATCH_SET_PATH_WATCH_FACE		"f/"
#define WATCH_SET_PATH_APPLICATION	"a/"
#define WATCH_SET_PATH_UTILITY			"u/"

#define DATA_SOURCE_INTERNAL 0
#define DATA_SOURCE_EXTERNAL 1
#define DATA_SOURCE_SENSOR   2
#define DATA_SOURCE_STATIC   3
#define DATA_SOURCE_MODEL    4

#define DATA_SOURCE_BATTERY_LEVEL 0
#define DATA_SOURCE_TIME_IN_SECONDS 1
#define DATA_SOURCE_TIME_HOUR_24 2
#define DATA_SOURCE_TIME_HOUR_12 3
#define DATA_SOURCE_TIME_HOUR_12_DESIGNATOR 4
#define DATA_SOURCE_TIME_MINUTES 5
#define DATA_SOURCE_TIME_SECONDS 6
#define DATA_SOURCE_DATE_DAY_OF_WEEK 7
#define DATA_SOURCE_DATE_DAY_OF_MONTH 8
#define DATA_SOURCE_DATE_DAY_OF_YEAR 9
#define DATA_SOURCE_DATE_MONTH 10
#define DATA_SOURCE_DATE_YEAR 11

#define DATA_SOURCE_SENSOR_HR 0

#define WATCH_SET_OPERATION_NEXT_WATCH_FACE 			0x1
#define WATCH_SET_OPERATION_OPEN_APPLICATION			0x2
#define WATCH_SET_OPERATION_OPEN_UTILITY					0x3

void watchset_set_default_watch_face(struct spiffs_dirent* entry);

spiffs_file watchset_get_dafault_watch_face_fd(void);

bool watchset_default_watch_face_handle_event(uint32_t event_type, uint32_t event_param);

uint32_t watchset_internal_data_source_get_value(uint32_t data_source_id, uint8_t expected_range, uint8_t* data, bool* has_changed);

uint32_t watchset_sensor_data_source_get_value(uint32_t data_source_id, uint8_t expected_range);

uint32_t watchset_static_data_source_get_value(uint32_t data_source_id, uint8_t expected_range);

void watchset_invoke_internal_function(uint8_t function_id, uint32_t param);

void* watchset_get_converter(uint8_t key);

void watchset_set_watch_face(bool flag);

bool watchset_is_watch_face(void);

void watchset_next_watch_face(void);

void watchset_async_operation(uint8_t op, uint32_t param);

void watchset_process_async_operation(void);

#endif /* WATCHSET_H */

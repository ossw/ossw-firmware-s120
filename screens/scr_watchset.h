#ifndef SCR_WATCH_SET_H
#define SCR_WATCH_SET_H

#include <stdbool.h>
#include <stdint.h>


#define WATCH_SET_SECTION_SCREENS 1
#define WATCH_SET_SECTION_EXTERNAL_PROPERTIES 2
#define WATCH_SET_SECTION_STATIC_CONTENT 3

#define WATCH_SET_SCREEN_SECTION_CONTROLS 1
#define WATCH_SET_SCREEN_SECTION_ACTIONS 2
#define WATCH_SET_SCREEN_SECTION_MEMORY 3

#define WATCH_SET_END_OF_DATA 0xFF


#define WATCH_SET_FUNC_TOGGLE_BACKLIGHT 1
#define WATCH_SET_FUNC_TOGGLE_COLORS    2

#define WATCH_SET_FUNC_STOPWATCH_START        0x10
#define WATCH_SET_FUNC_STOPWATCH_RESET        0x11
#define WATCH_SET_FUNC_STOPWATCH_STOP         0x12
#define WATCH_SET_FUNC_STOPWATCH_START_STOP   0x13
#define WATCH_SET_FUNC_STOPWATCH_NEXT_LAP     0x14

#define WATCH_SET_FUNC_CHANGE_SCREEN 0xF0
#define WATCH_SET_FUNC_SHOW_SETTINGS 0xF1

#define WATCH_SET_FUNC_CLOSE 0xFE
#define WATCH_SET_FUNC_EXTENSION 0xFF

#define WATCH_SET_EXT_PROP_TYPE_NUMBER 1
#define WATCH_SET_EXT_PROP_TYPE_STRING 2
#define WATCH_SET_EXT_PROP_TYPE_ENUM 3

#define WATCH_SET_EXT_PROP_DESCRIPTOR_SIZE 4

#define DATA_SOURCE_INTERNAL 0
#define DATA_SOURCE_EXTERNAL 1
#define DATA_SOURCE_SENSOR   2

#define DATA_SOURCE_BATTERY_LEVEL 0
#define DATA_SOURCE_TIME_HOUR_24 1
#define DATA_SOURCE_TIME_HOUR_12 2
#define DATA_SOURCE_TIME_HOUR_12_DESIGNATOR 3
#define DATA_SOURCE_TIME_MINUTES 4
#define DATA_SOURCE_TIME_SECONDS 5
#define DATA_SOURCE_DATE_DAY_OF_WEEK 6
#define DATA_SOURCE_DATE_DAY_OF_MONTH 7
#define DATA_SOURCE_DATE_DAY_OF_YEAR 8
#define DATA_SOURCE_DATE_MONTH 9
#define DATA_SOURCE_DATE_YEAR 10

#define DATA_SOURCE_SENSOR_HR 0

typedef struct
{
	  uint8_t id;
	  uint16_t parameter;
} FUNCTION;

void scr_watch_set_invoke_function(const FUNCTION* function);

void scr_watch_set_invoke_internal_function(uint8_t function_id, uint16_t param);

void scr_watch_set_invoke_external_function(uint8_t function_id);

void scr_watch_set_handle_event(uint32_t event_type, uint32_t event_param);

void set_external_property_data(uint8_t property_id, uint8_t* data_ptr, uint8_t size);
	
#endif /* SCR_WATCH_SET_H */

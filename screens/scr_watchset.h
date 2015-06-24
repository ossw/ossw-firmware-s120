#ifndef SCR_WATCH_SET_H
#define SCR_WATCH_SET_H

#include <stdbool.h>
#include <stdint.h>


#define WATCH_SET_SECTION_SCREENS 1
#define WATCH_SET_SECTION_EXTERNAL_PROPERTIES 2
#define WATCH_SET_SECTION_STATIC_CONTENT 3

#define WATCH_SET_SCREEN_SECTION_CONTROLS 1
#define WATCH_SET_SCREEN_SECTION_ACTIONS 2

#define WATCH_SET_END_OF_DATA 0xFF


#define WATCH_SET_FUNC_TOGGLE_BACKLIGHT 1
#define WATCH_SET_FUNC_TOGGLE_COLORS    2

#define WATCH_SET_FUNC_CHANGE_SCREEN 0xF0
#define WATCH_SET_FUNC_SHOW_SETTINGS 0xF1

#define WATCH_SET_FUNC_EXTENSION 0xFF

#include <stdbool.h>
#include <stdint.h>

#define DATA_SOURCE_TIME_HOUR 0
#define DATA_SOURCE_TIME_MINUTES 1
#define DATA_SOURCE_TIME_SECONDS 2
#define DATA_SOURCE_DATE_DAY_OF_MONTH 3
#define DATA_SOURCE_DATE_MONTH 4
#define DATA_SOURCE_DATE_YEAR 5
#define DATA_SOURCE_BATTERY_LEVEL 6

#define DATA_SOURCE_SENSOR_HR 0x10
#define DATA_SOURCE_SENSOR_CYCLING_SPEED 0x11
#define DATA_SOURCE_SENSOR_CYCLING_CADENCE 0x12

uint16_t data_source_get_value(uint16_t data_source_id);

uint32_t (*(data_source_get_handle)(uint16_t data_source_id))(void);


typedef struct
{
	  uint8_t id;
	  uint16_t parameter;
} FUNCTION;

void scr_watch_set_invoke_function(const FUNCTION* function);

void scr_watch_set_invoke_internal_function(uint8_t function_id, uint16_t param);

void scr_watch_set_invoke_external_function(uint8_t function_id);

void scr_watch_set_handle_event(uint32_t event_type, uint32_t event_param);

#endif /* SCR_WATCH_SET_H */

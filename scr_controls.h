#ifndef SCR_CONTROLS_H
#define SCR_CONTROLS_H

#include <stdbool.h>
#include <stdint.h>

#define SCR_CONTROL_STATIC_RECT              0
#define SCR_CONTROL_NUMBER                   1
#define SCR_CONTROL_HORIZONTAL_PROGRESS_BAR  2
//#define SCR_CONTROL_VERTICAL_PROGRESS_BAR    3
//#define SCR_CONTROL_BATTERY_LEVEL	           4

#define NUMBER_FORMAT_0_99     0
//#define NUMBER_FORMAT_0_199    1
//#define NUMBER_FORMAT_0_999    2
//#define NUMBER_FORMAT_0_9999   3

#define NUMBER_FORMAT_FLAG_ZERO_PADDED    0x80

typedef struct
{
	  uint32_t last_value;
} CONTROL_DATA;

typedef struct
{
	  const uint8_t format;
	  const uint8_t x;
	  const uint8_t y;
	  const uint8_t width;
	  const uint8_t height;
	  const uint8_t thickness;
	  uint32_t (* const data_handle)();
	  CONTROL_DATA* data;
} SCR_CONTROL_NUMBER_CONFIG;	

typedef struct
{
	  const uint8_t x;
	  const uint8_t y;
	  const uint8_t width;
	  const uint8_t height;
} SCR_CONTROL_STATIC_RECT_CONFIG;	

typedef struct
{
	  const uint8_t x;
	  const uint8_t y;
	  const uint8_t width;
	  const uint8_t height;
	  const uint8_t max;
	  const uint8_t border;
	  uint32_t (* const data_handle)();
	  CONTROL_DATA* data;
} SCR_CONTROL_PROGRESS_BAR_CONFIG;	

typedef struct
{
	  const uint8_t type;
	  const void* config;
} SCR_CONTROL_DEFINITION;

typedef struct
{
	  const uint8_t controls_no;
	  const SCR_CONTROL_DEFINITION* controls;
} SCR_CONTROLS_DEFINITION;

void scr_controls_draw(const SCR_CONTROLS_DEFINITION*);

void scr_controls_redraw(const SCR_CONTROLS_DEFINITION*);

#endif /* SCR_CONTROLS_H */

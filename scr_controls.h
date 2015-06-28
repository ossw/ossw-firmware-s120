#ifndef SCR_CONTROLS_H
#define SCR_CONTROLS_H

#include <stdbool.h>
#include <stdint.h>

#define SCR_CONTROL_STATIC_RECT              0
#define SCR_CONTROL_NUMBER                   1
#define SCR_CONTROL_HORIZONTAL_PROGRESS_BAR  2
//#define SCR_CONTROL_VERTICAL_PROGRESS_BAR    3
#define SCR_CONTROL_TEXT  4

#define NUMBER_RANGE_0__9     0x10
#define NUMBER_RANGE_0__19    0x20
#define NUMBER_RANGE_0__99    0x30
#define NUMBER_RANGE_0__199   0x40
#define NUMBER_RANGE_0__999   0x50
#define NUMBER_RANGE_0__1999  0x60
#define NUMBER_RANGE_0__9999  0x70
#define NUMBER_RANGE_0__19999 0x80
#define NUMBER_RANGE_0__99999 0x90

#define NUMBER_FORMAT_FLAG_ZERO_PADDED    0x80000000

typedef struct
{
	  uint32_t last_value;
} NUMBER_CONTROL_DATA;

typedef struct
{
	  char last_value[17];
} TEXT_CONTROL_DATA;

typedef struct
{
	  uint8_t range;
	  uint8_t x;
	  uint8_t y;
	  uint32_t style;
	  uint32_t (* data_handle)(uint32_t);
	  uint32_t data_handle_param;
	  NUMBER_CONTROL_DATA* data;
} SCR_CONTROL_NUMBER_CONFIG;	

typedef struct
{
	  uint8_t format;
	  uint8_t x;
	  uint8_t y;
	  uint8_t width;
	  uint8_t height;
	  uint32_t (* data_handle)(uint32_t);
	  uint32_t data_handle_param;
	  TEXT_CONTROL_DATA* data;
} SCR_CONTROL_TEXT_CONFIG;	

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
	  uint32_t data_handle_param;
	  NUMBER_CONTROL_DATA* data;
} SCR_CONTROL_PROGRESS_BAR_CONFIG;	

typedef struct
{
	  uint8_t type;
	  void* config;
} SCR_CONTROL_DEFINITION;

typedef struct
{
	  uint8_t controls_no;
	  SCR_CONTROL_DEFINITION* controls;
} SCR_CONTROLS_DEFINITION;

void scr_controls_draw(const SCR_CONTROLS_DEFINITION*);

void scr_controls_redraw(const SCR_CONTROLS_DEFINITION*);

#endif /* SCR_CONTROLS_H */

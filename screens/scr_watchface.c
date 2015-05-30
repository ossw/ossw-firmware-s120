#include <string.h>
#include "scr_watchface.h"
#include "../scr_mngr.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../scr_controls.h"
#include "nrf_delay.h"
		
static CONTROL_DATA hour_ctrl_data;
		
static const SCR_CONTROL_NUMBER_CONFIG hour_config = {
		NUMBER_FORMAT_0_99 | NUMBER_FORMAT_FLAG_ZERO_PADDED,
	  5,
	  4,
	  135,
	  76,
	  8,
	  rtc_get_current_hour,
    &hour_ctrl_data
};

static CONTROL_DATA minutes_ctrl_data;

static const SCR_CONTROL_NUMBER_CONFIG minutes_config = {
		NUMBER_FORMAT_0_99 | NUMBER_FORMAT_FLAG_ZERO_PADDED,
	  5,
	  85,
	  135,
	  76,
	  6,
	  rtc_get_current_minutes,
    &minutes_ctrl_data
};

static CONTROL_DATA seconds_ctrl_data;

static const SCR_CONTROL_PROGRESS_BAR_CONFIG seconds_config = {
	  0,
	  MLCD_YRES - 3,
	  MLCD_XRES,
	  2,
	  60,
	  rtc_get_current_seconds,
    &seconds_ctrl_data
};

static const SCR_CONTROL_DEFINITION controls[] = {
	  {SCR_CONTROL_NUMBER, (void*)&hour_config},
		{SCR_CONTROL_NUMBER, (void*)&minutes_config},
		{SCR_CONTROL_HORIZONTAL_PROGRESS_BAR, (void*)&seconds_config}
};

static const SCR_CONTROLS_DEFINITION controls_definition = {
	  sizeof(controls)/sizeof(SCR_CONTROL_DEFINITION),
	  controls
};

static void scr_watchface_refresh_time() {
	  scr_controls_redraw(&controls_definition);
    mlcd_fb_flush();
}

static void scr_watchface_handle_button_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_UP:
            scr_mngr_show_screen(SCR_TEST);
            break;
    }
}

static void scr_watchface_handle_button_long_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_SELECT:
            scr_mngr_show_screen(SCR_SETTINGS);
            break;
    }
}

static void scr_watchface_init() {
    mlcd_fb_clear();
	  scr_controls_draw(&controls_definition);
    mlcd_fb_flush();
}

void scr_watchface_handle_event(uint32_t event_type, uint32_t event_param) {
    switch(event_type) {
        case SCR_EVENT_INIT_SCREEN:
            scr_watchface_init();
            break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_watchface_refresh_time();
            break;
        case SCR_EVENT_BUTTON_PRESSED:
            scr_watchface_handle_button_pressed(event_param);
            break;
        case SCR_EVENT_BUTTON_LONG_PRESSED:
            scr_watchface_handle_button_long_pressed(event_param);
            break;
    }
}

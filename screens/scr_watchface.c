#include <string.h>
#include "scr_watchface.h"
#include "../scr_mngr.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../scr_controls.h"
#include "../vibration.h"
#include "../watchset.h"
#include "../fs.h"
#include "../config.h"
#include "../graph.h"
#include "nrf_delay.h"
		
static NUMBER_CONTROL_DATA hour_ctrl_data;
		
static const SCR_CONTROL_NUMBER_CONFIG hour_config = {
		NUMBER_RANGE_0__99,
	  4,
	  4,
	  NUMBER_FORMAT_FLAG_ZERO_PADDED | 4 << 24 | 8 << 16 | 66 << 8 | 76,
	  (uint32_t (*)(uint32_t, uint8_t, uint8_t*, bool*))rtc_get_current_hour_24,
	  0,
    &hour_ctrl_data
};

static NUMBER_CONTROL_DATA minutes_ctrl_data;

static const SCR_CONTROL_NUMBER_CONFIG minutes_config = {
		NUMBER_RANGE_0__99,
	  4,
	  85,
	  NUMBER_FORMAT_FLAG_ZERO_PADDED | 4 << 24 | 6 << 16 | 66 << 8 | 76,
	  (uint32_t (*)(uint32_t, uint8_t, uint8_t*, bool*))rtc_get_current_minutes,
	  0,
    &minutes_ctrl_data
};

static NUMBER_CONTROL_DATA seconds_ctrl_data;

static const SCR_CONTROL_PROGRESS_BAR_CONFIG seconds_config = {
	  0,
	  MLCD_YRES - 3,
	  MLCD_XRES,
	  2,
	  60,
		0,
	  (uint32_t (*)(uint32_t, uint8_t, uint8_t*, bool*))rtc_get_current_seconds,
	  0,
    &seconds_ctrl_data
};

static const SCR_CONTROL_DEFINITION controls[] = {
	  {SCR_CONTROL_NUMBER, (void*)&hour_config},
		{SCR_CONTROL_NUMBER, (void*)&minutes_config},
		{SCR_CONTROL_PROGRESS_BAR, (void*)&seconds_config}
};

static const SCR_CONTROLS_DEFINITION controls_definition = {
	  sizeof(controls)/sizeof(SCR_CONTROL_DEFINITION),
	  (SCR_CONTROL_DEFINITION*)controls
};

static uint8_t h, m, s;

static void scr_watchface_refresh_time() {
	  //scr_controls_redraw(&controls_definition);

		lineHand(s, 70, 20);
		s = rtc_get_current_seconds();
		if (s == 0) {
				triangleHand(m, 60, 15, 6); 
				m = rtc_get_current_minutes();
				if (m % 12 == 0) {
						rectHand(h, 50, 10, 6);
						h = 5 * rtc_get_current_hour_12() + m / 12;
						rectHand(h, 50, 10, 6);
				}
				triangleHand(m, 60, 15, 6);
		}
		lineHand(s, 70, 20);
}

static void scr_watchface_init() {
	  spiffs_file fd = config_get_default_watch_face_fd();
		if (fd >= 0) {
				SPIFFS_lseek(&fs, fd, 0, SPIFFS_SEEK_SET);
				scr_mngr_show_screen_with_param(SCR_WATCH_SET, 1<<28 | 2<<24 | fd);
		}
}

static void scr_watchface_draw() {
		// test drawings
//		lineBresenham(10, 10, 12, 30);
//		lineBresenham(7, 30, 9, 10);
//		lineBresenham(16, 10, 14, 30);
//		lineBresenham(19, 30, 17, 10);
//		pixel(0, 0);
//		pixel(0, 1);
//		pixel(0, 2);
//		hLine(0, 1, 11);
//		hLine(1, 2, 11);
//		hLine(2, 3, 11);
//		hLine(3, 4, 11);
//		hLine(4, 5, 11);
//		hLine(5, 6, 11);
//		int_fast16_t x[] = {110, 120, 130, 140, 125};
//		int_fast16_t y[] = {20, 10, 10, 20, 30};
//		fillConvex(5, x, y);
		circle(72, 84, 71);
//		int_fast16_t x1[] = {20, 22, 18};
//		int_fast16_t y1[] = {10, 30, 30};
//		fillConvex(3, x1, y1);
	  //scr_controls_draw(&controls_definition);
		// seconds
		s = rtc_get_current_seconds();
		lineHand(s, 70, 20);
		// minutes
		m = rtc_get_current_minutes();
		triangleHand(m, 60, 15, 6); 
		// hours
		h = 5*rtc_get_current_hour_12() + m/12;
		rectHand(h, 50, 5, 6);
}

bool scr_watchface_handle_event(uint32_t event_type, uint32_t event_param) {
    switch(event_type) {
        case SCR_EVENT_INIT_SCREEN:
            scr_watchface_init();
            return true;
        case SCR_EVENT_DRAW_SCREEN:
            scr_watchface_draw();
            return true;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_watchface_refresh_time();
            return true;
    }
		return watchset_default_watch_face_handle_event(event_type, event_param);
}

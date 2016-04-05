#include <string.h>
#include "scr_watchface_analog.h"
#include "../scr_mngr.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../scr_controls.h"
#include "../vibration.h"
#include "../watchset.h"
#include "../fs.h"
#include "../config.h"
#include "../graph.h"

#define SEC_L1 70
#define SEC_L2 -20
#define MIN_L1 60
#define MIN_L2 -15
#define HOUR_L1 40
#define HOUR_L2 -3

static uint8_t h, m, s;

static void scr_watchface_refresh_time() {
		bool slow = rtc_get_refresh_interval() >= RTC_INTERVAL_MINUTE;
		if (!slow) {
				radialLine(CENTER_X, CENTER_Y, 6*s, SEC_L1, SEC_L2);
				s = rtc_get_current_seconds();
		}
		if (s == 0 || slow) {
				radialTriangle(CENTER_X, CENTER_Y, 6*m, MIN_L1, MIN_L2, 7);
				radialRect(CENTER_X, CENTER_Y, 30*h+(m>>1), HOUR_L1, HOUR_L2, 7);
				m = rtc_get_current_minutes();
				h = rtc_get_current_hour_12();
				radialRect(CENTER_X, CENTER_Y, 30*h+(m>>1), HOUR_L1, HOUR_L2, 7);
				radialTriangle(CENTER_X, CENTER_Y, 6*m, MIN_L1, MIN_L2, 7);
		}
		if (!slow)
				radialLine(CENTER_X, CENTER_Y, 6*s, SEC_L1, SEC_L2);
}

static void scr_watchface_init() {
}

static void scr_watchface_draw() {
//		for (int i=1; i<20; i++)
//				hLine(i, 1, i);
//		fillCircle(CENTER_X, CENTER_Y, 71);
		for (int deg = 0; deg < 360; deg += 30)
				radialRect(CENTER_X, CENTER_Y, deg, 60, 70, 3);
		bool slow = rtc_get_refresh_interval() >= RTC_INTERVAL_MINUTE;
		s = rtc_get_current_seconds();
		if (!slow) {
				radialLine(CENTER_X, CENTER_Y, 6*s, SEC_L1, SEC_L2);
		}
		m = rtc_get_current_minutes();
		radialTriangle(CENTER_X, CENTER_Y, 6*m, MIN_L1, MIN_L2, 7); 
		h = rtc_get_current_hour_12();
		radialRect(CENTER_X, CENTER_Y, 30*h+(m>>1), HOUR_L1, HOUR_L2, 7);
}

static bool scr_analog_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    return true;
		}
		return false;
}

bool scr_watchface_analog_handle_event(uint32_t event_type, uint32_t event_param) {
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
			  case SCR_EVENT_BUTTON_PRESSED:
				    if (scr_analog_handle_button_pressed(event_param))
								return true;
    }
		return watchset_default_watch_face_handle_event(event_type, event_param);
}

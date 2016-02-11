#include <string.h>
#include <stdio.h>
#include "scr_status.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../i18n/i18n.h"
#include "../battery.h"
#include "../scr_controls.h"
#include "../ossw.h"
#include "../fs.h"

static NUMBER_CONTROL_DATA battery_level_ctrl_data;
static uint8_t mode = 0;

static const SCR_CONTROL_PROGRESS_BAR_CONFIG battery_level_config = {
	  35,
		50,
	  74,
	  32,
	  100,
		2<<16,
	  (uint32_t (*)(uint32_t, uint8_t, uint8_t*, bool*))battery_get_level,
	  0,
    &battery_level_ctrl_data
};

static const SCR_CONTROL_DEFINITION controls[] = {
		{SCR_CONTROL_PROGRESS_BAR, (void*)&battery_level_config}
};

static const SCR_CONTROLS_DEFINITION controls_definition = {
	  sizeof(controls)/sizeof(SCR_CONTROL_DEFINITION),
	  (SCR_CONTROL_DEFINITION*)controls
};

static bool scr_status_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    return true;
		}
		return false;
}

static void scr_status_draw_battery_status() {
	
		mlcd_clear_rect(0, 25, MLCD_XRES, 20);
		if (mode == 1) {
				mlcd_draw_text("charging", 0, 25, MLCD_XRES, 20, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
		} else if (mode == 2) {
				mlcd_draw_text("full", 0, 25, MLCD_XRES, 20, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
		} else {
				mlcd_draw_text("battery", 0, 25, MLCD_XRES, 20, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
		}
}

static void scr_status_init(void) {
		spiffs_file fd = SPIFFS_open(&fs, "u/status", SPIFFS_RDONLY, 0);
		if (fd >= 0) {
				SPIFFS_lseek(&fs, fd, 0, SPIFFS_SEEK_SET);
				scr_mngr_show_screen_with_param(SCR_WATCH_SET, 2<<24 | fd);
		}
}

static void scr_status_refresh_screen() {
	  scr_controls_redraw(&controls_definition);
	
		uint8_t curr_mode = battery_is_charging()? (battery_is_full() ? 2 : 1) : 0;
		if (curr_mode != mode) {
				mode = curr_mode;
				scr_status_draw_battery_status();
		}
}

static void scr_status_draw_screen() {
		mlcd_draw_rect(109, 60, 6, 12);
		mode = battery_is_charging()? (battery_is_full() ? 2 : 1) : 0;
		scr_status_draw_battery_status();
							
		mlcd_draw_text(ossw_mac_address(), 0, 115, MLCD_XRES, NULL, FONT_NORMAL_BOLD, HORIZONTAL_ALIGN_CENTER);
	
		// Check hardware ID, how compatible with other SoftDevices
		// ex. 004D is for 2nd rev, WLCSP, 256kB/16kB 
		const char hex_str[]= "0123456789ABCDEF";
		char id[10]= "HWID: xxxx";
		unsigned char* pHwid = (unsigned char *)(0x1000005c); //HWID register address
		uint16_t hwid = (uint16_t) *pHwid;
		for (int i=0; i<4; i++) {
				id[9-i] = hex_str[hwid & 0x000F];
				hwid = hwid >> 4;
		}
		mlcd_draw_text(id, 0, 135, MLCD_XRES, NULL, FONT_NORMAL_BOLD, HORIZONTAL_ALIGN_CENTER);
		
		scr_controls_draw(&controls_definition);
}

bool scr_status_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_status_init();
				    return true;
        case SCR_EVENT_DRAW_SCREEN:
            scr_status_draw_screen();
            return true;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_status_refresh_screen();
            return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    return scr_status_handle_button_pressed(event_param);
		}
		return false;
}

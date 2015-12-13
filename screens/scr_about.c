#include <string.h>
#include <stdio.h>
#include "scr_about.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../i18n/i18n.h"
#include "../watchset.h"
#include "../ossw.h"

static bool scr_about_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_mngr_show_screen(SCR_SETTINGS);
				    return true;
		}
		return false;
}

static void scr_about_refresh_screen() {
}

static void scr_about_draw_screen() {
  	mlcd_draw_text(I18N_TRANSLATE(MESSAGE_ABOUT), 0, 30, MLCD_XRES, NULL, FONT_OPTION_BIG, HORIZONTAL_ALIGN_CENTER);
	
  	mlcd_draw_text(ossw_mac_address(), 0, 90, MLCD_XRES, NULL, FONT_NORMAL_BOLD, HORIZONTAL_ALIGN_CENTER);
  	mlcd_draw_text(ossw_firmware_version(), 0, 120, MLCD_XRES, NULL, FONT_NORMAL_BOLD, HORIZONTAL_ALIGN_CENTER);
}

bool scr_about_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
        case SCR_EVENT_DRAW_SCREEN:
            scr_about_draw_screen();
            return true;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_about_refresh_screen();
            return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    return scr_about_handle_button_pressed(event_param);
		}
		return true;
}

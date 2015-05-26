#include <string.h>
#include "scr_settings.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"

static void scr_settings_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    break;
		}
}

static void scr_settings_refresh_screen() {
	  mlcd_fb_flush();
}

static void draw_option(char *text, uint_fast8_t *yPos) {
  	mlcd_draw_text(text, 0, *yPos, MLCD_XRES, NULL, FONT_OPTION_NORMAL | ALIGN_CENTER);
	  *yPos += 23;
}

static void scr_settings_init() {
	  mlcd_fb_clear();
	
	  mlcd_draw_text("23", 1, 0, 20, NULL, FONT_OPTION_NORMAL | ALIGN_RIGHT);
	  mlcd_draw_text(":", 23, 0, NULL, NULL, FONT_OPTION_NORMAL);
	  mlcd_draw_text("47", 27, 0, 20, NULL, FONT_OPTION_NORMAL | ALIGN_LEFT);
	
	  mlcd_draw_rect_border(117, 2, 25, 12, 1);
	  mlcd_draw_rect(119, 4, 17, 8);
	
	  mlcd_draw_rect(0, 16, MLCD_XRES, 1);
	
	  uint_fast8_t yPos = 25;  
	
  	draw_option("Date", &yPos);
  	draw_option("Time", &yPos);
  	draw_option("Sensors", &yPos);
  	draw_option("Applications", &yPos);
  	draw_option("Watchfaces", &yPos);
  	draw_option("Reset", &yPos);
	
    scr_settings_refresh_screen();
}

void scr_settings_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_settings_init();
				    break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_settings_refresh_screen();
            break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_settings_handle_button_pressed(event_param);
				    break;
		}
}

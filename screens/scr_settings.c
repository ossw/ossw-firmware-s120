#include <string.h>
#include "scr_settings.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../i18n/i18n.h"

static int8_t selectedOption = 0;

typedef struct
{
	  const char* label;
	  const uint16_t offset;
} MENU_OPTION;	


static void scr_settings_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  selectedOption--;
				    if (selectedOption < 0) {
								selectedOption = 0;
						}
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  selectedOption++;
				    if (selectedOption > 4) {
								selectedOption = 4;
						}
				    break;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  mlcd_colors_toggle();
				    break;
		}
}

static void scr_settings_refresh_screen() {
	  mlcd_fb_flush();
}

static void draw_option(const char *text, uint_fast8_t *yPos) {
  	mlcd_draw_text(text, 0, *yPos, MLCD_XRES, NULL, FONT_OPTION_NORMAL | ALIGN_CENTER);
	  *yPos += 23;
}

static void scr_settings_init() {
	  mlcd_fb_clear();
	
	  mlcd_draw_text("23", 1, 1, 20, NULL, FONT_OPTION_NORMAL | ALIGN_RIGHT);
	  mlcd_draw_text(":", 23, 1, NULL, NULL, FONT_OPTION_NORMAL);
	  mlcd_draw_text("47", 27, 1, 20, NULL, FONT_OPTION_NORMAL | ALIGN_LEFT);
	
	  mlcd_draw_rect_border(119, 3, 23, 11, 1);
	  mlcd_draw_simple_progress(200, 255, 121, 5, 19, 7);
	
	  mlcd_draw_rect(0, 17, MLCD_XRES, 1);
	
	  uint_fast8_t yPos = 25;  
	
  	draw_option(I18N_TRANSLATE(MESSAGE_DATE), &yPos);
  	draw_option(I18N_TRANSLATE(MESSAGE_TIME), &yPos);
  	draw_option(I18N_TRANSLATE(MESSAGE_DISPLAY), &yPos);
  	draw_option(I18N_TRANSLATE(MESSAGE_SENSORS), &yPos);
  	draw_option(I18N_TRANSLATE(MESSAGE_RESTART), &yPos);
	
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

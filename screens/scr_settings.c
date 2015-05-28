#include <string.h>
#include "scr_settings.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../i18n/i18n.h"
#include "nrf_soc.h"

static int8_t selectedOption = 0;
static int8_t lastSelectedOption = 0xFF;

typedef struct
{
	  const uint16_t message_key;
	  void (*handler)();
} MENU_OPTION;	

static void opt_handler_do_nothing() {};
	
static void opt_handler_change_date() {
    scr_mngr_show_screen(SCR_CHANGE_DATE);
};
	
static void opt_handler_change_time() {
    scr_mngr_show_screen(SCR_CHANGE_TIME);
};

static const MENU_OPTION settings_menu[] = {
	  {MESSAGE_DATE, opt_handler_change_date},
		{MESSAGE_TIME, opt_handler_change_time},
		{MESSAGE_DISPLAY, mlcd_colors_toggle},
		{MESSAGE_SENSORS, opt_handler_do_nothing},
		{MESSAGE_RESTART, NVIC_SystemReset}
};

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
					  settings_menu[selectedOption].handler();
				    break;
		}
}

static void draw_option(const char *text, uint_fast8_t *yPos) {
  	mlcd_draw_text(text, 0, *yPos, MLCD_XRES, NULL, FONT_OPTION_NORMAL | ALIGN_CENTER);
	  *yPos += 23;
}

static void draw_selected_option(const char *text, uint_fast8_t *yPos) {
  	mlcd_draw_text(text, 0, *yPos, MLCD_XRES, NULL, FONT_OPTION_BIG | ALIGN_CENTER);
	  *yPos += 30;
}

static void scr_settings_refresh_screen() {
	
	  if (lastSelectedOption == selectedOption) {
			  return;
		}
	
	  uint_fast8_t yPos = 25;  
		
		mlcd_clear_rect(0, 18, MLCD_XRES, MLCD_YRES-18);
	
	  int menu_size = sizeof(settings_menu)/sizeof(MENU_OPTION);
	  for (int i=0; i<menu_size; i++) {
			  if (i==selectedOption){
						draw_selected_option(I18N_TRANSLATE(settings_menu[i].message_key), &yPos);
				} else {
						draw_option(I18N_TRANSLATE(settings_menu[i].message_key), &yPos);
				}
		}
	
	  mlcd_fb_flush();
		lastSelectedOption = selectedOption;
}

static void scr_settings_init() {
		lastSelectedOption = 0xFF;
	
	  mlcd_fb_clear();
	
	  mlcd_draw_text("23", 1, 1, 20, NULL, FONT_OPTION_NORMAL | ALIGN_RIGHT);
	  mlcd_draw_text(":", 23, 1, NULL, NULL, FONT_OPTION_NORMAL);
	  mlcd_draw_text("47", 27, 1, 20, NULL, FONT_OPTION_NORMAL | ALIGN_LEFT);
	
	  mlcd_draw_rect_border(119, 3, 23, 11, 1);
	  mlcd_draw_simple_progress(200, 255, 121, 5, 19, 7);
	
	  mlcd_draw_rect(0, 17, MLCD_XRES, 1);
	
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

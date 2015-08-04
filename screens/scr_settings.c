#include <string.h>
#include <stdio.h>
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

//static void opt_handler_do_nothing() {};
	
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
	//	{MESSAGE_SENSORS, opt_handler_do_nothing},
		{MESSAGE_RESTART, NVIC_SystemReset}
};

static const uint8_t SIZE_OF_MENU = sizeof(settings_menu)/sizeof(MENU_OPTION);

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
				    if (selectedOption >= SIZE_OF_MENU) {
								selectedOption = SIZE_OF_MENU-1;
						}
				    break;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  settings_menu[selectedOption].handler();
				    break;
		}
}

static void draw_option(const char *text, uint_fast8_t *yPos) {
  	mlcd_draw_text(text, 0, *yPos, MLCD_XRES, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
	  *yPos += 23;
}

static void draw_selected_option(const char *text, uint_fast8_t *yPos) {
  	mlcd_draw_text(text, 0, *yPos, MLCD_XRES, NULL, FONT_OPTION_BIG, HORIZONTAL_ALIGN_CENTER);
	  *yPos += 30;
}

static void scr_settings_draw_options() {
		int menu_size = sizeof(settings_menu)/sizeof(MENU_OPTION);
		uint_fast8_t yPos = 25;  
		for (int i=0; i<menu_size; i++) {
				if (i==selectedOption){
						draw_selected_option(I18N_TRANSLATE(settings_menu[i].message_key), &yPos);
				} else {
						draw_option(I18N_TRANSLATE(settings_menu[i].message_key), &yPos);
				}
		}
}

static void scr_settings_refresh_screen() {
	  scr_mngr_redraw_notification_bar();
	
	  if (lastSelectedOption != selectedOption) {
				mlcd_clear_rect(0, 18, MLCD_XRES, MLCD_YRES-18);
				scr_settings_draw_options();
	  }
		lastSelectedOption = selectedOption;
}

static void scr_settings_init() {
		selectedOption = 0;
		lastSelectedOption = 0xFF;
}

static void scr_settings_draw_screen() {
	  scr_mngr_draw_notification_bar();
		scr_settings_draw_options();
}

void scr_settings_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_settings_init();
				    break;
        case SCR_EVENT_DRAW_SCREEN:
            scr_settings_draw_screen();
            break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_settings_refresh_screen();
            break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_settings_handle_button_pressed(event_param);
				    break;
		}
}

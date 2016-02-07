#include <string.h>
#include <stdio.h>
#include "scr_settings.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../i18n/i18n.h"
#include "../fs.h"
#include "../mcu.h"

static int8_t selectedOption = 0;
static int8_t lastSelectedOption = 0xFF;

typedef struct
{
	  const uint16_t message_key;
	  void (*handler)();
} MENU_OPTION;	

static void opt_handler_change_date() {
    scr_mngr_show_screen(SCR_CHANGE_DATE);
};
	
static void opt_handler_change_time() {
    scr_mngr_show_screen(SCR_CHANGE_TIME);
};

static void opt_handler_about() {
    scr_mngr_show_screen(SCR_ABOUT);
};

void fs_reformat(void);

static void reformat() {
		fs_reformat();
		scr_mngr_show_screen(SCR_WATCHFACE);
}

static const MENU_OPTION settings_menu[] = {
	  {MESSAGE_DATE, opt_handler_change_date},
		{MESSAGE_TIME, opt_handler_change_time},
		{MESSAGE_DISPLAY, mlcd_colors_toggle},
		{MESSAGE_FORMAT, reformat},
		{MESSAGE_RESTART, mcu_reset},
		{MESSAGE_ABOUT, opt_handler_about}
};

static const uint8_t SIZE_OF_MENU = sizeof(settings_menu)/sizeof(MENU_OPTION);

static bool scr_settings_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  selectedOption--;
				    if (selectedOption < 0) {
								selectedOption = 0;
						}
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  selectedOption++;
				    if (selectedOption >= SIZE_OF_MENU) {
								selectedOption = SIZE_OF_MENU-1;
						}
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  settings_menu[selectedOption].handler();
				    return true;
		}
		return false;
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
	
		int8_t curr_opt = selectedOption;
	  if (lastSelectedOption != curr_opt) {
				mlcd_clear_rect(0, 18, MLCD_XRES, MLCD_YRES-18);
				scr_settings_draw_options();
	  }
		lastSelectedOption = curr_opt;
}

static void scr_settings_init() {
		selectedOption = 0;
		lastSelectedOption = 0xFF;
	
		spiffs_file fd = SPIFFS_open(&fs, "u/settings", SPIFFS_RDONLY, 0);
		if (fd >= 0) {
				SPIFFS_lseek(&fs, fd, 0, SPIFFS_SEEK_SET);
				scr_mngr_show_screen_with_param(SCR_WATCH_SET, 2<<24 | fd);
		}
}

static void scr_settings_draw_screen() {
	  scr_mngr_draw_notification_bar();
		scr_settings_draw_options();
}

bool scr_settings_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_settings_init();
				    return true;
        case SCR_EVENT_DRAW_SCREEN:
            scr_settings_draw_screen();
            return true;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_settings_refresh_screen();
            return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    return scr_settings_handle_button_pressed(event_param);
		}
		return false;
}

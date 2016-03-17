#include <string.h>
#include <stdio.h>
#include "nrf_soc.h"
#include "scr_settings.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../graph.h"
#include "../i18n/i18n.h"
#include "../fs.h"
#include "../rtc.h"
#include "../alarm.h"
#include "../ext_ram.h"
#include "dialog_select.h"

#define MARGIN_LEFT 			5
#define SUMMARY_X					95
#define MENU_ITEM_HEIGHT	20

static int8_t selectedOption = 0;
static int8_t lastSelectedOption = 0xFF;

typedef struct {
	  const uint16_t message_key;
	  void (*select_handler)();
	  void (*long_select_handler)();
	  void (*summary_drawer)(uint8_t x, uint8_t y);
} MENU_OPTION;	

static void opt_handler_change_date() {
    scr_mngr_show_screen(SCR_CHANGE_DATE);
};
	
static void opt_handler_change_time() {
    scr_mngr_show_screen(SCR_CHANGE_TIME);
};

//static void opt_handler_about() {
//    scr_mngr_show_screen(SCR_ABOUT);
//};

static void opt_handler_set_alarm() {
		scr_mngr_show_screen(SCR_SET_ALARM);
};

void fs_reformat(void);

static void reformat() {
		fs_reformat();
		scr_mngr_show_screen(SCR_WATCHFACE);
}

static void draw_alarm_switch(uint8_t x, uint8_t y) {
		bool on = is_alarm_active();
		draw_switch(x+10, y+2, on);
}

static void draw_colors_switch(uint8_t x, uint8_t y) {
		bool on = is_mlcd_inverted();
		draw_switch(x+10, y+2, on);
}

static void draw_interval_summary(uint8_t x, uint8_t y) {
		uint16_t text = MESSAGE_1_SECOND;
		if (rtc_get_refresh_interval() == RTC_INTERVAL_MINUTE)
				text = MESSAGE_1_MINUTE;
		mlcd_draw_text(I18N_TRANSLATE(text), x, y, MLCD_XRES-SUMMARY_X, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
}

static void rtc_refresh_toggle() {
		uint16_t interval = rtc_get_refresh_interval();
		if (interval == RTC_INTERVAL_SECOND)
				rtc_set_refresh_interval(RTC_INTERVAL_MINUTE);
		else
				rtc_set_refresh_interval(RTC_INTERVAL_SECOND);
}

// TEST DIALOG
static void select_item_handler(uint8_t item) {
}

static void test_handler() {
		pack_dialog_select(0, &select_item_handler, FONT_OPTION_NORMAL, I18N_TRANSLATE(MESSAGE_ABOUT),
				12, "gypq\0TWO\0THREE\0FOUR\0FIVE\0SIX\0SEVEN\0EIGHT\0NINE\0TEN\0ELEVEN\0TWELVE\0");
		scr_mngr_show_screen_with_param(SCR_DIALOG_SELECT, EXT_RAM_DATA_DIALOG_TEXT);
}

static const MENU_OPTION settings_menu[] = {
		{MESSAGE_ALARM_CLOCK, opt_handler_set_alarm, alarm_toggle, draw_alarm_switch},
	  {MESSAGE_DATE, opt_handler_change_date, opt_handler_change_date, NULL},
		{MESSAGE_TIME, opt_handler_change_time, opt_handler_change_time, NULL},
		{MESSAGE_DISPLAY, mlcd_colors_toggle, mlcd_colors_toggle, draw_colors_switch},
		{MESSAGE_RTC_REFRESH, rtc_refresh_toggle, rtc_refresh_toggle, draw_interval_summary},
		{MESSAGE_FORMAT, reformat, reformat, NULL},
		{MESSAGE_ABOUT, test_handler, test_handler, NULL}
//		{MESSAGE_RESTART, NVIC_SystemReset, NVIC_SystemReset, NULL}
};

static const uint8_t SIZE_OF_MENU = sizeof(settings_menu)/sizeof(MENU_OPTION);

static void draw_option(const char *text, uint_fast8_t yPos) {
  	mlcd_draw_text(text, MARGIN_LEFT, yPos, MLCD_XRES, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_LEFT);
}

static void scr_settings_draw_options() {
		int menu_size = sizeof(settings_menu)/sizeof(MENU_OPTION);
		uint_fast8_t yPos = 22;  
		for (int i=0; i<menu_size; i++) {
				void (*s_drawer)(uint8_t x, uint8_t y) = settings_menu[i].summary_drawer;
						draw_option(I18N_TRANSLATE(settings_menu[i].message_key), yPos);
						if (i==selectedOption)
								fillRectangle(0, yPos-2, SUMMARY_X-2*MARGIN_LEFT, MENU_ITEM_HEIGHT);
						if (s_drawer != NULL)
								s_drawer(SUMMARY_X, yPos);
						yPos += MENU_ITEM_HEIGHT;
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

static void scr_refresh_summary() {
		void (*s_drawer)(uint8_t, uint8_t) = settings_menu[selectedOption].summary_drawer;
		if (s_drawer != NULL) {
				mlcd_clear_rect(0, 20+MENU_ITEM_HEIGHT*selectedOption, MLCD_XRES, MENU_ITEM_HEIGHT);
				scr_settings_draw_options();
		}
}

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
					  settings_menu[selectedOption].select_handler();
						scr_refresh_summary();
				    return true;
		}
		return false;
}

static bool scr_settings_handle_button_long_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_SELECT: {
						void (*ls_handler)() = settings_menu[selectedOption].long_select_handler;
						if (ls_handler != NULL) {
								ls_handler();
								scr_refresh_summary();
								return true;
						}
				}
		}
		return false;
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
			  case SCR_EVENT_BUTTON_LONG_PRESSED: {
						return scr_settings_handle_button_long_pressed(event_param);
				}
		}
		return false;
}

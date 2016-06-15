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
#include "../config.h"
#include "../accel.h"
#include "../watchset.h"
#include "../notifications.h"
#include "dialog_select.h"

#define MARGIN_LEFT 			3
#define SCROLL_HEIGHT			6
#define HEADER_HEIGHT			18
#define SUMMARY_X					102
#define MENU_ITEM_HEIGHT	20
#define MENU_ITEMS_PER_PAGE 7
#define MENU_SWITCH_PADDING_X 10

static int8_t selectedOption = 0;
static int8_t lastSelectedOption = 0;

typedef struct {
	const uint16_t message_key;
	void (*select_handler)();
	void (*long_select_handler)();
	void (*summary_drawer)(uint8_t x, uint8_t y);
} MENU_OPTION;	

static void opt_handler_change_date() {
	scr_mngr_show_screen(SCR_CHANGE_DATE);
}
	
static void opt_handler_change_time() {
	scr_mngr_show_screen(SCR_CHANGE_TIME);
}

//static void opt_handler_about() {
//    scr_mngr_show_screen(SCR_ABOUT);
//}

static void opt_handler_timer() {
	scr_mngr_show_screen(SCR_TIMER);
}

static void opt_handler_phone_discovery() {
	notifications_invoke_function(PHONE_FUNC_PHONE_DISCOVERY);
}

static void opt_handler_send_sms() {
	notifications_invoke_function(PHONE_FUNC_SEND_SMS);
}

static void opt_handler_call_contact() {
	notifications_invoke_function(PHONE_FUNC_CALL_CONTACT);
}

static void opt_handler_gtasks() {
	notifications_invoke_function(PHONE_FUNC_GTASKS);
}

static void opt_handler_notifications() {
	notifications_invoke_function(NOTIFICATIONS_FUNCTION_RESEND);
}
static void opt_handler_watch_faces() {
	scr_mngr_show_screen_with_param(SCR_WATCH_SET_LIST, WATCH_SET_TYPE_WATCH_FACE);
}

static void opt_handler_set_alarm() {
	scr_mngr_show_screen(SCR_SET_ALARM);
}

static void opt_handler_dark_hours() {
	scr_mngr_show_screen(SCR_DARK_HOURS);
}

static void opt_handler_silent_hours() {
	scr_mngr_show_screen(SCR_SILENT_HOURS);
}

static void opt_handler_light_delay() {
	uint8_t delay = get_ext_ram_byte(EXT_RAM_LIGHT_DURATION);
	if (++delay > 9)
		delay = 1;
	put_ext_ram_byte(EXT_RAM_LIGHT_DURATION, delay);
}

void fs_reformat(void);

static void reformat() {
	fs_reformat();
	scr_mngr_show_screen(SCR_WATCHFACE);
}

static void draw_alarm_switch(uint8_t x, uint8_t y) {
	bool on = is_alarm_active();
	draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_shake_light_switch(uint8_t x, uint8_t y) {
		default_action* default_actions = config_get_default_global_actions();
		bool on = default_actions[8].action_id;
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_notif_light_switch(uint8_t x, uint8_t y) {
		bool on = get_settings(CONFIG_NOTIFICATION_LIGHT);
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_buttons_light_switch(uint8_t x, uint8_t y) {
		bool on = get_settings(CONFIG_BUTTONS_LIGHT);
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_oclock_switch(uint8_t x, uint8_t y) {
		bool on = get_settings(CONFIG_OCLOCK);
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_colors_switch(uint8_t x, uint8_t y) {
		bool on = is_mlcd_inverted();
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_disconnect_alert_switch(uint8_t x, uint8_t y) {
	bool on = get_settings(CONFIG_DISCONNECT_ALERT);
	draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_accel_switch(uint8_t x, uint8_t y) {
	bool on = get_settings(CONFIG_ACCELEROMETER);
	draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_sleep_switch(uint8_t x, uint8_t y) {
	bool on = get_settings(CONFIG_SLEEP_AS_ANDROID);
	draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_silent_hours(uint8_t x, uint8_t y) {
	uint8_t h1 = get_ext_ram_byte(EXT_RAM_SILENT_HOURS);
	uint8_t h2 = get_ext_ram_byte(EXT_RAM_SILENT_HOURS + 1);
	char txt[6];
	txt[0] = '0' + h1/10;
	txt[1] = '0' + h1%10;
	txt[2] = '-';
	txt[3] = '0' + h2/10;
	txt[4] = '0' + h2%10;
	txt[5] = '\0';
	mlcd_draw_text(txt, x, y+2, MLCD_XRES-SUMMARY_X-MARGIN_LEFT, NULL, FONT_NORMAL_REGULAR, HORIZONTAL_ALIGN_RIGHT);
}

static void draw_dark_hours(uint8_t x, uint8_t y) {
	uint8_t h1 = get_ext_ram_byte(EXT_RAM_DARK_HOURS);
	uint8_t h2 = get_ext_ram_byte(EXT_RAM_DARK_HOURS + 1);
	char txt[6];
	txt[0] = '0' + h1/10;
	txt[1] = '0' + h1%10;
	txt[2] = '-';
	txt[3] = '0' + h2/10;
	txt[4] = '0' + h2%10;
	txt[5] = '\0';
	mlcd_draw_text(txt, x, y+2, MLCD_XRES-SUMMARY_X-MARGIN_LEFT, NULL, FONT_NORMAL_REGULAR, HORIZONTAL_ALIGN_RIGHT);
}

static void draw_light_delay(uint8_t x, uint8_t y) {
	uint8_t delay = get_ext_ram_byte(EXT_RAM_LIGHT_DURATION);
	char txt[3];
	txt[0] = '0' + delay;
	txt[1] = 's';
	txt[2] = '\0';
	mlcd_draw_text(txt, x, y, MLCD_XRES-SUMMARY_X-MARGIN_LEFT, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_RIGHT);
}

static void draw_interval_summary(uint8_t x, uint8_t y) {
	uint16_t text = MESSAGE_1_SECOND;
	if (rtc_get_refresh_interval() == RTC_INTERVAL_MINUTE)
		text = MESSAGE_1_MINUTE;
	mlcd_draw_text(I18N_TRANSLATE(text), x, y, MLCD_XRES-SUMMARY_X-MARGIN_LEFT, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_RIGHT);
}

static void draw_steps(uint8_t x, uint8_t y) {
	uint16_t s = get_steps();
	char count[6] = "    0\0";
	for (int i = 4; i >= 0 && s > 0; i--) {
		count[i] = '0' + s % 10;
		s /= 10;
	}
	mlcd_draw_text(count, x-42, y, MLCD_XRES-SUMMARY_X-MARGIN_LEFT, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_RIGHT);
	bool on = get_settings(CONFIG_PEDOMETER);
	draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void rtc_refresh_toggle() {
	settings_toggle(CONFIG_SLOW_REFRESH);
	rtc_toggle_refresh_interval();
}

static void disconnect_alert_toggle() {
	settings_toggle(CONFIG_DISCONNECT_ALERT);
}

static void notif_light_toggle() {
	settings_toggle(CONFIG_NOTIFICATION_LIGHT);
}

static void buttons_light_toggle() {
	settings_toggle(CONFIG_BUTTONS_LIGHT);
}

static void oclock_toggle() {
	settings_toggle(CONFIG_OCLOCK);
}

static void pedometer_toggle() {
	settings_toggle(CONFIG_PEDOMETER);
	accel_interrupts_reset();
}

static void sleep_as_android_toggle() {
	settings_toggle(CONFIG_SLEEP_AS_ANDROID);
	accel_interrupts_reset();
}

static void accelerometer_toggle() {
	settings_toggle(CONFIG_ACCELEROMETER);
	accel_interrupts_reset();
}

static void shake_light_toggle() {
		default_action* default_actions = config_get_default_global_actions();
		if (default_actions[8].action_id == 0)
				default_actions[8].action_id = WATCH_SET_FUNC_TEMPORARY_BACKLIGHT;
		else
				default_actions[8].action_id = 0;
		config_set_default_global_actions(default_actions);
}

// TEST DIALOG
//static void select_item_handler(uint8_t token, uint8_t buttons, uint8_t item) {
//}

//static void test_handler() {
//		pack_dialog_select(0, 0, &select_item_handler, FONT_OPTION_NORMAL, SELECT_CHECK, I18N_TRANSLATE(MESSAGE_ABOUT),
//				15, "One\0Two\0Three\0Four\0Five\0Six\0Seven\0Eight\0Nine\0Ten\0Eleven\0Twelve\0Thurteen\0Fourteen\0Fifteen\0");
//		set_modal_dialog(true);
//		scr_mngr_show_screen_with_param(SCR_DIALOG_SELECT, EXT_RAM_DATA_DIALOG_TEXT);
//}

static const MENU_OPTION settings_menu[] = {
		{MESSAGE_TIMER, opt_handler_timer, opt_handler_timer, NULL},
		{MESSAGE_SEND_SMS, opt_handler_send_sms, opt_handler_send_sms, NULL},
		{MESSAGE_CALL_CONTACT, opt_handler_call_contact, opt_handler_call_contact, NULL},
		{MESSAGE_GTASKS, opt_handler_gtasks, opt_handler_gtasks, NULL},
		{MESSAGE_NOTIFICATIONS, opt_handler_notifications, opt_handler_notifications, NULL},
		{MESSAGE_WATCH_FACES, opt_handler_watch_faces, opt_handler_watch_faces, NULL},
		{MESSAGE_PHONE_DISCOVERY, opt_handler_phone_discovery, opt_handler_phone_discovery, NULL},
		{MESSAGE_ALARM_CLOCK, opt_handler_set_alarm, alarm_toggle, draw_alarm_switch},
		{MESSAGE_DISPLAY, mlcd_colors_toggle, mlcd_colors_toggle, draw_colors_switch},
		{MESSAGE_SHAKE_LIGHT, shake_light_toggle, shake_light_toggle, draw_shake_light_switch},
		{MESSAGE_NOTIF_LIGHT, notif_light_toggle, notif_light_toggle, draw_notif_light_switch},
		{MESSAGE_BUTTONS_LIGHT, buttons_light_toggle, buttons_light_toggle, draw_buttons_light_switch},
		{MESSAGE_DARK_HOURS, opt_handler_dark_hours, opt_handler_dark_hours, draw_dark_hours},
		{MESSAGE_SILENT_HOURS, opt_handler_silent_hours, opt_handler_silent_hours, draw_silent_hours},
		{MESSAGE_LIGHT_DELAY, opt_handler_light_delay, opt_handler_light_delay, draw_light_delay},
		{MESSAGE_OCLOCK, oclock_toggle, oclock_toggle, draw_oclock_switch},
		{MESSAGE_DISCONNECT_ALERT, disconnect_alert_toggle, disconnect_alert_toggle, draw_disconnect_alert_switch},
		{MESSAGE_RTC_REFRESH, rtc_refresh_toggle, rtc_refresh_toggle, draw_interval_summary},
		{MESSAGE_STEPS, pedometer_toggle, reset_steps, draw_steps},
		{MESSAGE_SLEEP_AS_ANDROID, sleep_as_android_toggle, sleep_as_android_toggle, draw_sleep_switch},
		{MESSAGE_ACCELEROMETER, accelerometer_toggle, accelerometer_toggle, draw_accel_switch},
	  {MESSAGE_DATE, opt_handler_change_date, opt_handler_change_date, NULL},
		{MESSAGE_TIME, opt_handler_change_time, opt_handler_change_time, NULL},
		{MESSAGE_FORMAT, reformat, reformat, NULL},
		{MESSAGE_RESTART, NVIC_SystemReset, NVIC_SystemReset, NULL},
//		{MESSAGE_ABOUT, test_handler, test_handler, NULL}
};

static const uint8_t SIZE_OF_MENU = sizeof(settings_menu)/sizeof(MENU_OPTION);

static void draw_option(uint_fast8_t item) {
	uint_fast8_t yPos = HEADER_HEIGHT + 4 + MENU_ITEM_HEIGHT * (item % MENU_ITEMS_PER_PAGE);
 	mlcd_draw_text(I18N_TRANSLATE(settings_menu[item].message_key), MARGIN_LEFT, yPos, MLCD_XRES-MARGIN_LEFT, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_LEFT);
	void (*s_drawer)(uint8_t x, uint8_t y) = settings_menu[item].summary_drawer;
	uint8_t sel_width = MLCD_XRES;
	if (s_drawer != NULL) {
		s_drawer(SUMMARY_X, yPos);
		sel_width = SUMMARY_X;
	}
	if (item == selectedOption)
		fillRectangle(0, yPos-2, sel_width, MENU_ITEM_HEIGHT, DRAW_XOR);
}

static void scr_settings_draw_options() {
		uint8_t page_no = selectedOption / MENU_ITEMS_PER_PAGE;
		uint8_t start_item = page_no * MENU_ITEMS_PER_PAGE;
		uint8_t items_no;
		if (SIZE_OF_MENU - start_item < MENU_ITEMS_PER_PAGE)
				items_no = SIZE_OF_MENU - start_item;
		else
				items_no = MENU_ITEMS_PER_PAGE;
		for (int i=0; i<items_no; i++) {
				draw_option(start_item+i);
		}
		if (page_no > 0)
				fillUp(MLCD_XRES/2, HEADER_HEIGHT-SCROLL_HEIGHT-2, SCROLL_HEIGHT, DRAW_WHITE);
		if (page_no + 1 < CEIL(SIZE_OF_MENU, MENU_ITEMS_PER_PAGE))
				fillDown(MLCD_XRES/2, MLCD_YRES-2, SCROLL_HEIGHT, DRAW_WHITE);
		if (lastSelectedOption / MENU_ITEMS_PER_PAGE == 1 && page_no == 0) {
				fillRectangle(0, 0, MLCD_XRES, HEADER_HEIGHT, DRAW_BLACK);
				scr_mngr_draw_notification_bar();
		}
}

static void scr_settings_refresh_screen() {
  scr_mngr_redraw_notification_bar();
  if (lastSelectedOption == selectedOption)
		return;
  if (lastSelectedOption / MENU_ITEMS_PER_PAGE != selectedOption / MENU_ITEMS_PER_PAGE) {
		// on page change
		fillRectangle(0, HEADER_HEIGHT, MLCD_XRES, MLCD_YRES-HEADER_HEIGHT, DRAW_BLACK);
		scr_settings_draw_options();
  } else {
		// on item change
		uint8_t sel_width = MLCD_XRES;
		if (settings_menu[selectedOption].summary_drawer != NULL)
			sel_width = SUMMARY_X;
		uint_fast8_t yPos = HEADER_HEIGHT + 2 + MENU_ITEM_HEIGHT * (selectedOption % MENU_ITEMS_PER_PAGE);
		fillRectangle(0, yPos, sel_width, MENU_ITEM_HEIGHT, DRAW_XOR);
		sel_width = MLCD_XRES;
		if (settings_menu[lastSelectedOption].summary_drawer != NULL)
			sel_width = SUMMARY_X;
		yPos = HEADER_HEIGHT + 2 + MENU_ITEM_HEIGHT * (lastSelectedOption % MENU_ITEMS_PER_PAGE);
		fillRectangle(0, yPos, sel_width, MENU_ITEM_HEIGHT, DRAW_XOR);
	}
	lastSelectedOption = selectedOption;
}

static void scr_settings_init() {
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
				uint_fast8_t yPos = HEADER_HEIGHT+MENU_ITEM_HEIGHT*(selectedOption%MENU_ITEMS_PER_PAGE);
				fillRectangle(0, yPos+2, MLCD_XRES, MENU_ITEM_HEIGHT, DRAW_BLACK);
				draw_option(selectedOption);
		}
}

static bool scr_settings_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_UP:
				    if (selectedOption > 0)
								selectedOption--;
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				    if (selectedOption+1 < SIZE_OF_MENU)
								selectedOption++;
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
			  case SCR_EVENT_PARAM_BUTTON_UP:
				    if (selectedOption > MENU_ITEMS_PER_PAGE)
								selectedOption -= MENU_ITEMS_PER_PAGE;
						else
								selectedOption = 0;
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				    if (selectedOption + MENU_ITEMS_PER_PAGE < SIZE_OF_MENU)
								selectedOption += MENU_ITEMS_PER_PAGE;
						else
								selectedOption = SIZE_OF_MENU-1;
				    return true;
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

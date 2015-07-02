#include "scr_changedate.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../i18n/i18n.h"
#include <time.h>

static int8_t day;
static int8_t month;
static int16_t year;

#define DATE_Y_POS              96
#define DATE_HEIGHT             24
#define DATE_DIGIT_WIDTH        13
#define DATE_DIGIT_THICKNESS     3
#define MODE_DAY              0x01
#define MODE_MONTH            0x02
#define MODE_YEAR             0x03

static uint8_t change_mode;

static void scr_changedate_draw_day() {
	  mlcd_draw_digit(day/10, 3, DATE_Y_POS, DATE_DIGIT_WIDTH, DATE_HEIGHT, DATE_DIGIT_THICKNESS);
	  mlcd_draw_digit(day%10, 18, DATE_Y_POS, DATE_DIGIT_WIDTH, DATE_HEIGHT, DATE_DIGIT_THICKNESS);
}

static void scr_changedate_draw_month() {
	  mlcd_draw_digit(month/10, 43, DATE_Y_POS, DATE_DIGIT_WIDTH, DATE_HEIGHT, DATE_DIGIT_THICKNESS);
	  mlcd_draw_digit(month%10, 58, DATE_Y_POS, DATE_DIGIT_WIDTH, DATE_HEIGHT, DATE_DIGIT_THICKNESS);
}

static void scr_changedate_draw_year() {
	  mlcd_draw_digit(year/1000, 83, DATE_Y_POS, DATE_DIGIT_WIDTH, DATE_HEIGHT, DATE_DIGIT_THICKNESS);
	  mlcd_draw_digit(year%1000/100, 98, DATE_Y_POS, DATE_DIGIT_WIDTH, DATE_HEIGHT, DATE_DIGIT_THICKNESS);
	  mlcd_draw_digit(year%100/10, 113, DATE_Y_POS, DATE_DIGIT_WIDTH, DATE_HEIGHT, DATE_DIGIT_THICKNESS);
	  mlcd_draw_digit(year%10, 128, DATE_Y_POS, DATE_DIGIT_WIDTH, DATE_HEIGHT, DATE_DIGIT_THICKNESS);
}

static void scr_changedate_draw_all() {
	  mlcd_fb_clear();
	
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_SET_DATE), 20, 13, NULL, NULL, FONT_OPTION_BIG, 0);
	  mlcd_draw_rect(0, 50, MLCD_XRES, 2);
	
	  mlcd_draw_rect(35, DATE_Y_POS + DATE_HEIGHT - 4, 4, 4);
	  mlcd_draw_rect(75, DATE_Y_POS + DATE_HEIGHT - 4, 4, 4);
	
	  if (change_mode == MODE_DAY) {
        mlcd_draw_rect_border(0, DATE_Y_POS - 3, 34, DATE_HEIGHT+6, 1);
        mlcd_draw_arrow_up(0, DATE_Y_POS + 32, 34, 16, 6);
        mlcd_draw_arrow_down(0, DATE_Y_POS - 24, 34, 16, 6);
    } else if (change_mode == MODE_MONTH) {
        mlcd_draw_rect_border(40, DATE_Y_POS - 3, 34, DATE_HEIGHT+6, 1);
        mlcd_draw_arrow_up(40, DATE_Y_POS + 32, 34, 16, 6);
        mlcd_draw_arrow_down(40, DATE_Y_POS - 24, 34, 16, 6);
    } else if (change_mode == MODE_YEAR) {
        mlcd_draw_rect_border(80, DATE_Y_POS - 3, 64, DATE_HEIGHT+6, 1);
        mlcd_draw_arrow_up(95, DATE_Y_POS + 32, 34, 16, 6);
        mlcd_draw_arrow_down(95, DATE_Y_POS - 24, 34, 16, 6);
    }
    
	  scr_changedate_draw_day();
	  scr_changedate_draw_month();
	  scr_changedate_draw_year();
}

static void scr_changedate_handle_button_up(void) {
	  if (change_mode == MODE_DAY) {
			  if(++day > 31) {
					  day = 1;
				}
				scr_changedate_draw_day();
		} else if (change_mode == MODE_MONTH) {
			  if(++month > 12) {
					  month = 1;
				}
				scr_changedate_draw_month();
		} else if (change_mode == MODE_YEAR) {
		    if(year >= 2099) {
					  return;
				}
				year++;
				scr_changedate_draw_year();
		}
	  mlcd_fb_flush();
}

static void scr_changedate_handle_button_down(void) {
		if (change_mode == MODE_DAY) {
			  if(--day < 1) {
					  day = 31;
				}
				scr_changedate_draw_day();
		}	else if(change_mode == MODE_MONTH){
			  if(--month < 1) {
					  month = 12;
				}
				scr_changedate_draw_month();
		}	else if(change_mode == MODE_YEAR) {
		    if(year <= 2000) {
					  return;
				}
			  year--;
				scr_changedate_draw_year();
		}
	  mlcd_fb_flush();
}

static void scr_changedate_handle_button_select(void) {
	  if (change_mode == MODE_DAY) {
			  change_mode = MODE_MONTH;
			  scr_changedate_draw_all();
	      mlcd_fb_flush();
		} else if (change_mode == MODE_MONTH) {
			  change_mode = MODE_YEAR;
			  scr_changedate_draw_all();
	      mlcd_fb_flush();
		} else if (change_mode == MODE_YEAR) {
				time_t t;
				time(&t);
				struct tm* time_struct = localtime(&t);

				time_struct->tm_mday = day;
				time_struct->tm_mon = month - 1;
				time_struct->tm_year = year - 1900;
			  rtc_set_current_time(mktime(time_struct));
			  scr_mngr_show_screen(SCR_SETTINGS);
		}
}


static void scr_changedate_handle_button_back(void) {
	  
	  if (change_mode == MODE_DAY) {
		    scr_mngr_show_screen(SCR_SETTINGS);
		} else if (change_mode == MODE_MONTH) {
			  change_mode = MODE_DAY;
			  scr_changedate_draw_all();
	      mlcd_fb_flush();
		} else if (change_mode == MODE_YEAR) {
			  change_mode = MODE_MONTH;
			  scr_changedate_draw_all();
	      mlcd_fb_flush();
		}
}

static void scr_changedate_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
					  scr_changedate_handle_button_up();
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
					  scr_changedate_handle_button_down();
				    break;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  scr_changedate_handle_button_select();
				    break;
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_changedate_handle_button_back();
				    break;
		}
}

static void scr_changedate_init() {
	  time_t t;
	  time(&t);
	  struct tm* time_struct = localtime(&t);
	
	  day = time_struct->tm_mday;
	  month = time_struct->tm_mon + 1;
	  year = 1900 + time_struct->tm_year;
	
	  change_mode = MODE_DAY;
	  scr_changedate_draw_all();
	  mlcd_fb_flush();
}

void scr_changedate_handle_event(uint32_t event_type, uint32_t event_param) {
		switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_changedate_init();
				    break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_changedate_handle_button_pressed(event_param);
				    break;
		}
}

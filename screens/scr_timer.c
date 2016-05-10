#include <app_scheduler.h>
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../i18n/i18n.h"
#include "../ext_ram.h"
#include "../graph.h"
#include "../vibration.h"

#define TIMER_VIBRATION		0x0060E738
#define TIME_Y_POS  			50
#define ARROW_HEIGHT 			8
#define TIMER_DEFAULT			0x0100
#define TIMER_MINUTES			0x8000

#define MODE_EDIT_FIRST		0x01
#define MODE_PLAY					0x02
#define MODE_PAUSED				0x04
#define MODE_UNIT_MIN			0x08
#define MODE_OLD_VALUES		0x30
#define MODE_SAVE_NEW			0x40
#define MODE_VISIBLE			0x80

static app_timer_id_t     f_timer_id;
static uint8_t 						timer_mode;
static int8_t 						timer_1;
static int8_t 						timer_2;

static void scr_draw_timer_1() {
	if ((timer_mode & MODE_VISIBLE) == 0)
		return;
  mlcd_draw_digit(timer_1/10, 4, TIME_Y_POS, 28, 40, 4);
  mlcd_draw_digit(timer_1%10, 36, TIME_Y_POS, 28, 40, 4);
}

static void scr_draw_timer_2() {
	if ((timer_mode & MODE_VISIBLE) == 0)
		return;
  mlcd_draw_digit(timer_2/10, 80, TIME_Y_POS, 28, 40, 4);
  mlcd_draw_digit(timer_2%10, 112, TIME_Y_POS, 28, 40, 4);
}

static void scr_timer_draw_arrows() {
	if ((timer_mode & MODE_VISIBLE) == 0)
		return;
	if (timer_mode & MODE_EDIT_FIRST) {
		fillDown(34, TIME_Y_POS + 52, ARROW_HEIGHT, DRAW_XOR);
		fillUp(34, TIME_Y_POS - 13, ARROW_HEIGHT, DRAW_XOR);
	} else {
		fillDown(110, TIME_Y_POS + 52, ARROW_HEIGHT, DRAW_XOR);
		fillUp(110, TIME_Y_POS - 13, ARROW_HEIGHT, DRAW_XOR);
	}
}

static void scr_timer_draw_units() {
	if ((timer_mode & MODE_VISIBLE) == 0)
		return;
	fillRectangle(0, TIME_Y_POS+55, MLCD_XRES, 20, DRAW_BLACK);
	if (timer_mode & MODE_UNIT_MIN) {
		mlcd_draw_text(I18N_TRANSLATE(MESSAGE_HOURS), 0, TIME_Y_POS+55, MLCD_XRES/2, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
		mlcd_draw_text(I18N_TRANSLATE(MESSAGE_MINUTES), MLCD_XRES/2, TIME_Y_POS+55, MLCD_XRES/2, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
	} else {
		mlcd_draw_text(I18N_TRANSLATE(MESSAGE_MINUTES), 0, TIME_Y_POS+55, MLCD_XRES/2, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
		mlcd_draw_text(I18N_TRANSLATE(MESSAGE_SECONDS), MLCD_XRES/2, TIME_Y_POS+55, MLCD_XRES/2, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
	}
}

static void scr_mark_timer() {
	uint8_t marked = (timer_mode >> 4) & 3;
	rectangle((marked&1)*72, TIME_Y_POS+79+20*(marked>>1), MLCD_XRES/2, 17, DRAW_XOR);
}

static void scr_draw_saved_timers() {
	if ((timer_mode & MODE_VISIBLE) == 0)
		return;
	for (int i = 0; i < 4; i++) {
		uint16_t t = get_ext_ram_short(EXT_RAM_TIMER_0+(i<<1));
		uint8_t t1 = t >> 8 & 0x7F;
		uint8_t t2 = t & 0xFF;
		if (t == 0 || t2 > 59)
			return;
		char txt[7];
		txt[6] = '\0';
		if (t & TIMER_MINUTES) {
			txt[2] = 'h';
			txt[5] = 'm';
		} else {
			txt[2] = 'm';
			txt[5] = 's';
		}
		txt[0] = '0'+t1/10;
		if (txt[0]=='0')
			txt[0] = ' ';
		txt[1] = '0'+t1%10;
		txt[3] = '0'+t2/10;
		txt[4] = '0'+t2%10;
		mlcd_draw_text(txt, (i&1)*72, TIME_Y_POS+80+20*(i>>1), MLCD_XRES/2, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
	}
	scr_mark_timer();
}

static void scr_draw_timer_all() {
	if ((timer_mode & MODE_VISIBLE) == 0)
		return;
  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_TIMER), 10, 5, 80, NULL, FONT_OPTION_BIG, HORIZONTAL_ALIGN_LEFT);
	
  fillRectangle(69, TIME_Y_POS + 24, 5, 5, DRAW_WHITE);
  fillRectangle(69, TIME_Y_POS + 10, 5, 5, DRAW_WHITE);
  scr_draw_timer_1();
  scr_draw_timer_2();
	scr_timer_draw_units();
	if (timer_mode & MODE_PAUSED)
		scr_draw_saved_timers();
	if ((timer_mode & MODE_PLAY) == 0)
		scr_timer_draw_arrows();
}

static void scr_load_saved_timer(uint8_t index) {
	uint16_t t = get_ext_ram_short(EXT_RAM_TIMER_0 + (index<<1));
	if (t == 0)
		return;
	if (t & TIMER_MINUTES)
		timer_mode |= MODE_UNIT_MIN;
	else
		timer_mode &= ~MODE_UNIT_MIN;
	timer_1 = t >> 8 & 0x7F;
	timer_2 = t & 0xFF;
  scr_draw_timer_1();
  scr_draw_timer_2();
	scr_timer_draw_units();
}

static void scr_save_timer() {
	if (timer_1 == 0 && timer_2 == 0)
		return;
	uint16_t t = timer_1 << 8 | timer_2;
	if (timer_mode & MODE_UNIT_MIN)
		t |= TIMER_MINUTES;
	timer_mode &= ~MODE_OLD_VALUES;
	int8_t match_index = 3;
	for (int i = 0; i < 4; i++) {
		uint16_t saved = get_ext_ram_short(EXT_RAM_TIMER_0+(i<<1));
		if (saved == 0 || t == saved) {
			match_index = i;
			break;
		}
	}
	for (int i = match_index; i > 0; i--) {
		uint16_t saved = get_ext_ram_short(EXT_RAM_TIMER_0+((i-1)<<1));
		put_ext_ram_short(EXT_RAM_TIMER_0+(i<<1), saved);
	}
	put_ext_ram_short(EXT_RAM_TIMER_0, t);
}

static void scr_timer_roll_saved() {
	uint16_t t = 0x7fff & get_ext_ram_short(EXT_RAM_TIMER_0 + (((timer_mode >> 4) & 3) << 1));
	if (t != (timer_1 << 8 | timer_2)) {
		// if timer was started then just reset
		timer_1 = t >> 8;
		timer_2 = t & 0xff;
		scr_draw_timer_1();
		scr_draw_timer_2();
	} else {
		uint8_t saved_index = (timer_mode >> 4) + 1 & 3;
		scr_load_saved_timer(saved_index);
		scr_mark_timer();
		// store the index
		timer_mode &= ~MODE_OLD_VALUES;
		timer_mode |= (saved_index << 4) | MODE_SAVE_NEW;
		scr_mark_timer();
	}
}

static void scr_timer_pause() {
	if ((timer_mode & MODE_PLAY) == 0 || timer_mode & MODE_PAUSED)
		return;
	app_timer_stop(f_timer_id);
	timer_mode ^= MODE_PAUSED;
	scr_draw_saved_timers();
	mlcd_fb_flush();
}

static void scr_timer_resume() {
	if ((timer_mode & MODE_PLAY) == 0 || (timer_mode & MODE_PAUSED) == 0)
		return;
	if (timer_1 == 0 && timer_2 == 0)
		return;
	timer_mode ^= MODE_PAUSED;
	uint16_t t_unit;
	if (timer_mode & MODE_UNIT_MIN)
		t_unit = RTC_INTERVAL_MINUTE;
	else
		t_unit = RTC_INTERVAL_SECOND;
	uint32_t err_code = app_timer_start(f_timer_id, APP_TIMER_TICKS(1000*t_unit, APP_TIMER_PRESCALER), NULL);
//	APP_ERROR_CHECK(err_code);
	if (timer_mode & MODE_SAVE_NEW) {
		scr_save_timer();
		timer_mode &= ~MODE_SAVE_NEW;
	}
	fillRectangle(0, TIME_Y_POS+79, MLCD_XRES, 38, DRAW_BLACK);
}

static void scr_timer_toggle_play() {
	if ((timer_mode & MODE_PLAY) == 0)
		return;
	if (timer_mode & MODE_PAUSED)
		scr_timer_resume();
	else
		scr_timer_pause();
}

static void scr_timer_handle_button_up() {
	if (timer_mode & MODE_PLAY) {
		if (timer_mode & MODE_PAUSED) {
			timer_mode ^= MODE_PLAY;
			scr_timer_draw_arrows();
		}
	} else {
		if (timer_mode & MODE_EDIT_FIRST) {
			if (++timer_1 > 99) {
				timer_1 = 0;
			}
			scr_draw_timer_1();
		}	else {
			if (++timer_2 > 59) {
				timer_2 = 0;
			}
			scr_draw_timer_2();
		} 	
		mlcd_fb_flush();
	}
}

static void scr_timer_handle_button_down() {
	if (timer_mode & MODE_PLAY) {
		scr_timer_pause();
		scr_timer_roll_saved();
	} else {
		if (timer_mode & MODE_EDIT_FIRST) {
			if (--timer_1 < 0) {
				timer_1 = 99;
			}
			scr_draw_timer_1();
		}	else {
			if (--timer_2 < 0 ) {
				timer_2 = 59;
			}
			scr_draw_timer_2();
		} 	
		mlcd_fb_flush();
	}
}

static void scr_timer_handle_button_up_long() {
	if (timer_mode & MODE_PLAY) {
	} else {
		timer_mode ^= MODE_UNIT_MIN;
		scr_timer_draw_units();
	}
}

static void scr_timer_handle_button_down_long() {
	if (timer_mode & MODE_PLAY) {
	} else {
		scr_timer_roll_saved();
	}
}

static bool scr_timer_handle_button_pressed(uint32_t button_id) {
  switch (button_id) {
	  case SCR_EVENT_PARAM_BUTTON_UP:
			scr_timer_handle_button_up();
	    return true;
	  case SCR_EVENT_PARAM_BUTTON_DOWN:
			scr_timer_handle_button_down();
	    return true;
	  case SCR_EVENT_PARAM_BUTTON_SELECT:
			if (timer_mode & MODE_PLAY) {
				scr_timer_toggle_play();
			} else {
			// in edit mode
			// focus on next control in edit mode
				scr_timer_draw_arrows();
				timer_mode ^= MODE_EDIT_FIRST;
				scr_timer_draw_arrows();
			}
			return true;
		case SCR_EVENT_PARAM_BUTTON_BACK:
			if (timer_mode & MODE_PLAY) {
				timer_mode &= ~MODE_VISIBLE;
				scr_mngr_show_screen(SCR_SETTINGS);
			} else {
				// from edit mode to play mode
				scr_load_saved_timer(timer_mode >> 4 & 3);
				timer_mode ^= MODE_PLAY;
				scr_timer_draw_arrows();
			}
			return true;
	}
	return false;
}

static bool scr_timer_handle_button_long_pressed(uint32_t button_id) {
  switch (button_id) {
	  case SCR_EVENT_PARAM_BUTTON_UP:
			scr_timer_handle_button_up_long();
	    return true;
	  case SCR_EVENT_PARAM_BUTTON_DOWN:
			scr_timer_handle_button_down_long();
	    return true;
	  case SCR_EVENT_PARAM_BUTTON_SELECT:
			if (timer_mode & MODE_PLAY) {
			} else {
				// in edit mode
				if (timer_1 == 0 && timer_2 == 0)
					return true;
				// go to play mode and start the timer
				timer_mode |= MODE_PLAY | MODE_PAUSED | MODE_SAVE_NEW;
				scr_timer_resume();
				scr_timer_draw_arrows();
			}
			return true;
	}
	return false;
}

static void scr_timer_init() {
	if (timer_mode == 0) {
		timer_mode |= MODE_PLAY | MODE_PAUSED;
		for (int i = 0; i < 4; i++) {
			uint16_t t = get_ext_ram_short(EXT_RAM_TIMER_0+(i<<1));
			if (t == 0 || (t & 0xFF) > 59)
				put_ext_ram_short(EXT_RAM_TIMER_0+(i<<1), TIMER_DEFAULT);
		}
		scr_load_saved_timer(0);
	}
	timer_mode |= MODE_VISIBLE;
}

bool scr_timer_handle_event(uint32_t event_type, uint32_t event_param) {
	switch(event_type) {
	  case SCR_EVENT_INIT_SCREEN:
	    scr_timer_init();
	    return true;
	  case SCR_EVENT_DRAW_SCREEN:
			scr_draw_timer_all();
	    return true;
	  case SCR_EVENT_BUTTON_PRESSED:
	    return scr_timer_handle_button_pressed(event_param);
	  case SCR_EVENT_BUTTON_LONG_PRESSED:
	    return scr_timer_handle_button_long_pressed(event_param);
	}
	return false;
}

void timer_tick_event(void * p_event_data, uint16_t event_size)
{
	timer_2--;
	if (timer_2 < 0) {
		timer_2 = 59;
		timer_1--;
	}
	scr_draw_timer_1();
	scr_draw_timer_2();
	mlcd_fb_flush();
	if (timer_2 == 0 && timer_1 == 0) {
		scr_timer_pause();
		vibration_vibrate(TIMER_VIBRATION, 3000, true);
		mlcd_backlight_blink(500, 3);
		scr_load_saved_timer(0);
	}
}

static void timer_timeout_handler(void * p_context) {
	uint32_t err_code = app_sched_event_put(NULL, 0, timer_tick_event);
	APP_ERROR_CHECK(err_code);
}

void timer_feature_init(void) {
	uint32_t err_code;	 
	err_code = app_timer_create(&f_timer_id,
                              APP_TIMER_MODE_REPEATED,
                              timer_timeout_handler);
	APP_ERROR_CHECK(err_code);
}

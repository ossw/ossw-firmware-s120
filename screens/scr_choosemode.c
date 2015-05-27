#include <string.h>
#include "scr_test.h"
#include "nrf_delay.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../utf8.h"
#include "../i18n/i18n.h"

#include "../ble/ble_central.h"
#include "../ble/ble_peripheral.h"

#define OPT1_POS 30
#define OPT2_POS 114

static void scr_choosemode_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
  	        ble_peripheral_mode_init();
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
	          ble_central_mode_init();
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    break;
		}
}

static void scr_choosemode_init() {
	  mlcd_fb_clear();
	                 
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_MODE_PERIPHERAL), 7, OPT1_POS, NULL, NULL, FONT_OPTION_BIG);
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_MODE_CENTRAL), 30, OPT2_POS, NULL, NULL, FONT_OPTION_BIG);
	  mlcd_draw_rect(0, 83, MLCD_XRES, 2);
	
	  mlcd_fb_flush();
}

void scr_choosemode_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_choosemode_init();
				    break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_choosemode_handle_button_pressed(event_param);
				    break;
		}
}

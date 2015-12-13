#include <string.h>
#include "scr_choosemode.h"
#include "nrf_delay.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../utf8.h"
#include "../i18n/i18n.h"

#include "../ble/ble_central.h"
#include "../ble/ble_peripheral.h"

static bool scr_choosemode_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
  	        ble_peripheral_mode_init();
					  scr_mngr_show_screen(SCR_WATCHFACE);
						return true;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  scr_mngr_show_screen(SCR_WATCHFACE);
						return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
	          ble_central_mode_init();
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    return true;
		}
		return false;
}

static void scr_choosemode_draw() {
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_MODE_PERIPHERAL), 0, 16, MLCD_XRES, 30, FONT_OPTION_BIG, HORIZONTAL_ALIGN_CENTER);
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_MODE_OFFLINE), 0, 74, MLCD_XRES, 30, FONT_OPTION_BIG, HORIZONTAL_ALIGN_CENTER);
	  mlcd_draw_text(I18N_TRANSLATE(MESSAGE_MODE_CENTRAL), 0, 129, MLCD_XRES, 30, FONT_OPTION_BIG, HORIZONTAL_ALIGN_CENTER);
	  mlcd_draw_rect(0, 55, MLCD_XRES, 2);
		mlcd_draw_rect(0, 113, MLCD_XRES, 2);
}

bool scr_choosemode_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_DRAW_SCREEN:
				    scr_choosemode_draw();
				    return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    return scr_choosemode_handle_button_pressed(event_param);
		}
		return false;
}

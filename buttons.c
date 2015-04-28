#include "buttons.h"
#include "scr_mngr.h"
#include "app_button.h"
#include "board.h"

static app_timer_id_t         m_button_long_press_timer_id;

static void button_handler(uint8_t pin_no, uint8_t button_action) {
    uint32_t err_code;
	  if(APP_BUTTON_PUSH == button_action){
		
		  err_code = app_timer_start(m_button_long_press_timer_id,
                               BUTTON_LONG_PRESS_DELAY,
                               (void *)(pin_no));
      if (err_code != NRF_SUCCESS)
      {
          // The impact in app_button of the app_timer queue running full is losing a button press.
          // The current implementation ensures that the system will continue working as normal. 
      }
   }
}

static app_button_cfg_t buttons[] =
{
    {BUTTON_UP, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_handler},
    {BUTTON_DOWN, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_handler},
    {BUTTON_SELECT, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_handler},
    {BUTTON_BACK, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_handler}
};

static void button_long_press_timeout_handler(void * p_context) {
    uint32_t err_code;
	  uint8_t pin_no = (uint8_t)((uint32_t)p_context&0xFF);
	  bool pushed = false;

	  for (uint32_t i = 0; i < 4; i++)
    {
        app_button_cfg_t * p_btn = &buttons[i];

        if (pin_no == p_btn->pin_no)
        {
						err_code = app_button_is_pushed(i, &pushed);
					  if (err_code != NRF_SUCCESS)
					  {
							  // do nothing
						}
							
						if (pushed) {
								scr_mngr_handle_event(SCR_EVENT_BUTTON_LONG_PRESSED, 1<<i);
						} else {
								scr_mngr_handle_event(SCR_EVENT_BUTTON_PRESSED, 1<<i);
						}
				}
	  }
}

void buttons_init(void) {
    uint32_t err_code;
	  app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY);
	
	  err_code = app_timer_create(&m_button_long_press_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                button_long_press_timeout_handler);
    APP_ERROR_CHECK(err_code);
	
		app_button_enable();
}

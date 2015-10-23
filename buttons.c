#include "buttons.h"
#include "scr_mngr.h"
#include "app_button.h"
#include "board.h"

static app_timer_id_t         m_button_long_press_timer_id;


static void button_handler(uint8_t pin_no, uint8_t button_action);

static app_button_cfg_t buttons[] =
{
    {BUTTON_UP, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_handler},
    {BUTTON_DOWN, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_handler},
    {BUTTON_SELECT, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_handler},
    {BUTTON_BACK, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_handler}
};

static const uint8_t BUTTONS_NO = sizeof(buttons)/sizeof(buttons[0]);

static uint32_t long_press_handled = 0;

static void button_handler(uint8_t pin_no, uint8_t button_action) {
    uint32_t err_code;
	
	  if (APP_BUTTON_PUSH == button_action){
			long_press_handled &= ~(1<<pin_no);
		  err_code = app_timer_start(m_button_long_press_timer_id,
                               BUTTON_LONG_PRESS_DELAY,
                               (void *)((uint32_t)pin_no));
      if (err_code != NRF_SUCCESS)
      {
          // The impact in app_button of the app_timer queue running full is losing a button press.
          // The current implementation ensures that the system will continue working as normal. 
      }
   } else {
			if (!(long_press_handled & (1<<pin_no))) {
					for (uint32_t i = 0; i < BUTTONS_NO; i++)
					{
							app_button_cfg_t * p_btn = &buttons[i];

							if (pin_no == p_btn->pin_no)
							{
									app_timer_stop(m_button_long_press_timer_id);
									scr_mngr_handle_event(SCR_EVENT_BUTTON_PRESSED, 1<<i);
							}
					}
			}
	 }
}

static void button_long_press_timeout_handler(void * p_context) {
    uint32_t err_code;
	  uint8_t pin_no = (uint8_t)((uint32_t)p_context&0xFF);
	  bool pushed = false;

	  for (uint32_t i = 0; i < BUTTONS_NO; i++)
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
								// button is still pushed so this is a long press
								// mark this pin as used so normal handler that will be invoked after button release will not trigger event
								long_press_handled |= 1<<pin_no;
								scr_mngr_handle_event(SCR_EVENT_BUTTON_LONG_PRESSED, 1<<i);
						}
				}
	  }
}

void buttons_init(void) {
    uint32_t err_code;
	  err_code = app_button_init(buttons, BUTTONS_NO, BUTTON_DETECTION_DELAY);
		APP_ERROR_CHECK(err_code);
	
	  err_code = app_timer_create(&m_button_long_press_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                button_long_press_timeout_handler);
    APP_ERROR_CHECK(err_code);
	
		app_button_enable();
}

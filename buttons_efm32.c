#include "buttons.h"
#include "scr_mngr.h"
#include "target.h"
#include "gpiointerrupt.h"

static void button_back_callback(uint8_t pin)
{
		scr_mngr_handle_event(SCR_EVENT_BUTTON_PRESSED, SCR_EVENT_PARAM_BUTTON_BACK);
}

static void button_up_callback(uint8_t pin)
{
		scr_mngr_handle_event(SCR_EVENT_BUTTON_PRESSED, SCR_EVENT_PARAM_BUTTON_UP);
}

static void button_select_callback(uint8_t pin)
{
		scr_mngr_handle_event(SCR_EVENT_BUTTON_PRESSED, SCR_EVENT_PARAM_BUTTON_SELECT);
}

static void button_down_callback(uint8_t pin)
{
		scr_mngr_handle_event(SCR_EVENT_BUTTON_PRESSED, SCR_EVENT_PARAM_BUTTON_DOWN);
}


void buttons_init(void) {
		GPIOINT_CallbackRegister(BUTTON_BACK & 0xF, button_back_callback);
		GPIOINT_CallbackRegister(BUTTON_UP & 0xF, button_up_callback);
		GPIOINT_CallbackRegister(BUTTON_SELECT & 0xF, button_select_callback);
		GPIOINT_CallbackRegister(BUTTON_DOWN & 0xF, button_down_callback);
}

void button_is_pushed(uint8_t pin_no, bool *p_is_pushed) {
}

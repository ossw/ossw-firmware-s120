#include "buttons.h"
#include "scr_mngr.h"
#include "target.h"
#include "gpiointerrupt.h"
#include "gpio.h"
#include "timer.h"

static timer_id_t button_debouncing_timer_id;
static timer_id_t button_long_press_timer_id;
static uint32_t last_pressed_pin;

static uint32_t int_pin_to_gpio_pin(uint8_t pin) {
		switch (pin) {
			case BUTTON_UP&0xF:
					return BUTTON_UP;
			case BUTTON_SELECT&0xF:
					return BUTTON_SELECT;
			case BUTTON_DOWN&0xF:
					return BUTTON_DOWN;
			case BUTTON_BACK&0xF:
					return BUTTON_BACK;
		}
		return 0xFFFFFFFF;
}

static uint32_t gpio_pin_to_button_id(uint32_t pin) {
		switch (pin) {
			case BUTTON_UP:
					return SCR_EVENT_PARAM_BUTTON_UP;
			case BUTTON_SELECT:
					return SCR_EVENT_PARAM_BUTTON_SELECT;
			case BUTTON_DOWN:
					return SCR_EVENT_PARAM_BUTTON_DOWN;
			case BUTTON_BACK:
					return SCR_EVENT_PARAM_BUTTON_BACK;
		}
		return 0xFFFFFFFF;
}

static void button_pressed_callback(uint8_t pin)
{
		//timer_stop(button_debouncing_timer_id);
		int gpio_pin = int_pin_to_gpio_pin(pin);
		bool pressed;
		button_is_pushed(gpio_pin, &pressed);
		if (pressed) {
				if (gpio_pin == SCR_EVENT_PARAM_BUTTON_UP) {
						bool another_pressed = false;
						button_is_pushed(SCR_EVENT_PARAM_BUTTON_DOWN, &another_pressed);
						if (another_pressed) {
								mcu_reset();
						}
				} else if (gpio_pin == SCR_EVENT_PARAM_BUTTON_DOWN) {
						bool another_pressed = false;
						button_is_pushed(SCR_EVENT_PARAM_BUTTON_UP, &another_pressed);
						if (another_pressed) {
								mcu_reset();
						}
				}
			
				timer_start_with_param(button_debouncing_timer_id, BUTTON_DETECTION_DELAY, (void*)gpio_pin);
		} else {
				if (gpio_pin == last_pressed_pin) {
						timer_stop(button_long_press_timer_id);
						scr_mngr_handle_event(SCR_EVENT_BUTTON_PRESSED, gpio_pin_to_button_id(gpio_pin));
				}
				last_pressed_pin = 0xFFFFFFFF;
		}
}

static void button_press_debouncing_timeout(uint32_t id, uint32_t gpio_pin) {
		timer_stop(button_long_press_timer_id);
		bool pressed;
		button_is_pushed(gpio_pin, &pressed);
		if (pressed) {
				last_pressed_pin = gpio_pin;
				timer_start(button_long_press_timer_id, BUTTON_LONG_PRESS_DELAY);
		}
}

static void button_long_press_timeout(uint32_t id) {
		bool pressed;
		button_is_pushed(last_pressed_pin, &pressed);
		if (pressed) {
				scr_mngr_handle_event(SCR_EVENT_BUTTON_LONG_PRESSED, gpio_pin_to_button_id(last_pressed_pin));
				last_pressed_pin = 0xFFFFFFFF;
		}
}

void buttons_init(void) {
		GPIOINT_CallbackRegister(BUTTON_BACK & 0xF, button_pressed_callback);
		GPIOINT_CallbackRegister(BUTTON_UP & 0xF, button_pressed_callback);
		GPIOINT_CallbackRegister(BUTTON_SELECT & 0xF, button_pressed_callback);
		GPIOINT_CallbackRegister(BUTTON_DOWN & 0xF, button_pressed_callback);
	
		timer_create(&button_debouncing_timer_id,
                                TIMER_TYPE_SINGLE_SHOT,
                                button_press_debouncing_timeout);
		timer_create(&button_long_press_timer_id,
                                TIMER_TYPE_SINGLE_SHOT,
                                button_long_press_timeout);
}

void button_is_pushed(uint8_t pin_no, bool *value) {
		*value = !gpio_pin_is_set(pin_no);
}

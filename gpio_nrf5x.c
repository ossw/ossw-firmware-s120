#include "gpio.h"
#include "nrf_gpio.h"

void gpio_pin_clear(uint32_t pin) {
		nrf_gpio_pin_clear(pin);
}

void gpio_pin_set(uint32_t pin) {
		nrf_gpio_pin_set(pin);
}

bool gpio_pin_is_set(uint32_t pin) {
		return nrf_gpio_pin_read(pin);
}

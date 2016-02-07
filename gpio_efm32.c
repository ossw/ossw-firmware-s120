#include "gpio.h"
#include "target.h"
#include "em_gpio.h"

void gpio_pin_clear(uint32_t pin) {
		GPIO_PinOutClear((GPIO_Port_TypeDef)(pin>>4), pin&0xF);
}

void gpio_pin_set(uint32_t pin) {
		GPIO_PinOutSet((GPIO_Port_TypeDef)(pin>>4), pin&0xF);
}

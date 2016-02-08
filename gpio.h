#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <stdint.h>

void gpio_pin_clear(uint32_t pin);
void gpio_pin_set(uint32_t pin);
bool gpio_pin_is_set(uint32_t pin);

#endif /* GPIO_H */

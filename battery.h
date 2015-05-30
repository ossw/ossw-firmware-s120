#ifndef BATTERY_H
#define BATTERY_H

#include <stdbool.h>
#include <stdint.h>

void battery_init(void);

uint8_t battery_get_level(void);

#endif /* BATTERY_H */

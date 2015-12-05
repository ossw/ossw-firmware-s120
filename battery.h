#ifndef BATTERY_H
#define BATTERY_H

#include <stdbool.h>
#include <stdint.h>

void battery_init(void);

uint32_t battery_get_level(void);

bool battery_is_charging(void);

bool battery_is_full(void);


#endif /* BATTERY_H */

#ifndef ACCEL_H
#define ACCEL_H

#include <stdbool.h>
#include <stdint.h>

void accel_init(void);

void accel_get_values(int8_t * x, int8_t * y, int8_t * z);

void accel_write_register(uint8_t reg, uint8_t value);

void accel_read_register(uint8_t reg, uint8_t* value);

void accel_read_multi_register(uint8_t reg, uint8_t* value, uint8_t legth);

#endif /* ACCEL_H */

#ifndef ACCEL_H
#define ACCEL_H

#include <stdbool.h>
#include <stdint.h>

void accel_init(void);
void accel_get_values(int8_t * x, int8_t * y, int8_t * z);
void accel_init_timer(void);


#endif /* ACCEL_H */

#ifndef MCU_H
#define MCU_H

#include <stdbool.h>
#include <stdint.h>

void mcu_delay_ms(uint32_t ms);
void mcu_delay_us(uint32_t us);
void mcu_power_manage(void);
void mcu_reset(void);

#endif /* MCU_H */

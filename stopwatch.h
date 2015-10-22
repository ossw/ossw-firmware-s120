#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <stdbool.h>
#include <stdint.h>

void stopwatch_init(void);

void stopwatch_fn_start(void);
void stopwatch_fn_stop(void);
void stopwatch_fn_reset(void);
void stopwatch_fn_start_stop(void);
void stopwatch_fn_next_lap(void);

uint32_t stopwatch_get_current_lap_number(void);
uint32_t stopwatch_get_current_lap_time(void);
uint32_t stopwatch_get_current_lap_h(void);
uint32_t stopwatch_get_current_lap_m(void);
uint32_t stopwatch_get_current_lap_s(void);
uint32_t stopwatch_get_current_lap_cs(void);
uint32_t stopwatch_get_current_lap_ms(void);

uint32_t stopwatch_get_last_lap_time(void);
uint32_t stopwatch_get_last_lap_h(void);
uint32_t stopwatch_get_last_lap_m(void);
uint32_t stopwatch_get_last_lap_s(void);
uint32_t stopwatch_get_last_lap_cs(void);
uint32_t stopwatch_get_last_lap_ms(void);

uint32_t stopwatch_get_total_time(void);
uint32_t stopwatch_get_total_h(void);
uint32_t stopwatch_get_total_m(void);
uint32_t stopwatch_get_total_s(void);
uint32_t stopwatch_get_total_cs(void);
uint32_t stopwatch_get_total_ms(void);

#endif /* STOPWATCH_H */

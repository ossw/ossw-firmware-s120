#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <stdbool.h>
#include <stdint.h>

#define STOPWATCH_STATE_EMPTY 		0
#define STOPWATCH_STATE_RUNNING 	1
#define STOPWATCH_STATE_PAUSED 		2

void stopwatch_init(void);
void stopwatch_process(void);

void stopwatch_fn_start(void);
void stopwatch_fn_stop(void);
void stopwatch_fn_reset(void);
void stopwatch_fn_start_stop(void);
void stopwatch_fn_next_lap(void);

uint32_t stopwatch_get_current_lap_number(void);
uint32_t stopwatch_get_current_lap_time(void);
uint32_t stopwatch_get_current_lap_split(void);
uint32_t stopwatch_get_last_lap_time(void);
uint32_t stopwatch_get_total_time(void);
uint32_t stopwatch_get_recall_lap_time(uint16_t recall_lap_no);
uint32_t stopwatch_get_recall_lap_split(uint16_t recall_lap_no);
uint32_t stopwatch_get_state(void);

#endif /* STOPWATCH_H */

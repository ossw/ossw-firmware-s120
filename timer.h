#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

#define TIMER_TYPE_SINGLE_SHOT 0
#define TIMER_TYPE_PERIODIC 1

typedef uint32_t timer_id_t;
typedef uint32_t timer_type_t;

void timer_init(void);

void timer_create(timer_id_t*, timer_type_t, void* handler);

void timer_start(timer_id_t, uint32_t timeout);
	
void timer_stop(timer_id_t);

void timer_cnt_get(uint32_t* result);

void timer_cnt_ms_diff_compute(uint32_t current, uint32_t last, uint32_t* result);
	
#endif /* TIMER_H */

#include "timer.h"
#include "target.h"
#include "app_timer.h"

void timer_init() {
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);
}

void timer_create(timer_id_t* id, timer_type_t type, void* handler) {
    int err_code = app_timer_create((app_timer_id_t*)&id,
                                (app_timer_mode_t)type,
                                (app_timer_timeout_handler_t)handler);
    APP_ERROR_CHECK(err_code);
}

void timer_start(timer_id_t id, uint32_t timeout) {
		app_timer_start((app_timer_id_t)id, APP_TIMER_TICKS(timeout, APP_TIMER_PRESCALER), NULL);	
}
	
void timer_stop(timer_id_t id) {
		app_timer_stop((app_timer_id_t)id);
}

void timer_cnt_get(uint32_t* result) {
		app_timer_cnt_get(result);
}

void timer_cnt_ms_diff_compute(uint32_t current, uint32_t last, uint32_t* result) {
		app_timer_cnt_diff_compute(current, last, result);
}


//1000*diff/APP_TIMER_CLOCK_FREQ
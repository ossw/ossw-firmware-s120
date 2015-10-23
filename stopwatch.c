#include "stopwatch.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "board.h"

// interrupt is only to increase number of fps in main loop
#define INTERRUPT_INTERVAL            APP_TIMER_TICKS(125, APP_TIMER_PRESCALER)
#define MS_COUNTER_UPDATE_INTERVAL    APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)

#define TIME_H_PART(v)	(v/3600000)
#define TIME_M_PART(v)	((v%3600000)/60000)
#define TIME_S_PART(v)	((v%60000)/1000)
#define TIME_CS_PART(v)	((v%1000)/10)
#define TIME_MS_PART(v) (v%1000)

static app_timer_id_t stopwatch_timer_id;

static volatile uint32_t ms_counter = 0;
static bool ms_counter_active = false;
static uint32_t ms_counter_last_ticks = 0;

static uint32_t lap_start = 0;
static uint8_t lap_no = 1;
static uint32_t last_lap_length = 0;

static void stopwatch_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);

		if (ms_counter_active) {
				uint32_t current_ticks;
				uint32_t diff;
				app_timer_cnt_get(&current_ticks);
				app_timer_cnt_diff_compute(current_ticks, ms_counter_last_ticks, &diff);

				if (diff >= MS_COUNTER_UPDATE_INTERVAL) {
						ms_counter += 1000*diff/APP_TIMER_CLOCK_FREQ;
						ms_counter_last_ticks = current_ticks;
				}
		}
}

void stopwatch_init(void) {
    uint32_t err_code;	 
    err_code = app_timer_create(&stopwatch_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                stopwatch_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

static uint32_t stopwatch_get_ms_counter_value() {
		volatile uint32_t ms_count1, ms_count2;
		uint32_t start_ticks, curr_ticks;
		do {
				ms_count1 = ms_counter;
				if (ms_counter_active) {
						app_timer_cnt_get(&curr_ticks);
						start_ticks = ms_counter_last_ticks;
				} else {
						return ms_count1;
				}
				ms_count2 = ms_counter;
		} while (ms_count1 != ms_count2); // should not happen more that once every 120s
		
		uint32_t diff;
		app_timer_cnt_diff_compute(curr_ticks, start_ticks, &diff);
		return ms_count1 + (1000*diff/APP_TIMER_CLOCK_FREQ);
}

void stopwatch_fn_start(void) {
		if (ms_counter_active) {
				return;
		}
		app_timer_cnt_get(&ms_counter_last_ticks);
		ms_counter_active = true;
    uint32_t err_code = app_timer_start(stopwatch_timer_id, INTERRUPT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

void stopwatch_fn_stop(void) {
		if (!ms_counter_active) {
				return;
		}
		
    uint32_t err_code = app_timer_stop(stopwatch_timer_id);
    APP_ERROR_CHECK(err_code);
		
		uint32_t current_ticks;
		uint32_t diff;
		ms_counter_active = false;
		app_timer_cnt_get(&current_ticks);
		app_timer_cnt_diff_compute(current_ticks, ms_counter_last_ticks, &diff);
		ms_counter += 1000*diff/APP_TIMER_CLOCK_FREQ;
}

void stopwatch_fn_start_stop(void) {
		if (ms_counter_active) {
				stopwatch_fn_stop();
		} else {
				stopwatch_fn_start();
		}
}

void stopwatch_fn_reset(void) {
		app_timer_cnt_get(&ms_counter_last_ticks);
		ms_counter = 0;
		lap_start = 0; 
		lap_no = 1;
		last_lap_length = 0;
}

void stopwatch_fn_next_lap(void) {
		if (stopwatch_get_current_lap_time() == 0) {
				// do not count empty lap
				return;
		}
		uint32_t curr = stopwatch_get_ms_counter_value();
		last_lap_length = curr - lap_start;
		lap_start = curr;
		lap_no++;
}

uint32_t stopwatch_get_current_lap_number(void) {
    return lap_no;
}

uint32_t stopwatch_get_current_lap_time(void) {
    return stopwatch_get_ms_counter_value() - lap_start;
}

uint32_t stopwatch_get_current_lap_h(void) {
    return TIME_H_PART(stopwatch_get_current_lap_time());
}

uint32_t stopwatch_get_current_lap_m(void) {
    return TIME_M_PART(stopwatch_get_current_lap_time());
}

uint32_t stopwatch_get_current_lap_s(void) {
    return TIME_S_PART(stopwatch_get_current_lap_time());
}

uint32_t stopwatch_get_current_lap_cs(void) {
    return TIME_CS_PART(stopwatch_get_current_lap_time());
}

uint32_t stopwatch_get_current_lap_ms(void) {
    return TIME_MS_PART(stopwatch_get_current_lap_time());
}

uint32_t stopwatch_get_last_lap_time(void) {
		return last_lap_length;
}

uint32_t stopwatch_get_last_lap_h(void) {
		return TIME_H_PART(last_lap_length);
}

uint32_t stopwatch_get_last_lap_m(void) {
		return TIME_M_PART(last_lap_length);
}

uint32_t stopwatch_get_last_lap_s(void) {
		return TIME_S_PART(last_lap_length);
}

uint32_t stopwatch_get_last_lap_cs(void) {
		return TIME_CS_PART(last_lap_length);
}

uint32_t stopwatch_get_last_lap_ms(void) {
		return TIME_MS_PART(last_lap_length);
}

uint32_t stopwatch_get_total_time(void) {
		return stopwatch_get_ms_counter_value();
}

uint32_t stopwatch_get_total_h(void) {
		return TIME_H_PART(stopwatch_get_ms_counter_value());
}

uint32_t stopwatch_get_total_m(void) {
		return TIME_M_PART(stopwatch_get_ms_counter_value());
}

uint32_t stopwatch_get_total_s(void) {
		return TIME_S_PART(stopwatch_get_ms_counter_value());
}

uint32_t stopwatch_get_total_cs(void) {
		return TIME_CS_PART(stopwatch_get_ms_counter_value());
}

uint32_t stopwatch_get_total_ms(void) {
		return TIME_MS_PART(stopwatch_get_ms_counter_value());
}


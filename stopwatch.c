#include "stopwatch.h"
#include "timer.h"
#include "target.h"
#include "scr_mngr.h"
#include "ext_ram.h"

// interrupt is only to increase number of fps in main loop
#define INTERRUPT_INTERVAL            215
#define MS_COUNTER_UPDATE_INTERVAL    120000

#define STOPWATCH_RECALL_SIZE 200

static timer_id_t stopwatch_timer_id;

static volatile uint32_t ms_counter = 0;
static bool ms_counter_active = false;
static uint32_t ms_counter_last_ticks = 0;

static uint32_t current_lap_start = 0;
static uint8_t current_lap_no = 1;
static uint32_t last_lap_length = 0;
static bool lock_next_lap = false;

static void stopwatch_timeout_handler(void * p_context) {
		if (ms_counter_active) {
				uint32_t current_ticks;
				uint32_t diff;
				timer_cnt_get(&current_ticks);
				timer_cnt_ms_diff_compute(current_ticks, ms_counter_last_ticks, &diff);

				if (diff >= MS_COUNTER_UPDATE_INTERVAL) {
						ms_counter += diff;
						ms_counter_last_ticks = current_ticks;
				}
		}
		scr_mngr_redraw();
}

void stopwatch_init(void) {
    timer_create(&stopwatch_timer_id,
                                TIMER_TYPE_PERIODIC,
                                stopwatch_timeout_handler);
}

static uint32_t stopwatch_get_ms_counter_value() {
		volatile uint32_t ms_count1, ms_count2;
		uint32_t start_ticks, curr_ticks;
		do {
				ms_count1 = ms_counter;
				if (ms_counter_active) {
						timer_cnt_get(&curr_ticks);
						start_ticks = ms_counter_last_ticks;
				} else {
						return ms_count1;
				}
				ms_count2 = ms_counter;
		} while (ms_count1 != ms_count2); // should not happen more that once every 120s
		
		uint32_t diff;
		timer_cnt_ms_diff_compute(curr_ticks, start_ticks, &diff);
		return ms_count1 + diff;
}

void stopwatch_fn_start(void) {
		if (ms_counter_active) {
				return;
		}
		timer_cnt_get(&ms_counter_last_ticks);
		ms_counter_active = true;
    timer_start(stopwatch_timer_id, INTERRUPT_INTERVAL);
}

void stopwatch_fn_stop(void) {
		if (!ms_counter_active) {
				return;
		}
		
    timer_stop(stopwatch_timer_id);
		
		uint32_t current_ticks;
		uint32_t diff;
		ms_counter_active = false;
		timer_cnt_get(&current_ticks);
		timer_cnt_ms_diff_compute(current_ticks, ms_counter_last_ticks, &diff);
		ms_counter += diff;
}

void stopwatch_fn_start_stop(void) {
		if (ms_counter_active) {
				stopwatch_fn_stop();
		} else {
				stopwatch_fn_start();
		}
}

void stopwatch_fn_reset(void) {
		timer_cnt_get(&ms_counter_last_ticks);
		ms_counter = 0;
		current_lap_start = 0; 
		current_lap_no = 1;
		last_lap_length = 0;
}

static void stopwatch_remember_n_lap_start(uint32_t store_lap_no, uint32_t store_lap_start) {
		if (current_lap_no-store_lap_no >= STOPWATCH_RECALL_SIZE) {
				return;
		}
		uint32_t slot_no = (store_lap_no - 2)%STOPWATCH_RECALL_SIZE;
		ext_ram_write_data(EXT_RAM_DATA_STOPWATCH_RECALL + slot_no * 4, (uint8_t *)&store_lap_start, 4);
}

static uint32_t stopwatch_get_n_lap_start(uint32_t read_lap_no) {
		if (current_lap_no-read_lap_no >= STOPWATCH_RECALL_SIZE) {
				return 0;
		}
		uint32_t slot_no = (read_lap_no - 2)%STOPWATCH_RECALL_SIZE;
		uint32_t read_lap_start;
		ext_ram_read_data(EXT_RAM_DATA_STOPWATCH_RECALL + slot_no * 4, (uint8_t *)&read_lap_start, 4);
		return read_lap_start;
}

void stopwatch_fn_next_lap(void) {
		if (stopwatch_get_current_lap_time() == 0) {
				// do not count empty lap
				return;
		}
		if (lock_next_lap) {
				// previous lap time was not yet stored
				return;
		}
		uint32_t curr = stopwatch_get_ms_counter_value();
		last_lap_length = curr - current_lap_start;
		current_lap_start = curr;
		current_lap_no++;
		lock_next_lap = true;
}

uint32_t stopwatch_get_current_lap_number(void) {
    return current_lap_no;
}

uint32_t stopwatch_get_current_lap_time(void) {
    return stopwatch_get_ms_counter_value() - current_lap_start;
}

uint32_t stopwatch_get_current_lap_split(void) {
    return current_lap_start;
}

uint32_t stopwatch_get_last_lap_time(void) {
		return last_lap_length;
}

uint32_t stopwatch_get_total_time(void) {
		return stopwatch_get_ms_counter_value();
}
/*
void stopwatch_fn_recall_prev_lap(void) {
		if (recall_lap_no > 1 && (current_lap_no - 2 < STOPWATCH_RECALL_SIZE || current_lap_no - recall_lap_no + 1 < STOPWATCH_RECALL_SIZE)) {
				recall_lap_no--;
		}
}

void stopwatch_fn_recall_next_lap(void) {
		if (recall_lap_no < current_lap_no) {
				recall_lap_no++;
		}
}

void stopwatch_fn_recall_first_lap(void) {
		if (current_lap_no > STOPWATCH_RECALL_SIZE + 1) {
				recall_lap_no = current_lap_no - STOPWATCH_RECALL_SIZE + 1;
		} else {
				recall_lap_no = 1;
		}
}

void stopwatch_fn_recall_last_lap(void) {
		recall_lap_no = current_lap_no;
}

uint32_t stopwatch_get_recall_lap_number(void) {
    return recall_lap_no;
}*/

uint32_t stopwatch_get_recall_lap_time(uint16_t recall_lap_no) {
		if (recall_lap_no == current_lap_no) {
				return stopwatch_get_current_lap_time();
		}
		if (recall_lap_no == 1) {
				return stopwatch_get_n_lap_start(2);
		}
    return stopwatch_get_n_lap_start(recall_lap_no+1) - stopwatch_get_n_lap_start(recall_lap_no);
}

uint32_t stopwatch_get_recall_lap_split(uint16_t recall_lap_no) {
		if (recall_lap_no == current_lap_no) {
				return current_lap_start;
		}
		if (recall_lap_no == 1) {
				return 0;
		}
    return stopwatch_get_n_lap_start(recall_lap_no);
}

void stopwatch_process(void) {
		if (lock_next_lap) {
				stopwatch_remember_n_lap_start(current_lap_no, current_lap_start);
				lock_next_lap = false;
		}
}

uint32_t stopwatch_get_state(void) {
		if (ms_counter_active) {
				return STOPWATCH_STATE_RUNNING;
		} else {
			return ms_counter == 0? STOPWATCH_STATE_EMPTY : STOPWATCH_STATE_PAUSED;
		}
}


#include <math.h>
#include "vibration.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "board.h"
#include "nrf_gpio.h"
#include "ext_ram.h"
#include "rtc.h"

// 32bit pattern format: VVNNNNTT TTTTTTTT PPPPPPPP PPPPPPPP
// V - reserved for future use - just 0, 
// N - number of steps 1-16 (0 means all 16 steps), 
// T - length of a single step in milliseconds
// P - steps where 1 means vibration motor should be on, 0 means it should be off

static app_timer_id_t      m_vibration_pattern_timer_id;
static uint32_t m_pattern;
static int16_t step_counter;
static uint8_t m_next_step;

static void vibration_motor_on(void) {
    nrf_gpio_pin_set(VIBRATION_MOTOR);
}

static void vibration_motor_off(void) {
    nrf_gpio_pin_clear(VIBRATION_MOTOR);
}

void vibration_next_step(void * p_event_data, uint16_t event_size) {
		if (--step_counter <= 0) {
			  vibration_motor_off();
				uint32_t err_code = app_timer_stop(m_vibration_pattern_timer_id);
				APP_ERROR_CHECK(err_code);
				return;
		}
		
		if (m_pattern >> (15 - m_next_step) & 0x1) {
				vibration_motor_on();
		} else {
			  vibration_motor_off();
		}
		
	  m_next_step++;
		uint8_t steps_no = (m_pattern >> 26)&0xF;
		if (steps_no == 0) {
				steps_no = 16;
		}
	  if (m_next_step >= steps_no) {
				m_next_step = 0;
		}
}

static void vibration_pattern_change_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
		uint32_t err_code = app_sched_event_put(NULL, NULL, vibration_next_step);
		APP_ERROR_CHECK(err_code);
}

void vibration_init(void) {
  nrf_gpio_cfg_output(VIBRATION_MOTOR);
  nrf_gpio_pin_clear(VIBRATION_MOTOR);
	
  uint32_t err_code;	 
  err_code = app_timer_create(&m_vibration_pattern_timer_id,
                              APP_TIMER_MODE_REPEATED,
                              vibration_pattern_change_handler);
  APP_ERROR_CHECK(err_code);
	uint8_t s_hour1 = get_ext_ram_byte(EXT_RAM_SILENT_HOURS);
	uint8_t s_hour2 = get_ext_ram_byte(EXT_RAM_SILENT_HOURS + 1);
	if (s_hour1 > 23 || s_hour2 > 24)
		put_ext_ram_short(EXT_RAM_SILENT_HOURS, 0);
}

void vibration_vibrate(uint32_t pattern, uint16_t timeout, bool force) {
	uint8_t hour1 = get_ext_ram_byte(EXT_RAM_SILENT_HOURS);
	uint8_t hour2 = get_ext_ram_byte(EXT_RAM_SILENT_HOURS + 1);
	if (!force && rtc_in_hour_interval(hour1, hour2))
		return;
	
	m_pattern = pattern;
	m_next_step = 0;
	uint32_t step_length = (pattern >> 16) & 0x3FF;
	if (step_length == 0) {
		vibration_motor_off();
		return;
	}
	step_counter = CEIL(timeout, step_length);
	vibration_next_step(NULL, 0);
  uint32_t err_code;
  err_code = app_timer_start(m_vibration_pattern_timer_id, APP_TIMER_TICKS(step_length, APP_TIMER_PRESCALER), NULL);
  APP_ERROR_CHECK(err_code);
}

void vibration_stop() {
    uint32_t err_code;	 
	  err_code = app_timer_stop(m_vibration_pattern_timer_id);
    APP_ERROR_CHECK(err_code);
	  vibration_motor_off();
}

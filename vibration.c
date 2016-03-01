#include "vibration.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "board.h"
#include "nrf_gpio.h"

static app_timer_id_t      m_vibration_pattern_timer_id;
static app_timer_id_t      m_vibration_timeout_timer_id;
static uint32_t m_pattern;
static uint8_t m_next_step;

static void vibration_motor_on(void) {
    nrf_gpio_pin_set(VIBRATION_MOTOR);
}

static void vibration_motor_off(void) {
    nrf_gpio_pin_clear(VIBRATION_MOTOR);
}

void vibration_next_step(void) {
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
		vibration_next_step();
}

void vibration_timeout_event(void * p_event_data, uint16_t event_size) {
    uint32_t err_code;	 
	  err_code = app_timer_stop(m_vibration_pattern_timer_id);
    APP_ERROR_CHECK(err_code);
	  vibration_motor_off();
}

static void vibration_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
		uint32_t err_code = app_sched_event_put(NULL, NULL, vibration_timeout_event);
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
	
    err_code = app_timer_create(&m_vibration_timeout_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                vibration_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

void vibration_vibrate(uint32_t pattern, uint16_t timeout) {
	  m_pattern = pattern;
	  m_next_step = 0;
	  vibration_next_step();
	  uint32_t step_length = (pattern >> 16) & 0x3FF;
		if (step_length == 0) {
			  vibration_motor_off();
				return;
		}
	
    uint32_t err_code;
    err_code = app_timer_start(m_vibration_pattern_timer_id, APP_TIMER_TICKS(step_length, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
	
	  if (timeout != 0) {
				err_code = app_timer_start(m_vibration_timeout_timer_id, APP_TIMER_TICKS(timeout, APP_TIMER_PRESCALER), NULL);
				APP_ERROR_CHECK(err_code);
		}
}

void vibration_stop() {
    uint32_t err_code;	 
	  err_code = app_timer_stop(m_vibration_pattern_timer_id);
    APP_ERROR_CHECK(err_code);
	  err_code = app_timer_stop(m_vibration_timeout_timer_id);
    APP_ERROR_CHECK(err_code);
	  vibration_motor_off();
}

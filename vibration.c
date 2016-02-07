#include "vibration.h"
#include "timer.h"
#include "target.h"
#include "gpio.h"

static timer_id_t      m_vibration_pattern_timer_id;
static timer_id_t      m_vibration_timeout_timer_id;
static uint32_t m_pattern;
static uint8_t m_next_step;

static void vibration_motor_on(void) {
    gpio_pin_set(VIBRATION_MOTOR);
}

static void vibration_motor_off(void) {
    gpio_pin_clear(VIBRATION_MOTOR);
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
		vibration_next_step();
}

static void vibration_timeout_handler(void * p_context) {
    timer_stop(m_vibration_pattern_timer_id);
	  vibration_motor_off();
}

void vibration_init(void) {
	
    //nrf_gpio_cfg_output(VIBRATION_MOTOR);
    gpio_pin_clear(VIBRATION_MOTOR);
	
    timer_create(&m_vibration_pattern_timer_id,
                                TIMER_TYPE_PERIODIC,
                                vibration_pattern_change_handler);
	
    timer_create(&m_vibration_timeout_timer_id,
                                TIMER_TYPE_SINGLE_SHOT,
                                vibration_timeout_handler);
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
	
    timer_start(m_vibration_pattern_timer_id, step_length);
    
	  if (timeout != 0) {
				timer_start(m_vibration_timeout_timer_id, timeout);
		}
}

void vibration_stop() {
	  timer_stop(m_vibration_pattern_timer_id);
    timer_stop(m_vibration_timeout_timer_id);
    vibration_motor_off();
}

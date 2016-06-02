#include "accel.h"
#include "nrf_drv_gpiote.h"
#include "nordic_common.h"
#include "app_error.h"
#include "twi_master.h"
#include "board.h"
#include "mlcd.h"
#include "mlcd_draw.h"
#include "vibration.h"
#include "app_scheduler.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "mlcd.h"
#include "scr_mngr.h"
#include "notifications.h"
#include "ble/ble_peripheral.h"
#include "config.h"

#define DEFAULT_I2C_ADDR	0x1D
#define FIFO_PACKET_LEN		6	// 6 vectors fit one 20 byte message
#define FIFO_BYTES				3	// x+y+z
#define FIFO_PACKET_SIZE	FIFO_PACKET_LEN*FIFO_BYTES
#define FIFO_LENGTH				30	// take FIFO length divisible by packet length
#define FIFO_PACKETS			CEIL(FIFO_LENGTH,FIFO_PACKET_LEN)
#define FIFO_SIZE					FIFO_BYTES*FIFO_LENGTH

#define SLEEP_SAMPLE_COUNT				125	// number of stats samples for 10 seconds
#define SLEEP_MAX_BATCH_SIZE			12
#define SLEEP_BYTES								3	// min+max+avg

#define PED_AMP_MIN								30
#define PED_AMP_MAX								120
#define PED_DELAY_MIN							10
#define PED_DELAY_MAX							20
#define PED_DEBOUNCE_THRESHOLD		5
#define PED_WAIT_MAX							0x01
#define PED_WALKING								0x02

//#define DEBUG

uint16_t steps;
uint8_t steps_debounce;
int8_t old_dir;
int8_t peak_max;
int8_t peak_max_count;
int8_t peak_min;
int8_t peak_min_count;
int8_t next_max;
int8_t next_max_count;
int8_t next_min;
int8_t next_min_count;
uint8_t ped_mode;
int8_t sx;
int8_t sy;
int8_t sz;
uint8_t orientation;

uint8_t sleep_min;
uint8_t sleep_max;
uint16_t sleep_sum;
uint8_t sleep_count;
uint8_t sleep_batch_size;
uint8_t sleep_batch_count;
uint8_t sleep_batch[3*SLEEP_MAX_BATCH_SIZE];

uint16_t get_steps() {
	return steps;
}

void reset_steps() {
	steps = 0;
}

static void reset_sleep() {
	sleep_min = 0xff;
	sleep_max = 0;
	sleep_sum = 0;
	sleep_count = 0;
}

static void sleep_check_batch_queue() {
	if (++sleep_batch_count >= sleep_batch_size) {
		uint8_t packets = CEIL(sleep_batch_count, FIFO_PACKET_LEN);
		int shift = 0;
		uint8_t pos = SLEEP_BYTES*sleep_batch_count;;
		for (int p = 0; p < packets; p++) {
			ble_peripheral_invoke_notification_function_with_data(PHONE_FUNC_SLEEP_AS_ANDROID,
				sleep_batch+shift, MIN(pos-shift, FIFO_PACKET_SIZE));
			shift += FIFO_PACKET_SIZE;
		}
		sleep_batch_count = 0;
	}
}

static void sleep_handle_value(uint16_t val) {
	sleep_count++;
	sleep_sum += val;
	if (val < sleep_min)
		sleep_min = val;
	if (val > sleep_max)
		sleep_max = val;
	if (sleep_count >= SLEEP_SAMPLE_COUNT) { // calculate statistics and store
		uint8_t pos = SLEEP_BYTES*sleep_batch_count;
		sleep_batch[pos] = sleep_min;
		sleep_batch[pos+1] = sleep_max;
		sleep_batch[pos+2] = sleep_sum / sleep_count;
		reset_sleep();
		sleep_check_batch_queue();
	}
}

void sleep_set_batch_size(uint8_t size) {
	sleep_batch_size = size;
	sleep_check_batch_queue();
}

static inline void accel_process_step(bool count) {
	if (count) {
		if (ped_mode & PED_WALKING) {
			steps++;
			steps_debounce = PED_DEBOUNCE_THRESHOLD;
		} else {
			steps_debounce++;
			if (steps_debounce >= PED_DEBOUNCE_THRESHOLD) {
				ped_mode |= PED_WALKING;
				steps += steps_debounce;
			}
		}
		ped_mode ^= PED_WAIT_MAX;
	} else if (steps_debounce > 0) {
		steps_debounce--;
		if (steps_debounce == 0) {
			ped_mode &= ~PED_WALKING;
		}
	}
}

static void accel_int_handler(void * p_event_data, uint16_t event_size) {
	uint8_t int_src;
	uint8_t int_detail;
	accel_read_register(0x0C, &int_src);
//	const char hex_str[]= "0123456789ABCDEF";
//	char value[5] = "0x00\0";
#ifdef DEBUG
	value[2] = hex_str[int_src >> 4];
	value[3] = hex_str[int_src & 0xf];
	mlcd_draw_text(value, 15, 15, MLCD_XRES, NULL, FONT_NORMAL_BOLD, 0);
#endif

	if (int_src & 0x10) {
		accel_read_register(0x10, &int_detail);
		// tilt event
		if (orientation == 0x87 && int_detail == 0x85) {
			scr_mngr_handle_event(SCR_EVENT_WRIST_SHAKE, 0);
			orientation = 0;
		}
		if ((int_detail & 0x40) == 0) // if not in z-lockout zone
			orientation = int_detail;
	}
		
	// FIFO
	if (int_src & 0x40) {
		accel_read_register(0x00, &int_detail);
		// F_STATUS
		if (int_detail & 0x40) {
			int8_t data[2*FIFO_SIZE];
			accel_read_multi_register(0x01, (uint8_t *)data, 2*FIFO_SIZE);
			bool acc_sleep = get_settings(CONFIG_SLEEP_AS_ANDROID);
			bool acc_ped = get_settings(CONFIG_PEDOMETER);
			bool acc_all = get_settings(CONFIG_ACCELEROMETER);
			if (acc_sleep || acc_all || acc_ped) {
				for (int i = 0; i < FIFO_SIZE; i++)
					data[i] = data[i << 1];
				
				if (acc_sleep || acc_ped) {
					int dx, dy, dz, acc;
					for (int i = 0; i < FIFO_LENGTH; i++) {
						int pos = FIFO_BYTES * i;
						if (i == 0) {
							dx = data[pos]-sx;
							dy = data[pos+1]-sy;
							dz = data[pos+2]-sz;
							acc = sy;
						} else {
							dx = data[pos]-data[pos-3];
							dy = data[pos+1]-data[pos-2];
							dz = data[pos+2]-data[pos-1];
							acc = data[pos-2];
						}
						
						if (acc_ped) {
							int direction = SIGN(dy);
							if (steps_debounce > 0) {
								peak_min_count++;
								peak_max_count++;
								next_max_count++;
								next_min_count++;
								// up peak
								if (old_dir >= 0 && direction < 0) {
									// if not in walking mode start at any peak
									if (peak_max_count < PED_DELAY_MIN) {
										// restart searching if better peak found
										if (acc > peak_max) {
											peak_max = acc;
											peak_max_count = 0;
											next_max = -128;
											next_max_count = 0;
										}
									} else if (peak_max_count <= PED_DELAY_MAX && acc > next_max) {
										// choose the max in the window
										next_max = acc;
										next_max_count = 0;
									}
								}
								// down peak
								if (old_dir <= 0 && direction > 0) {
									// if not in walking mode start at any peak
									if (peak_min_count < PED_DELAY_MIN) {
										// restart searching if better peak found
										if (acc < peak_min) {
											peak_min = acc;
											peak_min_count = 0;
											next_min = 127;
											next_min_count = 0;
										}
									} else if (peak_min_count <= PED_DELAY_MAX && acc < next_min) {
										// choose the min in the window
										next_min = acc;
										next_min_count = 0;
									}	
								}
								
								// out of max window
								if (peak_max_count > PED_DELAY_MAX) {
									accel_process_step(ped_mode & PED_WAIT_MAX && next_max - peak_min > PED_AMP_MIN);
									peak_max = next_max;
									peak_max_count = next_max_count;
									next_max = -128;
									next_max_count = 0;
								}
								// out of min window
								if (peak_min_count > PED_DELAY_MAX) {
									accel_process_step((ped_mode & PED_WAIT_MAX) == 0 && peak_max - next_min > PED_AMP_MIN);
									peak_min = next_min;
									peak_min_count = next_min_count;
									next_min = 127;
									next_min_count = 0;
								}
							} else { // debounce counter == 0
								if (old_dir >= 0 && direction < 0) {
									steps_debounce++;
									ped_mode &= ~PED_WAIT_MAX;
									peak_max = acc;
									peak_max_count = 0;
									next_max = -128;
									next_max_count = 0;
								}
								if (old_dir <= 0 && direction > 0) {
									steps_debounce++;
									ped_mode |= PED_WAIT_MAX;
									peak_min = acc;
									peak_min_count = 0;
									next_min = 127;
									next_min_count = 0;
								}
							}
							
							old_dir = direction;
							// debug: replace x,z acceleration by peak value
							if (i > 0) {
								data[pos-3] = peak_max;
								data[pos-1] = peak_min;
							}
//		value[2] = hex_str[steps_debounce >> 4];
//		value[3] = hex_str[steps_debounce & 0xf];
//		mlcd_draw_text(value, 50, 0, MLCD_XRES, NULL, FONT_NORMAL_BOLD, 0);
//		value[2] = hex_str[ped_mode >> 4];
//		value[3] = hex_str[ped_mode & 0xf];
//		mlcd_draw_text(value, 50, 20, MLCD_XRES, NULL, FONT_NORMAL_BOLD, 0);
//		mlcd_fb_flush();
						}

						if (acc_sleep) {
							uint16_t delta = abs(dx) + abs(dy) + abs(dz);
							sleep_handle_value(delta);
						}
					}
					sx = data[FIFO_SIZE-3];
					sy = data[FIFO_SIZE-2];
					sz = data[FIFO_SIZE-1];
					// debug: replace x,z acceleration by peak value
					data[FIFO_SIZE-3] = peak_max;
					data[FIFO_SIZE-1] = peak_min;
				}

				if (acc_all)
					for (int p = 0; p < FIFO_PACKETS; p++)
						ble_peripheral_invoke_notification_function_with_data(PHONE_FUNC_ACCELEROMETER,
							(uint8_t *)&data[FIFO_PACKET_SIZE*p], FIFO_PACKET_SIZE);
			}
		}
	}
}

static void accel_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	uint32_t err_code = app_sched_event_put(NULL, NULL, accel_int_handler);
	APP_ERROR_CHECK(err_code);
}

static uint32_t accel_int_init(uint8_t pin_no) {
	uint32_t err_code;
	if (!nrf_drv_gpiote_is_init()) {
		err_code = nrf_drv_gpiote_init();
		if (err_code != NRF_SUCCESS) {
			return err_code;
		}
	}
	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
	config.pull = NRF_GPIO_PIN_PULLUP;

	err_code = nrf_drv_gpiote_in_init(pin_no, &config, accel_event_handler);
	if (err_code != NRF_SUCCESS) {
		return err_code;
	}
	nrf_drv_gpiote_in_event_enable(pin_no, true);
	return NRF_SUCCESS;
}

static void accel_fifo_init() {
	// 0x09 F_SETUP
	accel_write_register(0x09, 0x40 | FIFO_LENGTH);
}

static void accel_tilt_init() {
	// 0x11 PL_CFG
	accel_write_register(0x11, 0xC0);
	// 0x12 PL_COUNT
	accel_write_register(0x12, 0x02);
	// 0x13 PL_BF_ZCOMP
	accel_write_register(0x13, 0xC4);
	// 0x14 P_L_THS_REG
	accel_write_register(0x14, 0xA2);
}

//static void accel_motion_init() {
//	// 0x15 FF_MT_CFG
//	accel_write_register(0x15, 0xB8);
//	// 0x17 FF_MT_THS
//	accel_write_register(0x17, 0x08);
//	// 0x18 FF_MT_COUNT
//	accel_write_register(0x18, 0x04);
//}

//static void accel_transient_init() {
//	// 0x1D TRANSIENT_CFG
//	accel_write_register(0x1D, 0x16);
//	// 0x1F TRANSIENT_THS
//	accel_write_register(0x1F, 0x0A);
//}
//	
//static void accel_pulse_init() {
//	// 0x21 PULSE_CFG Pulse Configuration register
//	accel_write_register(0x21, 0x60);
//	// 0x23–0x25 PULSE_THSX, Y, Z Pulse Threshold for X, Y and Z registers
//	//accel_write_register(0x23, 0x20);
//	//accel_write_register(0x24, 0x20);
//	accel_write_register(0x25, 0x0A);
//	// 0x26 PULSE_TMLT Pulse Time Window 1 register
//	accel_write_register(0x26, 0x08);
//	// 0x27 PULSE_LTCY Pulse Latency Timer register
//	accel_write_register(0x27, 0x03);
//	// 0x28 PULSE_WIND register (Read/Write)
//	accel_write_register(0x28, 0x12);
//}

void accel_interrupts_reset() {
	// 0x2A: CTRL_REG1 System Control 1 register
	// go to standby mode
	accel_write_register(0x2A, 0);

	// 0x0E: XYZ_DATA_CFG register
	// HPF_OUT - disable high pass filter
	// FS - 2g
	accel_write_register(0x0E, 0x00);

	// 0x2B: CTRL_REG2 System Control 2 register
	// set Low Noise Low Power mode
	accel_write_register(0x2B, 0x09);
	// set Low Power mode
	//accel_write_register(0x2B, 0x1B);

	// 0x2C: CTRL_REG3 Interrupt Control register
	//accel_write_register(0x2C, 0x30);//0x40

	uint8_t interrupt = 0;
	if (get_settings(CONFIG_ACCELEROMETER) || get_settings(CONFIG_SLEEP_AS_ANDROID || get_settings(CONFIG_PEDOMETER))) {
		reset_sleep();
		accel_fifo_init();
		interrupt |= 0x40;
	}
	accel_tilt_init();
	interrupt |= 0x10;
	//accel_motion_init();
	//accel_transient_init();
	//accel_pulse_init();
	
	// 0x2D: CTRL_REG4 Interrupt Enable register (Read/Write)
	// transient+orientation+pulse+freefall
	//accel_write_register(0x2D, 0x3c);
	// FIFO + orientation
	accel_write_register(0x2D, interrupt);

	// 0x0F: HP_FILTER_CUTOFF 
	accel_write_register(0x0F, 0);
	
	// 0x29: ASLP_COUNT 
	//accel_write_register(0x29, 0x1);

	// go to active mode
	// 0x2A: CTRL_REG1 System Control 1 register
	// ASLP_RATE 01 = DR 101 = 12,5Hz
	accel_write_register(0x2A, 0x69);
	// ASLP_RATE 10 = DR 110 = 6,25Hz
	//accel_write_register(0x2A, 0xB1);
	// ASLP_RATE 00 = DR 100 = 50Hz
	// accel_write_register(0x2A, 0x23);
	next_min = 127;
	next_max = -128;
	ped_mode = 0;
	steps_debounce = 0;
	peak_max_count = 0;
	peak_min_count = 0;
}

void accel_init(void) {
	twi_master_init();
	
	uint32_t err_code;
	// err_code = accel_int_init(ACCEL_INT1);
	// APP_ERROR_CHECK(err_code);
	err_code = accel_int_init(ACCEL_INT2);
	APP_ERROR_CHECK(err_code);
	// 0x2B: CTRL_REG2 System Control 2 register
	// reset
	accel_write_register(0x2B, 0x40);
	nrf_delay_ms(1);
	accel_interrupts_reset();
	sleep_batch_size = 1;
}

void accel_write_register(uint8_t reg, uint8_t value) {
	uint8_t data[] = {reg, value};
	twi_master_transfer	(	DEFAULT_I2C_ADDR<<1, data, 2, true);
}

void accel_read_register(uint8_t reg, uint8_t* value) {
	twi_master_transfer	(	DEFAULT_I2C_ADDR<<1, &reg, 1, false);
	twi_master_transfer	(	(DEFAULT_I2C_ADDR<<1)|TWI_READ_BIT, value, 1, true);
}

void accel_read_multi_register(uint8_t reg, uint8_t* value, uint8_t length) {
	twi_master_transfer	(	DEFAULT_I2C_ADDR<<1, &reg, 1, false);
	twi_master_transfer	(	(DEFAULT_I2C_ADDR<<1)|TWI_READ_BIT, value, length, true);
}

#include "accel.h"
#include "nrf_drv_gpiote.h"
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
#include "ble/ble_peripheral.h"

#define DEFAULT_I2C_ADDR 0x1D
#define FIFO_SIZE 96
//#define DEBUG

uint16_t steps;
uint8_t orientation;
int8_t swing;

uint16_t get_steps() {
	return steps;
}

void reset_steps() {
	steps = 0;
}

static void accel_int_handler(void * p_event_data, uint16_t event_size) {
		uint8_t int_src;
		uint8_t int_detail;
		accel_read_register(0x0C, &int_src);
#ifdef DEBUG
		const char hex_str[]= "0123456789ABCDEF";
		char value[5] = "0x00\0";
		value[2] = hex_str[int_src >> 4];
		value[3] = hex_str[int_src & 0xf];
		mlcd_draw_text(value, 15, 15, MLCD_XRES, NULL, FONT_NORMAL_BOLD, 0);
#endif		
	
		if (int_src & 0x10) {
			accel_read_register(0x10, &int_detail);
#ifdef DEBUG
			value[2] = hex_str[int_detail >> 4];
			value[3] = hex_str[int_detail & 0xf];
			mlcd_draw_text(value, 15, 35, MLCD_XRES, NULL, FONT_NORMAL_BOLD, 0);
#endif
			if (int_detail == 0x87 || int_detail == 0x85) {
				if (swing == -1) {
					steps++;					
//					vibration_vibrate(0x04408000, 0x0080, true);
				}
				swing = 1;
			}	else if (int_detail == 0x81 || int_detail == 0x83) {
				if (swing == 1) {
					steps++;					
//					vibration_vibrate(0x04408000, 0x0100, true);
				}
				swing = -1;
			} else swing = 0;
			
			// tilt event
			if (orientation == 0x87 && int_detail == 0x85) {
				scr_mngr_handle_event(SCR_EVENT_WRIST_SHAKE, 0);
				orientation = 0;
			}
			if ((int_detail & 0x40) == 0) // if not in z-lockout zone
				orientation = int_detail;
		}

		// pulse detection
		if (int_src & 0x08) {
			accel_read_register(0x22, &int_detail);
//			value[2] = hex_str[int_detail >> 4];
//			value[3] = hex_str[int_detail & 0xf];
//			mlcd_draw_text(value, 15, 50, MLCD_XRES, NULL, FONT_NORMAL_BOLD, 0);
			// double tap
			if (int_detail & 0xc8)
				mlcd_backlight_blink(200, 2);
		}

		if (int_src & 0x04) {
			accel_read_register(0x16, &int_detail);
//			value[2] = hex_str[int_detail >> 4];
//			value[3] = hex_str[int_detail & 0xf];
//			mlcd_draw_text(value, 15, 65, MLCD_XRES, NULL, FONT_NORMAL_BOLD, 0);
			if (int_detail & 0x8a) {
				if (swing == 1) {
					steps++;
//					vibration_vibrate(0x04408000, 0x0100, true);
				}
				swing = -1;
//				char count[5] = "0000\0";
//				uint16_t s = steps;
//				count[3] = hex_str[s % 10];
//				s /= 10;
//				count[2] = hex_str[s % 10];
//				s /= 10;
//				count[1] = hex_str[s % 10];
//				s /= 10;
//				count[0] = hex_str[s];
//				mlcd_draw_text(count, 15, 120, MLCD_XRES, NULL, FONT_OPTION_NORMAL, 0);
			}
		}

		if (int_src & 0x20) {
			accel_read_register(0x1E, &int_detail);
//			value[2] = hex_str[int_detail >> 4];
//			value[3] = hex_str[int_detail & 0xf];
//			mlcd_draw_text(value, 15, 80, MLCD_XRES, NULL, FONT_NORMAL_BOLD, 0);
			if (int_detail & 0x4a) {
				if (swing == -1) {
					steps++;
//					vibration_vibrate(0x0C40A000, 0x0100, true);
				}
				swing = 1;
			}
		}
}

static void accel_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	uint32_t err_code = app_sched_event_put(NULL, NULL, accel_int_handler);
	APP_ERROR_CHECK(err_code);
}

void accel_get_values(int8_t * x, int8_t * y, int8_t * z) {
	int8_t data[FIFO_SIZE];
	accel_read_multi_register(0x01, (uint8_t*)data, FIFO_SIZE);
	*x = data[0];
	*y = data[1];
	*z = data[2];
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
	accel_write_register(0x09, 0x40);
}

static void accel_tilt_init() {
	// 0x11 PL_CFG
	accel_write_register(0x11, 0xC0);
	// 0x12 PL_COUNT
	accel_write_register(0x12, 0x02);
	// 0x13 PL_BF_ZCOMP
	accel_write_register(0x13, 0xC4);
	// 0x14 P_L_THS_REG
	accel_write_register(0x14, 0x9A);
}

static void accel_motion_init() {
	// 0x15 FF_MT_CFG
	accel_write_register(0x15, 0xB8);
	// 0x17 FF_MT_THS
	accel_write_register(0x17, 0x08);
	// 0x18 FF_MT_COUNT
	accel_write_register(0x18, 0x04);
}

static void accel_transient_init() {
	// 0x1D TRANSIENT_CFG
	accel_write_register(0x1D, 0x16);
	// 0x1F TRANSIENT_THS
	accel_write_register(0x1F, 0x0A);
}
	
static void accel_pulse_init() {
	// 0x21 PULSE_CFG Pulse Configuration register
	accel_write_register(0x21, 0x60);
	// 0x23–0x25 PULSE_THSX, Y, Z Pulse Threshold for X, Y and Z registers
	//accel_write_register(0x23, 0x20);
	//accel_write_register(0x24, 0x20);
	accel_write_register(0x25, 0x0A);
	// 0x26 PULSE_TMLT Pulse Time Window 1 register
	accel_write_register(0x26, 0x08);
	// 0x27 PULSE_LTCY Pulse Latency Timer register
	accel_write_register(0x27, 0x03);
	// 0x28 PULSE_WIND register (Read/Write)
	accel_write_register(0x28, 0x12);
}

void accel_init(void) {
		twi_master_init();
	
    uint32_t err_code;
		//err_code = accel_int_init(ACCEL_INT1);
    //APP_ERROR_CHECK(err_code);
		err_code = accel_int_init(ACCEL_INT2);
    APP_ERROR_CHECK(err_code);
	
		//0x2B: CTRL_REG2 System Control 2 register
		//reset
		accel_write_register(0x2B, 0x40);
	
		nrf_delay_ms(1);

		//0x2A: CTRL_REG1 System Control 1 register
		//go to standby mode
		accel_write_register(0x2A, 0);

		//0x0E: XYZ_DATA_CFG register
		//HPF_OUT - disable high pass filter
		//FS - 4g
		//accel_write_register(0x0E, 0x1);

		//0x2B: CTRL_REG2 System Control 2 register
		//set Low Noise Low Power mode
		accel_write_register(0x2B, 0x09);
		//set Low Power mode
		//accel_write_register(0x2B, 0x1B);

		//0x2C: CTRL_REG3 Interrupt Control register
		//accel_write_register(0x2C, 0x30);//0x40

		//accel_fifo_init();
		accel_tilt_init();
		//accel_motion_init();
		//accel_transient_init();
		//accel_pulse_init();
		
		//0x2D: CTRL_REG4 Interrupt Enable register (Read/Write)
		// transient+orientation+pulse+freefall
		//accel_write_register(0x2D, 0x3c);
		// orientation
		accel_write_register(0x2D, 0x10);

		//0x0F: HP_FILTER_CUTOFF 
		accel_write_register(0x0F, 0);
		
		//0x29: ASLP_COUNT 
		//accel_write_register(0x29, 0x1);

		// go to active mode
		// 0x2A: CTRL_REG1 System Control 1 register
		// ASLP_RATE 01 = DR 101 = 12,5Hz
		accel_write_register(0x2A, 0x6B);
		// ASLP_RATE 00 = DR 100 = 50Hz
		// accel_write_register(0x2A, 0x23);
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

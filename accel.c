#include "accel.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
//#include "nrf_drv_twi.h"
//#include "app_util_platform.h"
#include "twi_master.h"
#include "board.h"
#include "mlcd.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "mlcd.h"
#include "ble/ble_peripheral.h"

#define DEFAULT_I2C_ADDR 0x1D

#define WHIRST_MOVE_MAX_TIME            APP_TIMER_TICKS(60, APP_TIMER_PRESCALER)

uint32_t wirst_start = 0;

//static const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(0);
/*static const nrf_drv_twi_t twi = {
         .p_reg       = NRF_TWI0,
         .irq         = NULL,//GPIOTE_IRQn,//SPI0_TWI0_IRQn,
         .instance_id = TWI0_INSTANCE_INDEX
        };*/
				 

//static const nrf_drv_twi_config_t twi_config = NRF_DRV_TWI_DEFAULT_CONFIG(0);

static void accel_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
		uint8_t int_src;
		accel_read_register(0x0C, &int_src);
		uint8_t sysmod;
		accel_read_register(0x0B, &sysmod);
/*		switch(pin) {
			case ACCEL_INT1:
					mlcd_backlight_toggle();
					break;
			case ACCEL_INT2:
					mlcd_colors_toggle();
					break;
		}*/
				#ifdef OSSW_DEBUG
		//				printf("SYSMOD: 0x%02x\r\n", sysmod);
				#endif
	
		
				#ifdef OSSW_DEBUG
		//				printf("INT_SRC: 0x%02x\r\n", int_src);
				#endif

				
	/*	if (int_src & 0x08) {

				uint8_t pulse_src;
				accel_read_register(0x22, &pulse_src);

		//		if (pulse_src>>6&0x1) {
		//				mlcd_backlight_toggle();
	//			}
			
				#ifdef OSSW_DEBUG
			printf("PULSE_SRC: [x:%d, y:%d, z:%d, dbl: %d]\r\n", (pulse_src>>4&0x1)*(pulse_src&0x1?-1:1), (pulse_src>>5&0x1)*(pulse_src>>1&0x1?-1:1), (pulse_src>>6&0x1)*(pulse_src>>2&0x1?-1:1), pulse_src>>3&0x1);
				#endif
		}*/
		if (int_src & 0x20) {

				uint8_t trans_src;
				accel_read_register(0x1E, &trans_src);
			
				int8_t data[3];
				uint8_t yMod;
				accel_read_multi_register(0x01, (uint8_t*)data, 3);
			
				yMod = abs(data[1]);
			
				if (data[0] < -50 && data[2]>30) {
						// first part of whirst move
						app_timer_cnt_get(&wirst_start);
				} else if (data[0] > 0 && data[2]<30 && yMod <= 20) {
						// second part of whirst move
						if (wirst_start != 0) {
								uint32_t curr;
								app_timer_cnt_get(&curr);
								uint32_t diff;
								app_timer_cnt_diff_compute(curr, wirst_start, &diff);
							
				#ifdef OSSW_DEBUG
						printf("DIFF: 0x%06x\r\n", diff);
				#endif
							
								if (diff<=WHIRST_MOVE_MAX_TIME) {
										mlcd_backlight_temp_on();
								}
						}
						wirst_start = 0;		
				}			
			
				#ifdef OSSW_DEBUG
						printf("DATA: [x:%d, y:%d, z:%d]\r\n", data[0], data[1], data[2]);
				#endif
			
			
			
				#ifdef OSSW_DEBUG
		//	printf("TRANS_SRC: [x:%d, y:%d, z:%d]\r\n", (trans_src>>1&0x1)*(trans_src&0x1?-1:1), (trans_src>>3&0x1)*(trans_src>>2&0x1?-1:1), (trans_src>>5&0x1)*(trans_src>>4&0x1?-1:1));
				#endif
		}
	/*	if (int_src & 0x10) {

				uint8_t lndc_src;
				accel_read_register(0x10, &lndc_src);

				#ifdef OSSW_DEBUG
						printf("SRC_LNDPRT: 0x%02x\r\n", lndc_src);
				#endif
			
		}*/
		
		accel_read_register(0x0B, &sysmod);
		#ifdef OSSW_DEBUG
				printf("SYSMOD: 0x%02x\r\n", sysmod);
		#endif
}

static uint32_t accel_int_init(uint8_t pin_no)
{
    uint32_t err_code;

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }

    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    config.pull = NRF_GPIO_PIN_PULLDOWN;

    err_code = nrf_drv_gpiote_in_init(pin_no, &config, accel_event_handler);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    nrf_drv_gpiote_in_event_enable(pin_no, true);
		return NRF_SUCCESS;
}

void accel_init(void) {
		twi_master_init();

    uint32_t err_code;
		err_code = accel_int_init(ACCEL_INT1);
    APP_ERROR_CHECK(err_code);
		err_code = accel_int_init(ACCEL_INT2);
    APP_ERROR_CHECK(err_code);

		//ret_code_t err_code = nrf_drv_twi_init(&twi, &twi_config, NULL);
		//APP_ERROR_CHECK(err_code);
		//nrf_drv_twi_enable(&twi);
	
	
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
		accel_write_register(0x0E, 0x1);

		//0x2B: CTRL_REG2 System Control 2 register
		//set Low Noise Low Power mode
		//accel_write_register(0x2B, 0xD);
		accel_write_register(0x2B, 0x1F);

		//0x2C: CTRL_REG3 Interrupt Control register
		accel_write_register(0x2C, 0x40);

		//0x2D: CTRL_REG4 Interrupt Enable register (Read/Write)
		accel_write_register(0x2D, 0x38);

		 //0x21: PULSE_CFG Pulse Configuration register
		accel_write_register(0x21, 0x7F);

		//0x23 – 0x25: PULSE_THSX, Y, Z Pulse Threshold for X, Y and Z registers
		accel_write_register(0x23, 0x20);
		accel_write_register(0x24, 0x20);
		accel_write_register(0x25, 0x20);

		//0x26: PULSE_TMLT Pulse Time Window 1 register
		accel_write_register(0x26, 0x2);

		 //0x27: PULSE_LTCY Pulse Latency Timer register
		accel_write_register(0x27, 0x1);

		//0x28 PULSE_WIND register (Read/Write)
		accel_write_register(0x28, 0x5);
		
		//0x11 PL_CFG
		accel_write_register(0x11, 0x40);
		
		//0x12 PL_COUNT
		accel_write_register(0x12, 0x0);
		
		//0x1D TRANSIENT_CFG
		accel_write_register(0x1D, 0x1E);
		
		//0x1F TRANSIENT_THS
		//accel_write_register(0x1F, 0x40);
		accel_write_register(0x1F, 0x0A);

		//0x0F: HP_FILTER_CUTOFF 
		accel_write_register(0x0F, 0);

		//0x2A: CTRL_REG1 System Control 1 register
		//ASLP_RATE 01 - 12,5Hz
		//DR 101 - 12,5Hz
		//go to active mode
		accel_write_register(0x2A, 0x23);
		//accel_write_register(0x2A, 0x6B);
}

void accel_write_register(uint8_t reg, uint8_t value) {
		uint8_t data[] = {reg, value};
		//nrf_drv_twi_tx(&twi, DEFAULT_I2C_ADDR, data, 2, false);
		twi_master_transfer	(	DEFAULT_I2C_ADDR<<1, data, 2, true);
}

void accel_read_register(uint8_t reg, uint8_t* value) {
		//nrf_drv_twi_tx(&twi, DEFAULT_I2C_ADDR, &reg, 1, true);
		//nrf_drv_twi_rx(&twi, DEFAULT_I2C_ADDR, value, 1, false);
		twi_master_transfer	(	DEFAULT_I2C_ADDR<<1, &reg, 1, false);
		twi_master_transfer	(	(DEFAULT_I2C_ADDR<<1)|TWI_READ_BIT, value, 1, true);
}

void accel_read_multi_register(uint8_t reg, uint8_t* value, uint8_t length) {
		//nrf_drv_twi_tx(&twi, DEFAULT_I2C_ADDR, &reg, 1, true);
		//nrf_drv_twi_rx(&twi, DEFAULT_I2C_ADDR, value, length, false);
		twi_master_transfer	(	DEFAULT_I2C_ADDR<<1, &reg, 1, false);
		twi_master_transfer	(	(DEFAULT_I2C_ADDR<<1)|TWI_READ_BIT, value, length, true);
}

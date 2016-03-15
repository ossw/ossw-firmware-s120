#include "accel.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "twi_master.h"
#include "board.h"
#include "mlcd.h"
#include "app_timer.h"
#include "mlcd.h"
#include "nordic_common.h"
#include <nrf_delay.h>
#include "time.h"

#define DEFAULT_I2C_ADDR 0x1D
//#define ACC_INTERVAL                      APP_TIMER_TICKS(30, APP_TIMER_PRESCALER)
    
//static volatile int8_t xVal = 0;
//static volatile int8_t yVal = 0;
//static volatile int8_t zVal = 0;

//static app_timer_id_t      m_acc_timer_id;
void accel_write_register(uint8_t reg, uint8_t value);
void accel_read_register(uint8_t reg, uint8_t* value);
void accel_read_multi_register(uint8_t reg, uint8_t* value, uint8_t legth);

/*
static void accel_timeout_handler(void * p_context) {
	uint8_t trans_src;
	int8_t data[3];
	accel_read_register(0x1E, &trans_src);
	accel_read_multi_register(0x01, (uint8_t*)data, 3);
	xVal = data[0];
	yVal = data[1];
	zVal = data[2];
}
*/
/*
static void accel_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

}
*/

void accel_get_values(int8_t * x, int8_t * y, int8_t * z) {
	uint8_t trans_src;
	int8_t data[3];
	accel_read_register(0x1E, &trans_src);
	accel_read_multi_register(0x01, (uint8_t*)data, 3);
	*x = data[0];
	*y = data[1];
	*z = data[2];
}
	
/*
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

    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(pin_no, &config, accel_event_handler);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    nrf_drv_gpiote_in_event_enable(pin_no, true);
		return NRF_SUCCESS;
}
*/
/*
void accel_init_timer() {
		uint32_t err_code;
		err_code = app_timer_create(&m_acc_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                accel_timeout_handler);
    APP_ERROR_CHECK(err_code);
	
    err_code = app_timer_start(m_acc_timer_id, ACC_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}
*/
void accel_init(void) {
		twi_master_init();
	
    //uint32_t err_code;
		//err_code = accel_int_init(ACCEL_INT1);
    //APP_ERROR_CHECK(err_code);
		//err_code = accel_int_init(ACCEL_INT2);
    //APP_ERROR_CHECK(err_code);
	
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
		accel_write_register(0x2C, 0x30);//0x40

		//0x2D: CTRL_REG4 Interrupt Enable register (Read/Write)
		accel_write_register(0x2D, 0x08);

		 //0x21: PULSE_CFG Pulse Configuration register
		accel_write_register(0x21, 0x50);

		//0x23 – 0x25: PULSE_THSX, Y, Z Pulse Threshold for X, Y and Z registers
		accel_write_register(0x23, 0x20);
		accel_write_register(0x24, 0x20);
		accel_write_register(0x25, 0x30);

		//0x26: PULSE_TMLT Pulse Time Window 1 register
		accel_write_register(0x26, 0x3);

		 //0x27: PULSE_LTCY Pulse Latency Timer register
		accel_write_register(0x27, 0xA);

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
		
		//0x29: ASLP_COUNT 
		accel_write_register(0x29, 0x1);

		//0x2A: CTRL_REG1 System Control 1 register
		//ASLP_RATE 01 - 12,5Hz
		//DR 101 - 12,5Hz
		//go to active mode
		//accel_write_register(0x2A, 0x63);
		accel_write_register(0x2A, 0x6B);
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

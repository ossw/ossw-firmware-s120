/** 
 * @brief OSSW S120 firmware main file.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_button.h"
#include "nrf51_bitfields.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "softdevice_handler.h"
#include "spi_slave.h"
#include "../../target.h"
#include "../../command.h"
#include "../../command_rx_buffer.h"
#include "../../command_over_spi.h"
#include "../../mcu.h"
#include "../../gpio.h"
#include "../../timer.h"
#include "../../ble/ble_peripheral.h"

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define TX_BUF_SIZE   256              
#define RX_BUF_SIZE   TX_BUF_SIZE       
#define DEF_CHARACTER 0xABu             
#define ORC_CHARACTER 0x0u        
static uint8_t m_tx_buf[TX_BUF_SIZE];        
static uint8_t m_rx_buf[RX_BUF_SIZE]; 

static uint8_t m_last_command_data[256];
static uint8_t m_last_command_size;
static void (*m_last_command_resp_handler)(uint8_t);
	 
/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void spi_slave_event_handle(spi_slave_evt_t event)
{
    uint32_t err_code;
     
    if (event.evt_type == SPI_SLAVE_XFER_DONE)
    {   
				uint8_t tx_size = 0;
				switch(m_rx_buf[0]) {
						case SPI_CMD_SET_READ_REG:
								switch(m_rx_buf[1]) {
									case SPI_CMD_REG_CMD_SIZE:
											tx_size = 2;
											m_tx_buf[0] = 0xFF;
											m_tx_buf[1] = m_last_command_size;
											break;
									case SPI_CMD_REG_CMD_DATA:
											tx_size = m_last_command_size + 1;
											m_tx_buf[0] = 0xFF;
											memcpy(m_tx_buf+1, m_last_command_data, m_last_command_size);
											break;
								}
								break;
						case SPI_CMD_READ_REG:
								tx_size = 0;
								break;
						case SPI_CMD_WRITE:
								switch(m_rx_buf[1]) {
										case SPI_CMD_REG_CMD_RESP:
												tx_size = 0;
												m_last_command_resp_handler(0);
												m_last_command_size = 0;
												m_last_command_resp_handler = NULL;
												break;
								}
								break;
				}
			
        err_code = spi_slave_buffers_set(m_tx_buf, m_rx_buf, tx_size, (uint8_t)sizeof(m_rx_buf));
        APP_ERROR_CHECK(err_code);          
    }
}
 
uint32_t nrf51_spi_slave_init(void)
{
    uint32_t err_code;
	
		err_code = spi_slave_evt_handler_register(spi_slave_event_handle);
		APP_ERROR_CHECK(err_code);    
   
		spi_slave_config_t spi_slave_config;
		spi_slave_config.pin_miso           = SPIS0_MISO;
		spi_slave_config.pin_mosi           = SPIS0_MOSI;
		spi_slave_config.pin_sck            = SPIS0_SCK;
		spi_slave_config.pin_csn            = SPIS0_SS;
		spi_slave_config.mode               = SPI_MODE_0;           // CPOL : 0  / CPHA : 1    From Cortex-M3
		spi_slave_config.bit_order          = SPIM_LSB_FIRST;            
		spi_slave_config.def_tx_character   = DEF_CHARACTER;
		spi_slave_config.orc_tx_character   = ORC_CHARACTER;
	
		err_code = spi_slave_init(&spi_slave_config);
		APP_ERROR_CHECK(err_code);
	
    err_code = spi_slave_buffers_set(m_tx_buf, m_rx_buf, 0, (uint8_t)sizeof(m_rx_buf));
    APP_ERROR_CHECK(err_code);            
    
    return NRF_SUCCESS;
}

void command_receive(uint8_t *rx_data, uint8_t rx_size, void (*handler)(uint8_t)) {
		
		m_last_command_size = rx_size;
		memcpy(m_last_command_data, rx_data, rx_size);
		m_last_command_resp_handler = handler;
	
		// interrupt master
		gpio_pin_clear(MASTER_INT);
		nrf_delay_ms(1);
		gpio_pin_set(MASTER_INT);

}

/**@brief Function for application main entry.
 */
int main(void)
{
		nrf51_spi_slave_init();
	
    nrf_gpio_cfg_output(MASTER_INT);
    nrf_gpio_pin_set(MASTER_INT);
	
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
	
		timer_init();
		
		ble_peripheral_mode_init();
	
    // Enter main loop.
    for (;;)
    {	
				command_rx_buffer_process();	
        mcu_power_manage();
    }
}

void __aeabi_assert(const char * a, const char * b, int c){
}

void app_error_fault_handler(uint32_t arg0, uint32_t arg1, uint32_t arg2){
}

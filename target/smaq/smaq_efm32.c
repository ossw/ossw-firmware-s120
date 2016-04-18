#include "../../target.h"
#include "../../spi.h"
#include "../../ext_flash.h"
#include "../../mlcd.h"
#include "../../buttons.h"
#include "../../battery.h"
#include "../../rtc.h"
#include "../../timer.h"
#include "../../gpio.h"
#include "../../ossw.h"
#include "../../mcu.h"
#include "../../command.h"
#include "../../command_over_spi.h"
#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_chip.h" 
#include "em_usart.h" 
#include "rtcdriver.h" 
#include "gpiointerrupt.h" 
#include "sleep.h" 



void smaq_clocks_init(void) {
		SystemCoreClockUpdate();

	//	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	RTCDRV_Init();
		//CMU_OscillatorEnable(cmuOsc_HFXO,true,true); 	
		//CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);  			// select HF XTAL osc as system clock source (24MHz)
	//	CMU_ClockEnable(cmuClock_HFPER, true);						// Enable HF peripheral clock
	  
		//CMU_OscillatorEnable(cmuOsc_LFXO,true,true); 	
//	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_CORELEDIV2);
	
		CMU_ClockEnable(cmuClock_CORELE, true);         // enable the Low Energy Peripheral Interface clock
		CMU_ClockEnable(cmuClock_GPIO, true);						// Enable clock for GPIO
		CMU_ClockEnable(cmuClock_USART0, true);					// Enable clock for USART0
		CMU_ClockEnable(cmuClock_USART1, true);					// Enable clock for USART1
		CMU_ClockEnable(cmuClock_USART2, true);					// Enable clock for USART2
}

void smaq_gpio_init(void) {
	
	//button down
	GPIO_PinModeSet(gpioPortA, 1, gpioModeInputPull, 0);
	
	//back button
	GPIO_PinModeSet(gpioPortA, 6, gpioModeInputPull, 0);
	
	//vibration motor
	GPIO_PinModeSet(gpioPortA, 8, gpioModePushPull, 0);
	
	//lcd backlight
	GPIO_PinModeSet(gpioPortA, 10, gpioModePushPull, 0);
	
	//SPI2 TX
	GPIO_PinModeSet(gpioPortC, 2, gpioModePushPull, 0);
	
	//SPI2 RX
	GPIO_PinModeSet(gpioPortC, 3, gpioModeInput, 0);
	
	//SPI2 CLK
	GPIO_PinModeSet(gpioPortC, 4, gpioModePushPull, 0);
	
	//CS for ext flash
	GPIO_PinModeSet(gpioPortC, 5, gpioModePushPull, 1);
	
	//up button
	GPIO_PinModeSet(gpioPortC, 10, gpioModeInputPull, 0);
	
	//SPI1 TX
	GPIO_PinModeSet(gpioPortD, 0, gpioModePushPull, 0);
	
	//SPI1 CLK
	GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 0);
	
	//CS for MLCD
	GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 0);
	
	//NRF data interrupt
	GPIO_PinModeSet(gpioPortD, 5, gpioModeInputPull, 0);
	
	//MLCD ON/OFF
	GPIO_PinModeSet(gpioPortD, 6, gpioModePushPull, 0);
	
//	// $[Port E Configuration]

	//SPI0 TX
	GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 0);
	
	//SPI0 RX
	GPIO_PinModeSet(gpioPortE, 11, gpioModeInput, 0);
	
	//SPI0 CLK
	GPIO_PinModeSet(gpioPortE, 12, gpioModePushPull, 0);
	
	//CS for NRF
	GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1);
	
	//select button
	GPIO_PinModeSet(gpioPortF, 12, gpioModeInputPull, 0);
	
  GPIO_IntConfig(gpioPortA, 1, true, true, true);
  GPIO_IntConfig(gpioPortD, 5, false, true, true);
  GPIO_IntConfig(gpioPortA, 6, true, true, true);
  GPIO_IntConfig(gpioPortC, 10, true, true, true);
  GPIO_IntConfig(gpioPortF, 12, true, true, true);
}

void smaq_spi_init(void) {
		USART_InitSync_TypeDef initsync = USART_INITSYNC_DEFAULT;
		initsync.baudrate = 8000000;
		initsync.databits = usartDatabits8;
		initsync.master = 1;
		initsync.msbf = 0;
		initsync.clockMode = usartClockMode0;
	#if defined( USART_INPUT_RXPRS ) && defined( USART_TRIGCTRL_AUTOTXTEN )
		initsync.prsRxEnable = 0;
	//	initsync.prsRxCh = 0;
		initsync.autoTx = 0;
	#endif
		USART_InitSync(USART0, &initsync);
		USART_PrsTriggerInit_TypeDef initprs = USART_INITPRSTRIGGER_DEFAULT;
		initprs.rxTriggerEnable = 0;
		initprs.txTriggerEnable = 0;
		initprs.prsTriggerChannel = usartPrsTriggerCh0;
		USART_InitPrsTrigger(USART0, &initprs);
		
		initsync.baudrate = 8000000;
		initsync.databits = usartDatabits8;
		initsync.master = 1;
		initsync.msbf = 1;
		initsync.clockMode = usartClockMode0;
	#if defined( USART_INPUT_RXPRS ) && defined( USART_TRIGCTRL_AUTOTXTEN )
		initsync.prsRxEnable = 0;
	//	initsync.prsRxCh = 0;
		initsync.autoTx = 0;
	#endif
		USART_InitSync(USART1, &initsync);
		initprs.rxTriggerEnable = 0;
		initprs.txTriggerEnable = 0;
		initprs.prsTriggerChannel = usartPrsTriggerCh0;
		USART_InitPrsTrigger(USART1, &initprs);
	
		initsync.baudrate = 8000000;
		initsync.databits = usartDatabits8;
		initsync.master = 1;
		initsync.msbf = 1;
		initsync.clockMode = usartClockMode0;
	#if defined( USART_INPUT_RXPRS ) && defined( USART_TRIGCTRL_AUTOTXTEN )
		initsync.prsRxEnable = 0;
	//	initsync.prsRxCh = 0;
		initsync.autoTx = 0;
	#endif
		USART_InitSync(USART2, &initsync);
		initprs.rxTriggerEnable = 0;
		initprs.txTriggerEnable = 0;
		initprs.prsTriggerChannel = usartPrsTriggerCh0;
		USART_InitPrsTrigger(USART2, &initprs);	
		
		
	/* Module PCNT0 is configured to location 2 */
//	PCNT0->ROUTE = (PCNT0->ROUTE & ~_PCNT_ROUTE_LOCATION_MASK)
//			| PCNT_ROUTE_LOCATION_LOC2;

	/* Enable signals CLK, CS, RX, TX */
	USART0->ROUTE |= USART_ROUTE_CLKPEN  | USART_ROUTE_RXPEN
			| USART_ROUTE_TXPEN;

	/* Module USART1 is configured to location 1 */
	USART1->ROUTE = (USART1->ROUTE & ~_USART_ROUTE_LOCATION_MASK)
			| USART_ROUTE_LOCATION_LOC1;

	/* Enable signals CLK, TX */
	USART1->ROUTE |= USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN;
	USART1->CMD = USART_CMD_RXDIS;
	
	/* Enable signals CLK, RX, TX */
	USART2->ROUTE |= USART_ROUTE_CLKPEN | USART_ROUTE_RXPEN
			| USART_ROUTE_TXPEN;
}

static void before_sleep() {
}

static void after_wakeup() {
}

static void nrf_send_command_response(uint8_t resp_code) {
    uint8_t command[] = {SPI_CMD_WRITE, SPI_CMD_REG_CMD_RESP, resp_code};
    spi_master_tx(NRF_SPI, NRF_SPI_SS, command, 3);
}

static bool process_command = false;

static void nrf_process_command(void) {
	
    uint8_t data[256];
		mlcd_backlight_toggle();
	
    uint8_t command[] = {SPI_CMD_SET_READ_REG, SPI_CMD_REG_CMD_SIZE};
    spi_master_tx(NRF_SPI, NRF_SPI_SS, command, 2);
		
		mcu_delay_ms(40);
		
    command[0] = SPI_CMD_READ_REG;
		uint8_t data_size;
    spi_master_rx_data(NRF_SPI, NRF_SPI_SS, command, 1, &data_size, 1, NULL);
		
		if (data_size == 0) {
				return;
		}
		
		mcu_delay_ms(40);
		
    command[0] = SPI_CMD_SET_READ_REG;
		command[1] = SPI_CMD_REG_CMD_DATA;
    spi_master_tx(NRF_SPI, NRF_SPI_SS, command, 2);
		
		mcu_delay_ms(50);
		
    command[0] = SPI_CMD_READ_REG;
    spi_master_rx_data(NRF_SPI, NRF_SPI_SS, command, 1, data, data_size, NULL);
			
		command_receive(data, data_size, nrf_send_command_response);
		
}

static void nrf_int_handler(uint8_t pin)
{
		process_command = true;
}


/**@brief Function for application main entry.
 */
int main(void)
{
	  /* Chip revision alignment and errata fixes */
		CHIP_Init();
	
#ifdef OSSW_DEBUG
		init_uart();
#endif
	
	  smaq_clocks_init();
	  smaq_gpio_init();
	
		// dont't know what it does but it fixes lcd screen :)
		gpio_pin_set(gpioPortC<<4 | 0);
		mcu_delay_ms(10);
		gpio_pin_clear(gpioPortC<<4 | 0);
	
	  smaq_spi_init();
	
		SLEEP_Init((SLEEP_CbFuncPtr_t)before_sleep, (SLEEP_CbFuncPtr_t)after_wakeup);

		GPIOINT_Init();
	
    mlcd_init();
	
		//accel_init();
	
    // Initialize.
    timer_init();
	  rtc_timer_init();
		buttons_init();
	  battery_init();
		
		
		GPIOINT_CallbackRegister(NRF_INT & 0xF, nrf_int_handler);
	
		ossw_init();
		
	
		 // Enter main loop.
    for (;;)
    {
				if (process_command) {
						process_command = false;
						nrf_process_command();
				}
			
			  ossw_process();
				mcu_power_manage();
    }
}

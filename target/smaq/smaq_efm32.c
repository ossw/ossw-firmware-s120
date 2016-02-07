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
#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_chip.h" 
#include "em_usart.h" 
#include "gpiointerrupt.h" 
#include "sleep.h" 

void smaq_clocks_init(void) {
		SystemCoreClockUpdate();

		CMU_OscillatorEnable(cmuOsc_HFXO,true,true);   
		CMU_OscillatorEnable(cmuOsc_LFXO,true,true); 				// enable XTAL osc and wait for it to stabilize
	//	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);  			// select HF XTAL osc as system clock source (24MHz)
		CMU_ClockEnable(cmuClock_HFPER, true);						// Enable HF peripheral clock
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_CORELEDIV2);  	// select HFCORECLK/2 as clock source to LFB
		CMU_ClockEnable(cmuClock_CORELE, true);                  	// enable the Low Energy Peripheral Interface clock
		CMU_ClockEnable(cmuClock_GPIO, true);						// Enable clock for GPIO
		CMU_ClockEnable(cmuClock_USART0, true);					// Enable clock for USART0
		CMU_ClockEnable(cmuClock_USART1, true);					// Enable clock for USART1
		CMU_ClockEnable(cmuClock_USART2, true);						// Enable clock for USART2
		CMU_ClockEnable(cmuClock_TIMER0, true);	
}

void smaq_gpio_init(void) {
		// $[Port A Configuration]
	
	GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE1_MASK)
			| GPIO_P_MODEL_MODE1_INPUTPULL;
	GPIO->P[0].DOUT |= (1 << 1);
	
	/* Pin PA5 is configured to Push-pull */
	GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE5_MASK)
			| GPIO_P_MODEL_MODE5_PUSHPULL;
	GPIO->P[0].DOUT |= (1 << 5);
	
	GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE6_MASK)
			| GPIO_P_MODEL_MODE6_INPUTPULL;
	GPIO->P[0].DOUT |= (1 << 6);
	
	// [Port A Configuration]$

	// $[Port B Configuration]
	// [Port B Configuration]$

	// $[Port C Configuration]

	/* Pin PC2 is configured to Push-pull */
	GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE2_MASK)
			| GPIO_P_MODEL_MODE2_PUSHPULL;

	/* Pin PC3 is configured to Input enabled */
	GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE3_MASK)
			| GPIO_P_MODEL_MODE3_INPUT;

	/* Pin PC4 is configured to Push-pull */
	GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE4_MASK)
			| GPIO_P_MODEL_MODE4_PUSHPULL;

	/* Pin PC5 is configured to Push-pull */
	GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE5_MASK)
			| GPIO_P_MODEL_MODE5_PUSHPULL;
			
	GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE10_MASK)
			| GPIO_P_MODEH_MODE10_INPUTPULL;
	GPIO->P[2].DOUT |= (1 << 10);
	// [Port C Configuration]$

	// $[Port D Configuration]

	/* Pin PD0 is configured to Push-pull */
	GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE0_MASK)
			| GPIO_P_MODEL_MODE0_PUSHPULL;

	/* Pin PD1 is configured to Input enabled */
	GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE1_MASK)
			| GPIO_P_MODEL_MODE1_INPUT;

	/* Pin PD2 is configured to Push-pull */
	GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE2_MASK)
			| GPIO_P_MODEL_MODE2_PUSHPULL;

	/* Pin PD3 is configured to Push-pull */
	GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE3_MASK)
			| GPIO_P_MODEL_MODE3_PUSHPULL;

	/* Pin PD5 is configured to Input enabled with pull-up */
	GPIO->P[3].DOUT |= (1 << 5);
	GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE5_MASK)
			| GPIO_P_MODEL_MODE5_INPUTPULL;
			
	/* Pin PD6 is configured to Push-pull */
	GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE6_MASK)
			| GPIO_P_MODEL_MODE6_PUSHPULL;
	// [Port D Configuration]$

	// $[Port E Configuration]

	/* Pin PE10 is configured to Push-pull */
	GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE10_MASK)
			| GPIO_P_MODEH_MODE10_PUSHPULL;

	/* Pin PE11 is configured to Input enabled */
	GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE11_MASK)
			| GPIO_P_MODEH_MODE11_INPUT;

	/* Pin PE12 is configured to Push-pull */
	GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE12_MASK)
			| GPIO_P_MODEH_MODE12_PUSHPULL;

	/* Pin PE13 is configured to Push-pull */
	GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE13_MASK)
			| GPIO_P_MODEH_MODE13_PUSHPULL;
	GPIO->P[4].DOUT |= (1 << 13);
	// [Port E Configuration]$

	// $[Port F Configuration]
	GPIO->P[5].MODEH = (GPIO->P[5].MODEH & ~_GPIO_P_MODEH_MODE12_MASK)
			| GPIO_P_MODEH_MODE12_INPUTPULL;
	GPIO->P[5].DOUT |= (1 << 12);
	// [Port F Configuration]$

	// $[Route Configuration]

	/* Module PCNT0 is configured to location 2 */
	PCNT0->ROUTE = (PCNT0->ROUTE & ~_PCNT_ROUTE_LOCATION_MASK)
			| PCNT_ROUTE_LOCATION_LOC2;

	/* Enable signals CLK, CS, RX, TX */
	USART0->ROUTE |= USART_ROUTE_CLKPEN  | USART_ROUTE_RXPEN
			| USART_ROUTE_TXPEN;

	/* Module USART1 is configured to location 1 */
	USART1->ROUTE = (USART1->ROUTE & ~_USART_ROUTE_LOCATION_MASK)
			| USART_ROUTE_LOCATION_LOC1;

	/* Enable signals CLK, TX */
	USART1->ROUTE |= USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN;

	USART1->CMD = USART_CMD_RXDIS;
	
	/* Enable signals CLK, CS, RX, TX */
	USART2->ROUTE |= USART_ROUTE_CLKPEN | USART_ROUTE_RXPEN
			| USART_ROUTE_TXPEN;
	// [Route Configuration]$
	
	gpio_pin_clear(MLCD_SPI_SS);
	gpio_pin_set(EXT_FLASH_SPI_SS);
	gpio_pin_clear(VIBRATION_MOTOR);
	gpio_pin_clear(LCD_BACKLIGHT);
	
  GPIO_IntConfig(gpioPortA, 1, false, true, true);
  //GPIO_IntConfig(gpioPortD, 5, false, true, true);
  GPIO_IntConfig(gpioPortA, 6, false, true, true);
  GPIO_IntConfig(gpioPortC, 10, false, true, true);
  GPIO_IntConfig(gpioPortF, 12, false, true, true);

	/*
    nrf_gpio_cfg_output(LCD_ENABLE);
    nrf_gpio_cfg_output(LCD_BACKLIGHT);
    nrf_gpio_cfg_output(LCD_VOLTAGE_REG);
    nrf_gpio_pin_clear(LCD_ENABLE);
    nrf_gpio_pin_clear(LCD_BACKLIGHT);
    nrf_gpio_pin_clear(LCD_VOLTAGE_REG);*/
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
}

static void before_sleep() {
}

static void after_wakeup() {
}

/**@brief Function for application main entry.
 */
int main(void)
{
	
#ifdef OSSW_DEBUG
		init_uart();
#endif
	
	  smaq_clocks_init();
	  smaq_spi_init();
	  smaq_gpio_init();
	
		SLEEP_Init(	before_sleep, after_wakeup);
	
		GPIOINT_Init();
	
    mlcd_init();
	
		//accel_init();
	
    // Initialize.
    timer_init();
	  rtc_timer_init();
		buttons_init();
	  battery_init();
	
		ossw_main();
}

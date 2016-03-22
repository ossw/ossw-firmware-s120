/** 
 * @brief OSSW S120 firmware main file.
 *
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_button.h"
#include "nrf51_bitfields.h"
#include "board.h"
#include "app_trace.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "spi.h"
#include "mlcd.h"
#include "mlcd_draw.h"
#include "ext_ram.h"
#include "ext_flash.h"
#include "rtc.h"
#include "scr_mngr.h"
#include "buttons.h"
#include "battery.h"
#include "vibration.h"
#include "notifications.h"
#include "softdevice_handler.h"
#include "command.h"
#include "stopwatch.h"
#include "fs.h"
#include "accel.h"
#include "watchset.h"
#include "config.h"
#include "pedometer_ac.h"

#ifdef OSSW_DEBUG
		#include "app_uart.h"
#endif

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
		
		
#ifdef OSSW_DEBUG
void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

void init_uart(void) {
    uint32_t err_code;
		const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          0xFF,
          0xFF,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud230400
      };

    APP_UART_FIFO_INIT(&comm_params,
                         1,
                         128,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);
	}
#endif

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

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

static uint_fast8_t splashscreen_draw_func(uint_fast8_t x, uint_fast8_t y)
{
	 y = MLCD_YRES - y - 1;
	 if(x>5 && x<69 && y>87 && y < 163) {
				if(x>12 && x < 62 && y > 94 && y < 156){
						return 0;
				}
				return 1;
		}
		
		if(x>75 && x<139 && y>87 && y < 163) {
				if(x>82 && y > 128 && y < 156){
						return 0;
				}
				if( x < 132 && y > 94 && y < 122){
						return 0;
				}
				return 1;
		}
		
		if(x>5 && x<69 && y>5 && y < 81) {
				if(x>12 && y > 46 && y < 74){
						return 0;
				}
				if( x < 62 && y > 12 && y < 40){
						return 0;
				}
				return 1;
		}
		
		if(x>75 && x<139 && y>5 && y < 81) {
				if(x>103 && x < 110 && y > 12 && y < 45){
						return 1;
				}
				if(x>82 && x < 132 && y > 12 && y < 81){
						return 0;
				}
				return 1;
		}
		return 0;
}

static void timers_init(void)
{
    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);
}

static void init_lcd_with_splash_screen() {
    mlcd_init();
    mlcd_power_on();
	  
		// make sure lcd is working
		nrf_delay_ms(10);

	  mlcd_fb_draw_with_func(splashscreen_draw_func, 0, 0, MLCD_XRES, MLCD_YRES);
	
	  mlcd_fb_flush();
    mlcd_display_on();
}

/**@brief Function for application main entry.
 */
int main(void)
{
	
#ifdef OSSW_DEBUG
		init_uart();
#endif
	
	  spi_init();
	  ext_ram_init();
	  init_lcd_with_splash_screen();

		accel_init();
	
    // Initialize.
    timers_init();
	  rtc_timer_init();
		buttons_init();
	  battery_init();
	
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
	
	  // splash screen
		nrf_delay_ms(500);
	
		fs_init();
		config_init();
		scr_mngr_init();
		vibration_init();
		notifications_init();
		
		stopwatch_init();
		pedometerACInitTimer();
		mlcd_timers_init();
		
    // Enter main loop.
    for (;;)
    {
			  if (rtc_should_store_current_time()) {
					  rtc_store_current_time();
				}
				
				stopwatch_process();
				
				command_process();
				
				watchset_process_async_operation();
			  
				scr_mngr_draw_screen();
				
				pedometerACProcess();
				
        power_manage();
    }
}

void __aeabi_assert(const char * a, const char * b, int c){
}

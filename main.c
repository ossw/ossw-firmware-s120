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
#include "app_gpiote.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "spi.h"
#include "mlcd.h"
#include "mlcd_draw.h"
#include "ext_ram.h"
#include "rtc.h"
#include "scr_mngr.h"
#include "buttons.h"
#include "battery.h"
#include "vibration.h"
#include "notifications.h"
#include "softdevice_handler.h"

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
			
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
	  spi_init();
	  ext_ram_init();
	  init_lcd_with_splash_screen();

    // Initialize.
    timers_init();
	  rtc_timer_init();
    APP_GPIOTE_INIT(1);
	
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
		
	  // splash screen
		nrf_delay_ms(500);
	
		scr_mngr_init();
		buttons_init();
	  battery_init();
		vibration_init();
		notifications_init();

    // Enter main loop.
    for (;;)
    {
			  if (rtc_should_store_current_time()) {
					  rtc_store_current_time();
				}
			  
				scr_mngr_draw_screen();
				
        power_manage();
    }
}

void __aeabi_assert(const char * a, const char * b, int c){
}

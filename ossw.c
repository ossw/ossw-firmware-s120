#include "ossw.h"
#include <stdint.h>
#include <string.h>
#include "target.h"
#include "mlcd.h"
#include "mlcd_draw.h"
#include "rtc.h"
#include "scr_mngr.h"
#include "buttons.h"
#include "battery.h"
#include "vibration.h"
#include "notifications.h"
#include "command_rx_buffer.h"
#include "stopwatch.h"
#include "fs.h"
#include "accel.h"
#include "watchset.h"
#include "config.h"
#include "mcu.h"


static const char* firmware_version = FIRMWARE_VERSION;
static char device_mac[18] = "00:00:00:00:00:00";

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

void ossw_init(void) {
	  mlcd_fb_draw_with_func(splashscreen_draw_func, 0, 0, MLCD_XRES, MLCD_YRES);
	
	  mlcd_fb_flush();
    mlcd_display_on();
		mcu_delay_ms(500);
	
		fs_init();
		config_init();
		scr_mngr_init();
		vibration_init();
		notifications_init();
		
		stopwatch_init();
		
		mlcd_timers_init();
}


void ossw_process(void) {
    
		if (rtc_should_store_current_time()) {
				rtc_store_current_time();
		}
		
		stopwatch_process();
		
		watchset_process_async_operation();
		
		scr_mngr_draw_screen();

}

const char* ossw_firmware_version(void) {
		return firmware_version;
}

const char* ossw_mac_address(void) {
//		static char hex_str[]= "0123456789ABCDEF";
//		ble_gap_addr_t addr;
//		uint32_t err = sd_ble_gap_address_get(&addr);
//		if (err == NRF_SUCCESS) {
//			for (int i = 0; i < 6; i++)
//			{   
//					device_mac[(5-i) * 3 + 0] = hex_str[(addr.addr[i] >> 4) & 0x0F];
//					device_mac[(5-i) * 3 + 1] = hex_str[(addr.addr[i]     ) & 0x0F];
//			}
//		}
		return device_mac;
}

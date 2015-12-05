#include "ossw.h"
#include "nrf.h"
#include "nrf_adc.h"
#include "ble_gap.h"
#include "nrf_soc.h"

static const char* firmware_version = FIRMWARE_VERSION;
static char device_mac[18] = "00:00:00:00:00:00";

const char* ossw_firmware_version(void) {
		return firmware_version;
}

const char* ossw_mac_address(void) {
		static char hex_str[]= "0123456789ABCDEF";
		ble_gap_addr_t addr;
		uint32_t err = sd_ble_gap_address_get(&addr);
		if (err == NRF_SUCCESS) {
			for (int i = 0; i < 6; i++)
			{   
					device_mac[(5-i) * 3 + 0] = hex_str[(addr.addr[i] >> 4) & 0x0F];
					device_mac[(5-i) * 3 + 1] = hex_str[(addr.addr[i]     ) & 0x0F];
			}
		}
		return device_mac;
}

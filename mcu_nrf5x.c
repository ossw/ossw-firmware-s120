#include "mcu.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "app_error.h"

void mcu_delay_ms(uint32_t ms) {
		nrf_delay_ms(ms);
}

void mcu_delay_us(uint32_t us) {
		nrf_delay_us(us);
}

void mcu_power_manage(void) {
		uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

void mcu_reset() {
		NVIC_SystemReset();
}

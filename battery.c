#include "battery.h"
#include "nrf.h"
#include "nrf_adc.h"

#define NRF_ADC_CONFIG { NRF_ADC_CONFIG_RES_8BIT,               \
                                 NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD, \
                                 NRF_ADC_CONFIG_REF_VBG }

#define BATTERY_MIN 107
#define BATTERY_MAX 149

void battery_init(void) {
    const nrf_adc_config_t nrf_adc_config = NRF_ADC_CONFIG;

    // Initialize and configure ADC
    nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
}

uint32_t battery_get_level(void) {
	  uint8_t val =  nrf_adc_convert_single(NRF_ADC_CONFIG_INPUT_6);
		if (val < BATTERY_MIN) {
			  return 0;
		}
		if (val > BATTERY_MAX) {
			  return 100;
		}
		return ((val-BATTERY_MIN)*100)/(BATTERY_MAX-BATTERY_MIN);
}

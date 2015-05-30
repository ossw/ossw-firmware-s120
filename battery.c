#include "battery.h"
#include "nrf.h"
#include "nrf_adc.h"

#define NRF_ADC_CONFIG { NRF_ADC_CONFIG_RES_8BIT,               \
                                 NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD, \
                                 NRF_ADC_CONFIG_REF_VBG }

void battery_init(void) {
    const nrf_adc_config_t nrf_adc_config = NRF_ADC_CONFIG;

    // Initialize and configure ADC
    nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
}

uint8_t battery_get_level(void) {
	  return (uint8_t)nrf_adc_convert_single(NRF_ADC_CONFIG_INPUT_6);
}

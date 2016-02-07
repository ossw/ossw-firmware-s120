#include "battery.h"
#include "nrf.h"
#include "mlcd.h"
#include "nrf_adc.h"
#include "nrf_drv_gpiote.h"
#include "target.h"
#include "scr_mngr.h"

#define NRF_ADC_CONFIG { NRF_ADC_CONFIG_RES_8BIT,               \
                                 NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD, \
                                 NRF_ADC_CONFIG_REF_VBG }

#define BATTERY_MIN 119
#define BATTERY_MAX 146
																 
static void battery_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
		if (BATTERY_CHARGING_INT_PIN == pin) {
				if (!nrf_drv_gpiote_in_is_set(BATTERY_CHARGING_INT_PIN)) {
						scr_mngr_show_screen(SCR_STATUS);
				} else {
						scr_mngr_show_screen(SCR_WATCHFACE);
				}
		}
}
																 
static uint32_t battery_int_init(uint8_t pin_no, nrf_gpio_pin_pull_t pull, nrf_drv_gpiote_evt_handler_t event_handler)
{
    uint32_t err_code;

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }

    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    config.pull = pull;

    err_code = nrf_drv_gpiote_in_init(pin_no, &config, event_handler);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    nrf_drv_gpiote_in_event_enable(pin_no, true);
		return NRF_SUCCESS;
}

void battery_init(void) {
    const nrf_adc_config_t nrf_adc_config = NRF_ADC_CONFIG;

		battery_int_init(BATTERY_CHARGING_INT_PIN, NRF_GPIO_PIN_NOPULL, battery_event_handler);
		battery_int_init(BATTERY_FULLY_CHARGED_INT_PIN, NRF_GPIO_PIN_PULLUP, battery_event_handler);
	
    // Initialize and configure ADC
    nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
}

bool battery_is_charging(void) {
		return !nrf_drv_gpiote_in_is_set(BATTERY_CHARGING_INT_PIN);
}

bool battery_is_full(void) {
		return nrf_drv_gpiote_in_is_set(BATTERY_FULLY_CHARGED_INT_PIN);
}

uint32_t battery_get_level(void) {
	  uint8_t val =  nrf_adc_convert_single(BATTERY_LEVEL_ADC_PIN);
		if (val < BATTERY_MIN) {
			  return 0;
		}
		if (val > BATTERY_MAX) {
			  return 100;
		}
		return ((val-BATTERY_MIN)*100)/(BATTERY_MAX-BATTERY_MIN);
}

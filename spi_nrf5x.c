#include <stdint.h>
#include <string.h>
#include "spi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "target.h"

bool spi_master_tx_rx(void *spi_base_address, uint32_t device, uint16_t transfer_size, const uint8_t *tx_data, uint8_t *rx_data)
{
    uint32_t counter = 0;
    uint16_t number_of_txd_bytes = 0;
    uint32_t SEL_SS_PINOUT = device;
    NRF_SPI_Type *spi_base = (NRF_SPI_Type *)spi_base_address;

    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(SEL_SS_PINOUT);

    while(number_of_txd_bytes < transfer_size)
    {
        spi_base->TXD = (uint32_t)(tx_data[number_of_txd_bytes]);
        counter = 0;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while ((spi_base->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER))
        {
            counter++;
        }

        if (counter == TIMEOUT_COUNTER)
        {
            /* timed out, disable slave (slave select active low) and return with error */
            nrf_gpio_pin_set(SEL_SS_PINOUT);
            return false;
        }
        else
        {   /* clear the event to be ready to receive next messages */
            spi_base->EVENTS_READY = 0U;
        }

        rx_data[number_of_txd_bytes] = (uint8_t)spi_base->RXD;
        number_of_txd_bytes++;
    };

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(SEL_SS_PINOUT);

    return true;
}

bool spi_master_tx_data_no_cs(void *spi_base_address, const uint8_t* tx_data, uint32_t tx_data_size) {
    NRF_SPI_Type *spi_base = (NRF_SPI_Type *)spi_base_address;
	  uint32_t number_of_txd_bytes = 0;
    uint32_t counter = 0;
    while(number_of_txd_bytes < tx_data_size)
    {
        spi_base->TXD = (uint32_t)(tx_data[number_of_txd_bytes]);
        counter = 0;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while ((spi_base->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER))
        {
            counter++;
        }

        if (counter == TIMEOUT_COUNTER)
        {
            /* timed out, disable slave (slave select active low) and return with error */
            return false;
        }
        else
        {   /* clear the event to be ready to receive next messages */
            spi_base->EVENTS_READY = 0U;
        }

        spi_base->RXD;
        number_of_txd_bytes++;
    };
    return true;
}

bool spi_master_rx_to_tx_no_cs(void *src_spi_base_address, uint32_t *dest_spi_base_address, uint32_t data_size, bool revert) {
    NRF_SPI_Type *src_spi_base = (NRF_SPI_Type *)src_spi_base_address;
    NRF_SPI_Type *dest_spi_base = (NRF_SPI_Type *)dest_spi_base_address;
	  uint32_t number_of_rxd_bytes = 0;
    uint32_t counter = 0;
	
		src_spi_base->TXD = 0;
		counter = 0;

		/* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
		while ((src_spi_base->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER))
		{
				counter++;
		}

		if (counter == TIMEOUT_COUNTER)
		{
				/* timed out, disable slave (slave select active low) and return with error */
				return false;
		}
		else
		{   /* clear the event to be ready to receive next messages */
				src_spi_base->EVENTS_READY = 0U;
		}
		
		number_of_rxd_bytes++;
	
    while(number_of_rxd_bytes < data_size)
    {
			  if (revert) {
						dest_spi_base->TXD = ~src_spi_base->RXD;
				} else {
					  dest_spi_base->TXD = src_spi_base->RXD;
				}
			  src_spi_base->TXD = 0;
			
        counter = 0;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while ((dest_spi_base->EVENTS_READY == 0U || src_spi_base->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER))
        {
            counter++;
        }

        if (counter == TIMEOUT_COUNTER)
        {
            /* timed out, disable slave (slave select active low) and return with error */
            return false;
        }
        else
        {   /* clear the event to be ready to receive next messages */
            dest_spi_base->EVENTS_READY = 0U;
            src_spi_base->EVENTS_READY = 0U;
        }

        dest_spi_base->RXD;
				number_of_rxd_bytes++;
    };
		if (revert) {
			  dest_spi_base->TXD = ~src_spi_base->RXD;
		} else {
			  dest_spi_base->TXD = src_spi_base->RXD;
		}
		counter = 0;

		/* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
		while ((dest_spi_base->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER))
		{
				counter++;
		}

		if (counter == TIMEOUT_COUNTER)
		{
				/* timed out, disable slave (slave select active low) and return with error */
				return false;
		}
		else
		{   /* clear the event to be ready to receive next messages */
				dest_spi_base->EVENTS_READY = 0U;
		}

		dest_spi_base->RXD;
    return true;
}

bool spi_master_rx_data_no_cs(void *spi_base_address, uint8_t* rx_data, uint32_t rx_data_size, bool stop_on_zero, bool* has_changed) {
    NRF_SPI_Type *spi_base = (NRF_SPI_Type *)spi_base_address;
	  uint32_t number_of_rxd_bytes = 0;
    uint32_t counter = 0;
	
		bool perform_check = has_changed != NULL && *has_changed == false;
	
    while(number_of_rxd_bytes < rx_data_size)
    {
        spi_base->TXD = 0;
        counter = 0;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while ((spi_base->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER))
        {
            counter++;
        }

        if (counter == TIMEOUT_COUNTER)
        {
            /* timed out, disable slave (slave select active low) and return with error */
            return false;
        }
        else
        {   /* clear the event to be ready to receive next messages */
            spi_base->EVENTS_READY = 0U;
        }

				uint8_t new_value = (uint8_t)spi_base->RXD;
				if (perform_check && new_value != rx_data[number_of_rxd_bytes]) {
						*has_changed = true;
				}
        rx_data[number_of_rxd_bytes] = new_value;
				if (stop_on_zero && new_value == 0) {
						return true;
				}
        number_of_rxd_bytes++;
    };
    return true;
}

bool spi_master_tx_data(void *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, const uint8_t* tx_data, uint32_t tx_data_size)
{
    bool success;

    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(device);
  
    success = spi_master_tx_data_no_cs(spi_base_address, command, command_size);
  
    if (success) {
        success = spi_master_tx_data_no_cs(spi_base_address, tx_data, tx_data_size);
    }

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(device);

    return success;
}

bool spi_master_tx_value(void *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t value, uint32_t tx_data_size)
{
    bool success;

    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(device);
  
    success = spi_master_tx_data_no_cs(spi_base_address, command, command_size);
  
	  for(int i=0; i < tx_data_size; i++){
					
        if (success) {
						success = spi_master_tx_data_no_cs(spi_base_address, &value, 1);
				}
    }

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(device);

    return success;
}

bool spi_master_tx(void *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size)
{
    bool success;

    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(device);
  
    success = spi_master_tx_data_no_cs(spi_base_address, command, command_size);

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(device);

    return success;
}

bool spi_master_rx_data(void *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t* rx_data, uint32_t rx_data_size, bool* has_changed)
{
    bool success;

    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(device);
  
    success = spi_master_tx_data_no_cs(spi_base_address, command, command_size);
  
    if (success) {
        success = spi_master_rx_data_no_cs(spi_base_address, rx_data, rx_data_size, false, has_changed);
    }

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(device);

    return success;
}

bool spi_master_rx_text(void *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t* rx_data, uint32_t rx_data_size, bool* has_changed)
{
    bool success;

    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(device);
  
    success = spi_master_tx_data_no_cs(spi_base_address, command, command_size);
  
    if (success) {
        success = spi_master_rx_data_no_cs(spi_base_address, rx_data, rx_data_size, true, has_changed);
    }

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(device);

    return success;
}


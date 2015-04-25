#include "spi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "common.h"
#include "bsp/boards.h"

uint32_t* spi_master_init(SPIModuleNumber module_number, SPIMode mode, bool lsb_first)
{
    uint32_t config_mode;

    NRF_SPI_Type *spi_base_address = (SPI0 == module_number)? NRF_SPI0 : (NRF_SPI_Type *)NRF_SPI1;

    if(SPI0 == module_number)
    {
        /* Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI0 */
        nrf_gpio_cfg_output(SPI0_SCK);
        nrf_gpio_cfg_output(SPI0_MOSI);
        nrf_gpio_cfg_input(SPI0_MISO, NRF_GPIO_PIN_NOPULL);
        nrf_gpio_cfg_output(SPI0_SS0);
        nrf_gpio_cfg_output(SPI0_SS1);

        /* Configure pins, frequency and mode */
        spi_base_address->PSELSCK  = SPI0_SCK;
        spi_base_address->PSELMOSI = SPI0_MOSI;
        spi_base_address->PSELMISO = SPI0_MISO;
        nrf_gpio_pin_set(SPI0_SS0); /* disable Set slave select (inactive high) */
        nrf_gpio_pin_set(SPI0_SS1); /* disable Set slave select (inactive high) */
    }
    else
    {
        // Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI1
        nrf_gpio_cfg_output(SPI1_SCK);
        nrf_gpio_cfg_output(SPI1_MOSI);
        nrf_gpio_cfg_input(SPI1_MISO, NRF_GPIO_PIN_NOPULL);
        nrf_gpio_cfg_output(SPI1_SS0);

        // Configure pins, frequency and mode 
        spi_base_address->PSELSCK  = SPI1_SCK;
        spi_base_address->PSELMOSI = SPI1_MOSI;
        spi_base_address->PSELMISO = SPI1_MISO;
        nrf_gpio_pin_clear(SPI1_SS0);         // disable Set slave select (inactive low)
    }

    spi_base_address->FREQUENCY = (uint32_t) SPI_OPERATING_FREQUENCY;

    /*lint -e845 -save // A zero has been given as right argument to operator '!'" */
    /** @snippet [SPI Select mode] */
    switch (mode )
    {
       
        case SPI_MODE0:
            config_mode = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE1:
            config_mode = (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE2:
            config_mode = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE3:
            config_mode = (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
            break;
        default:
            config_mode = 0;
            break;
    
    }
    /** @snippet [SPI Select mode] */
    /*lint -restore */

    /*lint -e845 -save // A zero has been given as right argument to operator '!'" */
    /** @snippet [SPI Select endianess] */
    if (lsb_first)
    {
        spi_base_address->CONFIG = (config_mode | (SPI_CONFIG_ORDER_LsbFirst << SPI_CONFIG_ORDER_Pos));
    }
    else
    {
        spi_base_address->CONFIG = (config_mode | (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos));
    }
    /** @snippet [SPI Select endianess] */
    /*lint -restore */

    spi_base_address->EVENTS_READY = 0U;

    /* Enable */
    spi_base_address->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

    return (uint32_t *)spi_base_address;
}

bool spi_master_tx_rx(uint32_t *spi_base_address, uint32_t device, uint16_t transfer_size, const uint8_t *tx_data, uint8_t *rx_data)
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

bool spi_master_tx_data_no_cs(uint32_t *spi_base_address, const uint8_t* tx_data, uint32_t tx_data_size) {
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

bool spi_master_rx_to_tx_no_cs(uint32_t *src_spi_base_address, uint32_t *dest_spi_base_address, uint32_t data_size) {
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
			
        dest_spi_base->TXD = src_spi_base->RXD;
			  src_spi_base->TXD = 0;
			
        counter = 0;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while ((dest_spi_base->EVENTS_READY == 0U || src_spi_base->EVENTS_READY == 0U)  && (counter < TIMEOUT_COUNTER))
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
		dest_spi_base->TXD = src_spi_base->RXD;

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

bool spi_master_rx_data_no_cs(uint32_t *spi_base_address, uint8_t* rx_data, uint32_t rx_data_size) {
    NRF_SPI_Type *spi_base = (NRF_SPI_Type *)spi_base_address;
	  uint32_t number_of_rxd_bytes = 0;
    uint32_t counter = 0;
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

        rx_data[number_of_rxd_bytes] = (uint8_t)spi_base->RXD;
        number_of_rxd_bytes++;
    };
    return true;
}

bool spi_master_tx_data(uint32_t *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, const uint8_t* tx_data, uint32_t tx_data_size)
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

bool spi_master_tx_value(uint32_t *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t value, uint32_t tx_data_size)
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

bool spi_master_tx(uint32_t *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size)
{
    bool success;

    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(device);
  
    success = spi_master_tx_data_no_cs(spi_base_address, command, command_size);

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(device);

    return success;
}

bool spi_master_rx_data(uint32_t *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t* rx_data, uint32_t rx_data_size)
{
    bool success;

    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(device);
  
    success = spi_master_tx_data_no_cs(spi_base_address, command, command_size);
  
    if (success) {
        success = spi_master_rx_data_no_cs(spi_base_address, rx_data, rx_data_size);
    }

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(device);

    return success;
}

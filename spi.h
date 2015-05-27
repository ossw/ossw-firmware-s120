#ifndef SPI_H
#define SPI_H

#include <stdbool.h>
#include <stdint.h>
#include "nrf_gpio.h"

/** @file
* @brief Software controlled SPI Master driver.
*
*
* @defgroup lib_driver_spi_master Software controlled SPI Master driver
* @{
* @ingroup nrf_drivers
* @brief Software controlled SPI Master driver.
*
* Supported features:
* - Operate two SPI masters independently or in parallel.
* - Transmit and Receive given size of data through SPI.
* - configure each SPI module separately through @ref spi_master_init.
*/

/**
 *  SPI master operating frequency
 */
typedef enum
{
    Freq_125Kbps = 0,        /*!< drive SClk with frequency 125Kbps */
    Freq_250Kbps,            /*!< drive SClk with frequency 250Kbps */
    Freq_500Kbps,            /*!< drive SClk with frequency 500Kbps */
    Freq_1Mbps,              /*!< drive SClk with frequency 1Mbps */
    Freq_2Mbps,              /*!< drive SClk with frequency 2Mbps */
    Freq_4Mbps,              /*!< drive SClk with frequency 4Mbps */
    Freq_8Mbps               /*!< drive SClk with frequency 8Mbps */
} SPIFrequency_t;

/**
 *  SPI master module number
 */
typedef enum
{
    SPI0 = 0,               /*!< SPI module 0 */
    SPI1                    /*!< SPI module 1 */
} SPIModuleNumber;

/**
 *  SPI mode
 */
typedef enum
{
    //------------------------Clock polarity 0, Clock starts with level 0-------------------------------------------
    SPI_MODE0 = 0,          /*!< Sample data at rising edge of clock and shift serial data at falling edge */
    SPI_MODE1,              /*!< sample data at falling edge of clock and shift serial data at rising edge */
    //------------------------Clock polarity 1, Clock starts with level 1-------------------------------------------
    SPI_MODE2,              /*!< sample data at falling edge of clock and shift serial data at rising edge */
    SPI_MODE3               /*!< Sample data at rising edge of clock and shift serial data at falling edge */
} SPIMode;

void spi_init(void);
	
bool spi_master_tx_rx(uint32_t *spi_base_address, uint32_t device, uint16_t transfer_size, const uint8_t *tx_data, uint8_t *rx_data);

bool spi_master_tx(uint32_t *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size);

bool spi_master_tx_data(uint32_t *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, const uint8_t* tx_data, uint32_t tx_data_size);

bool spi_master_tx_data_no_cs(uint32_t *spi_base, const uint8_t* tx_data, uint32_t tx_data_size);

bool spi_master_rx_data(uint32_t *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t* rx_data, uint32_t rx_data_size);

bool spi_master_rx_data_no_cs(uint32_t *spi_base, uint8_t* rx_data, uint32_t rx_data_size);

bool spi_master_rx_to_tx_no_cs(uint32_t *src_spi_base, uint32_t *dest_spi_base, uint32_t data_size, bool revert);

bool spi_master_tx_value(uint32_t *spi_base_address, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t value, uint32_t tx_data_size);

/**
 *@}
 **/
 
#endif /* SPI_H */

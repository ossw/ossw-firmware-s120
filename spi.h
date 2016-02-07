#ifndef SPI_H
#define SPI_H

#include <stdbool.h>
#include <stdint.h>

bool spi_master_tx_rx(void *spi_handler, uint32_t device, uint16_t transfer_size, const uint8_t *tx_data, uint8_t *rx_data);

bool spi_master_tx(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size);

bool spi_master_tx_data(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size, const uint8_t* tx_data, uint32_t tx_data_size);

bool spi_master_tx_data_no_cs(void *spi_handler, const uint8_t* tx_data, uint32_t tx_data_size);

bool spi_master_rx_data(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t* rx_data, uint32_t rx_data_size, bool* has_changed);

bool spi_master_rx_text(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t* rx_data, uint32_t max_rx_data_size, bool* has_changed);

bool spi_master_rx_data_no_cs(void *spi_handler, uint8_t* rx_data, uint32_t rx_data_size, bool stop_on_zero, bool* has_changed);

bool spi_master_rx_to_tx_no_cs(void *spi_handler, uint32_t *dest_spi_base, uint32_t data_size, bool revert);

bool spi_master_tx_value(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t value, uint32_t tx_data_size);

/**
 *@}
 **/
 
#endif /* SPI_H */

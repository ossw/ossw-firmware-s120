#include <stdint.h>
#include <string.h>
#include "spi.h"
#include "mcu.h"
#include "gpio.h"
#include "target.h"
#include "em_usart.h"

void spi_init(void);
	
bool spi_master_tx_rx(void *spi_handler, uint32_t device, uint16_t transfer_size, const uint8_t *tx_data, uint8_t *rx_data) {
		return true;
}

bool spi_master_tx(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size) {
		gpio_pin_clear(device);
		spi_master_tx_data_no_cs(spi_handler, command, command_size);
		gpio_pin_set(device);
		return true;
}

bool spi_master_tx_data(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size, const uint8_t* tx_data, uint32_t tx_data_size){
		gpio_pin_clear(device);
		spi_master_tx_data_no_cs(spi_handler, command, command_size);
		spi_master_tx_data_no_cs(spi_handler, tx_data, tx_data_size);
		gpio_pin_set(device);
		return true;
}

bool spi_master_tx_data_no_cs(void *spi_handler, const uint8_t* tx_data, uint32_t tx_data_size){
		for (int i=0; i<tx_data_size; i++) {
				USART_Tx(spi_handler, tx_data[i]);
		}
		return true;
}

bool spi_master_rx_data(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t* rx_data, uint32_t rx_data_size, bool* has_changed){
		return true;
}

bool spi_master_rx_text(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t* rx_data, uint32_t max_rx_data_size, bool* has_changed){
		return true;
}

bool spi_master_rx_data_no_cs(void *spi_handler, uint8_t* rx_data, uint32_t rx_data_size, bool stop_on_zero, bool* has_changed){
		return true;
}

bool spi_master_rx_to_tx_no_cs(void *spi_handler, uint32_t *dest_spi_base, uint32_t data_size, bool revert){
		return true;
}

bool spi_master_tx_value(void *spi_handler, uint32_t device, const uint8_t* command, uint16_t command_size, uint8_t value, uint32_t tx_data_size){
		return true;
}

#include <stdint.h>
#include <string.h>
#include "ext_ram.h"
#include "spi.h"
#include "target.h"

bool ext_ram_init(void) {
    uint8_t command[] = {EXT_RAM_WRITE_STATUS_COMMAND, EXT_RAM_STATUS_SEQUENCE | EXT_RAM_HOLD_DISABLE};
    return spi_master_tx(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 2);
}

bool ext_ram_read_data(uint32_t ext_ram_address, uint8_t *buffer, uint32_t data_size){
	  uint8_t command[] = {EXT_RAM_READ_COMMAND, 0xFF, 0xFF};
    command[1] = ext_ram_address >> 8 & 0xFF;
    command[2] = ext_ram_address & 0xFF;
    return spi_master_rx_data(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 3, buffer, data_size, NULL);
}

bool ext_ram_read_text(uint32_t ext_ram_address, uint8_t *buffer, uint32_t data_size, bool* has_changed){
	  uint8_t command[] = {EXT_RAM_READ_COMMAND, 0xFF, 0xFF};
    command[1] = ext_ram_address >> 8 & 0xFF;
    command[2] = ext_ram_address & 0xFF;
    return spi_master_rx_text(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 3, buffer, data_size, has_changed);
}

bool ext_ram_write_data(uint32_t ext_ram_address, uint8_t *buffer, uint32_t data_size){
	  uint8_t command[] = {EXT_RAM_WRITE_COMMAND, 0xFF, 0xFF};
    command[1] = ext_ram_address >> 8 & 0xFF;
    command[2] = ext_ram_address & 0xFF;
    return spi_master_tx_data(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 3, buffer, data_size);
}

bool ext_ram_fill(uint32_t ext_ram_address, uint8_t value, uint32_t data_size){
	  uint8_t command[] = {EXT_RAM_WRITE_COMMAND, 0xFF, 0xFF};
    command[1] = ext_ram_address >> 8 & 0xFF;
    command[2] = ext_ram_address & 0xFF;
		return spi_master_tx_value(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 3, value, data_size);
}

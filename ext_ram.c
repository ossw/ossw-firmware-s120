#include <stdint.h>
#include <string.h>
#include "ext_ram.h"
#include "spi.h"
#include "board.h"

bool ext_ram_init(void) {
    uint8_t command[] = {EXT_RAM_WRITE_STATUS_COMMAND, EXT_RAM_STATUS_SEQUENCE | EXT_RAM_HOLD_DISABLE};
    return spi_master_tx(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 2);
}

bool ext_ram_read_data(uint16_t ext_ram_address, uint8_t *buffer, uint32_t data_size){
	  uint8_t command[] = {EXT_RAM_READ_COMMAND, 0xFF, 0xFF};
    command[1] = ext_ram_address >> 8 & 0xFF;
    command[2] = ext_ram_address & 0xFF;
    return spi_master_rx_data(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 3, buffer, data_size, NULL);
}

bool ext_ram_read_text(uint16_t ext_ram_address, uint8_t *buffer, uint32_t data_size, bool* has_changed){
	  uint8_t command[] = {EXT_RAM_READ_COMMAND, 0xFF, 0xFF};
    command[1] = ext_ram_address >> 8 & 0xFF;
    command[2] = ext_ram_address & 0xFF;
    return spi_master_rx_text(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 3, buffer, data_size, has_changed);
}

bool ext_ram_write_data(uint16_t ext_ram_address, uint8_t *buffer, uint32_t data_size){
	  uint8_t command[] = {EXT_RAM_WRITE_COMMAND, 0xFF, 0xFF};
    command[1] = ext_ram_address >> 8 & 0xFF;
    command[2] = ext_ram_address & 0xFF;
    return spi_master_tx_data(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 3, buffer, data_size);
}

bool ext_ram_fill(uint16_t ext_ram_address, uint8_t value, uint32_t data_size){
	  uint8_t command[] = {EXT_RAM_WRITE_COMMAND, 0xFF, 0xFF};
    command[1] = ext_ram_address >> 8 & 0xFF;
    command[2] = ext_ram_address & 0xFF;
		return spi_master_tx_value(EXT_RAM_SPI, EXT_RAM_SPI_SS, command, 3, value, data_size);
}

uint8_t get_next_byte(uint16_t *ptr) {
    uint8_t data;
	  ext_ram_read_data(*ptr, &data, 1);
	  (*ptr)++;
	  return data;
}

uint16_t get_next_short(uint16_t *ptr) {
    uint8_t data[2];
	  ext_ram_read_data(*ptr, data, 2);
	  (*ptr)+=2;		
	  return data[0] << 8 | data[1];
}

uint32_t get_next_int(uint16_t *ptr) {
    uint8_t data[4];
	  ext_ram_read_data(*ptr, data, 4);
	  (*ptr)+=4;		
	  return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

uint8_t get_ext_ram_byte(uint16_t address) {
	uint8_t data[1];
	ext_ram_read_data(address, data, 1);
	return data[0];
}

void put_ext_ram_byte(uint16_t address, uint8_t value) {
	uint8_t data[1];
	data[0] = value;
	ext_ram_write_data(address, data, 1);
}

uint16_t get_ext_ram_short(uint16_t address) {
	uint8_t data[2];
	ext_ram_read_data(address, data, 2);
	return data[0] << 8 | data[1];
}

void put_ext_ram_short(uint16_t address, uint16_t value) {
	uint8_t data[2];
	data[0] = value >> 8 & 0xFF;
	data[1] = value & 0xFF;
	ext_ram_write_data(address, data, 2);
}

uint32_t get_ext_ram_int(uint16_t address) {
	uint8_t data[4];
	ext_ram_read_data(address, data, 4);
	return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

void put_ext_ram_int(uint16_t address, uint32_t value) {
	uint8_t data[4];
	data[0] = value >> 24;
	data[1] = value >> 16 & 0xFF;
	data[2] = value >> 8 & 0xFF;
	data[3] = value & 0xFF;
	ext_ram_write_data(address, data, 4);
}

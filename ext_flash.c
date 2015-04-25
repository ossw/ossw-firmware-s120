#include "ext_flash.h"
#include "spi.h"
#include "nrf_delay.h"
#include "bsp/boards.h"

uint8_t ext_flash_read_status(uint8_t *status) {
    uint8_t command[] = {0x05};
    return spi_master_rx_data(p_spi0_base_address, EXT_FLASH_SPI_SS, command, 1, status, 1);
}

bool ext_flash_read_data(uint32_t ext_flash_address, uint8_t *buffer, uint32_t data_size) {
    uint8_t command[] = {0x03, 0xFF, 0xFF, 0xFF};
    command[1] = ext_flash_address >> 16 & 0xFF;
    command[2] = ext_flash_address >> 8 & 0xFF;
    command[3] = ext_flash_address & 0xFF;
    return spi_master_rx_data(p_spi0_base_address, EXT_FLASH_SPI_SS, command, 4, buffer, data_size);
}

bool ext_flash_write_enable() {
    uint8_t command[] = {0x06};
    return spi_master_tx(p_spi0_base_address, EXT_FLASH_SPI_SS, command, 1);
}

bool ext_flash_wait_until_ready() {
    uint8_t status = 0;
    ext_flash_read_status(&status);
    if (status & 0x01 == 0) {
         return true;
    }
    // first wait 10us, second 100us, third and following 1000us
    uint32_t wait_time = 10;
    do {
        nrf_delay_us(wait_time); //wait 1 ms
        ext_flash_read_status(&status);
        if (wait_time < 1000) {
            wait_time *= 10;
        }
    } while (status & 0x01);
    return true;
}

bool ext_flash_erase_page(uint32_t page_address) {
    bool success = ext_flash_write_enable();
    if (success) {
        uint8_t command[] = {0x20, 0xFF, 0xFF, 0x00};
        command[1] = page_address >> 16 & 0xFF;
        command[2] = page_address >> 8 & 0xFF;
        success = spi_master_tx(p_spi0_base_address, EXT_FLASH_SPI_SS, command, 4);
    }
    if (success) {
        success = ext_flash_wait_until_ready();
    }
    return success;
}

bool ext_flash_erase_chip() {
    bool success = ext_flash_write_enable();
    if (success) {
        uint8_t command[] = {0xC7};
        success = spi_master_tx(p_spi0_base_address, EXT_FLASH_SPI_SS, command, 1);
    }
    if (success) {
        success = ext_flash_wait_until_ready();
    }
    return success;
}

bool ext_flash_write_page(int32_t ext_flash_address, uint8_t *buffer, uint32_t data_size) {
    bool success = ext_flash_write_enable();
    if (success) {
        uint8_t command[] = {0x02, 0xFF, 0xFF, 0xFF};
        command[1] = ext_flash_address >> 16 & 0xFF;
        command[2] = ext_flash_address >> 8 & 0xFF;
        command[3] = ext_flash_address & 0xFF;
        success = spi_master_tx_data(p_spi0_base_address, EXT_FLASH_SPI_SS, command, 4, buffer, data_size);
    }
    if (success) {
        success = ext_flash_wait_until_ready();
    }
    return success;
}

bool ext_flash_write_data_block(int32_t ext_flash_address, uint8_t *buffer, uint32_t data_size) {
    int32_t end_address = ext_flash_address + data_size;
    int32_t first_page_offset = ext_flash_address & (EXT_FLASH_PAGE_SIZE-1);
    int32_t page_size = (data_size + first_page_offset > EXT_FLASH_PAGE_SIZE) ? EXT_FLASH_PAGE_SIZE - first_page_offset : data_size;
    bool success;
    success = ext_flash_write_page(ext_flash_address, buffer, page_size);
    ext_flash_address = (0xFFFFFF00&ext_flash_address) + EXT_FLASH_PAGE_SIZE;
    buffer+=page_size;
    while (ext_flash_address < end_address) {
        if (!success) {
            return false;
        }
        page_size = ((end_address - ext_flash_address) > EXT_FLASH_PAGE_SIZE) ? EXT_FLASH_PAGE_SIZE : (end_address - ext_flash_address);
        success = ext_flash_write_page(ext_flash_address, buffer, page_size);
        ext_flash_address += EXT_FLASH_PAGE_SIZE;
        buffer+=page_size;
    }
    return success;
}

bool ext_flash_check_equal(int32_t ext_flash_address, uint8_t *buffer, uint32_t data_size) {
    uint8_t cmp_buff[128];
    int32_t end_address = ext_flash_address + data_size;
    int32_t part_size;
    while (ext_flash_address < end_address) {
        part_size = end_address - ext_flash_address > 128 ? 128 : end_address - ext_flash_address;
        if (!ext_flash_read_data(ext_flash_address, cmp_buff, part_size)){
            return false;
        }
        
        for (uint32_t i = 0; i < part_size; i++) {
            if (buffer[i] != cmp_buff[i]) {
                return false;
            }
        }
        
        buffer += part_size;
        ext_flash_address += part_size;
    }
    return true;
}

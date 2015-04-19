#ifndef EXT_FLASH_H
#define EXT_FLASH_H

#include <stdbool.h>
#include <stdint.h>

#define EXT_FLASH_PAGE_SIZE 0x100

uint8_t ext_flash_read_status(uint8_t *status);

bool ext_flash_read_data(uint32_t ext_flash_address, uint8_t *buffer, uint32_t data_size);

bool ext_flash_write_enable(void);

bool ext_flash_wait_until_ready(void);

bool ext_flash_erase_page(uint32_t page_address);

bool ext_flash_erase_chip(void);

bool ext_flash_write_page(int32_t ext_flash_address, uint8_t *buffer, uint32_t data_size);

bool ext_flash_write_data_block(int32_t ext_flash_address, uint8_t *buffer, uint32_t data_size);

bool ext_flash_check_equal(int32_t ext_flash_address, uint8_t *buffer, uint32_t data_size);
 
#endif /* EXT_FLASH_H */

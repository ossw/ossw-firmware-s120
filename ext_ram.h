#ifndef EXT_RAM_H
#define EXT_RAM_H

#include <stdbool.h>
#include <stdint.h>

#define EXT_RAM_WRITE_STATUS_COMMAND 0x01
#define EXT_RAM_READ_COMMAND         0x03
#define EXT_RAM_WRITE_COMMAND        0x02

#define EXT_RAM_STATUS_BYTE          0x00
#define EXT_RAM_STATUS_PAGE          0x80
#define EXT_RAM_STATUS_SEQUENCE      0x40

#define EXT_RAM_HOLD_DISABLE         0x01
#define EXT_RAM_HOLD_ENABLE          0x00

#define EXT_RAM_PAGE_SIZE						 0x20

#define EXT_RAM_DATA_FRAME_BUFFER   0x0000 // 0x0000 - 0x0BCF
#define EXT_RAM_DATA_RTC            0x0BD0 // 0x0BD0 - 0x0BD3
#define EXT_RAM_DATA_ALARM					0x0BD4 // 0x0BD4 - 0x0BD6
#define EXT_RAM_SETTINGS						0x0BD7 // 0x0BD7 - 0x0BDA
#define EXT_RAM_LIGHT_DURATION			0x0BDB // 1 byte
#define EXT_RAM_LIGHT_HOURS					0x0BDC // 2 bytes

#define EXT_RAM_TIMER_0							0x0BF8 // 4*2 bytes
#define EXT_RAM_CONFIG							0x0C00

#define EXT_RAM_DATA_STOPWATCH_RECALL				0x0CE0 // 0x0CE0 - 0x0E6F
#define EXT_RAM_DATA_DIALOG_TEXT						0x0E70 // 0x0E70 - 0x0FFF
#define EXT_RAM_DATA_CURRENT_SCREEN_CACHE		0x1000 // 0x1000 - 0x17FF
#define EXT_RAM_DATA_CURRENT_SCREEN_CACHE_SIZE 0x800

#define EXT_RAM_DATA_NOTIFICATION_INFO_ADDRESS 0x1800
#define EXT_RAM_DATA_NOTIFICATION_UPLOAD_ADDRESS 0x1C00
// last address 0x2000


bool ext_ram_init(void);

bool ext_ram_read_data(uint16_t ext_ram_address, uint8_t *buffer, uint32_t data_size);

bool ext_ram_read_text(uint16_t ext_ram_address, uint8_t *buffer, uint32_t data_size, bool* has_changed);

bool ext_ram_write_data(uint16_t ext_ram_address, uint8_t *buffer, uint32_t data_size);

bool ext_ram_fill(uint16_t ext_ram_address, uint8_t value, uint32_t data_size);

uint8_t get_next_byte(uint16_t *ptr);

uint16_t get_next_short(uint16_t *ptr);

uint32_t get_next_int(uint16_t *ptr);

uint8_t get_ext_ram_byte(uint16_t address);

void put_ext_ram_byte(uint16_t address, uint8_t value);

uint16_t get_ext_ram_short(uint16_t address);

void put_ext_ram_short(uint16_t address, uint16_t value);

uint32_t get_ext_ram_int(uint16_t address);

void put_ext_ram_int(uint16_t address, uint32_t value);

#endif /* EXT_RAM_H */

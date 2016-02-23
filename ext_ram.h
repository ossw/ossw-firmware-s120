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

#define EXT_RAM_PAGE_SIZE 0x20

#define EXT_RAM_DATA_FRAME_BUFFER    0x0 // 0x0000 - 0x0BCF
#define EXT_RAM_DATA_RTC             0xBD0 // 0x0BD0 - 0x0BD3
#define EXT_RAM_DATA_ALARM					0x0BE0 // 0x0BE0 - 0x0BE2

#define EXT_RAM_CONFIG								0xC00 // 0xC00 - 0xCDF

#define EXT_RAM_DATA_STOPWATCH_RECALL     0xCE0 // 0xCE0 - 0xFFF
#define EXT_RAM_DATA_CURRENT_SCREEN_CACHE 0x1000 // 0x1000 - 0x17FF
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

#endif /* EXT_RAM_H */

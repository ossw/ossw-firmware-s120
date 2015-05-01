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

#define EXT_RAM_DATA_FB              0x0 // 0x0000 - 0x0BCF
#define EXT_RAM_DATA_RTC             0xBD0 // 0x0BD0 - 0x0BD3

// last address 0x2000


bool ext_ram_init(void);

bool ext_ram_read_data(uint16_t ext_ram_address, uint8_t *buffer, uint32_t data_size);

bool ext_ram_write_data(uint16_t ext_ram_address, uint8_t *buffer, uint32_t data_size);

bool ext_ram_fill(uint16_t ext_ram_address, uint8_t value, uint32_t data_size);

#endif /* EXT_RAM_H */

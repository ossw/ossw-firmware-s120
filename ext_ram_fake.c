#include <stdint.h>
#include <string.h>
#include "ext_ram.h"
#include "spi.h"
#include "target.h"

uint8_t fake_ext_ram[0x2000];

bool ext_ram_init(void) {
    return true;
}

bool ext_ram_read_data(uint32_t ext_ram_address, uint8_t *buffer, uint32_t data_size){
		memcpy(buffer, &fake_ext_ram[ext_ram_address], data_size);
		return true;
}

bool ext_ram_read_text(uint32_t ext_ram_address, uint8_t *buffer, uint32_t data_size, bool* has_changed){
		if (strcmp((char*)buffer, (char*)&fake_ext_ram[ext_ram_address])) {
				*has_changed = false;
		} else {
				*has_changed = true;
				strcpy((char*)buffer, (char*)&fake_ext_ram[ext_ram_address]);
		}
		return true;
}

bool ext_ram_write_data(uint32_t ext_ram_address, uint8_t *buffer, uint32_t data_size){
	  memcpy(&fake_ext_ram[ext_ram_address], buffer, data_size);
		return true;
}

bool ext_ram_fill(uint32_t ext_ram_address, uint8_t value, uint32_t data_size){
		memset(&fake_ext_ram[ext_ram_address], value, data_size);
		return true;
}

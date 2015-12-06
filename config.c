#include "config.h"
#include "fs.h"
#include "ext_ram.h"
#include "app_button.h"

#define OFFSET_MAGIC 0
#define OFFSET_CONFIG_VERSION (OFFSET_MAGIC+4)
#define OFFSET_DEFAULT_WATCH_FACE (OFFSET_CONFIG_VERSION+1)
#define OFFSET_CUSTOM_WATCHSET_1 (OFFSET_DEFAULT_WATCH_FACE+SPIFFS_OBJ_NAME_LEN)
#define OFFSET_CUSTOM_WATCHSET_2 (OFFSET_CUSTOM_WATCHSET_1+SPIFFS_OBJ_NAME_LEN)
#define OFFSET_CUSTOM_WATCHSET_3 (OFFSET_CUSTOM_WATCHSET_2+SPIFFS_OBJ_NAME_LEN)
#define OFFSET_CUSTOM_WATCHSET_4 (OFFSET_CUSTOM_WATCHSET_3+SPIFFS_OBJ_NAME_LEN)

void config_init(void) {
		uint8_t buf[5];
		ext_ram_read_data(EXT_RAM_CONFIG + OFFSET_MAGIC, buf, 5);
		bool force_reset;
		app_button_is_pushed(3, &force_reset); // force reset if back button is pressed
	
		if (force_reset || buf[0] != buf[2] || buf[0] != 0x05 || buf[1] != buf[3] || buf[1] != 0x5E) {
				// set up clear config
				buf[0] = 0x05;
				buf[1] = 0x5E;
				buf[2] = 0x05;
				buf[3] = 0x5E;
				buf[4] = 0x0;
				ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_MAGIC, buf, 5);
				buf[0] = 0;
				ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_DEFAULT_WATCH_FACE, buf, 1);
				ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_CUSTOM_WATCHSET_1, buf, 1);
				ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_CUSTOM_WATCHSET_2, buf, 1);
				ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_CUSTOM_WATCHSET_3, buf, 1);
				ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_CUSTOM_WATCHSET_4, buf, 1);
		}
}

void config_set_default_watch_face(char* file_name) {
		ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_DEFAULT_WATCH_FACE, (uint8_t*)file_name, SPIFFS_OBJ_NAME_LEN);
}

spiffs_file config_get_dafault_watch_face_fd(void) {
		char buf[32];
		ext_ram_read_data(EXT_RAM_CONFIG + OFFSET_DEFAULT_WATCH_FACE, (uint8_t*)buf, SPIFFS_OBJ_NAME_LEN);
		if (buf[0] == 0) {
				return -1;
		}
		return SPIFFS_open(&fs, buf, SPIFFS_RDONLY, 0);
}

void config_clear_dafault_watch_face(void) {
		uint8_t data = 0;
		ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_DEFAULT_WATCH_FACE, &data, 1);
}

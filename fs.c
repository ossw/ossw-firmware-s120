#include "fs.h"
#include "ext_flash.h"

static u8_t spiffs_work_buf[FS_LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[32*4];
	
spiffs fs;

	
static s32_t spiffs_spi_read(u32_t addr, u32_t size, u8_t *dst) {
		return ext_flash_read_data(addr, dst, size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

static s32_t spiffs_spi_write(u32_t addr, u32_t size, u8_t *src) {
		return ext_flash_write_data(addr, src, size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

static s32_t spiffs_spi_erase(u32_t addr, u32_t size) {
		return ext_flash_erase_data(addr, size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
} 
	
void fs_format(void) {
		ext_flash_erase_chip();
		SPIFFS_format(&fs);
}

int fs_mount(void) {
			spiffs_config cfg;
			cfg.hal_read_f = spiffs_spi_read;
			cfg.hal_write_f = spiffs_spi_write;
			cfg.hal_erase_f = spiffs_spi_erase;
	
			return SPIFFS_mount(&fs,
					&cfg,
					spiffs_work_buf,
					spiffs_fds,
					sizeof(spiffs_fds),
					NULL,
					0,
					0);
}

void fs_unmount(void) {
		SPIFFS_unmount(&fs);
}

void fs_reformat(void) {
		fs_unmount();
		fs_format();
		fs_mount();
}

void fs_init() {
		int res = fs_mount();
		//printf("mount res: %i\n", res);
		
		//	if(res == SPIFFS_ERR_NOT_A_FS) {
		if (res < 0) {
				fs_format();
				res = fs_mount();
		}
}

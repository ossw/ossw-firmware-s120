#include "command.h"
#include "nordic_common.h"
#include "ble/ble_peripheral.h"
#include "spiffs/spiffs.h"
#include "scr_mngr.h"

static uint32_t data_ptr = 0;
static uint8_t data_buf[256];
static bool handle_data = 0;

static spiffs_file data_upload_fd;

#define COMMAND_OPEN_FILE_STREAM 0x20
#define COMMAND_APPEND_DATA_TO_FILE_STREAM 0x21
#define COMMAND_CLOSE_FILE_STREAM 0x22

void command_process(void) {
		
		if (!handle_data) {
				return;
		}
		
		uint8_t respCode = 0;
	
	//	sd_nvic_critical_region_enter();
		
		uint8_t command_id = data_buf[0];
		switch (command_id) {
			case COMMAND_OPEN_FILE_STREAM:
				
				if (scr_mngr_current_screen() == SCR_WATCH_SET) {	
						scr_mngr_show_screen(SCR_WATCHFACE);
				}
			
				data_upload_fd = SPIFFS_open(&fs, "watchset", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
			  if (data_upload_fd < 0) {
						// file open error
						respCode = (SPIFFS_errno(&fs)*-1)-9999;
				}
				break;
			case COMMAND_APPEND_DATA_TO_FILE_STREAM:
				if (SPIFFS_write(&fs, data_upload_fd, data_buf+1, data_ptr-1) < 0 ) {
						//file write error
						respCode = (SPIFFS_errno(&fs)*-1)-9999;
				}
				break;
			case COMMAND_CLOSE_FILE_STREAM:
				if (SPIFFS_close(&fs, data_upload_fd) < 0) {
						// file close error
						respCode = (SPIFFS_errno(&fs)*-1)-9999;
				}
				scr_mngr_show_screen(SCR_WATCH_SET);
				break;
		}
		
		handle_data = false;
	//	sd_nvic_critical_region_exit();
		ble_peripheral_confirm_command_processed(respCode);
}

void command_reset_data() {
		data_ptr=0;
}

void command_append_data(uint8_t *data, uint8_t size) {
		memcpy(data_buf+data_ptr, data, size);
		data_ptr+=size;
}

void command_data_complete() {
		handle_data = true;
}

bool command_is_data_handled(void) {
		return !handle_data;
}

#include "command.h"
#include "ble/ble_peripheral.h"
#include "spiffs/spiffs.h"
#include "scr_mngr.h"
#include "notifications.h"
#include "screens/scr_watchset.h"
#include "ext_ram.h"
#include "config.h"
#include "fs.h"

#define COMMAND_SET_DEFAULT_GLOBAL_ACTIONS 0x10
#define COMMAND_SET_DEFAULT_WATCH_FACE_ACTIONS 0x11
#define COMMAND_OPEN_FILE_STREAM 0x20
#define COMMAND_APPEND_DATA_TO_FILE_STREAM 0x21
#define COMMAND_CLOSE_FILE_STREAM 0x22
#define COMMAND_SET_EXT_PROPERTY_VALUE 0x30

static spiffs_file data_upload_fd;

static uint32_t notification_upload_ptr;
static uint16_t notification_upload_size;

static int init_notification_upload(uint32_t size) {
		notification_upload_ptr = EXT_RAM_DATA_NOTIFICATION_UPLOAD_ADDRESS;
		notification_upload_size = size;
		return 0;
}

static void handle_notification_upload_part(uint8_t *data, uint32_t size) {
	  ext_ram_write_data(notification_upload_ptr, data, size);
	  notification_upload_ptr += size;
}

static void handle_notification_upload_done() {
	  notifications_handle_data(EXT_RAM_DATA_NOTIFICATION_UPLOAD_ADDRESS, notification_upload_size);
}

static void handle_notification_alert_extend(uint16_t notification_id, uint16_t timout) {
	  notifications_alert_extend(notification_id, timout);
}

static void handle_notification_alert_stop(uint16_t notification_id) {
	  notifications_alert_stop(notification_id);
}

static void handle_external_properties_change(uint8_t *data, uint32_t size) {
		int p=0;
		while (p < size) {
				uint8_t propId = data[p++];
				uint8_t propSize = data[p++];
				set_external_property_data(propId, &data[p], propSize);
				p += propSize;
		}
}

void command_receive(uint8_t *rx_data, uint8_t rx_size, void (*handler)(uint8_t)) {

		uint8_t respCode = 0;

		switch (rx_data[0]) {
			case COMMAND_OPEN_FILE_STREAM:
			{
					if (scr_mngr_current_screen() == SCR_WATCH_SET) {
							scr_mngr_show_screen(SCR_WATCHFACE);
					}

					void* name_ptr = rx_size > 1 ? &rx_data[4] : "watchset";
					data_upload_fd = SPIFFS_open(&fs, name_ptr, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
					if (data_upload_fd < 0) {
							// file open error
							respCode = (SPIFFS_errno(&fs)*-1)-9999;
					}
			}
				break;
			case COMMAND_APPEND_DATA_TO_FILE_STREAM:
					if (SPIFFS_write(&fs, data_upload_fd, rx_data+1, rx_size-1) < 0 ) {
							//file write error
							respCode = (SPIFFS_errno(&fs)*-1)-9999;
					}
					break;
			case COMMAND_CLOSE_FILE_STREAM:
					if (SPIFFS_close(&fs, data_upload_fd) < 0) {
							// file close error
							respCode = (SPIFFS_errno(&fs)*-1)-9999;
					}
					break;
			case COMMAND_SET_DEFAULT_GLOBAL_ACTIONS:
					config_set_default_global_actions((default_action*)&rx_data[1]);
					break;
			case COMMAND_SET_DEFAULT_WATCH_FACE_ACTIONS:
					config_set_default_watchface_actions((default_action*)&rx_data[1]);
					break;
			case COMMAND_SET_EXT_PROPERTY_VALUE:
			    // set ext param
					handle_external_properties_change(&rx_data[1], rx_size-1);
					scr_mngr_redraw();
					break;
		 case 0x40:
			    // init notification upload
					respCode = init_notification_upload((rx_data[1]<<8) | rx_data[2]);
					break;
		 case 0x41:
			    // upload notification part
		 			handle_notification_upload_part(&rx_data[1], rx_size - 1);
					break;
		 case 0x42:
			    // upload notification finished
					handle_notification_upload_done();
			    break;
		 case 0x43:
			    // extend alert notification
					handle_notification_alert_extend(rx_data[1] << 8 | rx_data[2], rx_data[3] << 8 | rx_data[4]);
			    break;
		 case 0x44:
			    // stop alert notification
					handle_notification_alert_stop(rx_data[1] << 8 | rx_data[2]);
			    break;
		}

		if (handler != NULL) {
				handler(respCode);
		}
}

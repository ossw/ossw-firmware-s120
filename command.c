#include "command.h"
#include "nordic_common.h"
#include "ble/ble_peripheral.h"
#include "scr_mngr.h"
#include "notifications.h"
#include "screens/scr_watchset.h"
#include "ext_ram.h"
#include "nrf_soc.h"
#include "config.h"
#include "fs.h"
#include "filemanager.h"

#define COMMAND_SET_DEFAULT_GLOBAL_ACTIONS 0x10
#define COMMAND_SET_DEFAULT_WATCH_FACE_ACTIONS 0x11
#define COMMAND_OPEN_FILE_STREAM 0x20
#define COMMAND_APPEND_DATA_TO_FILE_STREAM 0x21
#define COMMAND_CLOSE_FILE_STREAM 0x22
#define COMMAND_LIST_FILES 0x23
#define COMMAND_SET_EXT_PROPERTY_VALUE 0x30

static uint32_t data_ptr = 0;
static uint8_t data_buf[256];
static bool handle_data = false;



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
				#ifdef OSSW_DEBUG
						sd_nvic_critical_region_enter(0);
						printf("SET: 0x%02x 0x%02x 0x%02x\r\n", propId, propSize, data[p]);
						sd_nvic_critical_region_exit(0);
				#endif
				set_external_property_data(propId, &data[p], propSize);
				p += propSize;
		}
}

void command_process(void) {
		
		if (!handle_data) {
				return;
		}
		
		#ifdef OSSW_DEBUG
				sd_nvic_critical_region_enter(0);
				printf("CMD:");
				for (int i=0; i<data_ptr; i++) {
						printf(" %02x", data_buf[i]);
				}
				printf("\r\n");
				sd_nvic_critical_region_exit(0);
		#endif
		
		uint8_t respCode = 0;
	
	//	sd_nvic_critical_region_enter();
		
		switch (data_buf[0]) {
			case COMMAND_OPEN_FILE_STREAM:
			{
					if (scr_mngr_current_screen() == SCR_WATCH_SET) {	
							scr_mngr_show_screen(SCR_WATCHFACE);
					}
				
					void* name_ptr = data_ptr > 1 ? &data_buf[4] : "watchset";
					respCode =  create_file(name_ptr);
					
			}
				break;
			case COMMAND_APPEND_DATA_TO_FILE_STREAM:
					respCode = append_file(data_buf+1,data_ptr-1);
					break;
			case COMMAND_CLOSE_FILE_STREAM:
					respCode = close_file();
					break;
			case COMMAND_LIST_FILES:
					//get file list
					//send via bluetooth
					break;
			case COMMAND_SET_DEFAULT_GLOBAL_ACTIONS:
					config_set_default_global_actions((default_action*)&data_buf[1]);
					break;
			case COMMAND_SET_DEFAULT_WATCH_FACE_ACTIONS:
					config_set_default_watchface_actions((default_action*)&data_buf[1]);
					break;
			case COMMAND_SET_EXT_PROPERTY_VALUE:
			    // set ext param
					handle_external_properties_change(&data_buf[1], data_ptr-1);
					scr_mngr_redraw();
					break;
		 case 0x40:
			    // init notification upload
					respCode = init_notification_upload((data_buf[1]<<8) | data_buf[2]);
					break;
		 case 0x41:
			    // upload notification part
		 			handle_notification_upload_part(&data_buf[1], data_ptr - 1);
					break;
		 case 0x42:
			    // upload notification finished
					handle_notification_upload_done();
			    break;	
		 case 0x43:
			    // extend alert notification
					handle_notification_alert_extend(data_buf[1] << 8 | data_buf[2], data_buf[3] << 8 | data_buf[4]);
			    break;		
		 case 0x44:
			    // stop alert notification
					handle_notification_alert_stop(data_buf[1] << 8 | data_buf[2]);
			    break;
		}
		
		handle_data = false;
	//	sd_nvic_critical_region_exit();
		ble_peripheral_confirm_command_processed(respCode);
}

void command_reset_data() {
		data_ptr=0;
		#ifdef OSSW_DEBUG
				sd_nvic_critical_region_enter(0);
				printf("CMD RESET\r\n");
				sd_nvic_critical_region_exit(0);
		#endif
}

void command_append_data(uint8_t *data, uint8_t size) {
		#ifdef OSSW_DEBUG
				sd_nvic_critical_region_enter(0);
				printf("CMD APPEND 0x%04x 0x%02x\r\n", data_ptr, size);
				sd_nvic_critical_region_exit(0);
		#endif
		memcpy(data_buf+data_ptr, data, size);
		data_ptr+=size;
}

void command_data_complete() {
		handle_data = true;
		#ifdef OSSW_DEBUG
				sd_nvic_critical_region_enter(0);
				printf("CMD COMMIT\r\n");
				sd_nvic_critical_region_exit(0);
		#endif
}

bool command_is_data_handled(void) {
		return !handle_data;
}

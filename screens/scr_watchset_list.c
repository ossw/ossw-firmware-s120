#include <string.h>
#include "scr_watchset_list.h"
#include "../watchset.h"
#include "../scr_mngr.h"
#include "../ext_ram.h"
#include "../fs.h"
#include "../mlcd_draw.h"
#include "../i18n/i18n.h"


#define MODE_SELECT 0
#define MODE_REMOVE 1
#define MODE_NO_FILES_FOUND 0x10

#define FILE_PATH_LENGTH 2

#define ENTRY_BUF_SIZE sizeof(struct spiffs_dirent)
		
struct scr_watchset_list_data {
		spiffs_DIR dir;
		struct spiffs_dirent last_entry;
		uint8_t last_fetched_file_no;
		uint8_t last_rendered_file_no;
		uint8_t mode;
		uint8_t last_rendered_mode;
		uint8_t show_file_no;
		uint8_t type;
		void* path; 
};

static struct scr_watchset_list_data* data;

static void scr_watchset_list_store_file_info (uint8_t fileNo, struct spiffs_dirent* entry) {
		ext_ram_write_data(EXT_RAM_DATA_CURRENT_SCREEN_CACHE + (fileNo * ENTRY_BUF_SIZE), (uint8_t*)entry, ENTRY_BUF_SIZE);
}

static void scr_watchset_list_read_next_file_names(void) {
	
		struct spiffs_dirent* entryPtr = &data->last_entry;
		
		while ((entryPtr = SPIFFS_readdir(&data->dir, entryPtr))!=0) {
				if (strncmp((const char*) data->last_entry.name, data->path, FILE_PATH_LENGTH) == 0) {
						scr_watchset_list_store_file_info(data->last_fetched_file_no + 1, entryPtr);
						data->last_fetched_file_no+=1;
						
						if (data->last_fetched_file_no > data->show_file_no) {
								break;
						}
				}
		}
		if (entryPtr == NULL) {
				data->show_file_no = data->last_fetched_file_no;
		}
}

static void scr_watchset_list_init(uint32_t param) {
	
		data = malloc(sizeof(struct scr_watchset_list_data));
	
		data->type = param&0xFF;
		switch (param) {
				case WATCH_SET_TYPE_WATCH_FACE:
						data->path = WATCH_SET_PATH_WATCH_FACE;
						break;
				case WATCH_SET_TYPE_APPLICATION:
						data->path = WATCH_SET_PATH_APPLICATION;
						break;
				case WATCH_SET_TYPE_UTILITY:
						data->path = WATCH_SET_PATH_UTILITY;
						break;
		}
		
		SPIFFS_opendir(&fs, data->path, &data->dir);
		
		struct spiffs_dirent* entryPtr = &data->last_entry;
		
		while ((entryPtr = SPIFFS_readdir(&data->dir, entryPtr)) != 0) {
				if (strncmp((const char*) data->last_entry.name, data->path, FILE_PATH_LENGTH) == 0) {
					scr_watchset_list_store_file_info(0, entryPtr);
					break;
				}
		}
		data->last_fetched_file_no = 0;
		data->show_file_no = 0;
		if (entryPtr == NULL) {
				data->mode = MODE_NO_FILES_FOUND;
		} else {
				data->mode = MODE_SELECT;
				scr_watchset_list_read_next_file_names();
		}
}

static void scr_watchset_list_clenup(void) {	
		if (data != NULL) {
				SPIFFS_closedir(&data->dir);
				free(data);
				data = NULL;
		}
}

static void scr_watchset_list_remove_file(void) {
		struct spiffs_dirent entry;
		ext_ram_read_data(EXT_RAM_DATA_CURRENT_SCREEN_CACHE + (ENTRY_BUF_SIZE* data->show_file_no), (uint8_t*)&entry, ENTRY_BUF_SIZE);	
		
		spiffs_file fd = -1;
		fd = SPIFFS_open_by_dirent(&fs, &entry, SPIFFS_RDWR, 0);
		if (fd < 0) {
				return;
		}
		int res = SPIFFS_fremove(&fs, fd);
		if (res < 0) {
				return;
		}
		SPIFFS_close(&fs, fd);
	
		for (int i = data->show_file_no + 1; i <= data->last_fetched_file_no; i++) {
				ext_ram_read_data(EXT_RAM_DATA_CURRENT_SCREEN_CACHE + (ENTRY_BUF_SIZE*i), (uint8_t*)&entry, ENTRY_BUF_SIZE);
				ext_ram_write_data(EXT_RAM_DATA_CURRENT_SCREEN_CACHE + (ENTRY_BUF_SIZE*(i-1)), (uint8_t*)&entry, ENTRY_BUF_SIZE);		
		}
		if (data->last_fetched_file_no > 0) {
				data->last_fetched_file_no--;
				data->mode = MODE_SELECT;
		} else {
				data->mode = MODE_NO_FILES_FOUND;
		}
}

static void scr_watchset_list_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
						if (data->mode == MODE_REMOVE) {
								data->mode = MODE_SELECT;
						} else {
								scr_mngr_show_screen(SCR_WATCHFACE);
						}
				    break;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
						if (data->mode == MODE_SELECT) {
								data->show_file_no++;
						} else if (data->mode == MODE_REMOVE) {
								data->mode = MODE_SELECT;
						}
				    break;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
						if (data->mode == MODE_SELECT) {
								uint32_t ptr = EXT_RAM_DATA_CURRENT_SCREEN_CACHE + (ENTRY_BUF_SIZE*data->show_file_no);
								if (data->type == WATCH_SET_TYPE_WATCH_FACE) {
										struct spiffs_dirent entry;
										ext_ram_read_data(ptr, (uint8_t*)&entry, sizeof(struct spiffs_dirent));
										watchset_set_default_watch_face(&entry);
										scr_mngr_show_screen(SCR_WATCHFACE);
								} else {
										scr_mngr_show_screen_with_param(SCR_WATCH_SET, (1<<24) | ptr);
								}
						}
				    break;
			  case SCR_EVENT_PARAM_BUTTON_UP:
						if (data->mode == MODE_SELECT) {
								if (data->show_file_no > 0 ) {
										data->show_file_no--;
								}
						} else if (data->mode == MODE_REMOVE) {
								scr_watchset_list_remove_file();
						}
				    break;
		}
}

static void scr_watchset_list_handle_button_long_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
						if (data->mode == MODE_SELECT) {
								data->mode = MODE_REMOVE;
						}
				    break;
		}
}

static void scr_watchset_list_draw_current_mode() {
		switch (data->mode) {
				case MODE_NO_FILES_FOUND:
						mlcd_draw_text(i18n_resolve_message(MESSAGE_NO_FILES_FOUND), 2, 2, MLCD_XRES-4, MLCD_XRES-4, FONT_OPTION_BIG, HORIZONTAL_ALIGN_CENTER | VERTICAL_ALIGN_CENTER | MULTILINE);	
						break;
				case MODE_SELECT:		
				{					
						struct spiffs_dirent entry;
						ext_ram_read_data(EXT_RAM_DATA_CURRENT_SCREEN_CACHE + (ENTRY_BUF_SIZE*data->show_file_no) + FILE_PATH_LENGTH, (uint8_t*)&entry, ENTRY_BUF_SIZE);
						mlcd_draw_text((const char *)entry.name, 2, 30, MLCD_XRES-4, MLCD_YRES-60, FONT_OPTION_BIG, HORIZONTAL_ALIGN_CENTER | VERTICAL_ALIGN_CENTER | MULTILINE);
						if (data->show_file_no > 0) {
								ext_ram_read_data(EXT_RAM_DATA_CURRENT_SCREEN_CACHE + (ENTRY_BUF_SIZE*(data->show_file_no-1)) + FILE_PATH_LENGTH, (uint8_t*)&entry, ENTRY_BUF_SIZE);
								mlcd_draw_text((const char *)entry.name, 2, 0, MLCD_XRES-4, 30, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER | VERTICAL_ALIGN_CENTER);
						}
						if (data->show_file_no < data->last_fetched_file_no ) {
								ext_ram_read_data(EXT_RAM_DATA_CURRENT_SCREEN_CACHE + (ENTRY_BUF_SIZE*(data->show_file_no+1)) + FILE_PATH_LENGTH, (uint8_t*)&entry, ENTRY_BUF_SIZE);
								mlcd_draw_text((const char *)entry.name, 2, MLCD_YRES - 30, MLCD_XRES-4, 30, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER | VERTICAL_ALIGN_CENTER);
						}
				}
						break;
				
				case MODE_REMOVE:
						mlcd_draw_text(i18n_resolve_message(MESSAGE_YES), 2, 0, MLCD_XRES-4, 30, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
						mlcd_draw_text(i18n_resolve_message(MESSAGE_REMOVE), 2, 30, MLCD_XRES-4, MLCD_YRES-60, FONT_OPTION_BIG, HORIZONTAL_ALIGN_CENTER | VERTICAL_ALIGN_CENTER | MULTILINE);
						mlcd_draw_text(i18n_resolve_message(MESSAGE_NO), 2, MLCD_YRES-30, MLCD_XRES-4, 30, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_RIGHT | VERTICAL_ALIGN_CENTER);
						break;
		}
		data->last_rendered_mode = data->mode;
		data->last_rendered_file_no = data->show_file_no;
}

static void scr_watchset_list_draw() {
		scr_watchset_list_draw_current_mode();
}

static void scr_watchset_list_redraw() {
		if (data->mode == MODE_SELECT) {
				if (data->last_rendered_mode == MODE_SELECT && data->last_rendered_file_no == data->show_file_no) {
						return;
				}
				if (data->show_file_no + 1 > data->last_fetched_file_no) {
						scr_watchset_list_read_next_file_names();
				}
				
		} else if (data->mode == data->last_rendered_mode) {
				return;
		}
		
		mlcd_fb_clear();
		scr_watchset_list_draw_current_mode();
}

void scr_watchset_list_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_watchset_list_init(event_param);
				    break;
			  case SCR_EVENT_DRAW_SCREEN:
				    scr_watchset_list_draw();
				    break;
			  case SCR_EVENT_REFRESH_SCREEN:
				    scr_watchset_list_redraw();
				    break;
			  case SCR_EVENT_BUTTON_PRESSED:
				    scr_watchset_list_handle_button_pressed(event_param);
				    break;
			  case SCR_EVENT_BUTTON_LONG_PRESSED:
				    scr_watchset_list_handle_button_long_pressed(event_param);
				    break;
			  case SCR_EVENT_DESTROY_SCREEN:
						scr_watchset_list_clenup();
				    break;
		}
}

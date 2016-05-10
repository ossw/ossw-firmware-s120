#include "config.h"
#include "fs.h"
#include "ext_ram.h"
#include "app_button.h"
#include "watchset.h"
#include "scr_mngr.h"

#define CUSTOM_WATCHSET_NUMBER 5

#define OFFSET_MAGIC 0
#define OFFSET_CONFIG_VERSION (OFFSET_MAGIC+4)
#define OFFSET_DEFAULT_WATCH_FACE (OFFSET_CONFIG_VERSION+1)
#define OFFSET_CUSTOM_WATCHSET (OFFSET_DEFAULT_WATCH_FACE+SPIFFS_OBJ_NAME_LEN)
#define OFFSET_GLOBAL_ACTION_HANLDERS (OFFSET_CUSTOM_WATCHSET+(SPIFFS_OBJ_NAME_LEN*CUSTOM_WATCHSET_NUMBER))
#define OFFSET_WATCH_FACE_ACTION_HANLDERS (OFFSET_GLOBAL_ACTION_HANLDERS + (9 * 2))

#define SETTINGS_SIZE 1

static default_action default_global_actions[9];
static default_action default_watchface_actions[9];

static void config_store_global_actions(void) {
		ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_GLOBAL_ACTION_HANLDERS, (uint8_t*)default_global_actions, sizeof(default_global_actions));
}

static void config_load_global_actions(void) {
		ext_ram_read_data(EXT_RAM_CONFIG + OFFSET_GLOBAL_ACTION_HANLDERS, (uint8_t*)default_global_actions, sizeof(default_global_actions));
}

static void config_reset_global_actions(void) {
		memset(default_global_actions, 0, sizeof(default_global_actions));
		default_global_actions[7].action_id = WATCH_SET_FUNC_TOGGLE_BACKLIGHT;
		default_global_actions[8].action_id = WATCH_SET_FUNC_TEMPORARY_BACKLIGHT;
		config_store_global_actions();
}

static void config_store_watchface_actions(void) {
		ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_WATCH_FACE_ACTION_HANLDERS, (uint8_t*)default_watchface_actions, sizeof(default_watchface_actions));
}

static void config_load_watchface_actions(void) {
		ext_ram_read_data(EXT_RAM_CONFIG + OFFSET_WATCH_FACE_ACTION_HANLDERS, (uint8_t*)default_watchface_actions, sizeof(default_watchface_actions));
}

static void config_reset_watchface_actions(void) {
		memset(default_watchface_actions, 0, sizeof(default_watchface_actions));
		default_watchface_actions[0].action_id = WATCH_SET_FUNC_SHOW_NOTIFICATIONS;
		default_watchface_actions[1].action_id = WATCH_SET_FUNC_SHOW_APPLICATIONS;
		default_watchface_actions[2].action_id = WATCH_SET_FUNC_SHOW_NEXT_WATCH_FACE;
		default_watchface_actions[3].action_id = WATCH_SET_FUNC_SHOW_STATUS;
		default_watchface_actions[4].action_id = WATCH_SET_FUNC_SHOW_WATCH_FACES;
		default_watchface_actions[5].action_id = WATCH_SET_FUNC_SHOW_SETTINGS;
		default_watchface_actions[6].action_id = WATCH_SET_FUNC_SHOW_UTILS;
		config_store_watchface_actions();
}

default_action* config_get_default_global_actions(void) {
		return default_global_actions;
}

default_action* config_get_default_watchface_actions(void) {
		return default_watchface_actions;
}

void config_set_default_global_actions(default_action* actions) {
		memcpy(default_global_actions, actions, sizeof(default_global_actions));
		config_store_global_actions();
}

void config_set_default_watchface_actions(default_action* actions) {
		memcpy(default_watchface_actions, actions, sizeof(default_watchface_actions));
		config_store_watchface_actions();
}

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
				ext_ram_fill(EXT_RAM_CONFIG + OFFSET_DEFAULT_WATCH_FACE, 0, 1);
				for (int i=0; i<CUSTOM_WATCHSET_NUMBER; i++) {
						ext_ram_fill(EXT_RAM_CONFIG + OFFSET_CUSTOM_WATCHSET+(SPIFFS_OBJ_NAME_LEN*i), 0, 1);
				}
				config_reset_global_actions();
				config_reset_watchface_actions();
		} else {
				config_load_global_actions();
				config_load_watchface_actions();
		}
}

void config_set_default_watch_face(char* file_name) {
		ext_ram_write_data(EXT_RAM_CONFIG + OFFSET_DEFAULT_WATCH_FACE, (uint8_t*)file_name, SPIFFS_OBJ_NAME_LEN);
}

void config_get_default_watch_face(char* file_name) {
		ext_ram_read_data(EXT_RAM_CONFIG + OFFSET_DEFAULT_WATCH_FACE, (uint8_t*)file_name, SPIFFS_OBJ_NAME_LEN);
}

spiffs_file config_get_default_watch_face_fd(void) {
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

int config_get_handler_index_from_event(uint32_t event_type, uint32_t event_param) {
		switch(event_type) {
				case SCR_EVENT_BUTTON_PRESSED:
						switch (event_param) {
								case SCR_EVENT_PARAM_BUTTON_UP:
										return 0;
								case SCR_EVENT_PARAM_BUTTON_SELECT:
										return 1;
								case SCR_EVENT_PARAM_BUTTON_DOWN:
										return 2;
								case SCR_EVENT_PARAM_BUTTON_BACK:
										return 3;
						}
						return -1;
				case SCR_EVENT_BUTTON_LONG_PRESSED:
						switch (event_param) {
								case SCR_EVENT_PARAM_BUTTON_UP:
										return 4;
								case SCR_EVENT_PARAM_BUTTON_SELECT:
										return 5;
								case SCR_EVENT_PARAM_BUTTON_DOWN:
										return 6;
								case SCR_EVENT_PARAM_BUTTON_BACK:
										return 7;
						}
						return -1;
				case SCR_EVENT_WRIST_SHAKE:
						return 8;
		}
		return -1;
}

bool get_settings(uint32_t mask) {
		uint32_t settings = get_ext_ram_int(EXT_RAM_SETTINGS);
		return settings & mask;
}

void settings_toggle(uint32_t mask) {
		uint32_t settings = get_ext_ram_int(EXT_RAM_SETTINGS);
		settings ^= mask;
		put_ext_ram_int(EXT_RAM_SETTINGS, settings);
}

void settings_on(uint32_t mask) {
		uint32_t settings = get_ext_ram_int(EXT_RAM_SETTINGS);
		settings |= mask;
		put_ext_ram_int(EXT_RAM_SETTINGS, settings);
}

void settings_off(uint32_t mask) {
		uint32_t settings = get_ext_ram_int(EXT_RAM_SETTINGS);
		settings &= ~mask;
		put_ext_ram_int(EXT_RAM_SETTINGS, settings);
}

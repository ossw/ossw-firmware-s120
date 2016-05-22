#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdbool.h>
#include <stdint.h>
#include "fs.h"

#define CONFIG_DISCONNECT_ALERT			0x01
#define CONFIG_NOTIFICATION_LIGHT		0x02
#define CONFIG_DISPLAY_INVERT				0x04
#define CONFIG_SLOW_REFRESH					0x08
#define CONFIG_CENTRAL_MODE					0x10
#define CONFIG_BLUETOOTH_ON					0x20
#define CONFIG_BUTTONS_LIGHT				0x40
#define CONFIG_OCLOCK								0x80
#define CONFIG_ACCELEROMETER				0x100
#define CONFIG_SLEEP_AS_ANDROID			0x200

typedef struct {
		uint8_t action_id;
		uint8_t parameter;
} default_action;

void config_init(void);

void config_clear_dafault_watch_face(void);

void config_set_default_watch_face(char* file_name);

spiffs_file config_get_default_watch_face_fd(void);

void config_get_default_watch_face(char* file_name);
	
default_action* config_get_default_global_actions(void);

default_action* config_get_default_watchface_actions(void);

int config_get_handler_index_from_event(uint32_t event_type, uint32_t event_param);

void config_set_default_global_actions(default_action* actions);

void config_set_default_watchface_actions(default_action* actions);

bool get_settings(uint32_t mask);

void settings_toggle(uint32_t mask);

void settings_on(uint32_t mask);

void settings_off(uint32_t mask);

#endif /* CONFIGURATION_H */

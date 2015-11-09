#include "watchset.h"
#include "fs.h"
#include "scr_mngr.h"
#include "mlcd.h"

static spiffs_file default_watch_face_fd = -1;

void watchset_set_default_watch_face(struct spiffs_dirent* entry) {
		SPIFFS_close(&fs, default_watch_face_fd); 
		default_watch_face_fd = SPIFFS_open_by_dirent(&fs, entry, SPIFFS_RDONLY, 0);
}

spiffs_file watchset_get_dafault_watch_face_fd(void) {
		return default_watch_face_fd;
}

static void watchset_default_watch_face_handle_button_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_SELECT:
            scr_mngr_show_screen_with_param(SCR_WATCH_SET_LIST, WATCH_SET_TYPE_APPLICATION);
            break;
    }
}

static void watchset_default_watch_face_handle_button_long_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_BACK:
            mlcd_backlight_toggle();
            break;
        case SCR_EVENT_PARAM_BUTTON_SELECT:
            scr_mngr_show_screen(SCR_SETTINGS);
            break;
        case SCR_EVENT_PARAM_BUTTON_UP:
            scr_mngr_show_screen_with_param(SCR_WATCH_SET_LIST, WATCH_SET_TYPE_WATCH_FACE);
            break;
        case SCR_EVENT_PARAM_BUTTON_DOWN:
            scr_mngr_show_screen_with_param(SCR_WATCH_SET_LIST, WATCH_SET_TYPE_UTILITY);
            break;
    }
}

void watchset_default_watch_face_handle_event(uint32_t event_type, uint32_t event_param) {
		switch(event_type) {
        case SCR_EVENT_BUTTON_PRESSED:
            watchset_default_watch_face_handle_button_pressed(event_param);
            break;
        case SCR_EVENT_BUTTON_LONG_PRESSED:
            watchset_default_watch_face_handle_button_long_pressed(event_param);
            break;
		}
}

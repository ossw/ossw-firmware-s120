#ifndef SCR_MNGR_H
#define SCR_MNGR_H

#include <stdbool.h>
#include <stdint.h>

#define SCR_CHOOSE_MODE                0x00
#define SCR_WATCHFACE                  0x01
#define SCR_CHANGE_DATE                0x02
#define SCR_CHANGE_TIME                0x03
#define SCR_SETTINGS                   0x04
#define SCR_WATCH_SET                  0x05

#define SCR_EVENT_INIT_SCREEN          0x01
#define SCR_EVENT_REFRESH_SCREEN       0x02
#define SCR_EVENT_DESTROY_SCREEN       0x03
#define SCR_EVENT_RTC_TIME_CHANGED     0x10
#define SCR_EVENT_BUTTON_PRESSED       0x11
#define SCR_EVENT_BUTTON_LONG_PRESSED  0x12

#define SCR_EVENT_PARAM_BUTTON_UP      0x01
#define SCR_EVENT_PARAM_BUTTON_DOWN    0x02
#define SCR_EVENT_PARAM_BUTTON_SELECT  0x04
#define SCR_EVENT_PARAM_BUTTON_BACK    0x08

void scr_mngr_init(void);

void scr_mngr_handle_event(uint32_t event_type, uint32_t event_param);

void scr_mngr_show_screen(uint32_t screen_id);

void scr_mngr_draw_notification_bar(void);

void scr_mngr_redraw_notification_bar(void);

#endif /* SCR_MNGR_H */

#ifndef SCR_MNGR_H
#define SCR_MNGR_H

#include <stdbool.h>
#include <stdint.h>

#define SCR_CHOOSE_MODE                0x01
#define SCR_WATCHFACE                  0x02
#define SCR_CHANGE_DATE                0x03
#define SCR_CHANGE_TIME                0x04
#define SCR_SETTINGS                   0x05
#define SCR_WATCH_SET                  0x06
#define SCR_WATCH_SET_LIST             0x07
#define SCR_ABOUT			                 0x08
#define SCR_STATUS				             0x09
#define SCR_WATCHFACE_ANALOG           0x0A
#define SCR_SET_ALARM                  0x0B

#define SCR_ALERT_NOTIFICATION         0xA0
#define SCR_NOTIFICATIONS              0xA1

#define SCR_NOT_SET                    0x00

#define SCR_EVENT_INIT_SCREEN          0x01
#define SCR_EVENT_DRAW_SCREEN          0x02
#define SCR_EVENT_REFRESH_SCREEN       0x03
#define SCR_EVENT_DESTROY_SCREEN       0x0f
#define SCR_EVENT_RTC_TIME_CHANGED     0x10
#define SCR_EVENT_BUTTON_PRESSED       0x11
#define SCR_EVENT_BUTTON_LONG_PRESSED  0x12
#define SCR_EVENT_WRIST_SHAKE					 0x13
#define SCR_EVENT_APP_CONNECTION_CONFIRMED 0x20

#define SCR_EVENT_PARAM_BUTTON_UP      0x01
#define SCR_EVENT_PARAM_BUTTON_DOWN    0x02
#define SCR_EVENT_PARAM_BUTTON_SELECT  0x04
#define SCR_EVENT_PARAM_BUTTON_BACK    0x08

#define SCR_NOTIFICATIONS_STATE_NONE  		0
#define SCR_NOTIFICATIONS_STATE_INIT  		1
#define SCR_NOTIFICATIONS_STATE_SHOW  		2
#define SCR_NOTIFICATIONS_STATE_REDRAW  	3
#define SCR_NOTIFICATIONS_STATE_CLOSE 		4

#define SCR_ALERT_NOTIFICATION_STATE_NONE  0
#define SCR_ALERT_NOTIFICATION_STATE_INIT  1
#define SCR_ALERT_NOTIFICATION_STATE_SHOW  2
#define SCR_ALERT_NOTIFICATION_STATE_CLOSE 3

typedef struct {
		bool force_colors;
} scr_mngr_draw_ctx;

void scr_mngr_init(void);

void scr_mngr_draw_screen(void);

void scr_mngr_handle_event(uint32_t event_type, uint32_t event_param);

void scr_mngr_show_screen(uint16_t screen_id);

void scr_mngr_show_screen_with_param(uint16_t screen_id, uint32_t param);
	
void scr_mngr_draw_notification_bar(void);

void scr_mngr_redraw_notification_bar(void);

void scr_mngr_show_alert_notification(uint32_t address);

void scr_mngr_close_alert_notification(void);

void scr_mngr_show_notifications(void);

void scr_mngr_close_notifications(void);

uint8_t scr_mngr_current_screen(void);

void scr_mngr_redraw(void);

#endif /* SCR_MNGR_H */

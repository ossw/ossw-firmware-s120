#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <stdbool.h>
#include <stdint.h>

#define NOTIFICATIONS_CATEGORY_INCOMING_CALL					1

#define NOTIFICATIONS_FUNCTION_DISMISS_ALL     			  0x10
#define NOTIFICATIONS_FUNCTION_RESEND			     			  0x11
#define NOTIFICATIONS_FUNCTION_ALERT_DISMISS          0x20
#define NOTIFICATIONS_FUNCTION_ALERT_OPTION_1         0x21
#define NOTIFICATIONS_FUNCTION_ALERT_OPTION_2         0x22

#define NOTIFICATIONS_SHOW_FIRST						          0x30
#define NOTIFICATIONS_NEXT_PART							          0x31
#define NOTIFICATIONS_NEXT									          0x32
#define NOTIFICATIONS_PREV_PART							          0x33
#define NOTIFICATIONS_OPEN							              0x34

#define DIALOG_RESULT										              0x35
#define PHONE_FUNC_PHONE_DISCOVERY										0x40
#define PHONE_FUNC_SEND_SMS							              0x41
#define PHONE_FUNC_CALL_CONTACT					              0x42
#define PHONE_FUNC_GTASKS								              0x43
#define PHONE_FUNC_ACCELEROMETER				              0x45
#define PHONE_FUNC_SLEEP_AS_ANDROID			              0x46

#define NOTIFICATIONS_TYPE_ALERT 											1
#define NOTIFICATIONS_TYPE_INFO												2
#define NOTIFICATIONS_TYPE_UPDATE											3
#define NOTIFICATIONS_TYPE_DIALOG_SELECT							4
#define NOTIFICATIONS_TYPE_DIALOG_OPTIONS							5
#define NOTIFICATIONS_TYPE_SLEEP_START								20
#define NOTIFICATIONS_TYPE_SLEEP_STOP									21
#define NOTIFICATIONS_TYPE_SLEEP_BATCH_SIZE						22
#define NOTIFICATIONS_TYPE_DIALOG_CLOSE								255

#define NOTIFICATIONS_CATEGORY_SUMMARY								0
#define NOTIFICATIONS_CATEGORY_INCOMING_CALL					1
#define NOTIFICATIONS_CATEGORY_MESSAGE								2
#define NOTIFICATIONS_CATEGORY_EMAIL									3
#define NOTIFICATIONS_CATEGORY_SOCIAL									4
#define NOTIFICATIONS_CATEGORY_ALARM									5
#define NOTIFICATIONS_CATEGORY_OTHER									255
		
void notifications_init(void);

void notifications_open(uint16_t notification_id);

void notifications_handle_data(uint16_t notification_id, uint16_t address);

void notifications_info_notify(uint16_t time, uint32_t vibration_pattern);

void notifications_info_update(void);
	
void notifications_info_clear_all(void);

void notifications_alert_notify(uint16_t notification_id, uint16_t address, uint16_t timeout, uint32_t vibration_pattern);

void notifications_alert_extend(uint16_t notification_id, uint16_t timeout);
	
void notifications_alert_stop(uint16_t notification_id);

void notifications_invoke_function(uint8_t function_id);
	
void notifications_next_part(uint16_t notification_id, uint8_t part_no);

void notifications_prev_part(uint16_t notification_id, uint8_t part_no);

void notifications_next(uint16_t notification_id);

void notifications_dismiss_all(void);

uint16_t notifications_get_current_data(void);

#endif /* NOTIFICATIONS_H */

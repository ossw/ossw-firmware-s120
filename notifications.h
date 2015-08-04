#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <stdbool.h>
#include <stdint.h>

#define NOTIFICATIONS_CATEGORY_INCOMING_CALL					1

#define NOTIFICATIONS_FUNCTION_DISMISS_ALL     			  0x10
#define NOTIFICATIONS_FUNCTION_ALERT_DISMISS          0x20
#define NOTIFICATIONS_FUNCTION_ALERT_OPTION_1         0x21
#define NOTIFICATIONS_FUNCTION_ALERT_OPTION_2         0x22

#define NOTIFICATIONS_TYPE_ALERT 											1
#define NOTIFICATIONS_TYPE_INFO												2
#define NOTIFICATIONS_TYPE_UPDATE											3

void notifications_init(void);

void notifications_process(void);

void notifications_handle_data(uint16_t notification_id, uint16_t address);

bool notifications_is_data_handled(void);

void notifications_info_notify(uint16_t address, uint16_t time, uint32_t vibration_pattern);

void notifications_info_update(uint16_t address);
	
void notifications_info_clear_all(void);

void notifications_alert_notify(uint16_t notification_id, uint16_t address, uint16_t timeout, uint32_t vibration_pattern);

void notifications_alert_extend(uint16_t notification_id, uint16_t timeout);
	
void notifications_alert_stop(uint16_t notification_id);

void notifications_invoke_function(uint8_t function_id);

void notifications_dismiss_all(void);

#endif /* NOTIFICATIONS_H */

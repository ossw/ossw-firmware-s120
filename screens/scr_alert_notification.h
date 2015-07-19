#ifndef SCR_ALERT_NOTIFICATION_H
#define SCR_ALERT_NOTIFICATION_H

#include <stdbool.h>
#include <stdint.h>

#define NOTIFICATION_TYPE_INCOMING_CALL   1

#define NOTIFICATION_FUNCTION_REJECT      1

void scr_alert_notification_handle_event(uint32_t event_type, uint32_t event_param);
	
#endif /* SCR_ALERT_NOTIFICATION_H */

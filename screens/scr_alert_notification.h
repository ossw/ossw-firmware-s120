#ifndef SCR_ALERT_NOTIFICATION_H
#define SCR_ALERT_NOTIFICATION_H

#include <stdbool.h>
#include <stdint.h>

bool scr_alert_notification_handle_event(uint32_t event_type, uint32_t event_param);
	
#endif /* SCR_ALERT_NOTIFICATION_H */

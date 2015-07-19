#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <stdbool.h>
#include <stdint.h>

void notifications_init(void);

void notifications_notify(uint16_t notification_id, uint16_t address, uint16_t timeoutt, uint32_t vibration_pattern);

void notifications_extend(uint16_t notification_id, uint16_t timeout);
	
void notifications_stop(uint16_t notification_id);

void notifications_invoke_function(uint8_t function_id);

#endif /* NOTIFICATIONS_H */

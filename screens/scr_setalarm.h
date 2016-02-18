#ifndef SCR_SETALARM_H
#define SCR_SETALARM_H

#include <stdbool.h>
#include <stdint.h>

#define DAY_IN_SECONDS 24*60*60

bool scr_set_alarm_handle_event(uint32_t event_type, uint32_t event_param);
void alarm_clock_init(void);

#endif /* SCR_SETALARM_H */

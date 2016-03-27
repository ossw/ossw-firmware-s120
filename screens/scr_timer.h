#ifndef SCR_TIMER_H
#define SCR_TIMER_H

#include <stdbool.h>
#include <stdint.h>

bool scr_timer_handle_event(uint32_t event_type, uint32_t event_param);
void timer_feature_init(void);

#endif /* SCR_TIMER_H */

#ifndef RTC_H
#define RTC_H

#include <stdbool.h>
#include <stdint.h>
#include "app_timer.h"

#define RTC_INTERVAL       APP_TIMER_TICKS(1000, 0)

void rtc_timer_init(void);

uint32_t rtc_current_time(void);

void rtc_set_current_time(uint32_t time);

#endif /* RTC_H */

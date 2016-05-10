#ifndef RTC_H
#define RTC_H

#include <stdbool.h>
#include <stdint.h>
#include "app_timer.h"
#include "board.h"

#define RTC_INTERVAL_SECOND					1
#define RTC_INTERVAL_MINUTE         60

void rtc_timer_init(void);
uint16_t rtc_get_refresh_interval(void);
void rtc_set_refresh_interval(uint16_t new_interval);
void rtc_toggle_refresh_interval(void);

uint32_t rtc_get_current_time(void);
void rtc_set_current_time(uint32_t);
bool rtc_should_store_current_time(void);
void rtc_store_current_time(void);

uint32_t rtc_get_current_time_in_seconds(void);
uint32_t rtc_get_current_hour_24(void);
uint32_t rtc_get_current_hour_12(void);
uint32_t rtc_get_current_hour_12_designator(void);
uint32_t rtc_get_current_minutes(void);
uint32_t rtc_get_current_seconds(void);
uint32_t rtc_get_current_day_of_week(void);
uint32_t rtc_get_current_day_of_month(void);
uint32_t rtc_get_current_day_of_year(void);
uint32_t rtc_get_current_month(void);
uint32_t rtc_get_current_year(void);

#endif /* RTC_H */

#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

#include <stdbool.h>
#include <stdint.h>

#define DATA_SOURCE_TIME_HOUR 0
#define DATA_SOURCE_TIME_MINUTES 1
#define DATA_SOURCE_TIME_SECONDS 2
#define DATA_SOURCE_DATE_DAY_OF_MONTH 3
#define DATA_SOURCE_DATE_MONTH 4
#define DATA_SOURCE_DATE_YEAR 5
#define DATA_SOURCE_BATTERY_LEVEL 6

#define DATA_SOURCE_SENSOR_HR 0x10
#define DATA_SOURCE_SENSOR_CYCLING_SPEED 0x11
#define DATA_SOURCE_SENSOR_CYCLING_CADENCE 0x12

void data_source_init(void);

uint16_t data_source_get_value(uint16_t data_source_id);

#endif /* DATA_SOURCE_H */

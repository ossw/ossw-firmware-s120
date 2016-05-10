#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include <stdint.h>
#include "app_timer.h"

#define BUTTON_DETECTION_DELAY           APP_TIMER_TICKS(5, APP_TIMER_PRESCALER)
#define BUTTON_LONG_PRESS_DELAY          APP_TIMER_TICKS(450, APP_TIMER_PRESCALER)

void buttons_init(void);

#endif /* BUTTONS_H */

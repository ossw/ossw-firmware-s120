#ifndef SCR_TEST_H
#define SCR_TEST_H

#include <stdbool.h>
#include <stdint.h>


#define WATCH_SET_SECTION_SCREENS 1
#define WATCH_SET_SECTION_STATIC_CONTENT 2

#define WATCH_SET_SCREEN_SECTION_CONTROLS 1
#define WATCH_SET_SCREEN_SECTION_ACTIONS 2

#define WATCH_SET_END_OF_DATA 0xFF

void scr_test_handle_event(uint32_t event_type, uint32_t event_param);

#endif /* SCR_TEST_H */

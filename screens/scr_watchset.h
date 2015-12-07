#ifndef SCR_WATCH_SET_H
#define SCR_WATCH_SET_H

#include <stdbool.h>
#include <stdint.h>
#include "../watchset.h"

void scr_watch_set_invoke_external_function(uint8_t function_id);

bool scr_watch_set_handle_event(uint32_t event_type, uint32_t event_param);

void set_external_property_data(uint8_t property_id, uint8_t* data_ptr, uint8_t size);
	
#endif /* SCR_WATCH_SET_H */

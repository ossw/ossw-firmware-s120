#ifndef WATCHSET_FUNCTIONS_H
#define WATCHSET_FUNCTIONS_H

#include <stdbool.h>
#include <stdint.h>

#define WATCHSET_FUNC_TOGGLE_BACKLIGHT 0

#define WATCHSET_FUNC_CHANGE_SCREEN 0xF0
#define WATCHSET_FUNC_SHOW_SETTINGS 0xF1

#define WATCHSET_FUNC_EXTENSION 0xFF

void internal_function_invoke(uint8_t internal_function_id, uint16_t);


#endif /* WATCHSET_FUNCTIONS_H */

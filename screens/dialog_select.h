#ifndef DIALOG_SELECT_H
#define DIALOG_SELECT_H

#include <stdbool.h>
#include <stdint.h>

#define CANCEL					0xFF

bool dialog_select_handle_event(uint32_t event_type, uint32_t event_param);
void pack_dialog_select(uint8_t init, void (*d_callback)(uint8_t), uint8_t font, const char *title, uint8_t list_size, const char *list);
#endif /* DIALOG_SELECT_H */

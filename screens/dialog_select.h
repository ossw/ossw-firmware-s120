#ifndef DIALOG_SELECT_H
#define DIALOG_SELECT_H

#include <stdbool.h>
#include <stdint.h>

#define CANCEL					0xFF
#define SELECT_RADIO		0x01
#define SELECT_CHECK		0x02
#define SELECT_STRIKE		0x03

void dialog_select_init(void (*d_callback)(uint8_t));
bool dialog_select_handle_event(uint32_t event_type, uint32_t event_param);
void pack_dialog_select(uint8_t init, void (*d_callback)(uint8_t), uint8_t font, uint8_t st, const char *title, uint8_t list_size, const char *list);

#endif /* DIALOG_SELECT_H */

#ifndef DIALOG_OPTION_TEXT_H
#define DIALOG_OPTION_TEXT_H

#include <stdbool.h>
#include <stdint.h>

bool dialog_option_text_handle_event(uint32_t event_type, uint32_t event_param);
void pack_dialog_option(bool (*dialog_callback)(uint32_t), uint8_t font,
		const char * title, const char *op1, const char *op2, const char *op3, const char *op4);

#endif /* DIALOG_OPTION_TEXT_H */

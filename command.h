#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>
#include <stdint.h>

void command_process(void);

void command_reset_data(void);

void command_append_data(uint8_t *data, uint8_t size);

void command_data_complete(void);

bool command_is_data_handled(void);

#endif /* COMMAND_H */

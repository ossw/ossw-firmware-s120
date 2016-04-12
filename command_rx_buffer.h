#ifndef COMMAND_RX_BUFFER_H
#define COMMAND_RX_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

void command_rx_buffer_process(void);

void command_rx_buffer_reset(void);

void command_rx_buffer_append(uint8_t *data, uint8_t size);

void command_rx_buffer_commit(void (*)(uint8_t));

#endif /* COMMAND_RX_BUFFER_H */

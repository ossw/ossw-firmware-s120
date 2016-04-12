#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>
#include <stdint.h>

void command_receive(uint8_t *rx_data, uint8_t rx_size, void (*handler)(uint8_t));

void command_send(uint8_t *tx_data, uint8_t tx_size, void (*handler)(uint8_t));

#endif /* COMMAND_H */

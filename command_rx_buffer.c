#include "command_rx_buffer.h"
#include "command.h"

static uint32_t data_ptr = 0;
static uint8_t data_buf[256];
static bool handle_data = false;
static void (*resp_handler)(uint8_t);

void command_rx_buffer_process() {
		if (!handle_data) {
				return;
		}
		command_receive(data_buf, data_ptr, resp_handler);
		handle_data = false;
}

void command_rx_buffer_reset() {
		data_ptr=0;
}

void command_rx_buffer_append(uint8_t *data, uint8_t size) {
		memcpy(data_buf+data_ptr, data, size);
		data_ptr+=size;
}

void command_rx_buffer_commit(void (*handler)(uint8_t)) {
		resp_handler = handler;
		handle_data = true;
}

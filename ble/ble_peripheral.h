#ifndef BLE_PERIPHERAL_H
#define BLE_PERIPHERAL_H

#include <stdint.h>

#define OSSW_RX_COMMAND_INVOKE_EXTERNAL_FUNCTION 0x10

void ble_peripheral_mode_init(void);

void ble_peripheral_invoke_external_function(uint8_t function_id);

void ble_peripheral_set_watch_set_id(uint32_t watch_set_id);
 
#endif

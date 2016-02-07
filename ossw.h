#ifndef OSSW_H
#define OSSW_H

#include <stdbool.h>
#include <stdint.h>

#define FIRMWARE_VERSION                 "0.5.0-s120-snapshot"                           /* Firmware version. */

void ossw_main(void);
const char* ossw_firmware_version(void);
const char* ossw_mac_address(void);
	
#endif /* OSSW_H */

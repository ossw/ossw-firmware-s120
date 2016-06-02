#ifndef OSSW_H
#define OSSW_H

#include <stdbool.h>
#include <stdint.h>

#define FIRMWARE_VERSION     "20160601-vaspa" /* Firmware version. */

const char* ossw_firmware_version(void);
const char* ossw_mac_address(void);
	
#endif /* OSSW_H */

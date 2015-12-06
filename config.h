#ifndef CONFIGUTARION_H
#define CONFIGUTARION_H

#include <stdbool.h>
#include <stdint.h>
#include "fs.h"

void config_init(void);

void config_clear_dafault_watch_face(void);

void config_set_default_watch_face(char* file_name);

spiffs_file config_get_dafault_watch_face_fd(void);
	
#endif /* CONFIGUTARION_H */

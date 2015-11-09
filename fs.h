#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stdint.h>
#include "spiffs/spiffs.h"

#define FS_LOG_PAGE_SIZE       256
  
extern spiffs fs;
	
void fs_format(void);

int fs_mount(void);
	
void fs_unmount(void);

void fs_reformat(void);

void fs_init(void);

#endif /* FS_H */

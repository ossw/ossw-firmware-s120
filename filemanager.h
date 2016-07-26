#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <stdint.h>


uint8_t list_directory(void);
uint8_t create_file(void* name_ptr);
uint8_t append_file(uint8_t* data_buf, uint32_t data_ptr);
uint8_t close_file();

#endif /* FILEMANAGER_H */
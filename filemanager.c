 #include "filemanager.h"
 #include "spiffs/spiffs.h"
 #include "fs.h"
 
 static spiffs_file data_upload_fd;
 
 uint8_t list_directory(void) {

 spiffs_DIR d;
  struct spiffs_dirent e;
  struct spiffs_dirent *pe = &e;
  SPIFFS_opendir(&fs, "/", &d);
  while ((pe = SPIFFS_readdir(&d, pe))) {
    printf("%s [%04x] size:%i\n", pe->name, pe->obj_id, pe->size);
  }
  SPIFFS_closedir(&d);
	return 0;
}
 
uint8_t create_file(void* name_ptr){
	
	data_upload_fd = SPIFFS_open(&fs, name_ptr, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
	if (data_upload_fd < 0) {
	// file open error
		return (SPIFFS_errno(&fs)*-1)-9999;
	}
	return 0;
}
uint8_t append_file(uint8_t* data_buf, uint32_t data_ptr){
	if (SPIFFS_write(&fs, data_upload_fd, data_buf, data_ptr) < 0 ) {
		//file write error
		return (SPIFFS_errno(&fs)*-1)-9999;
	}
	return 0;
}

uint8_t close_file(){
	if (SPIFFS_close(&fs, data_upload_fd) < 0) {
		// file close error
		return (SPIFFS_errno(&fs)*-1)-9999;
	}
	return 0;
}

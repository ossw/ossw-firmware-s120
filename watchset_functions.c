#include "watchset_functions.h"
#include "string.h"
#include "mlcd.h"

void watchset_functions_invoke(uint8_t internal_function_id, uint16_t param) {
	  switch(internal_function_id) {
			  case WATCHSET_FUNC_TOGGLE_BACKLIGHT:
				    mlcd_backlight_toggle();
			  break;
			  //case WATCHSET_FUNC_EXTERNAL:
				
			 // break;
		}
}

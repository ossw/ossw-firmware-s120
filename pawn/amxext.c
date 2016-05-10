#include <assert.h>

#include "amxext.h"
#include "amx.h"
#include "nrf_delay.h"

#include "../mlcd.h"

static cell AMX_NATIVE_CALL n_lcd_bl(AMX* amx, const cell params[])
{
	  if (params[1] == 0) {
	      mlcd_backlight_off();
		} else {
		  	mlcd_backlight_long();
		}
    return(0);
}

static cell AMX_NATIVE_CALL n_wait_ms(AMX *amx, const cell *params)
{
    nrf_delay_ms((int)params[1]);
    return 0;
}

const AMX_NATIVE_INFO ext_Natives[] = {
  { "lcd_bl",       n_lcd_bl },
  { "wait_ms",      n_wait_ms },
  { NULL, NULL }        /* terminator */
};

int AMXEXPORT AMXAPI amxext_init(AMX *amx)
{
  return amx_Register(amx, ext_Natives, -1);
}

int AMXEXPORT AMXAPI amxext_cleanup(AMX *amx)
{
  return AMX_ERR_NONE;
}

#ifndef TARGET_H
#define TARGET_H

#if defined(TARGET_WELOOP)
  #include "target/weloop/weloop.h"
#elif defined(TARGET_SMAQ_EFM32)
  #include "target/smaq/smaq_efm32.h"
#elif defined(TARGET_SMAQ_NRF51)
  #include "target/smaq/smaq_nrf51.h"
#else
	#error "Target is not defined"
#endif

#endif // TARGET_H

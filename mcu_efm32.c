#include "mcu.h"
#include "sleep.h"
#include "em_timer.h"

void mcu_delay_ms(uint32_t ms) {
		for (int i=0; i<ms; i++) {
				mcu_delay_us(1000);
		}
}

void mcu_delay_us(uint32_t us) {
		
  /* adjustment factor for 14MHz oscillator, based on the timing of this whole function with speed optimization on, could probably be done in a prettier way. */
  uint16_t cycle_delay = us * 14 - 28;

  /* Reset Timer */
  TIMER_CounterSet(TIMER0, 0);

  /* Start TIMER0 */
  TIMER0->CMD = TIMER_CMD_START;

  /* Wait until counter value is over top */
  while(TIMER0->CNT < cycle_delay){
  /* Do nothing, just wait */
  }
}

void mcu_power_manage(void) {
		SLEEP_Sleep();
}

void mcu_reset(void) {
		//NVIC_SystemReset
}

#include "timer.h"
#include <string.h>
#include <stdlib.h>
#include "rtcdriver.h"

typedef uint32_t timer_id_t;
typedef uint32_t timer_type_t;

typedef struct {
		RTCDRV_TimerID_t id;
		RTCDRV_TimerType_t type;
		RTCDRV_Callback_t  cb;
} timer_definition;

void timer_init() {
		//RTCDRV_Init();
}

void timer_create(timer_id_t* id, timer_type_t type, void* handler) {
    timer_definition* def = (timer_definition*)malloc(sizeof(timer_definition));
		Ecode_t code = RTCDRV_AllocateTimer(&def->id);
		def->type = (RTCDRV_TimerType_t)type;
		def->cb = (RTCDRV_Callback_t)handler;
		*id = (timer_id_t)def;
}

void timer_start(timer_id_t id, uint32_t timeout) {
		timer_start_with_param(id, timeout, NULL);
}

void timer_start_with_param(timer_id_t id, uint32_t timeout, void* param) {
		timer_definition* def = (timer_definition*)id;
		RTCDRV_StartTimer(def->id, def->type, timeout, def->cb, param);	
}
	
void timer_stop(timer_id_t id) {
		timer_definition* def = (timer_definition*)id;
		RTCDRV_StopTimer(def->id);
}

void timer_cnt_get(uint32_t* result) {
		*result = 0;
}

void timer_cnt_ms_diff_compute(uint32_t current, uint32_t last, uint32_t* result) {
		*result = 0;
}


#ifndef PEDOMETER_AC_H
#define PEDOMETER_AC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "fastlist.h"
	
	
int pedometerACGetLastDataBuffer(uint8_t * buf, int size);
int pedometerACGetSteps(void);
void pedometerACInitTimer(void);
void pedometerACProcess(void);
	
#ifdef QTAPP
void pedometerACInit(void);
void pedometerACAnalyseSteps(int x, int y, int z);
#endif
	
#ifdef __cplusplus
}
#endif

#endif // PEDOMETER_AC_H

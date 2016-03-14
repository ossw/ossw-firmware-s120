
#include "pedometer_ac.h"
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "mlcd.h"
#include "mlcd_draw.h"

#ifndef QTAPP
#include "app_timer.h"
#include "board.h"
#include "accel.h"
#else
#include "stdbool.h"
#endif

#define FASTLIST_ITEM_INT
	
#include "fastlist.h"

#define PEDO_INTERVAL                      APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)

#ifndef _WIN32
static app_timer_id_t      m_pedo_timer_id;
#endif

static FastListHandler listExtrems = 0;
static FastListHandler listTimes = 0;
static FastListHandler listAvgFreq = 0;
static FastListHandler list = 0;


static uint32_t totalFrames = 0;
static int16_t frIdx = 0;
static int16_t fr[3] = {0,0,0};
static uint32_t steps = 0;
static int8_t direction = 0;
static int16_t asum = 0;
//int lastFreq[2];
static uint32_t startFreq = 0;
static double lastSample = 0;
static uint32_t lastProcessedFrame = 0;
static uint16_t lSteps = 0;
static int8_t lastX = 0,lastY = 0,lastZ = 0;
static bool aCTimerHandler = 0;

#define AVERAGE_SAMPLES 4
#define FREQ_SCOPE 6
#define MIN_FREQ_SCOPE 3
#define MAX_FREQ_SCOPE 25
#define AMPL_SCOPE 8
#define MAX_DIFF_AMPL 20


#ifndef _WIN32
static void pedometerACInit(void);
static void pedometerACAnalyseSteps(int x, int y, int z);

static void pedometer_timeout_handler(void * p_context) {
	  accel_get_values(&lastX,&lastY,&lastZ);
		aCTimerHandler = true;
}

/*
static void drawPedometerDebug(int sum1, int sum2, int sum3, int freq1, int freq2, double avgScope) {
		char ssss[100];
		sprintf(ssss, "S: [%d %d %d]", sum1, sum2, sum3);
		mlcd_clear_rect(2,30, MLCD_XRES-4, 22);
		mlcd_draw_text(ssss, 2, 30, MLCD_XRES-4, MLCD_XRES-4, FONT_NORMAL_BOLD, HORIZONTAL_ALIGN_CENTER | MULTILINE);
		sprintf(ssss, "F: [%d %d]", freq1, freq2);
		mlcd_clear_rect(2,60, MLCD_XRES-4, 22);
		mlcd_draw_text(ssss, 2, 60, MLCD_XRES-4, MLCD_XRES-4, FONT_NORMAL_BOLD, HORIZONTAL_ALIGN_CENTER | MULTILINE);
		sprintf(ssss, "A: [%1.2f]", avgScope);
		mlcd_clear_rect(2, 90, MLCD_XRES-4, 22);
		mlcd_draw_text(ssss, 2, 90, MLCD_XRES-4, MLCD_XRES-4, FONT_NORMAL_BOLD, HORIZONTAL_ALIGN_CENTER | MULTILINE);
		sprintf(ssss, "STEPS: [%d]", steps);
		mlcd_clear_rect(2,110, MLCD_XRES-4, 22);
		mlcd_draw_text(ssss, 2, 110, MLCD_XRES-4, MLCD_XRES-4, FONT_NORMAL_BOLD, HORIZONTAL_ALIGN_CENTER | MULTILINE);
		mlcd_fb_flush();
}
*/
void pedometerACInitTimer() {
    pedometerACInit();
    uint32_t err_code;
    err_code = app_timer_create(&m_pedo_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                pedometer_timeout_handler);
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_start(m_pedo_timer_id, PEDO_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}
#endif

static void pedometerACInit(void) {
    listExtrems = FastListCreate();
    listTimes = FastListCreate();
    listAvgFreq = FastListCreate();
    list = FastListCreate();
    totalFrames = 0;
    memset(fr, 0, sizeof(fr));
    frIdx = 0;
    steps = 0;
    direction = 0;
    asum = 0;
    startFreq = 0;
    lastSample = 0.0;
		lastProcessedFrame = 0;
		lSteps = 0;
		lastX = 0;
		lastY = 0;
		lastZ = 0;
		aCTimerHandler = false;
}

static void pedometerAnalise(FastListHandler listExtrems, FastListHandler listTimes) {
    if (fastListSize(listExtrems) >= 3) {
        double avgScope = 0.0;
        int sum1 = fastListGet(listExtrems, 0);
        int sum2 = fastListGet(listExtrems, 1);
        int sum3 = fastListGet(listExtrems, 2);
        int freq1 = fastListGet(listTimes, 1) - fastListGet(listTimes, 0);
        int freq2 = fastListGet(listTimes, 2) - fastListGet(listTimes, 1);
        
        fastListPushBack(listAvgFreq, freq1);
        fastListPushBack(listAvgFreq, freq2);
        
        avgScope = fastListAvg(listAvgFreq);
        
        if (fastListSize(listAvgFreq) >= 6) {
            fastListPopFront(listAvgFreq);
            fastListPopFront(listAvgFreq);
        }
        //drawPedometerDebug(sum1, sum2, sum3, freq1, freq2, avgScope);
        bool amplCorrect = sum1 >= AMPL_SCOPE && sum2 >= AMPL_SCOPE && sum3 >= AMPL_SCOPE;
        
        if (amplCorrect) {
            bool amplDiffCorrect = abs(sum2 - sum1) < MAX_DIFF_AMPL && abs(sum3 - sum2) < MAX_DIFF_AMPL && abs(sum3 - sum1) < MAX_DIFF_AMPL;
            bool freqCorrect = freq1 >= avgScope - FREQ_SCOPE && freq1 <= avgScope + FREQ_SCOPE &&
										freq2 >= avgScope - FREQ_SCOPE && freq2 <= avgScope + FREQ_SCOPE &&							
                    freq1 >= MIN_FREQ_SCOPE && freq1 <= MAX_FREQ_SCOPE &&
                    freq2 >= MIN_FREQ_SCOPE && freq2 <= MAX_FREQ_SCOPE;
            
            if (freqCorrect && amplDiffCorrect) {
								steps++;
								lSteps++;
            } 
        } 
        
        fastListPopFront(listExtrems);
        fastListPopFront(listTimes);
        
    }
}



static void pedometerACAnalyseSteps(int x, int y, int z) {
    double sample = 0.0;
    double highPass = 0.0;
    totalFrames++;
		//acc vector
    sample = sqrt((double)(x*x) + (double)(y*y) + (double)(z*z));
    //highPass filter;
    highPass = lastSample + 0.40 *(sample - lastSample);
    
    lastSample = sample;
	
    fastListPushBack(list, (int)(highPass + 0.5));
    
    if (fastListSize(list) >= AVERAGE_SAMPLES) {
			
        fr[frIdx++] = (short)fastListAvg(list);
			
				fastListPopFront(list);
			
        if (frIdx == 3) {
            frIdx = 2;
            
            if (fr[0] < fr[1] && fr[1] < fr[2]) {
                if (direction == -1) {
                    startFreq = totalFrames;
                }
                direction = 1;
            }
            if (fr[0] < fr[1] && direction == 1) {
                asum += fr[1] - fr[0];
            }
            if (fr[0] > fr[1] && fr[1] > fr[2]) {
                if (direction == 1) {
                    fastListPushBack(listExtrems, asum);
                    fastListPushBack(listTimes, totalFrames);
                    pedometerAnalise(listExtrems, listTimes);
                    asum = 0;
										lastProcessedFrame = totalFrames;
                }
                direction = -1;
            }
            fr[0] = fr[1];
            fr[1] = fr[2];

        }
				if (fastListSize(listExtrems) > 100) {
						fastListPopFront(listExtrems);
				}
				if (fastListSize(listTimes) > 100) {
						fastListPopFront(listTimes);
				}
				if (totalFrames - lastProcessedFrame > 60) {
						fastListClear(listExtrems);
						fastListClear(listTimes);
						lastProcessedFrame = totalFrames;
						if (lSteps >= 3) {
							steps += 1;
						}
						lSteps = 0;
				}
    }
}

void pedometerACProcess() {
		if (!aCTimerHandler) {
				return;
		}
    pedometerACAnalyseSteps(lastX, lastY, lastZ);
		aCTimerHandler = false;
}

int pedometerACGetSteps(void) {
    return steps;
}


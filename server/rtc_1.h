#ifndef __RTC_H
#define __RTC_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "cmsis_os2.h"

/* Defines related to Clock configuration */
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */
#define ENCENDERTIMALARM 0x00440

extern osThreadId_t TID_RTC;

void RTC_CalendarShow(char *showtime, char *showdate);
void Thread_RTC (void *argument);
void RTC_init(void);
void RTC_CalendarConfig(void);
int Init_Timers_Alarma (void);
void RTC_Alarm(void);





#endif

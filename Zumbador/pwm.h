#ifndef _PWM_H
#define _PWM_H

#include "cmsis_os2.h"  
#include "stm32f4xx_hal.h"
#include "defines.h"


void initModZumbador(void);
void Init_Th_Zumbador_Test(void);

osThreadId_t getModPWMThreadID(void);

#endif

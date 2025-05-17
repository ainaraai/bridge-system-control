#ifndef __TEMP_H
#define __TEMP_H

#include "cmsis_os2.h" 
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
#include "principal.h"
#include "Driver_I2C.h"

#define SIZE_MSGQUEUE_TEMP			3

typedef struct {                               
  float temp;
} MSGQUEUE_TEMP_t;


void initModTemp(void);

void Init_Th_Temp_Test(void);

osThreadId_t getModTempID(void);


#endif

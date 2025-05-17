#ifndef __THSERVO_H
#define __THSERVO_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "principal.h"

#define CLOSE_FLAG    0x01
#define OPEN_FLAG     0x02
#define STOP_FLAG			0x04
#define START_FLAG		0x08


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

osThreadId_t getModServoThreadID(void);
void InitModServo(void);
osThreadId_t getModServoSTOPThreadID(void);


  /* Exported thread functions,  
  Example: extern void app_main (void *arg); */
#endif /* __MAIN_H */

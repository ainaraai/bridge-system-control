#ifndef __SLEEP_MODE_H
#define __SLEEP_MODE_H

#define EXIT_SLEEP_MODE_FLAG 0x00000001U

#include "cmsis_os2.h"
void init_usr_button(void);
void sleep_mode(void);
int Init_Th_sleep_mode(void);
osThreadId_t getSleepModeThreadID(void);

#endif

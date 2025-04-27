#ifndef __TECLADO_H
#define __TECLADO_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include <stdio.h>

#define ROW1_PIN GPIO_PIN_2
#define ROW1_PORT GPIOD

#define ROW2_PIN GPIO_PIN_2
#define ROW2_PORT GPIOG

#define ROW3_PIN GPIO_PIN_3
#define ROW3_PORT GPIOG

#define ROW4_PIN GPIO_PIN_1
#define ROW4_PORT GPIOG

#define COL1_PIN GPIO_PIN_9
#define COL1_PORT GPIOC

#define COL2_PIN GPIO_PIN_10
#define COL2_PORT GPIOC

#define COL3_PIN GPIO_PIN_12
#define COL3_PORT GPIOC

#define COL4_PIN GPIO_PIN_11
#define COL4_PORT GPIOC

#define MAX_DIGITOS 4

void teclado_init(void);
char teclado_read(void);
int Init_ThreadTeclado (void);
void ThreadTeclado (void *argument);


#endif

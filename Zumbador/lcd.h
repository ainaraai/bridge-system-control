#ifndef __LCD_H
#define __LCD_H

#include "cmsis_os2.h" 
#include "stm32f4xx_hal.h"
#include "Driver_SPI.h"
#include <stdio.h>

#define SIZE_MSGQUEUE_LCD			3

typedef struct {                               
  uint8_t nLin;
	char buf[24];
} MSGQUEUE_LCD_t;


void initModLCD(void);
void LCD_reset(void);
void LCD_init(void);
void LCD_LineaToLocalBuffer(char linea[30], uint8_t nLin);
void LCD_update_L1(void);
void LCD_update_L2(void);

osMessageQueueId_t getModLCDQueueID(void);


void Init_Th_LCD_Test(void);


#endif


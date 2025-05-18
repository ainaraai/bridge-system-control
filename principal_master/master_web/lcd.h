#ifndef __THLCD_H
#define __THLCD_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdio.h"
#define SIZE_MSGQUEUE_LCD			3
void LCD_start(void);
void LCD_clean(void);
void LCD_write(uint8_t line, char a[]);


void initModLCD(void);
void LCD_reset(void);
void LCD_init(void);
void LCD_LineaToLocalBuffer(char linea[30], uint8_t nLin);
void LCD_update_L1(void);
void LCD_update_L2(void);

osMessageQueueId_t getModLCDQueueID(void);


void Init_Th_LCD_Test(void);

void initModLCD(void);

void LCD_clear(void);
void LCD_clear_L2(void);

typedef struct {                               
  uint8_t nLin;
	char buf[24];
} MSGQUEUE_LCD_t;

#endif

#ifndef __THPRIN_H
#define __THPRIN_H

#include "Driver_SPI.h"

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file

//FLAGS
#define TRANSFER_COMPLETE 				0x01 
#define SLAVEIRQ 				0x01
#define EMERGENCY       0x02
#define SUBIR           0x04
#define RESUME          0x08
#define BAJAR           0x10

//RFID 0x10


//MODULES
#include "temp.h"
#include "servomotor.h"
#include "sleep_mode.h"
//#include "Teclado.h"
//#include "lcd.h"

//ESTADOS PRINCIPAL
#define COMPRUEBA 0x01
#define UP 0x02
#define SPI 0x03
#define MANUAL 0x20




typedef struct {
    uint8_t type;       // message type (from enum above)
   // uint8_t data[10];   // optional payload (e.g., flags, status)
} SPI_Message;

typedef enum {
    EVENT_IR_LOST        = 0x01,
    EVENT_IR_RESTORED    = 0x02,
	  EVENT_TEMP_HIGH      = 0x03,
    CMD_ACTIVATE_SERVO   = 0x10,
    CMD_RESUME_ALL       = 0x11,
    CMD_CLOSE_SERVO      = 0x12,
    CMD_STOP_ALL         = 0x13,
    CMD_ENTER_SLEEP_MODE = 0x20,
    CMD_EXIT_SLEEP_MODE  = 0x21
	  
} SPI_MessageType;

void IR_Signal_Lost_Handler(void);
void IR_Signal_Restored_Handler(void);

void Init_Thread_principal(void);
osThreadId_t getThIDPrinAccesoManual(void);
osThreadId_t getThIDSlave(void);
void High_temp(void);
void sleep_mode_wakeUp(void);

#endif

#ifndef __THPRIN_H
#define __THPRIN_H

#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file

//FLAGS
#define SLAVEIRQ 				0x01
#define EMERGENCY       0x02
#define SUBIR           0x04
#define RESUME          0x08
#define BAJAR           0x10

//RFID 0x10


//MODULES
#include "flash.h"
#include "rfid.h"
#include "Teclado.h"
#include "lcd.h"


//ESTADOS PRINCIPAL
#define COMPRUEBA 0x01
#define UP 0x02
#define SPI 0x03
#define MANUAL 0x20






typedef struct {                               
	uint8_t password_user[4];
	uint8_t id[5];
	char hexStringLocal[19];
	uint8_t estado[5];
} User_manag_t;



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
    CMD_STOP_ALL         = 0x13
} SPI_MessageType;

uint8_t Boat(void);
User_manag_t user_status(void);
//bool authentication_user(const char *input_name, const uint8_t input_password[4]);
void Init_Thread_principal(void);
osThreadId_t getThIDPrinAccesoManual(void);
osThreadId_t getThIDPrinc1(void);
osThreadId_t getThIDPrinWeb(void);
osMessageQueueId_t getMessageWebID(void);
#endif

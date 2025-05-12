#ifndef __THFLASH_H
#define __THFLASH_H

/* Includes ------------------------------------------------------------------*/
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "string.h"
#include "principal.h"


//FLAGS
#define TRANSFER_COMPLETE 				0x01              //internal flag used by SPI to know that a single transfer has been completed
#define DELETE_USER								0x02
#define GET_ALL_USERS							0x04
#define ADD_USER                  0x08
#define ADD_EVENT                 0x10
#define DELETE_EVENTS             0x20
#define GET_ALL_EVENTS            0x40
#define GET_USER									0x80
#define DONE  										0x01
#define FLAG_POWER_UP             0x100
#define FLAG_POWER_DOWN           0x200



typedef struct {
	uint8_t numUsers;
  uint8_t id[5];
  char name[19];
  uint8_t password[4];
	uint8_t found;

} MSGQUEUE_USER_t;

typedef struct {
	uint8_t numEvents;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  char description[19];
} MSGQUEUE_EVENT_t;

typedef struct {
	uint8_t password[4];
} MSGQUEUE_GEN_PASS_t;
void add_user1(char nombre[],uint8_t password[], uint8_t id0,uint8_t id1,uint8_t id2,uint8_t id3,uint8_t id4);

void Init_ThFlash (void);
osThreadId_t getModFlashThreadID(void);
osMessageQueueId_t getMsgRXFlashID(void );
osMessageQueueId_t getMsgTXFlashID(void);
osMessageQueueId_t getMsgFlashID2(void);

void add_event1(void);
#endif /* __MAIN_H */

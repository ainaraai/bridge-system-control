#include "principal.h"
#include "flash.h"
#include "rfid.h"
#include "stdio.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Principal
 *---------------------------------------------------------------------------*/
 
 #define OPERACION 				0x04

bool mensaje_Recibido = false;
  //thread
osThreadId_t tid_Thread_principal;                        // thread id
void test_flash_users (void *argument);                   // thread function
void test_flash_events (void*argument);
void test_generic_password (void *argument);

//colas
static osMessageQueueId_t rfid_queue;
extern osThreadId_t tid_flash;
//mensajes
static MSGQUEUE_RC522_t msg_rfid;
static MSGQUEUE_USER_t user;
static MSGQUEUE_USER_t m_user;

static MSGQUEUE_EVENT_t event;
static MSGQUEUE_GEN_PASS_t generic_password;

int Init_Thread_principal (void) {
   const static osThreadAttr_t th_attr = {.stack_size = 7000};
  tid_Thread_principal = osThreadNew(test_flash_users, NULL, &th_attr);
  if (tid_Thread_principal == NULL) {
    return(-1);
  }
	return 0;
}

void test_flash_users (void *argument) {
	uint32_t flags;
	uint8_t id[5];
	uint8_t password[4];
  //rfid_queue = Init_ThMFRC522();
   Init_ThFlash();
	 osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);

  while (1) {
		//add 3 different users
		memcpy(password, (uint8_t[]){0x1, 0x3, 0x6, 0x7}, 4);
    
		add_user1("ainara",password,0xD5,0xB2,0x02,0xF8,0x99); 
    osThreadFlagsWait(ADD_USER, osFlagsWaitAny, osWaitForever);
		id[0] = 0xD6;
		id[1] = 0xB2;
		id[2] = 0x02;
		id[3] = 0xF8;
		id[4] = 0x99;
		memcpy(user.id, id, 5);
		osMessageQueuePut(getMsgFlashID(), &user, NULL, 0U);
		osThreadFlagsSet(tid_flash, GET_USER);
		osThreadFlagsWait(GET_USER, osFlagsWaitAny, osWaitForever);
		osMessageQueueGet(getMsgFlashID(), &m_user, NULL, osWaitForever); //amaya
    //we add user 1
		add_user2();
		osThreadFlagsWait(ADD_USER, osFlagsWaitAny, osWaitForever);

		id[0] = 0x33;
	  id[1] = 0x31;
	  id[2] = 0x0D;
	  id[3] = 0xF8;
	  id[4] = 0xF7;
		memcpy(user.id, id, 5);
		osMessageQueuePut(getMsgFlashID(), &user, NULL, 0U);
		osThreadFlagsSet(tid_flash, GET_USER);
		osThreadFlagsWait(GET_USER, osFlagsWaitAny, osWaitForever);
		osMessageQueueGet(getMsgFlashID(), &m_user, NULL, osWaitForever); //amaya
   	//printf("USER:%s",m_user.id);

//    osThreadFlagsSet(tid_flash, GET_ALL_USERS);
//		osMessageQueueGet(getMsgFlashID(), &user, NULL, osWaitForever); //amaya
//		osMessageQueueGet(getMsgFlashID(), &user, NULL, osWaitForever); //ainara
//    osMessageQueueGet(getMsgFlashID(), &user, NULL, osWaitForever); //lorena
//    osMessageQueueGet(getMsgFlashID(), &user, NULL, osWaitForever); //
//		//we wait
//		osMessageQueueGet(getMsgFlashID(), &user, NULL, osWaitForever);
//		osMessageQueueGet(getMsgFlashID(), &user, NULL, osWaitForever);
//		osMessageQueueGet(getMsgFlashID(), &user, NULL, osWaitForever);
//		osMessageQueueGet(getMsgFlashID(), &user, NULL, osWaitForever);
////    osMessageQueueGet(rfid_queue, &msg_rfid, NULL, osWaitForever);
//    mensaje_Recibido = true;
  }
}

void test_flash_events (void *argument) {
  	uint32_t flags;
  //rfid_queue = Init_ThMFRC522();
  Init_ThFlash();
  while (1) {
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    //add_user1();
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    add_event1();
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    add_event1();
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    osThreadFlagsSet(tid_flash, GET_ALL_EVENTS);
		osMessageQueueGet(getMsgFlashID2(), &event, NULL, osWaitForever);
		osMessageQueueGet(getMsgFlashID2(), &event, NULL, osWaitForever);
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    osThreadFlagsSet(tid_flash, DELETE_EVENTS);
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    add_event1();
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    osMessageQueueGet(getMsgFlashID2(), &event, NULL, osWaitForever);
  }
}


void test_generic_password (void *argument) {
  uint32_t flags;
  Init_ThFlash();
  while (1) {
    osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
		//add_user1(); //natalia
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    osThreadFlagsSet(tid_flash,  FLAG_POWER_DOWN);
    osDelay(2000);
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    add_user2();
    flags = osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
    osThreadFlagsSet(tid_flash,  FLAG_POWER_UP);
    osThreadFlagsWait(READY, osFlagsWaitAny, osWaitForever);
		osThreadFlagsSet(tid_flash, GET_ALL_USERS);
		osMessageQueueGet(getMsgFlashID2(), &user, NULL, osWaitForever); //
    osMessageQueueGet(getMsgFlashID2(), &user, NULL, osWaitForever); //
    osMessageQueueGet(getMsgFlashID2(), &user, NULL, osWaitForever); //
  }
}

#include "main.h"
                           
/* Thread declarations */
extern void Th_LCD (void *arg);
extern void TH_SNTP (void *arg);
extern void Th_bridge (void *arg);
extern void Th_authentication (void *arg);

/* Thread IDs */
osThreadId_t TID_SNTP;
osThreadId_t TID_bridge;
osThreadId_t TID_authentication;

/* Thread attributes */
static osThreadAttr_t bridge_Th_attr;
static osThreadAttr_t SNTP_Th_attr;
static osThreadAttr_t auth_Th_attr;

/* Queues IDs */
osMessageQueueId_t bridge_Q;
osMessageQueueId_t auth_Q;

/* Queues attributes */
static osMessageQueueAttr_t bridge_Q_attr;
static osMessageQueueAttr_t auth_Q_attr;

/* Flags IDs */
osEventFlagsId_t auth_event_id;

/* Flags attributes*/
static osEventFlagsAttr_t auth_Flags_attr; 

extern RTC_HandleTypeDef RtcHandle;
static uint32_t exec2_SNTP;
osTimerId_t tim_id2_SNTP;

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/

static void Timer2_Callback_SNTP (void const *arg) {
	  get_time ();
}


int Init_Timers_SNTP (void) {
  osStatus_t status;                            // function return status
  
 
  // Create periodic timer
  exec2_SNTP = 2U;

  tim_id2_SNTP = osTimerNew((osTimerFunc_t)&Timer2_Callback_SNTP, osTimerPeriodic, &exec2_SNTP, NULL);
  if (tim_id2_SNTP != NULL) {  // Periodic timer created
    // start timer with periodic 1000ms interval
    status = osTimerStart(tim_id2_SNTP, 30000U);    //30 segundos        
    if (status != osOK) {
      return -1;
    }
  }
  return NULL;
}

__NO_RETURN void Th_main (void *arg) {
  (void)arg;
	
	RTC_init();
	if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != 0x32F2)
  {
    /* Configure RTC Calendar */
    RTC_CalendarConfig();
  }
  
  netInitialize();
  osDelay(5000); // Tiempo de espera después de inicialización para la primera sincronización
  
	get_time();
	Init_Timers_SNTP();
//  // SNTP
//	SNTP_Th_attr.name = "Th_SNTP";
//  TID_SNTP = osThreadNew (TH_SNTP,  NULL, &SNTP_Th_attr);
//  
  // Bridge
  bridge_Q_attr.name = "Q_bridge";
  bridge_Q = osMessageQueueNew(10U, sizeof(uint8_t), &bridge_Q_attr);
  bridge_Th_attr.name = "Th_bridge";
  TID_bridge = osThreadNew (Th_bridge, NULL, &bridge_Th_attr);
  
  // Authentication
  auth_Flags_attr.name = "Flags_authentication";
  auth_event_id = osEventFlagsNew(&auth_Flags_attr);
  auth_Q_attr.name = "Q_authentication";
  auth_Q = osMessageQueueNew(10U, sizeof(credentials_t), &auth_Q_attr);
  auth_Th_attr.name = "Th_authentication";
  TID_authentication = osThreadNew (Th_authentication,  NULL, &auth_Th_attr);
  
  osThreadExit();
}

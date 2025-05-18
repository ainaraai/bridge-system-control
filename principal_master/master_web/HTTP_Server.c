#include "main.h"
                           
/* Thread declarations */
//extern void Th_LCD (void *arg);
//extern void TH_SNTP (void *arg);

/* Thread IDs */
//osThreadId_t TID_SNTP;

/* Thread attributes */
//static osThreadAttr_t SNTP_Th_attr;


/* Flags IDs */
//osEventFlagsId_t auth_event_id;

///* Flags attributes*/
//static osEventFlagsAttr_t auth_Flags_attr; 

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
  osDelay(700); // Tiempo de espera después de inicialización para la primera sincronización
  
	get_time();
	Init_Timers_SNTP();
  Init_ModADC();
//  // SNTP
//	SNTP_Th_attr.name = "Th_SNTP";
//  TID_SNTP = osThreadNew (TH_SNTP,  NULL, &SNTP_Th_attr);
//  
  osDelay(100);
 // osThreadExit();
}

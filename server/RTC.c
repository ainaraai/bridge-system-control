#include "rtc.h"

/* RTC handler declaration */
RTC_HandleTypeDef RtcHandle;

RTC_DateTypeDef sdatestructureget;
RTC_TimeTypeDef stimestructureget;
RTC_AlarmTypeDef salarmstructure;


/*RTC init function*/
void RTC_Init(void);
void RTC_CalendarConfig(void);
void RTC_Alarm(void);

/* Buffers used for displaying Time and Date */
char aShowTime[50] = {0};
char aShowDate[50] = {0};

uint8_t nextMin=0;

 osTimerId_t tim_id2_alarma;                            // timer id ALARMA
 uint32_t exec2_alarma;                          // argument for the timer call back function ALARMA


static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

uint8_t UpdatedMin;

uint8_t UpdatedWeekDay;

uint8_t encendido=0;



void RTC_init(void)
{
  /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follows:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */ 
	//__HAL_RCC_RTC_ENABLE();
  RtcHandle.Instance = RTC; 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);
  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}



/**
  * @brief  Configure the current time and date.
  * @param  None
  * @retval None
  */
void RTC_CalendarConfig(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure.Year = 99;
  sdatestructure.Month = RTC_MONTH_DECEMBER;
  sdatestructure.Date = 31;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  
  if(HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error */
   // Error_Handler();
  }

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = 23;
  stimestructure.Minutes = 59;
  stimestructure.Seconds = 40;
  stimestructure.TimeFormat = RTC_HOURFORMAT_24;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
  }

  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x32F2);
}


/**
  * @brief  Display the current time and date.
  * @param  showtime : pointer to buffer
  * @param  showdate : pointer to buffer
  * @retval None
  */
//void RTC_CalendarShow(char *showtime, char *showdate)
//{

//	MSGQUEUE_LCD_t localObject;
//  /* Get the RTC current Time */
//  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
//  /* Get the RTC current Date */
//  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
//  /* Display time Format : hh:mm:ss */
//  sprintf(localObject.buf, "%02d:%02d:%02d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
//	localObject.nLin=1;
//	//localObject.buf=(char *)showtime;
//	osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
////	LCD_LineaToLocalBuffer((char *)showtime, 1);
////	LCD_update_L1();
//  /* Display date Format : mm-dd-yy */
//	if(sdatestructureget.Year==99){
//		sprintf(localObject.buf, "%.2d-%.2d-%.2d", sdatestructureget.Month, sdatestructureget.Date, 1900 + sdatestructureget.Year);
//	}else{
//		sprintf(localObject.buf, "%.2d-%.2d-%.2d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
//	}
//  
//	localObject.nLin=2;
//	osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
////	LCD_LineaToLocalBuffer((char *)showdate, 2);
////	LCD_update_L2();
//}


void RTC_Alarm(void){
	
	HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
	UpdatedMin=stimestructureget.Minutes;
	if(UpdatedMin<59){
		nextMin=UpdatedMin++;
	}else{
		nextMin=0;
	}
	
	HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
	UpdatedWeekDay=sdatestructureget.WeekDay;

	/*##-3- Configure the RTC Alarm peripheral #################################*/	
	salarmstructure.AlarmTime.Minutes = RTC_ByteToBcd2(nextMin);
	salarmstructure.AlarmTime.TimeFormat = RTC_HOURFORMAT_24;
	salarmstructure.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	salarmstructure.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	salarmstructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
	salarmstructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  salarmstructure.AlarmDateWeekDay = 0x01;
	salarmstructure.Alarm = RTC_ALARM_A;
	if(HAL_RTC_SetAlarm_IT(&RtcHandle, &salarmstructure, RTC_FORMAT_BCD) != HAL_OK){
		Error_Handler();
	}


}


 
// Periodic Timer Function
// int cont_alarma =0;
//static void Timer2_Callback_Alarma (void const *arg) {
//  
//  if(encendido){
//    LED_Off(1);
//    encendido=0;
//  }else{
//    LED_On(1);
//    encendido=1;
//  }
//	cont_alarma++;
//	if (cont_alarma >= 20){
//		osTimerStop(tim_id2_alarma);
//		cont_alarma =0;
//		
//	}
//	
//}
// Example: Create and Start timers
//int Init_Timers_Alarma (void) {
//  osStatus_t status;                            // function return status

//  // Create periodic timer
//  exec2_alarma = 2U;
//  tim_id2_alarma = osTimerNew((osTimerFunc_t)&Timer2_Callback_Alarma, osTimerPeriodic, &exec2_alarma, NULL);
//  if (tim_id2_alarma != NULL) {  // Periodic timer created
//		
//    if (status != osOK) {
//      return -1;
//    }
//  }
//  return NULL;
//}

//void RTC_Alarm_IRQHandler(void) {
//  HAL_RTC_AlarmIRQHandler(&RtcHandle);
//}


///**
//  * @brief  Alarm callback
//  * @param  hrtc : RTC handle
//  * @retval None
//  */
//void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){
//  
//  osThreadFlagsSet(TID_RTC,ENCENDERTIMALARM);
//  RTC_Alarm();
//  
//  
//	
////  /* Turn LED1 on: Alarm generation */
////	uint8_t cnt=0;
////	RTC_Alarm();
////	while(cnt!=50){ //since 100ms*50=5000ms=5s
////		LED_SetOut(1);
////		osDelay(5000);
////		cnt++;
////	}
////	
////	
//}

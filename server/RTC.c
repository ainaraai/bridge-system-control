//#include "main.h"
//#include <stdio.h>

///* Defines related to Clock configuration */
//#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock, valor sacado del Reference-Manual */ 
//#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

///* RTC handler declaration */
////RTC_HandleTypeDef RtcHandle;
//RTC_DateTypeDef sdatestructure;
//RTC_TimeTypeDef stimestructure;

//uint8_t first_config = 1;

////void RTC_init(void)
////{
////  /*##-1- Configure the RTC peripheral #######################################*/
////  /* Configure RTC prescaler and RTC data registers */
////  /* RTC configured as follows:
////      - Hour Format    = Format 24
////      - Asynch Prediv  = Value according to source clock
////      - Synch Prediv   = Value according to source clock
////      - OutPut         = Output Disable
////      - OutPutPolarity = High Polarity
////      - OutPutType     = Open Drain */ 
////  RtcHandle.Instance = RTC; 
////  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
////  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
////  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
////  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
////  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
////  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
////  __HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);
////  RtcHandle.Instance->CR |= RTC_CR_BYPSHAD;
////  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
////  {
////    /* Initialization Error */
////    Error_Handler();
////  }
////}

//void RTC_SetDate (uint8_t day, uint8_t month, uint16_t year, uint8_t weekday) 
//{
//	sdatestructure.Date = day;
//  sdatestructure.Month = month;
//  sdatestructure.Year = year-2000;
//  sdatestructure.WeekDay = weekday;

//  if(HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BIN) != HAL_OK)
//  {
//    /* Initialization Error */
//    Error_Handler();
//  }
//}

//void RTC_SetTime(uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t AM_PM, uint8_t DST, uint8_t set)
//{
//  stimestructure.Seconds=Seconds;
//  stimestructure.Minutes=Minutes;
//  stimestructure.Hours=Hours;

//  if(AM_PM==1){ //AM 0 PM 1
//    stimestructure.TimeFormat = RTC_HOURFORMAT12_PM;
//  }
//  else{
//    stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
//  }

//	switch(DST){
//		case 0:
//			stimestructure.DayLightSaving=RTC_DAYLIGHTSAVING_NONE; //no DST
//			break;
//		case 1:
//			stimestructure.DayLightSaving=RTC_DAYLIGHTSAVING_ADD1H; //Add 1h
//			break;
//		case 2:
//			stimestructure.DayLightSaving=RTC_DAYLIGHTSAVING_SUB1H; //Subtract 1h
//			break;
//		default:
//			stimestructure.DayLightSaving=RTC_DAYLIGHTSAVING_NONE; //no DST (default)
//			break; 
//	}

//  if(set==1){ //Store operation set 1, reset 0
//      stimestructure.StoreOperation = RTC_STOREOPERATION_SET;
//  }
//  else{
//      stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
//  }
//  
//  if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN) != HAL_OK) {
//    /* Initialization Error */
//    Error_Handler();
//  }
//}


///**
//  * @brief  Configure the current time and date.
//  * @param  None
//  * @retval None
//  */
////void RTC_CalendarConfig(void)
////{
////  /*##-1- Configure the Date #################################################*/
////  /* Set Date: Tuesday February 18th 2014 */
////  sdatestructure.Year = 0x00;
////  sdatestructure.Month = 0x01;
////  sdatestructure.Date = 0x01;
////  sdatestructure.WeekDay = RTC_WEEKDAY_MONDAY;
////  
////  if(HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BCD) != HAL_OK)
////  {
////    /* Initialization Error */
////    Error_Handler();
////  }

////  /*##-2- Configure the Time #################################################*/
////  /* Set Time: 02:00:00 */
////  stimestructure.Hours = 0x09;
////  stimestructure.Minutes = 0x59;
////  stimestructure.Seconds = 0x50;
////  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
////  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
////  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

////  if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD) != HAL_OK)
////  {
////    /* Initialization Error */
////    Error_Handler();
////  }

////  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
//////  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x32F2);
////}

///**
//  * @brief  Display the current time and date.
//  * @param  showtime : pointer to buffer
//  * @param  showdate : pointer to buffer
//  * @retval None
//  */
//void RTC_CalendarInfo(uint8_t *time_buffer, uint8_t *date_buffer)
//{
//  RTC_DateTypeDef sdatestructureget;
//  RTC_TimeTypeDef stimestructureget;

//  /* Get the RTC current Time */
//  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
//  /* Get the RTC current Date */
//  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
//  /* Display time Format : hh:mm:ss */
//  sprintf((char *)time_buffer, "%02d:%02d:%02d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
//  /* Display date Format : mm-dd-yy */
//  sprintf((char *)date_buffer, "%02d-%02d-%02d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
//}

//void RTC_GetTime(uint8_t *time_buffer) {
//  RTC_TimeTypeDef stimestructureget;

//  /* Get the RTC current Time */
//  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
//  /* Display time Format : hh:mm:ss */
//  sprintf((char *)time_buffer, "%02d:%02d:%02d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
//}

//void RTC_AlarmConfig(void)
//{
//  RTC_AlarmTypeDef sAlarm;
//  RTC_TimeTypeDef sTime;
//  uint8_t min;
//  uint8_t next_min;
//  
//	HAL_RTC_GetTime(&RtcHandle, &sTime, RTC_FORMAT_BIN);
//  min = sTime.Minutes;
//  next_min = (min<59) ? ++min : 0;
//  
//  sAlarm.AlarmTime.Hours = 0x00; // enmascarado
//  sAlarm.AlarmTime.Minutes = RTC_ByteToBcd2(next_min);
//  sAlarm.AlarmTime.Seconds = 0x00;
//  sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
//  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
//  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS;
//  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
//  sAlarm.AlarmDateWeekDay = 0x01;
//  sAlarm.Alarm = RTC_ALARM_A;
//  if(HAL_RTC_SetAlarm_IT(&RtcHandle, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  if (first_config)
//  {
//    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
//    first_config = 0;
//  }
//}


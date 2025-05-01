/**
  ******************************************************************************
  * @file    Templates/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
extern RTC_HandleTypeDef RtcHandle;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS2_RTX5
void SVC_Handler(void)
{
}
#endif

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS2_RTX5
void PendSV_Handler(void)
{
}
#endif

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS2_RTX5
void SysTick_Handler(void)
{
  HAL_IncTick();
}
#endif

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

//void EXTI15_10_IRQHandler(void){
//		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
//}
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
//	if(GPIO_Pin==GPIO_PIN_13){//Si se pulsa
//	

//		RTC_DateTypeDef sdatestructure;
//		RTC_TimeTypeDef stimestructure;

//		/*##-1- Configure the Date #################################################*/
//		/* Set Date: Tuesday February 18th 2014 */
//		sdatestructure.Year = 0x00;
//		sdatestructure.Month = RTC_MONTH_JANUARY;
//		sdatestructure.Date = 0x01;
//		sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
//		
//		if(HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BCD) != HAL_OK)
//		{
//			/* Initialization Error */
//		 // Error_Handler();
//		}

//		/*##-2- Configure the Time #################################################*/
//		/* Set Time: 02:00:00 */
//		stimestructure.Hours = 0x00;
//		stimestructure.Minutes = 0x00;
//		stimestructure.Seconds = 0x00;
//		stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
//		stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
//		stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

//		if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD) != HAL_OK)
//		{
//			/* Initialization Error */
//			//Error_Handler();
//		}

//		/*##-3- Writes a data in a RTC Backup data Register1 #######################*/
//		HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x32F2);
//		
//	}
//}
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */

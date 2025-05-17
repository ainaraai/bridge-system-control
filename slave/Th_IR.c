#include "principal.h"
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
 const float IR_FREQ_38kHz = 38000;
 
osThreadId_t tid_Th_IR;                        // thread id
osEventFlagsId_t IR_event_id;
void Th_IR (void *argument);                   // thread function

TIM_HandleTypeDef tim3; // TIM3 OC 
TIM_HandleTypeDef tim4; // TIM4 IC
GPIO_InitTypeDef GPIO_InitStruct = {0};

extern float freq;

static void init_timer3(void);
static void init_timer4(void);
static void config_pin_tim3(void);
static void config_pin_tim4(void);

#define DETECTION 0x00000001U
#define NO_DETECTION 0x00000010U
 
int Init_Th_IR (void) {
 
  tid_Th_IR = osThreadNew(Th_IR, NULL, NULL);
  if (tid_Th_IR == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Th_IR (void *argument) {
  uint8_t state = NO_DETECTION;
  uint8_t countdown = 3;
  
 
  	
	init_timer3();
	init_timer4();
	config_pin_tim3();
	config_pin_tim4();
  
  while (1) {
    switch (state) {
      case DETECTION:
        if (freq > (IR_FREQ_38kHz - 1000) && freq < (IR_FREQ_38kHz + 1000)) {
          // Detects the signal, i.e.: there's no boat crossing
          if (--countdown == 0) {
            // Three counts until we assume there's really no boat.
            //osEventFlagsSet(IR_event_id, NO_DETECTION);
						IR_Signal_Restored_Handler();
            state = NO_DETECTION;
            countdown = 3;
          }
        }
        break;
      case NO_DETECTION:
        if (freq < (IR_FREQ_38kHz - 1000) || freq > (IR_FREQ_38kHz + 1000)) {
          // There's a frequency deviation --> IR detects something
          //osEventFlagsSet(IR_event_id, DETECTION);
					IR_Signal_Lost_Handler();
          state = DETECTION;
        }
        break;
    }
    osDelay(1000);
  }
}



/* --------------------- TIMER CONFIGURATIONS --------------------- */
/* ---------------------------------------------------------------- */

/* TIM3 EN OUTPUT COMPARE*/
static void init_timer3 ()
{
	TIM_OC_InitTypeDef TIM_Channel_InitStruct;
	__HAL_RCC_TIM3_CLK_ENABLE();

	tim3.Instance = TIM3;
	tim3.Init.Prescaler = 1;
	tim3.Init.Period = 552; 
	HAL_TIM_OC_Init(&tim3);

	TIM_Channel_InitStruct.OCMode			= TIM_OCMODE_TOGGLE;
	TIM_Channel_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_Channel_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&tim3, &TIM_Channel_InitStruct, TIM_CHANNEL_4);

	HAL_TIM_OC_Start(&tim3, TIM_CHANNEL_4);
}

static void init_timer4()
{
	TIM_IC_InitTypeDef TIM_Channel_InitStruct;
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	tim4.Instance = TIM4;
	tim4.Init.Prescaler = 0;
	tim4.Init.Period = 0x0000FFFF;
	tim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_IC_Init(&tim4);
	
	TIM_Channel_InitStruct.ICFilter = 0; // defines the frequency used to sample the external clock signal and the length of the digital filter applied to it.
	TIM_Channel_InitStruct.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	TIM_Channel_InitStruct.ICPrescaler = TIM_ICPSC_DIV1;
	TIM_Channel_InitStruct.ICSelection = TIM_ICSELECTION_DIRECTTI; // canal TI1 conectado a IC1, TI2-CI2, etc.
	
	HAL_TIM_IC_ConfigChannel(&tim4, &TIM_Channel_InitStruct, TIM_CHANNEL_4);
	HAL_TIM_IC_Start_IT(&tim4, TIM_CHANNEL_4);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

static void config_pin_tim3()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void config_pin_tim4()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


#include "pwm.h"


/*--------------------VARIABLES---------------------*/
osThreadId_t tid_Th_zumbador;                       
TIM_HandleTypeDef tim2;
osTimerId_t tim_id_PWM;  
osThreadId_t tid_Th_Zumbador_Test;
static uint32_t flag;
static uint8_t cont;
/*--------------------PROTOTIPOS DE FUNCIONES---------------------*/
void Init_Thread_Zumbador (void);
void Thread_Zumbador (void *argument);                   
void initTimer_PWM(void);
void initZumbador(void);
static void Timer_Callback (void const *arg);
void Init_Timer_ZUMB (void);                          
void Re_Init_Timer(void);
void init_Pulsador(void);
void Th_Zumbador_Test(void *argument); 
void Re_Init_Timer2(void);

 
/*------------------------FUNCIONES------------------------------*/
void Init_Thread_Zumbador (void) {
 
  tid_Th_zumbador= osThreadNew(Thread_Zumbador, NULL, NULL);

}
 
void Thread_Zumbador (void *argument) {
   flag = 0;

  Init_Timer_ZUMB();
  while (1) {
		flag=osThreadFlagsWait(ALARMA |ALERTA | ZSTOP,osFlagsWaitAny,osWaitForever);
		
    if(flag==ALARMA){
			Re_Init_Timer();	
		}
		if(flag==ALERTA){	
			Re_Init_Timer2();
		}
		if(flag==ZSTOP){
			osTimerStop(tim_id_PWM);
			HAL_TIM_PWM_Stop(&tim2, TIM_CHANNEL_4);
		}
		osThreadYield();                            
  }
}


void initTimer_PWM(void){
	TIM_OC_InitTypeDef TIM_Channel_InitStruct;
	
	__HAL_RCC_TIM2_CLK_ENABLE();

	tim2.Instance=TIM2;
	tim2.Init.Prescaler=99;
	tim2.Init.CounterMode=TIM_COUNTERMODE_UP;
	tim2.Init.Period= 1679; //2kHz
	tim2.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_PWM_Init(&tim2);
	
	TIM_Channel_InitStruct.OCMode = TIM_OCMODE_PWM1;
	TIM_Channel_InitStruct.Pulse =209;
	TIM_Channel_InitStruct.OCPolarity=TIM_OCPOLARITY_HIGH;
	TIM_Channel_InitStruct.OCFastMode= TIM_OCFAST_DISABLE;

	HAL_TIM_PWM_ConfigChannel(&tim2,&TIM_Channel_InitStruct,TIM_CHANNEL_4);
}

void initZumbador(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate=GPIO_AF1_TIM2;
	GPIO_InitStruct.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	
}

//TIMER DEL ZUMBADOR
//We turn start and stop the signal according to the frquency set(dependes on wether it is ALARMA or ALERTA)
static void Timer_Callback (void const *arg) {
	  
	  if(cont==1){
			
			HAL_TIM_PWM_Stop(&tim2, TIM_CHANNEL_4);
			cont=0;
		}else{
			HAL_TIM_PWM_Start(&tim2,TIM_CHANNEL_4);
			cont=1;
		
		}
	 
	  
}
 
void Init_Timer_ZUMB (void) {
  tim_id_PWM = osTimerNew((osTimerFunc_t)&Timer_Callback, osTimerPeriodic, NULL, NULL);

}

void Re_Init_Timer(void){
	osTimerStart(tim_id_PWM, 400);
  HAL_TIM_PWM_Start(&tim2,TIM_CHANNEL_4);
  cont=1;

}

void Re_Init_Timer2(void){
	osTimerStart(tim_id_PWM,5);
  HAL_TIM_PWM_Start(&tim2,TIM_CHANNEL_4);
	cont=1;
	//flag=osThreadFlagsWait(ZSTOP, osFlagsWaitAny, osWaitForever);
  
}
osThreadId_t getModPWMThreadID(void){
	return tid_Th_zumbador;
}

/*--------------------MÓDULO---------------------*/
void initModZumbador(void){
	initTimer_PWM();
	initZumbador();
	//Init_Thread_Zumbador();
	flag=0;
	cont=0;
}


/*--------------------TEST---------------------*/


void EXTI15_10_IRQHandler(void){
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin==GPIO_PIN_13){//Si se pulsa
		osDelay(50);
		osThreadFlagsSet(tid_Th_Zumbador_Test,0x01);
		
	}
}
void init_Pulsador(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}



void Init_Th_Zumbador_Test(void) {
 
  tid_Th_Zumbador_Test = osThreadNew(Th_Zumbador_Test, NULL, NULL);

}
 
void Th_Zumbador_Test (void *argument) {
	Init_Timer_ZUMB();
	init_Pulsador(); 
	Re_Init_Timer();
  while (1) {		
    flag=osThreadFlagsWait(0x01,osFlagsWaitAny,osWaitForever);
		
		if(flag==0x01){
			if(osTimerIsRunning(tim_id_PWM)==1){
				osTimerStop(tim_id_PWM);
				HAL_TIM_PWM_Stop(&tim2, TIM_CHANNEL_4);
			}else{
				Re_Init_Timer();
			}
			
		}
		
  }
}



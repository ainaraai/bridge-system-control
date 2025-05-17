#include "servomotor.h"

/*----------------------------------------------------------------------------
 *      Thread 'Servomotor.c': Sample thread
 *---------------------------------------------------------------------------*/
 
 /*
 Datos tecnicos:
 - El servomor funciona con una frecuencia de 50Hz
 - Para controlarlo, se va cambiando el periodo del pulso
Izquierda - 2ms (180º) Centro - 1.5ms (90º) Izquierda 1ms (0º)
 */
 
#define right_duty            		80
#define left_duty               	40
#define stop_duty									60
 
#define MIN_PULSE 40  // 1ms
#define MAX_PULSE 80  // 2ms

 //thread stuff
static osThreadId_t TID_Servo,TID_Servo_STOP;                        // thread id
static void Thread_servo (void *argument);                   // thread function
static uint8_t aux_j,aux_i;
static uint8_t i,j;
//PWM
static TIM_HandleTypeDef htim1;  //estructura tim1 PWM mode
static TIM_OC_InitTypeDef sConfigOC; //config tim1 PWM mode
static GPIO_InitTypeDef GPIO_InitStruct; //estructura config pin salida TIM1

//variables
static uint16_t dutyCycle; //ciclo de trabajo


//internal functions

static void open_bridge (void);
static void close_bridge(void);
static void continuos_open(void);
static void continuos_close(void);
static void start_position(void);
static void stop_position(void);
static void Thread_STOP_servo (void *argument);

void Init_ThServo (void) {
  TID_Servo = osThreadNew(Thread_servo, NULL, NULL);
  TID_Servo_STOP = osThreadNew(Thread_STOP_servo, NULL, NULL);

  if (TID_Servo == NULL) {
  }
 
}
 
static void Thread_servo (void *argument) {
  static uint32_t flags;
  
  while (1) {
    flags = osThreadFlagsWait(CLOSE_FLAG | OPEN_FLAG  | START_FLAG , osFlagsWaitAny, osWaitForever);
    if (flags == CLOSE_FLAG) {
				continuos_close();
    } else if (flags == OPEN_FLAG) {
				continuos_open();
    } else if (flags == START_FLAG) {
				start_position();
		}
		osThreadYield();
  }
}
static void Thread_STOP_servo (void *argument) {
  static uint32_t flags;
  
  while (1) {
    flags = osThreadFlagsWait( STOP_FLAG | START_FLAG, osFlagsWaitAny, osWaitForever);
    if (flags == STOP_FLAG) {
				stop_position();
		}else if (flags == START_FLAG) {
				start_position();
		}
		osThreadYield();
  }
}
/*
Funcion que inicializa el timer 1 para PWM
APB2 = 168MHz - SystemCoreClock
*/
static void initTim1PWM(void) { //funcion config tim1 PWM
  __HAL_RCC_TIM1_CLK_ENABLE();
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 4199;    //168MHz/4200=40000
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 799;         //40000/800 = 50Hz
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim1);
  
  //Como el period es 800, pulse valdrá entre 0 y 800
  //50Hz = 20ms
  //20ms/800 = 0.025ms cada numero de pulse
  //Para conseguir: 1ms, pulse tendra que valer 1/0.025 = 40				- izquierda
  //                1.5 ms, pulse tendra que valer 1.5/0.025=60			- stop
  //                2 ms, pulse tendra que valer 2/0.025=80					- derecha
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = stop_duty;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  dutyCycle = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
	
	sConfigOC.Pulse = right_duty;
	HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  dutyCycle = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_2);
}


static void initPins(void){ //Pin salida PE9 TIM1  PE11 TIM1_CH2
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}

static void updateServoDutyBarrera(uint8_t duty) {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
}
static void updateServoDutyPuente(uint8_t duty) {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
}
uint8_t angle_to_duty(uint8_t angle) {
    if (angle > 180) angle = 180;
    return MIN_PULSE + ((MAX_PULSE - MIN_PULSE) * angle) / 180;
}

static void barrera_close(void) {
   updateServoDutyBarrera(angle_to_duty(0));  // move to 90°
}
static void barrera_open(void) {
   updateServoDutyBarrera(angle_to_duty(90));  // move to 90°
}
static void continuos_move_barrera(uint8_t angle){
	if(aux_i==0 &&aux_j==0){
   updateServoDutyBarrera(angle_to_duty(angle));  // move to 90°
	}else{
	  i=0;
		j=200;
	}
}
static void continuos_move_puente(uint8_t angle){
	if(aux_i==0 &&aux_j==0){
   updateServoDutyPuente(angle_to_duty(angle));  // move to 90°
	}else{
	  i=0;
		j=200;
	}
}

static void continuos_open(void) {
	//move servo from 90º to 0º
	for( i=180; i>0; i=i-10){
		if(aux_i==0&& aux_j==0){
		continuos_move_barrera(i);
		osDelay(200);
			}else{
		 break;
		}
	}
  osDelay(1000);
	for( j=0; j<=110; j=j+10){
		if(aux_i==0&& aux_j==0){
		continuos_move_puente(j);
		osDelay(200);
				}else{
		 break;
		}
	}

}
static void start_position(void) {
	//move servo from 90º to 0º
	aux_i=0;
	aux_j=0;
	updateServoDutyBarrera(angle_to_duty(180));  // move to 90°
  osDelay(1000);
  updateServoDutyPuente(angle_to_duty(5));  // move to 90°

}

static void continuos_close(void) {
	//move servo from 90º to 0º
	
	for(i=110; i>0; i=i-10){
		if(aux_i==0&& aux_j==0){
		continuos_move_puente(i);
		osDelay(200);
		}else{
		 break;
		}
	}
	osDelay(1000);
		for(j=0; j<=180; j=j+10){
		continuos_move_barrera(j);
		osDelay(200);
	}

}
static void stop_position(void) {
	//move servo from 90º to 0º
	aux_i=i;
	aux_j=j;
	i=0;
	j=200;	
}



osThreadId_t getModServoThreadID(void){
		return  TID_Servo; 

}

osThreadId_t getModServoSTOPThreadID(void){
		return  TID_Servo_STOP; 

}


void InitModServo(void){
	initPins();
	initTim1PWM();
  Init_ThServo();

}

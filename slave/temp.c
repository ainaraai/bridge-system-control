#include "temp.h"

/*--------------------------VARIABLES----------------------------*/

//I2C register addresses
#define REG_TEMP    0x00

#define TRANSFER_COMPLETE 0x01

extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C* drv_I2C=&Driver_I2C1;

#define m_ADDR 0x48

osTimerId_t tim_temp; 
osMessageQueueId_t mid_MsgQueueTemp;
osThreadId_t tid_Th_Temp;                        
osThreadId_t tid_Th_Temp_Test;
float temperatura;

/*-------------------PROTOTIPOS DE FUNCIONES---------------------*/
void I2C_SignalEvent_TEMP (uint32_t event);
void initI2C(void);
float temperature(void);
int read16(uint8_t reg);
void Init_MsgQueue_Temp (void);
void Thread_Temp (void *argument);
void Init_Thread_Temperatura(void);
void Th_Temp_Test (void *argument);
/*---------------------------FUNCIONES---------------------------*/
//SENSOR TEMPERATURA
/* I2C Signal Event function callback */
void I2C_SignalEvent_TEMP (uint32_t event) {
 
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    osThreadFlagsSet(tid_Th_Temp, TRANSFER_COMPLETE);
  }

}
void initI2C(void){
	drv_I2C->Initialize(I2C_SignalEvent_TEMP);
	drv_I2C->PowerControl(ARM_POWER_FULL);
	drv_I2C->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
	drv_I2C->Control(ARM_I2C_BUS_CLEAR, 0);
}
float temperature(void){
    //Signed return value
    short value;
 
    //Read the 11-bit raw temperature value
    value = read16(REG_TEMP) >> 5;
 
    //Sign extend negative numbers
    if (value & (1 << 10))
        value |= 0xFC00;// this operation sets the upper 10 bits of value to 1, while leaving the lower bits unchanged.
 
    //Return the temperature in °C
    return value * 0.125;//So, if the 10th bit of value is set, the upper 10 bits of value will be set to 1 by performing a bitwise OR operation with 0xFC00. If the 10th bit is not set, nothing happens.
}
 
 
int read16(uint8_t reg){
//Create a temporary buffer
  uint8_t buff[2];

  //Select the register
  drv_I2C->MasterTransmit(m_ADDR, &reg, 1, false);
  /* Wait until transfer completed */
  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);


	//Read the 16-bit register
  drv_I2C->MasterReceive(m_ADDR, buff, 2, false);
  /* Wait until transfer completed */
  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	
	//Return the combined 16-bit value
  return (buff[0] << 8) | buff[1];
}


//THREAD TEMPERATURA
void Thread_Temp (void *argument) {
    float temp;
   while (1) {
			temp = temperature();
			if(temp>=40){
					High_temp();
			}
			osDelay(400);
			osThreadYield();                            // suspend thread
  }
}

void Init_Thread_Temperatura(void) {
  tid_Th_Temp = osThreadNew(Thread_Temp, NULL, NULL);
}
 
osThreadId_t getModTempID(void){
	return tid_Th_Temp;
}
/*---------------------------MODULO-----------------------------*/
void initModTemp(void){
	initI2C();
	Init_Thread_Temperatura();
} 
/*------------------------------TEST----------------------------*/
void Th_Temp_Test (void *argument) {
  MSGQUEUE_TEMP_t msg;
  osStatus_t status;
	temperatura=0;
  while (1) {
    status = osMessageQueueGet(mid_MsgQueueTemp, &msg, NULL, 1000);   // wait for message
    if (status == osOK) {
			temperatura=msg.temp;
	  }
		osThreadYield();
  }
}
void Init_Th_Temp_Test(void) {
	 tid_Th_Temp_Test = osThreadNew(Th_Temp_Test, NULL, NULL);
}


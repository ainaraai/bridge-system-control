#include "principal.h"

#include "stdio.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Principal
 *---------------------------------------------------------------------------*/
 
//SPI init
extern ARM_DRIVER_SPI Driver_SPI3;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI3;

bool mensaje_Recibido = false;
//thread
static osThreadId_t tid_Thread_slave; 
static osThreadId_t tid_Thread_Estados; 
static osThreadId_t tid_Thread_AccesoManual;


void test_flash_users (void *argument);                   // thread function
void test_flash_events (void*argument);
void test_generic_password (void *argument);
//Timer id
static osTimerId_t id_timer_30s;

//Functions
static void SPI_Callback(uint32_t event);

//VARIABLES INTERNAS
static uint32_t estado;
static bool autom;
//ID COLAS
static osMessageQueueId_t idcolaRFID;


void SPI_Slave_Init(void) {
    int32_t status;

    status = SPIdrv->Initialize(SPI_Callback);  // No callback needed if polling
    if (status != ARM_DRIVER_OK) { /* handle error */ }

    status = SPIdrv->PowerControl(ARM_POWER_FULL);
    if (status != ARM_DRIVER_OK) { /* handle error */ }

    status = SPIdrv->Control(ARM_SPI_MODE_SLAVE |
                             ARM_SPI_CPOL0_CPHA0 |
                             ARM_SPI_MSB_LSB |
                             ARM_SPI_DATA_BITS(8) |
                             ARM_SPI_SS_INACTIVE, 1000000);  // 1 MHz
    if (status != ARM_DRIVER_OK) { /* handle error */ }
}

static void SPI_Callback(uint32_t event){
  uint32_t error;
    switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        error = osThreadFlagsSet(tid_Thread_slave, TRANSFER_COMPLETE);
        if(error == osFlagsErrorUnknown) {
          __breakpoint(0);
        } else if (error == osFlagsErrorParameter) {
          //osThreadFlagsSet(tid_Thread_principal, TRANSFER_COMPLETE);
          __breakpoint(0);
        } else if (error == osFlagsErrorResource) {
          __breakpoint(0);
        }
        break;
    case ARM_SPI_EVENT_DATA_LOST:
        /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    case ARM_SPI_EVENT_MODE_FAULT:
        /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}
void notify_master_event(void) {
    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_X, GPIO_PIN_RESET);  // LOW = IRQ asserted
}
void release_irq_pin(void) {
    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_X, GPIO_PIN_SET);   // HIGH = IRQ idle
}

void IR_Signal_Lost_Handler(void) {
    SPI_Message msg;
    msg.type = EVENT_IR_LOST;
    notify_master_event();   // lower IRQ pin to notify master
}

void IR_Signal_Restored_Handler(void) {
    SPI_Message msg;
    msg.type = EVENT_IR_RESTORED;
    notify_master_event();   // lower IRQ pin to notify master
}

void High_temp(void) {
    SPI_Message msg;
    msg.type = EVENT_TEMP_HIGH;
    notify_master_event();   // lower IRQ pin to notify master
}
void SPI_Slave_Thread(void *arg) {
	static SPI_Message msg_tx = {0};
  static SPI_Message msg_rx = {0};
	
//---------------------TEST_LEDS---------------------------------
//	osDelay(1000);
//	LED_Off(0);
//	osDelay(1000);
//	LED_Off(1);
//	osDelay(1000);
//	LED_Off(2);
//---------------------------------------------------------------	
	
	while(1){
//		osThreadFlagsSet(getModServoThreadID(),START_FLAG);
//		osDelay(6000);
//		osThreadFlagsSet(getModServoThreadID(),OPEN_FLAG);
//		osDelay(10000);
//		osThreadFlagsSet(getModServoThreadID(),CLOSE_FLAG);
//		osDelay(10000);
//		osThreadFlagsSet(getModServoThreadID(),OPEN_FLAG);
//    osDelay(1500);
//		osThreadFlagsSet(getModServoSTOPThreadID(),STOP_FLAG);
//		osDelay(5000);
//		osThreadFlagsSet(getModServoSTOPThreadID(),START_FLAG);
//		osDelay(6000);
   // osThreadFlagsSet(getModServoThreadID(),CLOSE_FLAG);
		//osDelay(5000);
		//osThreadFlagsSet(getModServoSTOPThreadID(),STOP_FLAG);
	//	osDelay(5000);




		  
					SPIdrv->Transfer(&msg_tx, &msg_rx, sizeof(SPI_Message));
					osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);

					if (msg_rx.type == CMD_ACTIVATE_SERVO) {
						LED_On(2); //RED
           	osThreadFlagsSet(getModServoThreadID(),OPEN_FLAG);
		        osDelay(10000);
//            set_led_red();
					} else if (msg_rx.type == CMD_CLOSE_SERVO) {
						
							osThreadFlagsSet(getModServoThreadID(),CLOSE_FLAG);
	          	osDelay(10000);
							LED_On(0); //GREEN
//							set_led_green();
					} else if (msg_rx.type == CMD_STOP_ALL) {
							osThreadFlagsSet(getModServoSTOPThreadID(),STOP_FLAG);
		          osDelay(5000);
							LED_On(1); //BLUE
//							emergency_led();
					}else if (msg_rx.type == CMD_RESUME_ALL) {
					   osThreadFlagsSet(getModServoSTOPThreadID(),START_FLAG);
		         osDelay(5000);
					
					}

					release_irq_pin();
   }
	
}




void Init_Thread_principal(void){
	Init_LED1();
	Init_LED2();
	Init_LED3();
  SPI_Slave_Init();
	//initModTemp();
	InitModServo();
	tid_Thread_slave=osThreadNew (SPI_Slave_Thread, NULL, NULL);
	osThreadFlagsSet(getModServoThreadID(),START_FLAG);
	osDelay(3000);
}

 
  osThreadId_t getThIDSlave(void){
    return tid_Thread_slave;
 }

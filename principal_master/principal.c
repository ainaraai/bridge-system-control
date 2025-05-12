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
static osThreadId_t tid_Thread_principal; 
static osThreadId_t tid_Thread_Estados; 
static osThreadId_t tid_Thread_AccesoManual;
static osThreadId_t tid_Thread_test; // thread id


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


int Init_test(void) {
   const static osThreadAttr_t th_attr = {.stack_size = 7000};
  //tid_Thread_test = osThreadNew(test_flash_users, NULL, &th_attr);
  if (tid_Thread_test == NULL) {
    return(-1);
  }
	return 0;
}

const osThreadAttr_t threadPH_attr = {
  .priority = osPriorityHigh                    //Set initial thread priority to high   
};

void SPI_Master_Init(void) {
    int32_t status;

    status = SPIdrv->Initialize(SPI_Callback);  // No callback needed if polling
    if (status != ARM_DRIVER_OK) { /* handle error */ }

    status = SPIdrv->PowerControl(ARM_POWER_FULL);
    if (status != ARM_DRIVER_OK) { /* handle error */ }

    status = SPIdrv->Control(ARM_SPI_MODE_MASTER |
                             ARM_SPI_CPOL0_CPHA0 |
                             ARM_SPI_MSB_LSB |
                             ARM_SPI_DATA_BITS(8) |
                             ARM_SPI_SS_MASTER_UNUSED, 1000000);  // 1 MHz
    if (status != ARM_DRIVER_OK) { /* handle error */ }
}

static void SPI_Callback(uint32_t event){
  uint32_t error;
    switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        error = osThreadFlagsSet(tid_Thread_principal, TRANSFER_COMPLETE);
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
void Emergency(void) {
    SPI_Message msg = {0};
    msg.type = CMD_STOP_ALL;

    SPIdrv->Send(&msg, sizeof(SPI_Message));
	  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
}


void SPI_Master_Thread(void *arg) {
	static uint32_t flags = 0;
	static SPI_Message msg_tx = {0};
  static SPI_Message msg_rx = {0};
	static MSGQUEUE_USER_t m_user;
  static MSGQUEUE_TECLADO_t m_teclado;
	static MSGQUEUE_LCD_t m_lcd;


	
	while(1){
		  
		  switch(estado){
				case SPI:
						flags=osThreadFlagsWait(SLAVEIRQ | EMERGENCY | RESUME | MANUAL,osFlagsWaitAny, osWaitForever);
						if(flags & SLAVEIRQ){

							SPIdrv->Transfer(&msg_tx, &msg_rx, sizeof(SPI_Message));
							if (msg_rx.type == EVENT_IR_LOST) {
									 if (osTimerIsRunning(id_timer_30s)) {
										 osTimerStop(id_timer_30s);  // cancel closing
									 }else{
										// Espera flag del input web
										flags=osThreadFlagsWait(SUBIR | EMERGENCY, osFlagsWaitAny, osWaitForever);  // esta flag la activa el handler web
										if(flags & SUBIR){
											msg_tx.type = CMD_ACTIVATE_SERVO;
											SPIdrv->Send(&msg_tx, sizeof(SPI_Message));
											osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
										}
										if(flags & EMERGENCY){
											Emergency();
										}
									 }
							} else if(msg_rx.type == EVENT_IR_RESTORED){
										//TIMER
									 osTimerStart(id_timer_30s, 30000); 
							}
						}
						if(flags & EMERGENCY || flags & MANUAL ){
								estado=MANUAL;
						}
						if(flags & RESUME){
								estado=RESUME;
						}
						
				break;
				case MANUAL:
					  msg_tx.type = CMD_STOP_ALL;
						//SPIdrv->Send(&msg_tx, sizeof(SPI_Message));
						//osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
						//flags=osThreadFlagsWait(SUBIR | BAJAR | RESUME | MANUAL,osFlagsWaitAny, osWaitForever);
				    if(osMessageQueueGet(getModTecladoQueueID(),&m_teclado,NULL,osWaitForever)==osOK){ 
				        if(m_teclado.accion== SUBIR){
											msg_tx.type = CMD_ACTIVATE_SERVO;
											//SPIdrv->Send(&msg_tx, sizeof(SPI_Message));
											//osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
									    osDelay(600);
											m_lcd.nLin = 2;
											sprintf(m_lcd.buf, "");
	                    osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
											osDelay(300);
										}
						    else if(m_teclado.accion== BAJAR){
											msg_tx.type = CMD_CLOSE_SERVO;
											//SPIdrv->Send(&msg_tx, sizeof(SPI_Message));
											//osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
									    osDelay(600);
										  m_lcd.nLin = 2;
										  sprintf(m_lcd.buf, "");
	                    osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
											osDelay(300);
										}
					
					     if(m_teclado.accion== RESUME){
								  estado=RESUME;
						    }
						}
	      break;
				case RESUME:
				    msg_tx.type = CMD_RESUME_ALL;
						SPIdrv->Send(&msg_tx, sizeof(SPI_Message));
				    autom=true;
						//osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
						estado=SPI;
					break;
					
		}
 
   }
	
}


void Estado_Thread(void *arg) {
//	
//	while(1){
//			switch(estado){
//				case CLOSE:
//					
//					break;
//				case UP:
//					
//					break;
//			}
//	}
}


int passwords_match(const uint8_t password[4], const char passw[5]) {
    for (int i = 0; i < 4; i++) {
        if ((passw[i] - '0') != password[i]) {
            return 0; // Mismatch
        }
    }
    return 1; // Exact match
}
void AccesoManual_Thread(void *arg) {
	static uint32_t flags = 0;
	static MSGQUEUE_RC522_t m_rfid;
	static MSGQUEUE_TECLADO_t m_teclado;
	static MSGQUEUE_LCD_t m_lcd;
	static MSGQUEUE_USER_t m_user_tx,m_user_rx;
  static uint8_t id[5];
  uint8_t password[4];
	memcpy(password, (uint8_t[]){0x1, 0x3, 0x6, 0x7}, 4);
	osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);
	add_user1("Ainara",password,0x7A,0xD7,0xE5,0x00,0x48);
	osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);
	m_lcd.nLin = 1;
  sprintf(m_lcd.buf, "READY");
	osMessageQueuePut(getModLCDQueueID(), &m_lcd, NULL, 0U);
	osDelay(500);

  //osThreadFlagsSet(getModFlashThreadID(), ADD_USER);
	//osMessageQueuePut(getMsgRXFlashID(),&m_user_tx,NULL,0U);

	while(1){
//		  flags=osThreadFlagsWait(RFID_PRINC,osFlagsWaitAny, osWaitForever);
//		  if(flags & RFID_PRINC){
//				osMessageQueueGet(idcolaRFID,&m_rfid,NULL,osWaitForever);
//			}
				if(osMessageQueueGet(idcolaRFID,&m_rfid,NULL,osWaitForever)==osOK && autom){
					if(m_rfid.id[2]!=NULL){
										//memcpy(id,m_rfid.id,sizeof(uint8_t)*5);
										//estado=COMPRUEBA;
								memcpy(m_user_tx.id,m_rfid.id,sizeof(uint8_t)*5);
								osThreadFlagsSet(getModFlashThreadID(),GET_USER);
								osMessageQueuePut(getMsgRXFlashID(),&m_user_tx,NULL,0U);
								//osThreadFlagsSet(getModFlashThreadID(),GET_USER);
						    
								if(osMessageQueueGet(getMsgTXFlashID(),&m_user_rx,NULL,osWaitForever)==osOK ){
									   if(m_user_rx.found==1){
											 m_lcd.nLin = 1;
                       sprintf(m_lcd.buf, "VALID USER");
	                     osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
											 
											  //osThreadFlagsSet(getModTecladoThreadID(),TECLA_PULSADA);
                        osMessageQueueReset(getModTecladoQueueID());  // Instantly empties the queue											 
											  osDelay(100); 
											 if(osMessageQueueGet(getModTecladoQueueID(),&m_teclado,NULL,30000)==osOK){
							           // if(memcmp(m_teclado.passw,m_user_rx.password,4)==0){
													if(passwords_match(m_user_rx.password,m_teclado.passw)==1){
														   osDelay(400);
														   m_lcd.nLin = 2;
                               sprintf(m_lcd.buf, "OKEY PASS.MANUAL");
	                             osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);

														   m_lcd.nLin = 1;
													   	 sprintf(m_lcd.buf, "AD:UP CD:DO BD:AUTO");
	                             osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
														   osThreadFlagsSet(getModTecladoThreadID(),PASSWD_OK);			 
														   osThreadFlagsSet(tid_Thread_principal,MANUAL);
														   autom=false;
													}else{
															//Led_rojo_set();
														   osDelay(200);
														   m_lcd.nLin = 1;
                               sprintf(m_lcd.buf, "INCORRECT PASS");
	                             osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
														   osDelay(600);
														   m_lcd.nLin = 1;
														   sprintf(m_lcd.buf, "");
	                             osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
											         osDelay(300);
													}
												}else{
															//Led_rojo_set();
													  m_lcd.nLin = 1;
                            sprintf(m_lcd.buf, "TIMEOUT ENTER TAG");
	                          osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
													  osDelay(600);
													  m_lcd.nLin = 1;
														sprintf(m_lcd.buf, "");
	                          osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
													  osDelay(300);
													
													}	
							       }else{
										    //Led_rojo_set();
											  m_lcd.nLin = 1;
                        sprintf(m_lcd.buf, "USER NOT VALID");
	                      osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
											  osDelay(600);
											  m_lcd.nLin = 1;
												sprintf(m_lcd.buf, "");
	                      osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
											  osDelay(300);
											 }
						    }
				   }
			}
	 }
}



void Timer_Servo_Callback(void *arg) {
    SPI_Message msg;
    msg.type = CMD_CLOSE_SERVO; // close bridge
    SPIdrv->Send(&msg, sizeof(SPI_Message));
	  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
}

static void init_timer_30s(void) {
  id_timer_30s = osTimerNew(Timer_Servo_Callback, osTimerOnce, NULL, NULL);
}

void Init_Thread_principal(void){
	static MSGQUEUE_LCD_t m_lcd;
	Init_ThFlash();
	autom=true;
	Init_ThreadTeclado();
	idcolaRFID=Init_ThMFRC522();
	initModLCD();
	m_lcd.nLin = 2;
  sprintf(m_lcd.buf, "INICIADO");
	osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
	//osDelay(300);

	estado=SPI;
  
	tid_Thread_principal=osThreadNew (SPI_Master_Thread, NULL, NULL);
	tid_Thread_Estados=osThreadNew (Estado_Thread, NULL, NULL);
	tid_Thread_AccesoManual=osThreadNew (AccesoManual_Thread, NULL, NULL);

}

 
  osThreadId_t getThIDPrinc1(void){
    return tid_Thread_principal;
 }
  osThreadId_t getThIDPrinEstados(void){
    return tid_Thread_Estados;
 }
	  osThreadId_t getThIDPrinAccesoManual(void){
    return tid_Thread_AccesoManual;
 }
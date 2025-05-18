#include "principal.h"
#include <stdint.h>

#include "stdio.h"
#define TRANSFER_COMPLETE 0x100

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Principal
 *---------------------------------------------------------------------------*/
 
//SPI init
extern ARM_DRIVER_SPI Driver_SPI3;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI3;

bool mensaje_Recibido = false;
//thread
static osThreadId_t tid_Thread_principal; 
static osThreadId_t tid_Thread_Web; 
static osThreadId_t tid_Thread_AccesoManual;
static osThreadId_t tid_Thread_main; // thread id


void test_flash_users (void *argument);                   // thread function
void test_flash_events (void*argument);
void test_generic_password (void *argument);
//Timer id
static osTimerId_t id_timer_30s;

//Functions
static void SPI_Callback(uint32_t event);
extern __NO_RETURN void Th_main (void *arg);
//VARIABLES INTERNAS
static uint32_t estado;
static bool autom;
static uint8_t id[5][5];
static uint16_t warning;
//ID COLAS
static osMessageQueueId_t idcolaRFID;
static osMessageQueueId_t mid_MsgQueue_web;

static uint64_t	ids[5] = {
			((uint64_t)0xE3B0B7F7 << 8) | 0x13,
			((uint64_t)0x9319C2F7 << 8) | 0xBF,
			((uint64_t)0xABD8EF12 << 8) | 0x34,
			((uint64_t)0x56AD3E19 << 8) | 0x23,
			((uint64_t)0x78FE123F << 8) | 0xAB
	};

//int Init_test(void) {
//   const static osThreadAttr_t th_attr = {.stack_size = 7000};
//  //tid_Thread_test = osThreadNew(test_flash_users, NULL, &th_attr);
//  if (tid_Thread_test == NULL) {
//    return(-1);
//  }
//	return 0;
//}

//const osThreadAttr_t threadPH_attr = {
//  .priority = osPriorityHigh                    //Set initial thread priority to high   
//};

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
uint8_t Boat(void) {
   return warning;
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
						flags=osThreadFlagsWait(SLAVEIRQ | EMERGENCY | RESUME | SUBIR,osFlagsWaitAny, osWaitForever);
						if(flags & SLAVEIRQ){

							SPIdrv->Transfer(&msg_tx, &msg_rx, sizeof(SPI_Message));
							if (msg_rx.type == EVENT_IR_LOST) {
									 if (osTimerIsRunning(id_timer_30s)) {
										 osTimerStop(id_timer_30s);  // cancel closing
									 }else{
										// Espera flag del input web
										warning=1;
										flags=osThreadFlagsWait(SUBIR | EMERGENCY, osFlagsWaitAny, osWaitForever);  // esta flag la activa el handler web
										if(flags & SUBIR){
											msg_tx.type = CMD_ACTIVATE_SERVO;
											SPIdrv->Send(&msg_tx, sizeof(SPI_Message));
											osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
										}
										if(flags & EMERGENCY){
											//Emergency();
												estado=MANUAL;
										}
									 }
							} else if(msg_rx.type == EVENT_IR_RESTORED){
										//TIMER
									 osTimerStart(id_timer_30s, 15000); 
							}
						}
						if(flags & EMERGENCY  ){
							//|| flags & MANUAL
								estado=MANUAL;
						}
						if(flags & RESUME){
								estado=RESUME;
						}
						
				break;
				case MANUAL:
					  autom=false;
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
uint64_t pack_id(const uint8_t bytes[5]) {
    return ((uint64_t)bytes[0] << 32) |
           ((uint64_t)bytes[1] << 24) |
           ((uint64_t)bytes[2] << 16) |
           ((uint64_t)bytes[3] << 8)  |
           ((uint64_t)bytes[4]);
}
User_manag_t user_status(void){
    User_manag_t m_web;
	  uint8_t buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	  MSGQUEUE_USERS_t m_users;
		osThreadFlagsSet(getModFlashThreadID(),GET_ALL_USERS);
		osMessageQueueGet(getMsgTXBIGFlashID(),&m_users,NULL,osWaitForever);
	  for(int j=0; j<5; j++){
			 m_web.estado[j]=0;
		}
	  for(int j=0; j<5; j++){
				for(int i=0; i<5; i++){
						if(ids[j]==pack_id(m_users.us[i].id)&& memcmp(buffer, m_users.us[i].password, sizeof(buffer)) != 0){
							 m_web.estado[j]=1;
						}
					}
		}		
    return m_web;
		//osMessageQueuePut(mid_MsgQueue_web,&m_web,NULL,0U);
}
bool compare_passwords(const uint8_t a[4], const uint8_t b[4]) {
    return memcmp(a, b, 4) == 0;
}
//bool authentication_user(const char *input_name, const uint8_t input_password[4]){
//	  MSGQUEUE_USERS_t m_users;
//		osThreadFlagsSet(getModFlashThreadID(),GET_ALL_USERS);
//		osMessageQueueGet(getMsgTXFlashID(),&m_users,NULL,osWaitForever);
//   	  
//		for(int i=0; i<5; i++){
//				 if (strncmp(m_users.us[i].name, input_name, 18) == 0 &&  // up to 18 chars, leave room for '\0'
//            compare_passwords(m_users.us[i].password, input_password)) {
//            return true;  // match found
//        }
//			}
//		return false;	
//}

void Web_Thread(void *arg) {
		User_manag_t m_web_us;
	  User_manag_t m_web_us_cp;
		MSGQUEUE_USER_t m_user_rx_w,m_user_tx_w;
    uint8_t password[4];

	  osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);
		memcpy(password, (uint8_t[]){0x1, 0x1, 0x1, 0x1}, 4);
		add_user1("Ainara",password,0xE3,0xB0,0xB7,0xF7,0x13);
		osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);
		memcpy(password, (uint8_t[]){0x3, 0x3, 0x3, 0x3}, 4);
		add_user1("Lorena",password,0xAB,0xD8,0xEF,0x12,0x34);
		osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);
	
	  osThreadFlagsSet(tid_Thread_AccesoManual,DONE);

    while(1){ 
				if(osMessageQueueGet(mid_MsgQueue_web,&m_web_us,NULL,osWaitForever)==osOK && autom){
					      m_web_us_cp=m_web_us;
					      //memcpy(&m_web_us_cp, &m_web_us, sizeof(User_manag_t));  // Full byte-by-byte copy

								memcpy(m_user_tx_w.id,m_web_us.id,sizeof(uint8_t)*5);
								osThreadFlagsSet(getModFlashThreadID(),GET_USER);
								osMessageQueuePut(getMsgRXFlashID(),&m_user_tx_w,NULL,0U);
						if(osMessageQueueGet(getMsgTXFlashID(),&m_user_rx_w,NULL,osWaitForever)==osOK ){
								if(m_user_rx_w.found==1){			
										delete_user(m_web_us_cp.id[0],m_web_us_cp.id[1],m_web_us_cp.id[2],m_web_us_cp.id[3],m_web_us_cp.id[4]);
										osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);	
										add_user1(m_web_us_cp.hexStringLocal,m_web_us_cp.password_user,m_web_us_cp.id[0],m_web_us_cp.id[1],m_web_us_cp.id[2],m_web_us_cp.id[3],m_web_us_cp.id[4]);
										osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);									
								}else{
										add_user1(m_web_us_cp.hexStringLocal,m_web_us_cp.password_user,m_web_us_cp.id[0],m_web_us_cp.id[1],m_web_us_cp.id[2],m_web_us_cp.id[3],m_web_us_cp.id[4]);
										osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);		
								}
						 }
						
				}
	  }
}

int passwords_match(const uint8_t password[4], const char passw[5]) {
    for (int i = 0; i < 4; i++) {
        if ((passw[i] - '0') != password[i]) {
            return 0; // Mismatch
        }
    }
    return 1; // Exact match
}
//const osThreadAttr_t threadWEB_attr = {
//  .stack_size = 3072,   
//  	//Set initial thread priority to high   
//};
void AccesoManual_Thread(void *arg) {
	static uint32_t flags = 0;
	static MSGQUEUE_RC522_t m_rfid;
	static MSGQUEUE_TECLADO_t m_teclado;
	static MSGQUEUE_LCD_t m_lcd;
	static MSGQUEUE_USER_t m_user_tx,m_user_rx;
  //static uint8_t id[5];
	  uint8_t buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};

  osThreadFlagsWait(DONE , osFlagsWaitAny, osWaitForever);
//	initModLCD();
//	osDelay(200);
	m_lcd.nLin = 1;
	sprintf(m_lcd.buf, "READY");
	osMessageQueuePut(getModLCDQueueID(), &m_lcd, NULL, NULL);
	osDelay(700);
 // los demás hilos los lanza el main


  //osThreadFlagsSet(getModFlashThreadID(), ADD_USER);
	//osMessageQueuePut(getMsgRXFlashID(),&m_user_tx,NULL,0U);

	while(1){

			if(osMessageQueueGet(idcolaRFID,&m_rfid,NULL,osWaitForever)==osOK && autom){
				if(m_rfid.id[2]!=NULL){
									//memcpy(id,m_rfid.id,sizeof(uint8_t)*5);
									//estado=COMPRUEBA;
							memcpy(m_user_tx.id,m_rfid.id,sizeof(uint8_t)*5);
							osThreadFlagsSet(getModFlashThreadID(),GET_USER);
							osMessageQueuePut(getMsgRXFlashID(),&m_user_tx,NULL,0U);
							//osThreadFlagsSet(getModFlashThreadID(),GET_USER);
							
							if(osMessageQueueGet(getMsgTXFlashID(),&m_user_rx,NULL,osWaitForever)==osOK ){
									 if(m_user_rx.found==1&& memcmp(buffer, m_user_rx.password, sizeof(buffer)) != 0){
										  m_lcd.nLin = 1;
											sprintf(m_lcd.buf, "VALID.Enter PASS");
											osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
											 
												//osThreadFlagsSet(getModTecladoThreadID(),TECLA_PULSADA);
											osMessageQueueReset(getModTecladoQueueID());  // Instantly empties the queue											 
											osDelay(100); 
										 if(osMessageQueueGet(getModTecladoQueueID(),&m_teclado,NULL,30000)==osOK){
											 // if(memcmp(m_teclado.passw,m_user_rx.password,4)==0){
													if(passwords_match(m_user_rx.password,m_teclado.passw)==1){
															 osDelay(400);
															 m_lcd.nLin = 2;
                               sprintf(m_lcd.buf, "OKEY PASS:MANUAL");
															 osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);

															 m_lcd.nLin = 1;
															 sprintf(m_lcd.buf, "AD:UP CD:DO BD:AUT");
															 osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
															 osThreadFlagsSet(getModTecladoThreadID(),PASSWD_OK);			 
															 osThreadFlagsSet(tid_Thread_principal,EMERGENCY);
															 autom=false;
													}else{
															//Led_rojo_set();
															 osDelay(200);
															 m_lcd.nLin = 1;
															 sprintf(m_lcd.buf, "INCORRECT. TAG");
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
	  warning=0;
    SPI_Message msg;
    msg.type = CMD_CLOSE_SERVO; // close bridge
    SPIdrv->Send(&msg, sizeof(SPI_Message));
	  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
}

static void init_timer_30s(void) {
  id_timer_30s = osTimerNew(Timer_Servo_Callback, osTimerOnce, NULL, NULL);
}

void Init_Thread_principal(void){
	Init_ThFlash();
  warning=1;
	autom=true;
	initModLCD();
	Init_ThreadTeclado();
	idcolaRFID=Init_ThMFRC522();
	
//	m_lcd.nLin = 2;
//  sprintf(m_lcd.buf, "INICIADO");
//	osMessageQueuePut(getModLCDQueueID(), &m_lcd, 0U, 0U);
	//osDelay(300);

	estado=SPI;

	tid_Thread_principal=osThreadNew (SPI_Master_Thread, NULL, NULL);
	tid_Thread_Web=osThreadNew (Web_Thread, NULL, NULL);
	tid_Thread_AccesoManual=osThreadNew (AccesoManual_Thread, NULL, NULL);
	mid_MsgQueue_web = osMessageQueueNew(10, sizeof(User_manag_t), NULL);
	//mid_MsgQueue_webSMALL=osMessageQueueNew(10, sizeof(User_manag_t), NULL);
	tid_Thread_main=osThreadNew(Th_main, NULL, NULL);

}

 osMessageQueueId_t getMessageWebID(void){
    return mid_MsgQueue_web;
 }
//  osMessageQueueId_t getMessageWebIDSMALL(void){
//    return mid_MsgQueue_web;
// }
  osThreadId_t getThIDPrinc1(void){
    return tid_Thread_principal;
 }
  osThreadId_t getThIDPrinWeb(void){
    return tid_Thread_Web;
 }
	  osThreadId_t getThIDPrinAccesoManual(void){
    return tid_Thread_AccesoManual;
 }
#include "cmsis_os2.h"                         
#include "Teclado.h"
#include "lcd.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#define QUEUE_MAX										10                //maximum number of objects in a queue

/*----------------------------------------------------------------------------
 *      Thread 'ThreadTeclado':
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThreadTeclado;                        // thread id
 
void ThreadTeclado (void *argument);                   // thread function
static osMessageQueueId_t mid_teclado;

extern volatile uint16_t columna_activa;

char letras[4][4]={{'1','2','3','A'},
									 {'4','5','6','B'},
									 {'7','8','9','C'},
									 {'*','0','#','D'}};

GPIO_TypeDef* COL_PORTS[4] = {COL1_PORT, COL2_PORT, COL3_PORT, COL4_PORT};
uint16_t COL_PINS[4] = {COL1_PIN, COL2_PIN, COL3_PIN, COL4_PIN};

GPIO_TypeDef* ROW_PORTS[4] = {ROW1_PORT, ROW2_PORT, ROW3_PORT, ROW4_PORT};
uint16_t ROW_PINS[4] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};
 
int Init_ThreadTeclado (void) {
 
  tid_ThreadTeclado = osThreadNew(ThreadTeclado, NULL, NULL);
  mid_teclado = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_TECLADO_t), NULL);

	if (tid_ThreadTeclado == NULL) {
    return(-1);
  }
 
  return(0);
}

osMessageQueueId_t getModTecladoQueueID(void){
		return mid_teclado;
}
osThreadId_t getModTecladoThreadID(void){
	return tid_ThreadTeclado;
}
void teclado_init(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();


	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
	
	uint16_t row_pins[] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};
	GPIO_TypeDef* row_ports[] = {ROW1_PORT, ROW2_PORT, ROW3_PORT, ROW4_PORT};

	for (int i = 0; i < 4; i++) {
			GPIO_InitStruct.Pin = row_pins[i];
			HAL_GPIO_Init(row_ports[i], &GPIO_InitStruct);
	}

	for (int i = 0; i < 4; i++) {
			HAL_GPIO_WritePin(row_ports[i], row_pins[i], GPIO_PIN_RESET);
	}
	

	GPIO_InitStruct.Mode=GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull=GPIO_PULLUP;

	uint16_t col_pins[] = {COL1_PIN, COL2_PIN, COL3_PIN, COL4_PIN};
	GPIO_TypeDef* col_ports[] = {COL1_PORT, COL2_PORT, COL3_PORT, COL4_PORT};

	for (int i = 0; i < 4; i++) {
			GPIO_InitStruct.Pin = col_pins[i];
			HAL_GPIO_Init(col_ports[i], &GPIO_InitStruct);
	}
	
	
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);     // Para PC9
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);   // Para PC10, PC11, PC12

}

int get_col_index_from_pin(uint16_t pin) {
		for (int i = 0; i < 4; i++) {
        if (pin == COL_PINS[i]) return i;
    }
    return -1;
}
 
 
void ThreadTeclado (void *argument) {
	
	char teclaPulsada;
	bool finalizado=false;
	uint8_t indice=0;
	char digitos[MAX_DIGITOS+1];
	//={0}
	static MSGQUEUE_LCD_t localObject;
	static MSGQUEUE_TECLADO_t m_teclado;

	bool manual_mode = false;
	bool accion = false;

 
	initModLCD(); //LCD initialization
	teclado_init(); //keyboard initialization
	static uint32_t flagTeclado;
	flagTeclado=0;
	memset(digitos, 0, sizeof(digitos));

  while (1){
	  //osThreadFlagsClear(TECLA_PULSADA);  // Clear any stale flags

		flagTeclado=osThreadFlagsWait(TECLA_PULSADA | PASSWD_OK, osFlagsWaitAny, osWaitForever);
		
		//if( !manual_mode ){
			//flagTeclado == TECLA_PULSADA PASSWD_OK
      //flagTeclado=0;
			int col_idx = get_col_index_from_pin(columna_activa);
			teclaPulsada = 0;
	


			if (col_idx != -1) {
				for (int fila = 0; fila < 4; fila++) {
						HAL_GPIO_WritePin(ROW_PORTS[fila], ROW_PINS[fila], GPIO_PIN_SET);
						osDelay(2);
						if (HAL_GPIO_ReadPin(COL_PORTS[col_idx], COL_PINS[col_idx]) == GPIO_PIN_SET) {
								teclaPulsada = letras[fila][col_idx]; 
						}
						HAL_GPIO_WritePin(ROW_PORTS[fila], ROW_PINS[fila], GPIO_PIN_RESET);
						if (teclaPulsada) break;
				}


					while (HAL_GPIO_ReadPin(COL_PORTS[col_idx], COL_PINS[col_idx]) == GPIO_PIN_RESET) {
							osDelay(10);
					}
					
					HAL_GPIO_WritePin(ROW1_PORT,ROW1_PIN,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(ROW2_PORT,ROW2_PIN,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(ROW3_PORT,ROW3_PIN,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(ROW4_PORT,ROW4_PIN,GPIO_PIN_RESET);

					if(teclaPulsada>='0' && teclaPulsada <= '9' && indice < MAX_DIGITOS && !manual_mode) {
							digitos[indice++] = teclaPulsada;
					} else if(teclaPulsada == '#' && indice == MAX_DIGITOS && !manual_mode) {
						  //flagTeclado=0;
							digitos[4] = '\0';
							
							sprintf(localObject.buf, "%s", digitos);
  						sprintf(m_teclado.passw,"%s", digitos);
						  localObject.nLin = 1;
							osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
						  osDelay(300);
						  localObject.nLin = 1;
						  sprintf(localObject.buf, "");
	            osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
							osDelay(300);	
          	  osMessageQueuePut(getModTecladoQueueID(), &m_teclado, 0U, 0U);

							indice = 0;
							memset(digitos, 0, sizeof(digitos));
					} else if (teclaPulsada == '*') {
						  sprintf(localObject.buf, "ERASED");
						  localObject.nLin = 1;
							osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
              osDelay(600);
						  localObject.nLin = 1;
						  sprintf(localObject.buf, "");
	            osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
							osDelay(300);						
							indice = 0;
							memset(digitos, 0, sizeof(digitos));
						 
					}
			
			}
			columna_activa = 0xFFFF;
		 
		if(flagTeclado & PASSWD_OK || manual_mode){
				manual_mode=true;
				if((teclaPulsada == 'A' || teclaPulsada == 'C'|| teclaPulsada == 'B') && !accion){ 
							accion=true;
							
							for (int i = 0; i < MAX_DIGITOS; i++) {
									if (digitos[i] != 0) {
											memset(digitos, 0, sizeof(digitos));
									}
							}
							indice=0;
							digitos[indice] = teclaPulsada;
					} else if(teclaPulsada == 'D' && accion ){
							accion=false;
							//for (int i = 0; i < MAX_DIGITOS; i++) {
										if (digitos[0] == 'A') {
											sprintf(localObject.buf, "SUBIR"); 
											//osThreadFlagsSet(getThIDPrinc1(),SUBIR);
											sprintf(localObject.buf, "UP");
											m_teclado.accion=SUBIR;
											//osMessageQueuePut(getModTecladoQueueID(), &m_teclado, 0U, 0U);



										} else if (digitos[0] == 'C') {
											sprintf(localObject.buf, "BAJAR");
											indice=0;
											memset(digitos, 0, sizeof(digitos));
										//osThreadFlagsSet(getThIDPrinc1(),BAJAR);
											sprintf(localObject.buf, "DOWN");
											m_teclado.accion=BAJAR;
                     //	osMessageQueuePut(getModTecladoQueueID(), &m_teclado, 0U, 0U);


										}else if (digitos[0] == 'B') {
											 manual_mode=false;
										   //osThreadFlagsSet(getThIDPrinc1(),RESUME);
											 indice=0;
							         memset(digitos, 0, sizeof(digitos)); 
										   sprintf(localObject.buf, "AUTO");
											 localObject.nLin = 1;
  							       osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
											 sprintf(localObject.buf, "");
											 m_teclado.accion=RESUME;
											// osMessageQueuePut(getModTecladoQueueID(), &m_teclado, 0U, 0U);

										}
										localObject.nLin = 2;
  							    osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
										osMessageQueuePut(getModTecladoQueueID(), &m_teclado, 0U, 0U);
								}
							  
						}						
							//--------DEBUGGING(SUSTITUIR POR MENSAJE A COLA DEL PRINCIPAL)
//							localObject.nLin = 2;
//							osMessageQueuePut(getModLCDQueueID(), &localObject, 0U, 0U);
//							osDelay(300);
//							//------------------------------------------------------
//							indice=0;
//							memset(digitos, 0, sizeof(digitos)); 
//					} else if ( teclaPulsada == 'B'){
//						  manual_mode=false;
//					}
		// }
				

	 }

		
    osThreadYield();                            // suspend thread
  
}

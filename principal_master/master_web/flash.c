#include "flash.h"
#include <stdio.h>


/*
Flash memory is a device that operates via SPI.
D1 - input, connected to our MOSI.
D0 - output, connected to our MISO.

It is a 16Mbit memory, consisting of 32 blocks of 64KBytes each.
Each block has 16 sectors of 4KBytes. Each sector is composed of 16 pages, each page being 256 bytes.
Thus, the total number of pages is: 32 * 16 * 16 = 8192 pages, with a total of 2 million bytes.

It is a NOR memory, meaning that in order to write, the memory needs to be reset to FF.
Before writing, the memory must be erased. The minimum space to erase is 1 sector (4KBytes).

Therefore, we will use a function called W25Q16_Write, which saves the previous information, updates it,
erases the entire sector memory, and then rewrites the information.

REGARDING OUR APPLICATION
We have decided to make it scalable. To achieve this, we will store the number of users in the first byte of each page (maximum 9 users per page).
An entire sector has been designated to store user information (16 pages). In total we can have 9x16 = 144 users.
Consequently, we have also decided to use a single sector to store information for all events (sector 1).
Now, each event occupies only 25 bytes, so, besides the first byte used to store the number of events saved on that page,
10 events will be stored per page. In total we can have 10x16 = 160 events stored.
The generic password will be stored in sector 2. It occupies 4 bytes of memory. We dont have to check anything, just write and get the data

THAT WAS THE FIRST IDEA - GOT PROBLEMS!!!!!!

FINALLY WE DECIDED TO STORE EACH USER AND EVENT IN A DIFFERENT SECTOR IN ORDER TO GET LESS STACK MEMORY NEEDS
*/

//flags and hilos
#define QUEUE_MAX										10                //maximum number of objects in a queue
#define TRANSFER_COMPLETE 				0x01              //internal flag used by SPI to know that a single transfer has been completed

//memory consts
#define nBlock											32								//32 blocks in our 16MBytes memory
#define PAGES_FOR_SECTOR						16								//defines the number of pages in one sector
//user consts
#define US_SIZE_B             			28								//bytes used for 1 user information
#define US_COUNT_SIZE_B 						1									//bytes used to store the number of users in a single page
#define MAX_USERS										19 								//maximum pages for store user information (total 16*9 = 144 users)
//events consts
#define EVNT_SIZE_B                 25                //bytes used for 1 event information
#define EVNT_COUNT_SIZE_B           1                 //bytes used to store the number of events in a single page
#define MAX_EVNTS           				19                //maximum pages for store event data (total 16*10 = 160 events)


//INSTRUCTIONS
#define ENABLE_RESET									0x66
#define RESET_DEVICE									0x99
#define READ_ID												0x9F
#define READ_DATA											0x03
#define READ_FAST											0x0B
#define WRITE_ENABLE									0x06
#define WRITE_DISABLE									0x04
#define ERASE_BLOCK									  0xD8
#define ERASE_SECTOR                  0X20
#define PAGE_PROGRAM									0x02
#define POWER_DOWN                    0xB9
#define POWER_UP                      0xAB



//SPI init
extern ARM_DRIVER_SPI Driver_SPI4;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI4;

//thread
osThreadId_t tid_flash;															//flash thread id used in main thread
static osMessageQueueId_t mid_MsgQueue_rx,mid_MsgQueue_tx,mid_MsgQueue_txBIG;

static void Th_flash (void *argument);

//LOCAL FUNCTIONS
//initialize
static void W25Q16_Init_SPI(void);
static void SPI_callback(uint32_t event);
static void W25Q16_Init (void);
//recurrent functions
static void W25Q16_WriteInstruction(uint8_t val);
static void W25Q16_Reset(void);
static void W25Q16_Erase_64kBlock (uint16_t numBlock);
static void W25Q16_Erase_Sector (uint16_t numSector);
static uint16_t W25Q16_ReadID(uint8_t number_id);
static void W25Q16_Read (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
static void W25Q16_FastRead (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
static void W25Q16_WritePage_Clean (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
//static void W25Q16_Write (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
static void write_enable (void);
static void write_disable (void);
//static uint32_t bytesToModify (int32_t size, uint16_t offset);
static uint32_t bytesToWrite (uint32_t size, uint32_t offset);
static void erase_usable_memory (void);

//user functions
static uint8_t getNumUsers(void);
static uint8_t getLowestPosUser (void);
static bool find_user (uint8_t *id, uint8_t *pos);
static void bytesToUser(const uint8_t *user_data, MSGQUEUE_USER_t *user);
static void userToBytes(const MSGQUEUE_USER_t *user, uint8_t *user_data);

void addUser (MSGQUEUE_USER_t *user);
void getUser (uint8_t *id);
void deleteUser (uint8_t *id);
void getAllUsers (void);




//power up and down
void W25Q16_PoerDown (void);
void W25Q16_PowerUp (void);


osMessageQueueId_t getMsgRXFlashID(void){
		return mid_MsgQueue_rx;
}
osMessageQueueId_t getMsgTXFlashID(void){
		return mid_MsgQueue_tx;
}
osMessageQueueId_t getMsgTXBIGFlashID(void){
		return mid_MsgQueue_txBIG;
}
osThreadId_t getModFlashThreadID(void){
		return tid_flash;
}
//tests
static void leer_mem_entera(void) ;

//Init flash
void Init_ThFlash (void) {
  //const static osThreadAttr_t th_attr = {.stack_size = 7000};
	mid_MsgQueue_rx = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_USER_t), NULL);
	mid_MsgQueue_tx = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_USER_t), NULL);
	mid_MsgQueue_txBIG=osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_USERS_t), NULL);


	tid_flash = osThreadNew(Th_flash, NULL, NULL); 
	if (tid_flash == NULL) {
    
  }
  
}

static void Th_flash (void *argument) {
	static uint32_t flags = 0;
  static uint32_t error;
  static MSGQUEUE_USER_t read_user, transfer_user;
	static MSGQUEUE_GEN_PASS_t read_gen_pass;

  W25Q16_Init();
  erase_usable_memory();
  error = osThreadFlagsSet(getThIDPrinWeb(), DONE);
  while (1) {
		//error = osThreadFlagsSet(tid_Thread_principal, READY);
		if(error == osFlagsErrorUnknown) {
			__breakpoint(0);
		} else if (error == osFlagsErrorParameter) {
			osThreadFlagsSet(tid_flash, TRANSFER_COMPLETE);
			 __breakpoint(0);
		} else if (error == osFlagsErrorResource) {
			__breakpoint(0);
		}
		flags = osThreadFlagsWait( ADD_USER | DELETE_USER | GET_ALL_USERS | GET_USER | FLAG_POWER_DOWN | FLAG_POWER_UP, osFlagsWaitAny, osWaitForever);
		if (flags & ADD_USER) {
				osMessageQueueGet(mid_MsgQueue_rx, &read_user, NULL, osWaitForever);
				addUser(&read_user);
				error = osThreadFlagsSet(getThIDPrinWeb(), DONE);
		}
		if (flags & DELETE_USER) {
				osMessageQueueGet(mid_MsgQueue_rx, &read_user, NULL, osWaitForever);
				deleteUser(read_user.id);
				error = osThreadFlagsSet(getThIDPrinWeb(), DONE);
		}
		if (flags & GET_ALL_USERS) {
			getAllUsers();
			//error = osThreadFlagsSet(tid_Thread_principal, DONE);
		}
		if (flags & GET_USER) {
			osMessageQueueGet(mid_MsgQueue_rx, &read_user, NULL, osWaitForever);
			getUser(read_user.id);
			//error = osThreadFlagsSet(tid_Thread_principal, DONE);
		}
		if (flags & FLAG_POWER_DOWN) {
			W25Q16_PoerDown();
			error = osThreadFlagsSet(getThIDPrinAccesoManual(), DONE);
		}
		if (flags & FLAG_POWER_UP) {
			W25Q16_PowerUp();
			error = osThreadFlagsSet(getThIDPrinAccesoManual(), DONE);
		}
	}
}

//function that erase all the memory we will use to store user, event and password information
static void erase_usable_memory (void) {
	for (int i = 0; i < 3; i++) {
		W25Q16_Erase_64kBlock(i);
	}
}

//depuration code to read all memory data from sector 0
static void leer_mem_entera(void) {
	uint8_t previousData[50];
	W25Q16_FastRead((1+MAX_USERS+1+MAX_EVNTS)*PAGES_FOR_SECTOR, 0, 50, previousData);
	previousData[0] = 0x00;
}

//depuration test to see if the memory can write and read some data
static void test_write_read () {
	uint8_t num = 1;
	uint8_t TxData[32] =  {num};
  uint8_t RxData[32];
	while (1) {
		for (int e = 0; e < 1000; e++) {
       num++;
      TxData[0] = num;
      W25Q16_WritePage_Clean(0, num, 32,TxData);
      W25Q16_FastRead (0, 0, sizeof(RxData), RxData);
		}
	}
}

//add different users
void add_user1(char nombre[],uint8_t password[], uint8_t id0,uint8_t id1,uint8_t id2,uint8_t id3,uint8_t id4) {
	MSGQUEUE_USER_t user;
	uint8_t id[5];

		id[0] = id0;
		id[1] = id1;
		id[2] = id2;
		id[3] = id3;
		id[4] = id4;
  memcpy(user.id, id, 5);
  strncpy(user.name, nombre, sizeof(user.name) - 1);
	user.name[sizeof(user.name) - 1] = '\0';
  memcpy(user.password, password, 4);
	osThreadFlagsSet(tid_flash, ADD_USER);
	osMessageQueuePut(mid_MsgQueue_rx, &user, NULL, osWaitForever);
	//osThreadFlagsSet(tid_flash, ADD_USER);
	
}
void delete_user(uint8_t id0,uint8_t id1,uint8_t id2,uint8_t id3,uint8_t id4) {
	MSGQUEUE_USER_t user;
	uint8_t id[5];

		id[0] = id0;
		id[1] = id1;
		id[2] = id2;
		id[3] = id3;
		id[4] = id4;
  memcpy(user.id, id, 5);
 
	osThreadFlagsSet(tid_flash, DELETE_USER);
	osMessageQueuePut(mid_MsgQueue_rx, &user, NULL, osWaitForever);
	//osThreadFlagsSet(tid_flash, ADD_USER);
	
}
void add_user2(void) {
	MSGQUEUE_USER_t user;
	char nombre[19] = "Ainara Picot";
	uint8_t password[4] = {0x01, 0x01, 0x01, 0x01};
	uint8_t id[5];

	id[0] = 0x33;
	id[1] = 0x31;
	id[2] = 0x0D;
	id[3] = 0xF8;
	id[4] = 0xF7;
	
  memcpy(user.id, id, 5);
  strcpy(user.name, nombre);
  memcpy(user.password, password, 4);
	
	osMessageQueuePut(mid_MsgQueue_rx, &user, NULL, osWaitForever);
	osThreadFlagsSet(tid_flash, ADD_USER);
	
}



//Init SPI for W25Q16 memory
static void W25Q16_Init_SPI(void){
  __HAL_RCC_GPIOE_CLK_ENABLE();
  
  static GPIO_InitTypeDef GPIO_InitStruct_Flash;
  /*CS*/    //SPI_CS -- SPI_B_NSS       PE11
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_Flash.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_Flash.Pull = GPIO_PULLUP;
  GPIO_InitStruct_Flash.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_Flash.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct_Flash);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
  
  /*Reset*/   //SPI_MISO -- SPI_B_MISO  PA15
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_Flash.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_Flash.Pull = GPIO_PULLUP;
  GPIO_InitStruct_Flash.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_Flash.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_Flash);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
  
  /*SPI*/   
  SPIdrv->Initialize(SPI_callback);
  SPIdrv-> PowerControl(ARM_POWER_FULL);
  SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 1000000);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
  osDelay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
  osDelay(1000);
}

static void SPI_callback(uint32_t event){
  uint32_t error;
    switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        error = osThreadFlagsSet(tid_flash, TRANSFER_COMPLETE);
        if(error == osFlagsErrorUnknown) {
          __breakpoint(0);
        } else if (error == osFlagsErrorParameter) {
          osThreadFlagsSet(tid_flash, TRANSFER_COMPLETE);
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

//initialize the driver SPI and pin configuration and reset the device (NOT all data to FF)
static void W25Q16_Init (void) {
	W25Q16_Init_SPI();
	W25Q16_Reset();
}

//write a single instruction (not so useful)
static void W25Q16_WriteInstruction(uint8_t val) {
	//CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(&val, 1);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	//CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}

//reset the device (NOT all to FF)
static void W25Q16_Reset(void) {
	W25Q16_WriteInstruction(ENABLE_RESET);
	W25Q16_WriteInstruction(RESET_DEVICE);
}

//read the device id
static uint16_t W25Q16_ReadID(uint8_t number_id) {
	uint8_t instuction = READ_ID;
  uint32_t rx_data;
  //CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(&instuction, 1);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(&rx_data, 3);
  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
  //CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);

  return ((rx_data>>8)&0xFFFF);
}

//you can read the entire memory in a single instruction
//You need to send the read instruction followed by a 24-bit address - memory address
static void W25Q16_Read (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData) {
	uint8_t tData[4];
	uint32_t memAddr = (startPage*256) + offset; //page contains 256 bytes
	
	tData[0] = READ_DATA;	//read enable command (MSB)
	tData [1] = (memAddr>>16)&0xFF;
	tData [2] = (memAddr>>8)&0xFF;
	tData [3] = (memAddr)&0xFF;
  
  //CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(tData, 4);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(rData, size);
  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);

  //CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}

//Fast Read instruction operate at the highest possible frequency (Fr = 50Mhz)
//we need to add 8 dummy clocks after the 24 bits address
static void W25Q16_FastRead (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData) {
	uint8_t tData[5];
	uint32_t memAddr = (startPage*256) + offset;  //page is 256 bytes
	
	tData[0] 	=	READ_FAST;	//read fast enable command
	tData [1] =	(memAddr>>16)&0xFF;
	tData [2] =	(memAddr>>8)&0xFF;
	tData [3] =	(memAddr)&0xFF;
	tData[4] 	= 0;	//dummy clocks
  
    //CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(tData, 5);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(rData, size);
  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);

  //CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}

//write enable intruction, needed for writing and erasing
static void write_enable (void) {
	W25Q16_WriteInstruction(WRITE_ENABLE);
	osDelay(5);
}

//write disable. Needed after a write and erase instruction
static void write_disable (void) {
	W25Q16_WriteInstruction(WRITE_DISABLE);
	osDelay(5);
}

//the minimum space to erase is a sector (4kBytes)
//before erase you need to execute a write enable instruction
static void W25Q16_Erase_64kBlock (uint16_t numBlock) {
	uint8_t tData[4];
	uint32_t memAddr = numBlock*16*16*256;
	
	write_enable();
	
	tData[0] = ERASE_BLOCK;
	tData [1] =	(memAddr>>16)&0xFF;
	tData [2] =	(memAddr>>8)&0xFF;
	tData [3] =	(memAddr)&0xFF;
	
	//CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(tData, 4);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	//CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
	
	osDelay(2000); 	//max time for erase block
	
	write_disable();
}

static void W25Q16_Erase_Sector (uint16_t numSector) {
	uint8_t tData[4];
	uint32_t memAddr = numSector*16*256;
	
	write_enable();
	
	tData[0] = ERASE_SECTOR;
	tData [1] =	(memAddr>>16)&0xFF;
	tData [2] =	(memAddr>>8)&0xFF;
	tData [3] =	(memAddr)&0xFF;
	
	//CS low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
	//send address and data
	SPIdrv->Send(tData, 4);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	//CS high
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
	
	osDelay(450); 	//max time for erase block
	
	write_disable();
}

static uint32_t bytesToWrite (uint32_t size, uint32_t offset) {
	if ((size+offset) < 256) return size;
	else return 256-offset;
}

//you can write max data of a page, but you need to erase the sector first
//this functions erase the entire sector and then write in some pages
//if you exceed the number of bytes in a page, you will overwrite the data from the beggining
static void W25Q16_WritePage_Clean (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data) {
	//256 bytes to write, 4 bytes for the address and 1 byte for instruction
  uint8_t tData[266];
	uint32_t startPage = page;
	uint32_t endPage = startPage + ((size+offset-1)/256);
	uint32_t numPages = endPage-startPage+1;
	
	uint16_t startSector = startPage/16;
	uint16_t endSector = endPage/16;
	uint16_t numSectors = endSector-startSector+1;
	
	for(uint16_t i = 0; i < numSectors ;i++) {
		W25Q16_Erase_Sector(startSector+i);//antes ponia startSector++
	}
	
	uint32_t dataPosition = 0;
  //write the data
	for (uint32_t i = 0; i<numPages; i++) {
		uint32_t memAddr = (startPage*256)+offset;
		uint16_t bytesRemaining = bytesToWrite(size, offset);
		uint32_t indx = 0;
		
		write_enable();
		
		tData[0] = PAGE_PROGRAM;
		tData [1] =	(memAddr>>16)&0xFF;
		tData [2] =	(memAddr>>8)&0xFF;
		tData [3] =	(memAddr)&0xFF;
	
		indx = 4;
	
		uint16_t bytesToSend = bytesRemaining + indx;
		for (uint16_t i = 0; i < bytesRemaining; i++) {
			tData[indx++] = data[i+dataPosition];
		}
		
		if (bytesToSend > 250) {
			//CS low
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
       //send address and data
       SPIdrv->Send(tData, 100);
       osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
       SPIdrv->Send(tData+100, bytesToSend-100);
       osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
      //CS high
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
		} else {
			//CS low
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
       //send address and data
       SPIdrv->Send(tData, bytesToSend);
       osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
       //CS high
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
		}
		startPage++;
		offset = 0;
		size = size-bytesRemaining;
		dataPosition = dataPosition+bytesRemaining;
		
		//page programm max time 3 ms
		osDelay(5);
		write_disable();
	}
}


// Copy the fields of the MSGQUEUE_USER_t structure to the byte array (we need to write bytes)
static void userToBytes(const MSGQUEUE_USER_t *user, uint8_t *user_data) {
    memcpy(user_data, user->id, sizeof(user->id));
    memcpy(user_data + sizeof(user->id), user->name, sizeof(user->name));
    memcpy(user_data + sizeof(user->id) + sizeof(user->name), user->password, sizeof(user->password));
}
// Copy the fields from the byte array to the MSGQUEUE_USER_t structure (we read bytes)
static void bytesToUser(const uint8_t *user_data, MSGQUEUE_USER_t *user) {
    memcpy(user->id, user_data, sizeof(user->id));
    memcpy(user->name, user_data + sizeof(user->id), sizeof(user->name));
    memcpy(user->password, user_data + sizeof(user->id) + sizeof(user->name), sizeof(user->password));
}

/*
Adds a user to the flash memory.
If the user ID already exists, the provided data will be updated
*/
void addUser (MSGQUEUE_USER_t *user) {
	uint8_t user_data[US_SIZE_B];
	uint8_t pos;
	uint8_t numUsers;
	
	numUsers = getNumUsers();
	
	if (numUsers < MAX_USERS | numUsers == 0xFF) {
		if (!(find_user (user->id, &pos))) {// If the user ID is not found in memory -- remember this function updates pos to the position of the user if its found
			pos = getLowestPosUser(); // Find the lowest position to store the user mas baja donde meter el usuario
      numUsers = (numUsers == 0xFF) ? 1 : numUsers + 1;
			
		}
	}
	
	//write user information
	userToBytes (user, user_data);
	W25Q16_WritePage_Clean (pos*PAGES_FOR_SECTOR, 0, US_SIZE_B, user_data);
	//update number of users
	W25Q16_WritePage_Clean (0, 0, US_COUNT_SIZE_B, &numUsers);
}

/**
  * @brief  Retrieves user data from memory based on user ID.
  * @param  id: User ID to retrieve.
  * @retval None
  */
static void getUser (uint8_t *id) {
	uint8_t pos;
	MSGQUEUE_USER_t user;
	uint8_t user_data[US_SIZE_B];
	
	if (find_user (id, &pos)) {
		W25Q16_FastRead(pos*PAGES_FOR_SECTOR, 0, US_SIZE_B, user_data);	//read the user data
		bytesToUser(user_data, &user);//convert bytes to user structure
		user.found=1;
		osMessageQueuePut(mid_MsgQueue_tx, &user, NULL, 0U);
		user.found=0;
	}else{
		user.found=0;
		osMessageQueuePut(mid_MsgQueue_tx, &user, NULL, 0U);
	 
	}
}

/*
    Searches for a user ID in the flash memory.
    If found, returns the page and position of the user.
    Returns true if the ID is found, otherwise false.
*/
static bool find_user (uint8_t *id, uint8_t *pos) {
	uint8_t user_data[US_SIZE_B];// Read one user
	uint8_t numUsersProccessed;
	uint8_t numUsers;
	uint8_t usr;
	
	numUsers = getNumUsers();
	numUsersProccessed = 0;
  
	for (usr = 1; usr <= MAX_USERS ; usr++) {
		W25Q16_FastRead (usr*PAGES_FOR_SECTOR, 0, US_SIZE_B, user_data);
		if (memcmp (user_data, id, 5) != 0) {	//if they are different
			numUsersProccessed++;
		}	else {
			*pos = usr;
			return true;
		}			
		if (numUsersProccessed == numUsers) {
			break;
		}
	}
	return false;
}

/*
    Deletes a user from the flash memory based on the provided user ID.
    If the ID is found, the corresponding user data is overwritten with empty data.
*/
void deleteUser (uint8_t *id) {
	uint8_t pos;
	uint8_t empty_user[US_SIZE_B]; // Empty user data
	memset(empty_user, 0xFF, US_SIZE_B);  // Fill the empty user data with 0xFF
	uint8_t numUsers;
	
	if (find_user (id, &pos)) { // If the user ID is found
  // Overwrite the user data with empty data
	W25Q16_WritePage_Clean (pos*PAGES_FOR_SECTOR, 0, US_SIZE_B , empty_user); //you can write all to FF o best just erase the entire sector
    
	numUsers = getNumUsers();
	numUsers = (numUsers - 1);
  // Update the number of users in the flash memory page
	W25Q16_WritePage_Clean (0, 0, US_COUNT_SIZE_B , &numUsers);
	}
}

/*
    Retrieves all users from the flash memory.
    Reads multiple users at once for efficiency.
    Puts each user into the message queue for further processing.
*/
void getAllUsers (void) {
	uint8_t user_data[US_SIZE_B];
	MSGQUEUE_USERS_t users;
	uint8_t numUsers;
	uint8_t numUsersProccessed;
	uint8_t empty_user[US_SIZE_B];  //empty user data
	memset(empty_user, 0xFF, US_SIZE_B);

	numUsers = getNumUsers();
	numUsersProccessed = 0;
	//user.numUsers = numUsers;
	
	for (uint8_t usr = 1; usr <= MAX_USERS; usr ++) {
		W25Q16_FastRead (usr*PAGES_FOR_SECTOR, 0, US_SIZE_B, user_data);
		if (memcmp (user_data, empty_user, US_SIZE_B) != 0) {	//if they are different
			bytesToUser (user_data, &users.us[numUsersProccessed]);
			numUsersProccessed++;
		}
		
		if (numUsersProccessed == numUsers) {
			osMessageQueuePut (mid_MsgQueue_txBIG, &users, NULL, 0U);
			break;
		}
	}
}

//Returns the number of users within the flash memory.
static uint8_t getNumUsers(void) {
	uint8_t numUsers;
  // Read the number of users from the specified page in flash memory
	W25Q16_Read (0, 0, US_COUNT_SIZE_B, &numUsers);
	return numUsers;
}

/*
    Function that returns the lowest position where there is space for a new user to enter.
    Parameters:
        - page: The page where the function is applied.
    
    It reads all possible users within the page and then checks if there is any empty slot.
*/
static uint8_t getLowestPosUser (void) {
	uint8_t posicion = 0;
	uint8_t user_data[US_SIZE_B];
	uint8_t empty_user[US_SIZE_B];
	memset(empty_user, 0xFF, US_SIZE_B);
	
	for (uint8_t usr = 1; usr <= MAX_USERS; usr ++) {
		W25Q16_FastRead (usr*PAGES_FOR_SECTOR, 0, US_SIZE_B, user_data);
		if (memcmp (user_data, empty_user, US_SIZE_B) == 0) {	//si son iguales, es decir esta vacio
			posicion = usr;
      return posicion;
		}
	}
}


void W25Q16_PoerDown (void) {
  W25Q16_WriteInstruction(POWER_DOWN);
  osDelay (5);
}

void W25Q16_PowerUp(void) {
  W25Q16_WriteInstruction(POWER_UP);
  osDelay (5);
}

//HACER FUNCION QUE SOLO TE SAQUE 5 USUARIOS POR LAS COLAS


/* POINTER USAGE SUMMARY FOR UNDERSTANDING THE CODE
int *ptr;
int x = 10;

ptr = &x; assigns the memory address of x to the pointer, now the pointer points to where x is located
So if we look at (*ptr), the content of the pointer, it equals 10

*ptr = 20; Assigns the value of 20 to the content of the pointer.
Consequently, now x is worth 20.

*/

#ifndef __SERVER_H
#define __SERVER_H

#include "cmsis_os2.h" 
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "main.h"

#define AUTH_SUCCESS 0x00000001U
#define AUTH_FAILURE 0x00000010U

typedef struct {                               
	uint8_t password_user[4];
	uint8_t id[5];
	char hexStringLocal[20];
	uint8_t estado[5];
} User_manag_t;




#endif

/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V7.0.0
 *----------------------------------------------------------------------------*/
//! [code_HTTP_Server_CGI]
#include "HTTP_Server_CGI.h"

// TODO: move the auth vars to an authentication package with the flash read and other related utils

char time_1[24];
char date[24];
float system_consumption;

//extern osEventFlagsId_t auth_event_id;

//extern osMessageQueueId_t bridge_Q;
//extern osMessageQueueId_t auth_Q;


static uint8_t open_bridge;
static uint8_t open_bridge_last_status;
//static credentials_t credentials;
static uint8_t auth_done = 0;
static uint8_t user_sent = 0;
static uint8_t auth_success = 0;
static uint8_t barrier = 0;
static uint8_t emergency_stop = 0;
//static uint8_t warning_boat = 0;
uint8_t first_load=1;
uint8_t bajo_consumo=0;

extern RTC_HandleTypeDef RtcHandle;
extern RTC_TimeTypeDef stimestructureget;
extern RTC_DateTypeDef sdatestructureget;
static User_manag_t user_stat;


uint8_t hexCharToByte(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return 0; // car�cter inv�lido
}

// \brief Process query string received by GET request.
// \param[in]     qstr          pointer to the query string.
// \return        none.
void netCGI_ProcessQuery (const char *qstr) {
  char var[40];
 
  do {
    qstr = netCGI_GetEnvVar (qstr, var, sizeof (var));
    if (var[0] != 0) {

    }
  } while (qstr);
}
 
// \brief Process data received by POST request.
// \param[in]     code          callback context:
//                              - 0 = www-url-encoded form data,
//                              - 1 = filename for file upload (null-terminated string),
//                              - 2 = file upload raw data,
//                              - 3 = end of file upload (file close requested),
//                              - 4 = any other type of POST data (single or last stream),
//                              - 5 = the same as 4, but with more data to follow.
// \param[in]     data          pointer to POST data.
// \param[in]     len           length of POST data.
// \return        none.
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) {
  char var[40];
	User_manag_t user_data;
  uint32_t auth_error;
 
  if (code != 0) {
    // Ignore all other codes
    return;
  }
  do {
    data = netCGI_GetEnvVar (data, var, sizeof (var));
    if (var[0] != 0) {
      // Authentication process.
//      // TODO: logout web option and process
//      if (strncmp (var, "user=", 5) == 0) {
//        strncpy (credentials.user, var+5,19);
//        auth_done = 1;
//      } else if (strncmp (var, "passwd=", 7) == 0) {
//				const char *str = var + 7;
//				for (int i = 0; i < 4; i++) {
//					if (str[i] >= '0' && str[i] <= '9') {
//							credentials.password[i] = str[i] - '0'; 
//					}
//				}
//        auth_done = 1;
//      }
			if (strncmp (var, "nombre=", 7) == 0) {
				strncpy(user_data.hexStringLocal, var + 7, 18);  // Copy up to 18 chars
        user_data.hexStringLocal[18] = '\0';  
        
      } else if (strncmp (var, "pass=", 5) == 0) {
				const char *str = var + 5;
				  if (str == NULL || str[0] == '\0') {
						
						memset(user_data.password_user,0xFF, sizeof(user_data.password_user));
				  }else{
						for (int i = 0; i < 4; i++) {
								if (str[i] >= '0' && str[i] <= '9') {
										user_data.password_user[i] = str[i] - '0'; 
								}
						}
					}
        user_sent=1;

      } else if (strncmp (var, "id=", 3) == 0) {
				char *hexString = var + 3;
				for (size_t i = 0; i < 5; i++) {
					char high = hexString[i * 2];
					char low = hexString[i * 2 + 1];

					if (!isxdigit(high) || !isxdigit(low)) break;

					user_data.id[i] = (hexCharToByte(high) << 4) | hexCharToByte(low);
				}
			
      } else if (strncmp(var, "barrier=", 8) == 0) {
        barrier = atoi(var+8);
				if(barrier==1 && Boat()){
					barrier=0;
				  osThreadFlagsSet(getThIDPrinc1(),SUBIR);
				
				} else if (strncmp(var, "emergencyStop=", 14) == 0) {
           emergency_stop = 1;
					 osThreadFlagsSet(getThIDPrinc1(),EMERGENCY);
        }
      }
    }
  } while (data);
	
	//---------------ENVIO DATOS USUARIO--------------//
  if(user_sent==1){
    osMessageQueuePut(getMessageWebID() , &user_data, 0U, 0U);
    osDelay(1000);
    user_stat=user_status();
   // osMessageQueueGet(getMessageWebID(),&user_data,NULL,osWaitForever);
    user_sent=0;
  }

  
}
 
// \brief Generate dynamic web data based on a CGI script.
// \param[in]     env           environment string.
// \param[out]    buf           output data buffer.
// \param[in]     buf_len       size of output buffer (from 536 to 1440 bytes).
// \param[in,out] pcgi          pointer to a session's local buffer of 4 bytes.
//                              - 1st call = cleared to 0,
//                              - 2nd call = not altered by the system,
//                              - 3rd call = not altered by the system, etc.
// \return        number of bytes written to output buffer.
//                - return len | (1U<<31) = repeat flag, the system calls this function
//                                          again for the same script line.
//                - return len | (1U<<30) = force transmit flag, the system transmits
//                                          current packet immediately.
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buf_len, uint32_t *pcgi) {
  uint32_t len = 0;

  // Analyze a 'c' script line starting position 2
  switch (env[0]) {
    case 'r': // Barrier
      len = (uint32_t)sprintf (buf, &env[2], barrier);
      break;
		case 'i': // Time
			HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
			sprintf(time_1, "%02d:%02d:%02d", stimestructureget.Hours+1, stimestructureget.Minutes, stimestructureget.Seconds);
			len = (uint32_t)sprintf (buf, &env[1], time_1);
			break;
		case 'k': // Date
      HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
      sprintf(date, "%.2d-%.2d-%.2d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
			len = (uint32_t)sprintf (buf, &env[1], date);
			break;
    case 's': // Emergency stop
      len = (uint32_t)sprintf (buf, &env[2], emergency_stop);
//		  if(emergency_stop==1){
//				emergency_stop=0;
//				
//			}
				
      break;
    case 'w': // Warning boat
      len = (uint32_t)sprintf (buf, &env[2], Boat());
		  
      break;
    case 'a':
      if(first_load==1){
        user_stat=user_status();
        first_load=0;
        //osMessageQueueGet(getMessageWebID(), &user_data, NULL, osWaitForever);
      }
		
			if(user_stat.estado[0]==1){
				len = (uint32_t)sprintf (buf, &env[1],"Busy");
			}else{
				len = (uint32_t)sprintf (buf, &env[1],"Available");
			}
      
      break;
    case 'f':
			
      if(user_stat.estado[1]==1){
				len = (uint32_t)sprintf (buf, &env[1],"Busy");
			}else{
				len = (uint32_t)sprintf (buf, &env[1],"Available");
			}
      break;
    case 'd':
			
      if(user_stat.estado[2]==1){
				len = (uint32_t)sprintf (buf, &env[1],"Busy");
			}else{
				len = (uint32_t)sprintf (buf, &env[1],"Available");
			}
      break;
    case 'e':
			
      if(user_stat.estado[3]==1){
				len = (uint32_t)sprintf (buf, &env[1],"Busy");
			}else{
				len = (uint32_t)sprintf (buf, &env[1],"Available");
			}
      break;
    case 'g':
		
      if(user_stat.estado[4]==1){
				len = (uint32_t)sprintf (buf, &env[1],"Busy");
			}else{
				len = (uint32_t)sprintf (buf, &env[1],"Available");
			}

      break;
		case 'l':
      system_consumption=(ADC_getValue(1)/(10*0.33));
			len = (uint32_t)sprintf (buf, &env[1], system_consumption);
      break;
		case 'n':
			if(bajo_consumo==0){
				//activar bajo consumo
				bajo_consumo=1;
			}
			len = (uint32_t)sprintf (buf, &env[1], time_1);
      break;
		case 'p':
			if(bajo_consumo==1){
				//desactivar bajo consumo
				bajo_consumo=0;
			}
			len = (uint32_t)sprintf (buf, &env[1], time_1);
      break;
		
  }
  return (len);
}

/// \brief Redirect resource URL address to a new location. [\ref user-provided]
/// \param[in]     file_name     resource filename, a null-terminated string.
/// \return      URL redirection information:
///                - pointer to user defined Location.
///                - NULL for no URL address redirection.
const char *netCGI_Redirect (const char *file_name) {
  if (auth_success) {
		auth_success=0;
    return ("/functionalities.html");
  }
  return NULL;
}


//! [code_HTTP_Server_CGI]

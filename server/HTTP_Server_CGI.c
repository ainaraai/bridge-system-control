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

extern osEventFlagsId_t auth_event_id;

extern osMessageQueueId_t bridge_Q;
extern osMessageQueueId_t auth_Q;

extern char lcd_text[2][20+1];
extern osThreadId_t TID_Lcd;

static uint8_t open_bridge;
static uint8_t open_bridge_last_status;
static credentials_t credentials;
static uint8_t auth_done = 0;
static uint8_t auth_success = 0;


uint8_t hexCharToByte(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return 0; // carácter inválido
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
      // TODO: logout web option and process
      if (strncmp (var, "user=", 5) == 0) {
        strcpy (credentials.user, var+5);
        auth_done = 1;
      } else if (strncmp (var, "passwd=", 7) == 0) {
        strcpy (credentials.password, var+7);
        auth_done = 1;
      } else if (strncmp (var, "nombre=", 7) == 0) {
				strncpy(user_data.hexStringLocal, var + 7, 19);
        printf("------------------------------\nEl usuario registrado es: %s\n",user_data.hexStringLocal);
      }
			else if (strncmp (var, "pass=", 5) == 0) {
				const char *str = var + 5;
				  if (str == NULL || str[0] == '\0') {
						printf("borrando password");
						memset(user_data.password_user,0xFF, sizeof(user_data.password_user));
				  }else{
						for (int i = 0; i < 4; i++) {
								if (str[i] >= '0' && str[i] <= '9') {
										user_data.password_user[i] = str[i] - '0'; 
								}
						}
					}


				printf("Password como uint8_t: ");
				for (int i = 0; i < 4; i++) {
						printf("%d ", user_data.password_user[i]);
				}
				printf("\n");

      }
			 else if (strncmp (var, "id=", 3) == 0) {
				char *hexString = var + 3;
				for (size_t i = 0; i < 5; i++) {
					char high = hexString[i * 2];
					char low = hexString[i * 2 + 1];

					if (!isxdigit(high) || !isxdigit(low)) break;

					user_data.id[i] = (hexCharToByte(high) << 4) | hexCharToByte(low);
				}
				for (int i = 0; i < 5; i++) {
					printf("%02X\n ", user_data.id[i]);  // solo imprime los números en hexadecimal
				}
      }
    }
  } while (data);
	
	//---------------ENVIO DATOS USUARIO--------------//
	//osMessageQueuePut("NOMBRE COLA DEL PRINCIPAL_SERVER" , &user_data, 0U, 0U);
	
  if (auth_done) {
    osMessageQueuePut(auth_Q, &credentials, NULL, 0);
    auth_error = osEventFlagsWait(auth_event_id, AUTH_SUCCESS|AUTH_FAILURE, osFlagsWaitAny, 5000); // wait 5s max to check the user
    if (auth_error == AUTH_SUCCESS) {
      auth_success = 1;
      //netCGI_Redirect("estadisticas_generales.html");
    } else if (auth_error == AUTH_FAILURE) {
      auth_success = 0;
      // TODO
    } else if (auth_error == osFlagsErrorTimeout) {
      // TODO
    } else { // other errors
      // TODO
    }
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
    case 'b':
      osMessageQueueGet(bridge_Q, &open_bridge, NULL, 0U);   // wait for message
      if (open_bridge_last_status != open_bridge) {
        open_bridge_last_status = open_bridge;
      }
      len = (uint32_t)sprintf (buf, &env[2], open_bridge_last_status);
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

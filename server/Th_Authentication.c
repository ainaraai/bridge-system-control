#include "main.h"
#include "string.h"

/*----------------------------------------------------------------------------
 *      Thread 'Authentication': Tests the authentication process without the 
 *                               flash storage
 *---------------------------------------------------------------------------*/
 
 
extern osMessageQueueId_t auth_Q;

static credentials_t credentials;

// TODO: move the auth vars to an authentication package with the flash read and other related utils
#define AUTH_SUCCESS 0x00000001U
#define AUTH_FAILURE 0x00000010U
extern osEventFlagsId_t auth_event_id;


void Th_authentication (void *argument) {
const uint8_t expected[] = {0x01, 0x02, 0x03, 0x04};
  while (1) {
    osMessageQueueGet(auth_Q, &credentials, NULL, osWaitForever);
    // For this test, the credentials are hardcoded
    if (strncmp(credentials.user, "admin", 5) == 0 && 
        (memcmp(credentials.password, expected, 4) == 0)){
      osEventFlagsSet(auth_event_id, AUTH_SUCCESS);
    } else {
      osEventFlagsSet(auth_event_id, AUTH_FAILURE);
    }
  }
}
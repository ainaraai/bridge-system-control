#include "cmsis_os2.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      THIS THREAD TESTS THE BRIDGE VIA SOFTWARE
 *---------------------------------------------------------------------------*/
extern osMessageQueueId_t bridge_Q;

void Th_bridge (void *argument) {
  uint8_t open_bridge;
  
  osDelay(7000);
  while (1) {
    osDelay(5000);
    open_bridge = 0;
    osMessageQueuePut(bridge_Q, &open_bridge, 0U, 0U);
    osDelay(5000);
    open_bridge = 1;
    osMessageQueuePut(bridge_Q, &open_bridge, 0U, 0U);
  }
}

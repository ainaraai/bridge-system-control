#include "main.h"

#include <stdio.h>
#include <time.h>

static void time_callback (uint32_t seconds, uint32_t seconds_fraction);
            
struct tm now;

static void get_time (void) {
  const NET_ADDR4 ntp_server1 = { NET_ADDR_IP4, 0, 130, 206, 3, 166 }; // hora.rediris.es (no me ha funcionado tampoco)
	const NET_ADDR4 ntp_server2 = { NET_ADDR_IP4, 0, 150, 214, 94, 5 }; // hora.roa.es
  
  if (netSNTPc_GetTime ((NET_ADDR*)&ntp_server2, time_callback) == netOK) { // Null para coger el server desde la config (no funciona)
//    printf ("SNTP request sent.\n");
  } 
  else {
//    printf ("SNTP not ready or bad parameters.\n");
  }
}

static void time_callback (uint32_t seconds, uint32_t seconds_fraction) {
  if (seconds == 0) {
//    printf ("Server not responding or bad response.\n");
  }
  else {
//    printf ("%d seconds elapsed since 1.1.1970\n", seconds);
	  now = *localtime(&seconds);
		RTC_SetTime(now.tm_hour+2,now.tm_min,now.tm_sec,0,now.tm_isdst,0); //GMT+2
		RTC_SetDate(now.tm_mday,now.tm_mon+1,now.tm_year+1900,now.tm_wday);
  }
}

__NO_RETURN void TH_SNTP (void *arg) {
  while(1){
    get_time();
    osDelay(180000);
  }
}

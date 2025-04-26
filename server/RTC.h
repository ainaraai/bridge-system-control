#ifndef __RTC_H
#define __RTC_H

#include <stdint.h>

  void RTC_init(void);
  void RTC_CalendarConfig(void);
  void RTC_CalendarInfo(uint8_t *time_buffer, uint8_t *date_buffer);
  void RTC_AlarmConfig(void);
  void RTC_SetDate (uint8_t day, uint8_t month, uint16_t year, uint8_t weekday);
  void RTC_SetTime (uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t AM_PM, uint8_t DST, uint8_t set);
  void RTC_GetTime(uint8_t *time_buffer);


#endif

#ifndef __RTC_H
#define __RTC_H
#include "stm32f10x.h"

extern __IO uint32_t TimeDisplay;

typedef struct 
{
	uint16_t  year;
	uint8_t   month;
	uint8_t   day;
	uint8_t   week;
	uint8_t   huor;
	uint8_t   min;
	uint8_t   sec;
}RTC_TIME;

extern  RTC_TIME rtc_tiemr;

void NVIC_Configuration(void);
void RTC_Configuration(void);
void Time_Display(void);
void rtc_Init(void);

uint8_t RTC_Get(void);
uint8_t Is_Leap_Year(uint16_t year);
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day);
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);

void printf_RTC_clock(void);
void RTC_Time_Base(void);
void Random_Password(void);
#endif

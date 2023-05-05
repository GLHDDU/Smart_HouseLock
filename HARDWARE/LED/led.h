#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LED0 PAout(4)// PA4
#define Big_lamp PAout(1)// PA1	

void LED_Init(void);//≥ı ºªØ

void lamp_open(void);
void lamp_on_status(void);
void lamp_off_status(void); 

#endif

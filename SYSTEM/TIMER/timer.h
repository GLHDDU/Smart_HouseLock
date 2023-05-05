#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
 
extern int DHT11_time;
 
void TIMER1_Init(u16 per,u16 psc);
void TIM2_Init(u16 pre,u16 psc);       //USART3的接收中断二次触发的中断

void TIM4_ENABLE_30S(void);
void TIM4_ENABLE_2S(void);


void TIM1_UP_IRQHandler(void);
void TIM4_IRQHandler(void);
#endif

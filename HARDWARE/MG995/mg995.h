#ifndef __MG995_H
#define __MG995_H
#include "sys.h"

void TIM3_PWM_Init(u16 arr,u16 psc);   //pwm控制舵机
void exti_MG995(void);
void smart_door(void);
void door_ON(void);
void door_OFF(void);
#endif

#ifndef __HCSR04_H
#define __HCSR04_H

#include "sys.h"

#define Echo PBin(8) 	    //HC-SR04ģ���Echo�Ž�GPIOB8
#define Trig PBout(9)		//HC-SR04ģ���Trig�Ž�GPIOB9



extern int Distance_m;
extern int Distance_m_p;
void HC_SR04_Init(void);
int get_distance(void);
void printf_HCSR_04_dis(void);    //��ӡ���������--����
#endif

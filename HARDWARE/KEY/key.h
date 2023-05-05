#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define KEY0_PIN    			GPIO_Pin_0    	//����KEY0�ܽ�
#define KEY1_PIN    		    GPIO_Pin_1    	//����KEY1�ܽ�
#define KEY_diy1_PIN            GPIO_Pin_4    	//����KEY_diy1�ܽ�
#define KEY_diy2_PIN   		    GPIO_Pin_3  	//����KEY_diy2�ܽ�
#define KEY_diy3_PIN     	    GPIO_Pin_15    	//����KEY_diy3�ܽ�
#define KEY_diy4_PIN   		    GPIO_Pin_11  	//����KEY_diy4�ܽ�


#define  KEY_B_PORT 			GPIOB 		//����˿�
#define  KEY_A_PORT 		    GPIOA 		//����˿�

////�����������ֵ  
#define KEY_diy1_PRESS 	    1
#define KEY_diy2_PRESS		2
#define KEY_diy3_PRESS		3
#define KEY_diy4_PRESS		4

#define KEY0_PRES 	1	//KEY0����
#define KEY1_PRES	2	//KEY1����

#define KEY0  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)      //��ȡ����KEY0
#define KEY1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)      //��ȡ����KEY1
#define KEY_diy1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)  //��ȡ����KEY_diy1
#define KEY_diy2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)  //��ȡ����KEY_diy2
#define KEY_diy3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15)  //��ȡ����KEY_diy3
#define KEY_diy4  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)  //��ȡ����KEY_diy4


void KEY_Init(void);//IO��ʼ��
u8 KEY_Scan(void);

void Manual_Operation_Face_input(void);
void Manual_Operation_Face_deletion(void);
u8 key_scan(u8 mode);
#endif

#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define KEY0_PIN    			GPIO_Pin_0    	//定义KEY0管脚
#define KEY1_PIN    		    GPIO_Pin_1    	//定义KEY1管脚
#define KEY_diy1_PIN            GPIO_Pin_4    	//定义KEY_diy1管脚
#define KEY_diy2_PIN   		    GPIO_Pin_3  	//定义KEY_diy2管脚
#define KEY_diy3_PIN     	    GPIO_Pin_15    	//定义KEY_diy3管脚
#define KEY_diy4_PIN   		    GPIO_Pin_11  	//定义KEY_diy4管脚


#define  KEY_B_PORT 			GPIOB 		//定义端口
#define  KEY_A_PORT 		    GPIOA 		//定义端口

////定义各个按键值  
#define KEY_diy1_PRESS 	    1
#define KEY_diy2_PRESS		2
#define KEY_diy3_PRESS		3
#define KEY_diy4_PRESS		4

#define KEY0_PRES 	1	//KEY0按下
#define KEY1_PRES	2	//KEY1按下

#define KEY0  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)      //读取按键KEY0
#define KEY1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)      //读取按键KEY1
#define KEY_diy1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)  //读取按键KEY_diy1
#define KEY_diy2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)  //读取按键KEY_diy2
#define KEY_diy3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15)  //读取按键KEY_diy3
#define KEY_diy4  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)  //读取按键KEY_diy4


void KEY_Init(void);//IO初始化
u8 KEY_Scan(void);

void Manual_Operation_Face_input(void);
void Manual_Operation_Face_deletion(void);
u8 key_scan(u8 mode);
#endif

#include "exti.h"
#include "led.h"
#include "key.h"
#include "usart.h"	 
#include "delay.h"
#include "beep.h"
u8 exti1=0;//外部中断1触发标志
 
 
//外部中断初始化函数
void EXTIX_Init(void)
{
 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//外部中断，需要使能AFIO时钟

	KEY_Init();//初始化按键对应io模式

	
    //GPIOB.0 中断线以及中断初始化配置
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//使能按键KEY0所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
		
}

 //外部中断服务程序---KYE0
void EXTI0_IRQHandler(void)
{
	if(KEY0==1)
	{
	  delay_ms(50);    //消抖
		if(KEY0==1)
		{	
//			printf("KEY0\n");
			exti1=1;
		}
		EXTI_ClearITPendingBit(EXTI_Line0);  //清除LINE0线路挂起位
    }
}




#include "led.h"
#include "timer.h"
#include "sys.h"
#include "usart2.h" 
extern  int lamp_falg;
extern  int lamp_time;
//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能PA端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_1;				 //LED0-->PA.4 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.4
 GPIO_SetBits(GPIOA,GPIO_Pin_4);						 //PA.4输出高
 GPIO_ResetBits(GPIOA,GPIO_Pin_1);						 //PA1 输出低
}

void lamp_open(void)
{
	if(lamp_falg==1)
	{
		if(lamp_time>0)
		  Big_lamp=1;
		else
		  Big_lamp=0;
	}

}

 void lamp_on_status(void)
{
	Big_lamp=1;
	usart2_send_string("Biglamp is ON*");	
}
 void lamp_off_status(void)
{
	Big_lamp=0;
	usart2_send_string("Biglamp is OFF*");	
}

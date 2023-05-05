#include "hcsr04.h"
#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"



extern int HC_distance_time;
int Distance_cm;
int Distance_m;
int Distance_m_p;

void HC_SR04_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;					//定义结构体
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	//启用GPIOB的外设时钟	
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//设置GPIO口为推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				//设置GPIO口9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//设置GPIO口速度50Mhz
	GPIO_Init(GPIOB,&GPIO_InitStructure);					//初始化GPIOB
	Trig=0;				                                    //输出低电平
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//设置GPIO口为输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				//设置GPIO口8
	GPIO_Init(GPIOB,&GPIO_InitStructure);					//初始化GPIOB
	
}

int get_distance(void)									//测距并返回单位为毫米的距离结果
{
	u32 retry=0;
	
	  //Trig发送>10us高脉冲
	  Trig=1;
	  delay_us(15);
	  Trig=0;
	
	//等待Echo变高，等待60ms还没有变高，直接返回-1
	while(Echo==0)
	{
		retry++;
		delay_us(10);
		if(retry>6000)
			return -1;
	}
	
	retry=0;
	//等待Echo变低，计算高电平时间
	while(Echo==1)
	{
		retry++;
		delay_us(10);
		if(retry>6000)
			return -1;
	}
	
	return retry*10/58;
}


void printf_HCSR_04_dis(void)
{

		 Distance_cm=get_distance();	//获取距离测量结果，单位厘米（cm）		
		 Distance_m=Distance_cm/100;    //转换为米（m）为单位，将整数部分放入Distance_m
		 Distance_m_p=Distance_cm%100;	//转换为米（m）为单位，将小数部分放入Distance_m_p
	
	if(HC_distance_time>=5)
	{
//	printf("distance_cm : %d \n",Distance_cm);
	//printf("distance_m : %d.%d \n",Distance_m,Distance_m_p);
		HC_distance_time=0;
	}
}

#include "led.h"
#include "timer.h"
#include "sys.h"
#include "usart2.h" 
extern  int lamp_falg;
extern  int lamp_time;
//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ʹ��PA�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_1;				 //LED0-->PA.4 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.4
 GPIO_SetBits(GPIOA,GPIO_Pin_4);						 //PA.4�����
 GPIO_ResetBits(GPIOA,GPIO_Pin_1);						 //PA1 �����
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

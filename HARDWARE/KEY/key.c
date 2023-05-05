#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "usart.h"
#include "usart2.h" 								    
//������ʼ������
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);//ʹ��PORTA,Bʱ��	
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);  //ʹ��PA15��PB3��4

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1;//KEY0,1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(KEY_B_PORT, &GPIO_InitStructure);//��ʼ��GPIOB0,1

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15|GPIO_Pin_11;//KEY_diy3-KEY_diy4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(KEY_A_PORT, &GPIO_InitStructure);//��ʼ��GPIOA15,11
	
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4|GPIO_Pin_3;//KEY_diy1-KEY_diy2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(KEY_B_PORT, &GPIO_InitStructure);//��ʼ��GPIOB4,3
}

/*******************************************************************************
* �� �� ��         : KEY_Scan
* ��������		   : ����ɨ����

* ��    ��         : 0��δ�а�������
					 KEY_diy1_PRESS��KEY_diy1������
					 KEY_diy2_PRESS��KEY_diy2������
					 KEY_diy3_PRESS��KEY_diy3������
					 KEY_diy4_PRESS��KEY_diy4������
*******************************************************************************/
u8 KEY_Scan(void)
{
	static u8 Flag=0;
	if((KEY_diy1==0||KEY_diy2==0||KEY_diy3==0||KEY_diy4==0)&&Flag==0) //����һ����������
	{
		Flag=1;
		delay_ms(15);  //����;
		if(KEY_diy1==0)
			{
			//printf("KEY_diy1\r\n");
			return 1; 
			}
			
		else if(KEY_diy2==0)
			{
			//printf("KEY_diy2\r\n");
			return 2; 
			}
			
		else if(KEY_diy3==0)
			{
			//printf("KEY_diy3\r\n");
			return 3; 
			}
			
		else if(KEY_diy4==0)
			{
			//printf("KEY_diy4\r\n");
			return 4; 
			}
	}
	if(KEY_diy1==1&&KEY_diy2==1&&KEY_diy3==1&&KEY_diy4==1&&Flag==1)    //����Ƿ��ɿ�
		Flag=0;
	return 0;
}

vu8 key=0;
void Manual_Operation_Face_input(void)
{
	key=key_scan(0);	//�õ���ֵ
	   	if(key)
		{						   
			switch(key)
			{				 
				case KEY0_PRES:	//¼��xiaoming
					usart1_send_string("luxiaoming");
				    usart2_send_string("Face input xiaoming*\r\n");  //���Է���
					break;
				case KEY1_PRES:	//ɾ������	 
					usart1_send_string("all");
				    usart2_send_string("delete face*\r\n");  //���Է���
					break;
			}
		}
}

void Manual_Operation_Face_deletion(void)
{
	key=key_scan(0);	//�õ���ֵ
	   	if(key)
		{						   
			switch(key)
			{				 
				case KEY0_PRES:	//ɾ������
					usart1_send_string("all");
				    usart2_send_string("delete face*\r\n");  //���Է���
					break;
				case KEY1_PRES:	//ɾ������	 
					usart1_send_string("all");
					usart2_send_string("delete face*\r\n");  //���Է���
					break;
			}
		}
}


u8 key_scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0||KEY1==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY0==0)return KEY0_PRES;
		else if(KEY1==0)return KEY1_PRES;
	}else if(KEY0==1&&KEY1==1)key_up=1; 	    
 	return 0;// �ް�������
}

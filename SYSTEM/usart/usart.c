#include "sys.h"
#include "usart.h"	 
#include "usart2.h" 
#include "mg995.h"
#include "beep.h"
#include "led.h"
#include "string.h"
#include "delay.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 
 
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART1_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u16 USART1_RX_STA=0;                 //����״̬���	  1---���� 
u32 USART1_cnt=0 ;                   //��¼����1�յ������ݳ���	 
  
void uart_init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9

	//USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}

void USART1_IRQHandler(void)                	//����1�жϷ������
{
if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE); //��������жϱ�־λ
		
   		USART1_RX_BUF[USART1_cnt++]=USART_ReceiveData(USART1);  //���뻺����
			
		//�ж������Ƿ������ɣ�������*������
			if(USART1_RX_BUF[USART1_cnt-1]=='*' || USART1_cnt>=sizeof(USART1_RX_BUF))
			{
				USART1_RX_STA=1;
			}
     } 
} 

//�������յ�������
void parse_cmd(void)
{
	if(USART1_RX_STA) //���ݽ�����ɲŴ���
	{		
			if( strstr( (char*)USART1_RX_BUF,"succeed") )
			{
				//����ʶ��ɹ�-xiaoming
				if( strstr( (char*)USART1_RX_BUF,"xiaoming") )
				{
					door_ON();
					usart1_send_string("send MG995 is ON xiaoming\r\n");
					usart2_send_string("OPENxiaoming*\r\n");
				}
				//����ʶ��ɹ�-xiaomei
			   if( strstr( (char*)USART1_RX_BUF,"xiaomei") )
				{
					door_ON();
					usart1_send_string("send MG995 is ON xiaomei\r\n");
					usart2_send_string("OPENxiaomei*\r\n");
				}
					
			}
			//����ʶ��ʧ��
		   if( strstr( (char*)USART1_RX_BUF,"fail") )
			{
				BEEP=0;
				usart2_send_string("fail of face recognition*\r\n");
				delay_ms(500);
				BEEP=1;
			}
				
		USART1_RX_STA=0;   //������ɺ������־λ
		//��ջ�����
		memset((void*)USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
		USART1_cnt=0;
	}
}


void usart1_send_byte(char byte)
{
		/* ����һ���ֽ����ݵ�USART */
	USART_SendData(USART1,byte);
		
	/* �ȴ��������ݼĴ���Ϊ�� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	
}

void usart1_send_string(const char *str)
{
		unsigned int k=0;
  do 
  {
      usart1_send_byte( *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* �ȴ�������� */
  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET)
  {}
}

#include "usart2.h" 
#include "usart.h"	
#include "stdio.h"	 	 
#include "string.h"	 
#include "beep.h"
#include "mg995.h"
#include "led.h"
#include "delay.h"

//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART2_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u16 USART2_RX_STA=0;                 //����״̬���	  1---���� 
u32 USART2_cnt=0 ;                   //��¼����1�յ������ݳ���	 

char temp_password[16];  // ������ʱ��������
extern unsigned char Temp_Password[4];
void USART2_IRQHandler(void)
{     
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//�ж��ǲ��ǽ����ж�
	{	 
		//����жϱ�־λ
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		
		//���뻺����
		USART2_RX_BUF[USART2_cnt++]=USART_ReceiveData(USART2);	

			//�ж������Ƿ������ɣ�������*������
			if(USART2_RX_BUF[USART2_cnt-1]=='*' || USART2_cnt>=sizeof(USART2_RX_BUF))
			{
				USART2_RX_STA=1;
			}
	}  				 											 
}   


//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void USART2_Init(u32 bound)
{  
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);   //����2ʱ��ʹ��

 	USART_DeInit(USART2);  //��λ����3
	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
   
    //USART2_RX	  PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3
	
	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART2, &USART_InitStructure); //��ʼ������	2

	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
	
	//ʹ�ܽ����ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�   
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART2_RX_STA=0;		//����
}

void usart2_send_byte(char byte)
{
		/* ����һ���ֽ����ݵ�USART */
	USART_SendData(USART2,byte);
		
	/* �ȴ��������ݼĴ���Ϊ�� */
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);	
}

void usart2_send_string(const char *str)
{
		unsigned int k=0;
  do 
  {
      usart2_send_byte( *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* �ȴ�������� */
  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET)
  {}
}


//�������յ�������
void parse_cmd_usart2(void)
{
	if(USART2_RX_STA) //���ݽ�����ɲŴ���
	{
		   //�����ص�
			if( strstr( (char*)USART2_RX_BUF,"Biglamp") )
			{
				if( strstr( (char*)USART2_RX_BUF,"ON") )
				{
                   lamp_on_status();
				}
				
				if( strstr( (char*)USART2_RX_BUF,"OFF") )
				{
	
                   lamp_off_status();
				}
			}
			
			//�������Ŷ��
		    if( strstr( (char*)USART2_RX_BUF,"MG995") )
			{
				if( strstr( (char*)USART2_RX_BUF,"ON") )
				{
					//����	
                 door_ON();
				usart2_send_string("send MG995 is ON*\r\n");
				}
				
				if( strstr( (char*)USART2_RX_BUF,"OFF") )
				{
					//����
				door_OFF();
				usart2_send_string("send MG995 is OFF*\r\n");
				}
			}
			
			//¼������
		    if( strstr( (char*)USART2_RX_BUF,"lu") )
			{
				if( strstr( (char*)USART2_RX_BUF,"xiaoming") )
				{
				//¼��xiaoming
				usart1_send_string("luxiaoming");
				usart2_send_string("Face input xiaoming*\r\n");  //���Է���
				}
				
			   if( strstr( (char*)USART2_RX_BUF,"xiaomei") )
				{
				//¼��xiaomei
				usart1_send_string("luxiaomei");
				usart2_send_string("Face input xiaomei*\r\n");  //���Է���
				}
			}
			//ɾ������
		    if( strstr( (char*)USART2_RX_BUF,"all") )
				{
				//ɾ������
				usart1_send_string("all");
				usart2_send_string("delete face*\r\n");  //���Է���
				}
			
			 //��ʱ����
			if( strstr( (char*)USART2_RX_BUF,"temp_password") )
			{   int i;
				sscanf((char*)USART2_RX_BUF, "%*[^/]/%[^@]", temp_password);
					for( i=0;i<4;i++)
						{
							Temp_Password[i]=temp_password[i]-'0';
						}
						usart2_send_string("temp_password is set*\r\n");
			}
			
				
		USART2_RX_STA=0;   //������ɺ������־λ
		//��ջ�����
		memset((void*)USART2_RX_BUF,0,sizeof(USART2_RX_BUF));
		USART2_cnt=0;
	}
}


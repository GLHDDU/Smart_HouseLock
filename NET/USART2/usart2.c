#include "usart2.h" 
#include "usart.h"	
#include "stdio.h"	 	 
#include "string.h"	 
#include "beep.h"
#include "mg995.h"
#include "led.h"
#include "delay.h"

//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART2_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u16 USART2_RX_STA=0;                 //接收状态标记	  1---完整 
u32 USART2_cnt=0 ;                   //记录串口1收到的数据长度	 

char temp_password[16];  // 解析临时密码数组
extern unsigned char Temp_Password[4];
void USART2_IRQHandler(void)
{     
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//判断是不是接收中断
	{	 
		//清除中断标志位
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		
		//放入缓冲区
		USART2_RX_BUF[USART2_cnt++]=USART_ReceiveData(USART2);	

			//判断数据是否接收完成（数据以*结束）
			if(USART2_RX_BUF[USART2_cnt-1]=='*' || USART2_cnt>=sizeof(USART2_RX_BUF))
			{
				USART2_RX_STA=1;
			}
	}  				 											 
}   


//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void USART2_Init(u32 bound)
{  
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);   //串口2时钟使能

 	USART_DeInit(USART2);  //复位串口3
	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
    //USART2_RX	  PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3
	
	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART2, &USART_InitStructure); //初始化串口	2

	USART_Cmd(USART2, ENABLE);                    //使能串口 
	
	//使能接收中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断   
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	USART2_RX_STA=0;		//清零
}

void usart2_send_byte(char byte)
{
		/* 发送一个字节数据到USART */
	USART_SendData(USART2,byte);
		
	/* 等待发送数据寄存器为空 */
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
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET)
  {}
}


//处理串口收到的命令
void parse_cmd_usart2(void)
{
	if(USART2_RX_STA) //数据接收完成才处理
	{
		   //开，关灯
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
			
			//开，关门舵机
		    if( strstr( (char*)USART2_RX_BUF,"MG995") )
			{
				if( strstr( (char*)USART2_RX_BUF,"ON") )
				{
					//开锁	
                 door_ON();
				usart2_send_string("send MG995 is ON*\r\n");
				}
				
				if( strstr( (char*)USART2_RX_BUF,"OFF") )
				{
					//关锁
				door_OFF();
				usart2_send_string("send MG995 is OFF*\r\n");
				}
			}
			
			//录入人脸
		    if( strstr( (char*)USART2_RX_BUF,"lu") )
			{
				if( strstr( (char*)USART2_RX_BUF,"xiaoming") )
				{
				//录入xiaoming
				usart1_send_string("luxiaoming");
				usart2_send_string("Face input xiaoming*\r\n");  //测试返回
				}
				
			   if( strstr( (char*)USART2_RX_BUF,"xiaomei") )
				{
				//录入xiaomei
				usart1_send_string("luxiaomei");
				usart2_send_string("Face input xiaomei*\r\n");  //测试返回
				}
			}
			//删除人脸
		    if( strstr( (char*)USART2_RX_BUF,"all") )
				{
				//删除人脸
				usart1_send_string("all");
				usart2_send_string("delete face*\r\n");  //测试返回
				}
			
			 //临时密码
			if( strstr( (char*)USART2_RX_BUF,"temp_password") )
			{   int i;
				sscanf((char*)USART2_RX_BUF, "%*[^/]/%[^@]", temp_password);
					for( i=0;i<4;i++)
						{
							Temp_Password[i]=temp_password[i]-'0';
						}
						usart2_send_string("temp_password is set*\r\n");
			}
			
				
		USART2_RX_STA=0;   //处理完成后清除标志位
		//清空缓冲区
		memset((void*)USART2_RX_BUF,0,sizeof(USART2_RX_BUF));
		USART2_cnt=0;
	}
}


#include "sys.h"
#include "usart.h"	 
#include "usart2.h" 
#include "mg995.h"
#include "beep.h"
#include "led.h"
#include "string.h"
#include "delay.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 
 
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART1_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u16 USART1_RX_STA=0;                 //接收状态标记	  1---完整 
u32 USART1_cnt=0 ;                   //记录串口1收到的数据长度	 
  
void uart_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9

	//USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE); //清除接收中断标志位
		
   		USART1_RX_BUF[USART1_cnt++]=USART_ReceiveData(USART1);  //放入缓冲区
			
		//判断数据是否接收完成（数据以*结束）
			if(USART1_RX_BUF[USART1_cnt-1]=='*' || USART1_cnt>=sizeof(USART1_RX_BUF))
			{
				USART1_RX_STA=1;
			}
     } 
} 

//处理串口收到的命令
void parse_cmd(void)
{
	if(USART1_RX_STA) //数据接收完成才处理
	{		
			if( strstr( (char*)USART1_RX_BUF,"succeed") )
			{
				//人脸识别成功-xiaoming
				if( strstr( (char*)USART1_RX_BUF,"xiaoming") )
				{
					door_ON();
					usart1_send_string("send MG995 is ON xiaoming\r\n");
					usart2_send_string("OPENxiaoming*\r\n");
				}
				//人脸识别成功-xiaomei
			   if( strstr( (char*)USART1_RX_BUF,"xiaomei") )
				{
					door_ON();
					usart1_send_string("send MG995 is ON xiaomei\r\n");
					usart2_send_string("OPENxiaomei*\r\n");
				}
					
			}
			//人脸识别失败
		   if( strstr( (char*)USART1_RX_BUF,"fail") )
			{
				BEEP=0;
				usart2_send_string("fail of face recognition*\r\n");
				delay_ms(500);
				BEEP=1;
			}
				
		USART1_RX_STA=0;   //处理完成后清除标志位
		//清空缓冲区
		memset((void*)USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
		USART1_cnt=0;
	}
}


void usart1_send_byte(char byte)
{
		/* 发送一个字节数据到USART */
	USART_SendData(USART1,byte);
		
	/* 等待发送数据寄存器为空 */
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
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET)
  {}
}

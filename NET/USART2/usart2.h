#ifndef __USART2_H
#define __USART2_H
	 
#include "sys.h"

#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����6yn 
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
void USART2_Init(u32 bound);
void parse_cmd_usart2(void);
void usart2_send_byte(char byte);
void usart2_send_string(const char *str);
#endif





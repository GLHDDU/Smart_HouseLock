#include "hc05.h"
#include "delay.h"
#include "usart2.h" 

void HC_config(void)
{
	usart2_send_string("AT\r\n");
	delay_ms(50);
	
	usart2_send_string("AT+NAME?\r\n");
	delay_ms(50);
	
	usart2_send_string("AT+?\r\n");
	delay_ms(50);
}

#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "rtc.h"
#include "usart3.h"
#include "usart2.h"  

#include "led.h"
#include "beep.h"
#include "key.h"
#include "oled.h"
#include "hcsr04.h"
#include "dht11.h"
#include "rc522.h"
#include "mg995.h"
#include "menu.h"


/************************************************
                    ��������
�������------------------------------�����
OLED---------------------------------����ɲ˵�
AS608-------->�������----------------�����
RC522-------->�������----------------�����
���밴��------>�������----------------�����
RTCʱ��------->��ʾ������--------------�����
DHT11-------->�����������ʾ��ʪ��-----�����
HC-SR_05----->���---->����-----------�����

����---------�����ӻ�ͨ�Ŵ������� | �ֻ�����ͨ�ſ��� ----->����� �� 
************************************************/




char oledBuf[20];
unsigned char Temp_Password[4];   //��ʱ����

 int main(void)
 {		
	delay_init();	    	     //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600 );	         //����1��ʼ��Ϊ115200
	USART2_Init(115200);         //����ͨ��MCU
	USART3_Init(57600);          //����3��ʼ��Ϊ57600----AS608_baud---//ͨ�Ų����ʣ�����ָ��ģ�鲨���ʸ���
	BEEP_Init();                 //BEEP��ʼ��
 	LED_Init();			         //LED�˿ڳ�ʼ��
	KEY_Init();                  //KYE��ʼ��
    OLED_Init();                 //OLED��ʼ��
	OLED_Clear();                //����OLED����
	HC_SR04_Init();		         //��ʼ�����������ģ��
	DHT11_Init();                //DHT11��ʪ�ȳ�ʼ��
	rtc_Init();                  //������ʱ��
	Random_Password();           //����һ���������  
	TIM3_PWM_Init(199,7199);	 //(199+1)*(7199+1)/72*10^6  -----> ���0.02s,��20ms-----PWM���
    RC522_Init();                //��Ƶ��ģ���ʼ��
	 
	 while(1)
	{
		DHT_data_gather();
		
		parse_cmd();                       //����ʶ����Ϣ����
		
		parse_cmd_usart2();                //MCU����ͨ����Ϣ����
		
		GUI_Refresh(); 
		
		Manual_Operation_Face_input();     //��ɾ����

				
		lamp_open();
		
        printf_HCSR_04_dis();
			
		printf_RTC_clock();
	}	 
	
}	 
 






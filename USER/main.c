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
                    智能门锁
舵机控制------------------------------已完成
OLED---------------------------------已完成菜单
AS608-------->舵机开门----------------已完成
RC522-------->舵机开门----------------已完成
密码按键------>舵机开门----------------已完成
RTC时钟------->显示万年历--------------已完成
DHT11-------->配合万年历显示温湿度-----已完成
HC-SR_05----->测距---->开灯-----------已完成

蓝牙---------（主从机通信传输数据 | 手机蓝牙通信开门 ----->已完成 ） 
************************************************/




char oledBuf[20];
unsigned char Temp_Password[4];   //临时密码

 int main(void)
 {		
	delay_init();	    	     //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600 );	         //串口1初始化为115200
	USART2_Init(115200);         //串口通信MCU
	USART3_Init(57600);          //串口3初始化为57600----AS608_baud---//通信波特率，根据指纹模块波特率更改
	BEEP_Init();                 //BEEP初始化
 	LED_Init();			         //LED端口初始化
	KEY_Init();                  //KYE初始化
    OLED_Init();                 //OLED初始化
	OLED_Clear();                //清屏OLED函数
	HC_SR04_Init();		         //初始化超声波测距模块
	DHT11_Init();                //DHT11温湿度初始化
	rtc_Init();                  //万年历时间
	Random_Password();           //生成一次随机密码  
	TIM3_PWM_Init(199,7199);	 //(199+1)*(7199+1)/72*10^6  -----> 求出0.02s,即20ms-----PWM舵机
    RC522_Init();                //射频卡模块初始化
	 
	 while(1)
	{
		DHT_data_gather();
		
		parse_cmd();                       //人脸识别信息处理
		
		parse_cmd_usart2();                //MCU串口通信信息处理
		
		GUI_Refresh(); 
		
		Manual_Operation_Face_input();     //增删人脸

				
		lamp_open();
		
        printf_HCSR_04_dis();
			
		printf_RTC_clock();
	}	 
	
}	 
 






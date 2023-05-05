#include "menu.h"
#include "oled.h"
#include "key.h"
#include "led.h"
#include <stdio.h>
#include "delay.h"
#include "photo.h"
#include "rc522.h"
#include "doorlock.h"
#include "as608.h"
#include "mg995.h"
#include "usart3.h"
#include "rtc.h"

#include <string.h>
#include "stdlib.h"
#include "beep.h"

#pragma diag_suppress 870        //解决中文编码警告
/*--------------温湿度采集的数据-------------------*/
extern u8 humidityH;	  //湿度整数部分
extern u8 humidityL;	  //湿度小数部分
extern u8 temperatureH;   //温度整数部分
extern u8 temperatureL;   //温度小数部分

static u8 func_index=_Main_UI;//当前页面索引值
static void (*current_operation_func)(u8,u8);//定义一个函数指针
static u8 BEEP_swi=ON;
int equative_menu1=0;         //同级别菜单1刷新限制
int equative_menu2=0;         //同级别菜单2刷新限制


//指纹AS608
#define AS608_baud  57600//通信波特率，根据指纹模块波特率更改
extern u16 ValidN;//模块内有效指纹个数
extern SysPara AS608Para;//指纹模块AS608参数

//门卡RC522
extern u8 num[9];       //转16进制卡号
extern int subs_flag;   //卡号匹配标志

//密码锁
unsigned char Password1[4]={1,1,1,1};     //解锁密码
unsigned char Password[4]={0,0,0,0};      //存放按键输入密码数组
extern unsigned char Temp_Password[4];
unsigned char Password_count=1; 		  //密码位数
int save_falg=0;
int save_PassWord_flag=0;
int delete_PassWord_flag=0;


//索引表
Main_Menu table[20]=
{
    //当前索引项--上一页--下一页--确认--返回--当前索引执行的函数(界面)

	//主界面  0
	{_Main_UI,_Main_UI,_Main_UI,_fingerprint_Option,_OLED_Lock,Main_UI},
	//主菜单  1  2  3
	{_fingerprint_Option,_password_Option,_card_Option,_fingerprint_Child,_Main_UI,Main_Menu_Func},//指纹
	{_card_Option,_fingerprint_Option,_password_Option,_card_Child,_Main_UI,Main_Menu_Func},//刷卡
	{_password_Option,_card_Option,_fingerprint_Option,_password_Child,_Main_UI,Main_Menu_Func},//矩阵密码
	
	//指纹子菜单主页面 4 
	{_fingerprint_Child,_fingerprint_Child,_fingerprint_Child,_fingerprint_Child_add_Option,_fingerprint_Option,fingerprint_Child_menu},//fingerprint-主菜单界面

	//指纹子菜单加减-----操作页面  5 6
	{_fingerprint_Child_add_Option,_fingerprint_Child_del_Option,_fingerprint_Child_del_Option,_fingerprint_Child_add_menu,_fingerprint_Option,fingerprint_Child_menu_Option},
	{_fingerprint_Child_add_Option,_fingerprint_Child_add_Option,_fingerprint_Child_add_Option,_fingerprint_Child_del_menu,_fingerprint_Option,fingerprint_Child_menu_Option},
	
	
	//指纹子界面-增加指纹--界面7
	{_fingerprint_Child_add_menu,_fingerprint_Child_add_menu,_fingerprint_Child_add_menu,_fingerprint_Child_add_menu,_fingerprint_Child_add_Option,Fingerprint_Child_add_menu},
	
	//指纹子界面-删除指纹--界面8
    {_fingerprint_Child_del_menu,_fingerprint_Child_del_menu,_fingerprint_Child_del_menu,_fingerprint_Child_del_menu,_fingerprint_Child_add_Option,Fingerprint_Child_dle_menu},
	
	
	//刷卡子菜单  9
	{_card_Child,_card_Child,_card_Child,_card_Child,_card_Option,card_Child},//刷卡界面
	//矩阵密码子菜单  10
    {_password_Child,_password_Child,_password_Child,_password_Child,_password_Option,password_Child},//矩阵密码界面
	//OLED熄屏11
	{_OLED_Lock,_Main_UI,_Main_UI,_Main_UI,_Main_UI,OLED_Lock},//OLED熄屏
};

/*
函数功能：刷新界面
参数：无
返回值：无
*/
void GUI_Refresh(void)
{
	u8 key_val=KEY_Scan();
	if(key_val!=0)//只有按键按下才刷屏
	{
		switch(key_val)
		{
			case KEY_PREVIOUS: func_index=table[func_index].previous;//更新索引值
					break;
			case KEY_NEXT: func_index=table[func_index].next;//更新索引值
					break;
			case KEY_ENTER:func_index=table[func_index].enter;//更新索引值
					break;
			case KEY_BACK:func_index=table[func_index].back;//更新索引值
					break;
			default:break;
		}
		if(BEEP_swi==ON)
		{
			BEEP=0;
			delay_ms(50);
			BEEP=1;
		}
		
		if(table[func_index].current_operation==Main_Menu_Func)
		{
			if(equative_menu1==0)
			    OLED_Fill(0,0,127,63,0);//清屏
		    equative_menu1=1;
		}
		else if(table[func_index].current_operation==fingerprint_Child_menu_Option)
		{
			if(equative_menu2==0)
			    OLED_Fill(0,0,127,63,0);//清屏
		    equative_menu2=1;
		}		
		else OLED_Fill(0,0,127,63,0);//清屏
	}
	current_operation_func=table[func_index].current_operation;
	(*current_operation_func)(func_index,key_val);//执行当前索引对应的函数
		
}


/*
函数功能：显示主界面
参数：u8 page_index,u8 key_val
返回值：无
*/
void Main_UI(u8 page_index,u8 key_val)
{
               //静态图片
//	OLED_DrawBMP(0,6,16,8,Signal);//显示信号塔
//	OLED_DrawBMP(16,6,32,8,Signal_);//显示信号
//	OLED_DrawBMP(112,6,128,8,electric);//显示电量
//	OLED_DrawBMP(96,6,112,8,blooth);//显示蓝牙
//    OLED_DrawBMP(0,0,16,2,menu);//显示菜单
//	OLED_DrawBMP(112,0,128,2,lock_screen);//显示锁屏
//	OLED_DrawBMP(44,1,92,7,clock);//显示时间
     static int week_day;
	 equative_menu1=0;

	week_day=28+rtc_tiemr.week;
//           RTC--万年历
    OLED_ShowNum(8,0,rtc_tiemr.year,4,16);
	OLED_ShowFontHZ(8+16+16+2,0,24,16,1);     //年
	OLED_ShowNum(8+16+16+16,0,rtc_tiemr.month,2,16);
	OLED_ShowFontHZ(8+16+16+16+16,0,25,16,1);  //月
	OLED_ShowNum(8+16+16+16+16+16,0,rtc_tiemr.day,2,16);
	OLED_ShowFontHZ(8+16+16+16+16+16+16,0,26,16,1);  //日
	
	OLED_ShowFontHZ(0,20,27,16,1);     //星
	OLED_ShowFontHZ(16,20,28,16,1);    //期
	OLED_ShowFontHZ(16+16,20,week_day,16,1);    //具体周几
	
	OLED_ShowNum(70,20,temperatureH,2,16); //显示温度(整数)
	OLED_ShowString(86,20,"C",16);
	
	OLED_ShowNum(104,20,humidityH,2,16); //显示湿度(整数)
	OLED_ShowString(120,20,"%",16);	 

	
	OLED_ShowString(0,40,"   :  :  ",24);  //左移，下移（注意上下间距），字体，大小
	OLED_ShowNum(24+24+24+12,40,rtc_tiemr.sec,2,24);	//显示ASCII字符的码值
	OLED_ShowNum(24+24,40,rtc_tiemr.min,2,24);	//显示ASCII字符的码值
	OLED_ShowNum(16,40,rtc_tiemr.huor,2,24);	//显示ASCII字符的码

	OLED_Refresh_Gram();	//更新显示到OLED

}



/*
函数功能：主菜单显示函数
参数：u8 page_index,u8 key_val
返回值：无
*/            // 1-3  
void Main_Menu_Func(u8 page_index,u8 key_val)
{
	 	
	    OLED_ShowString(32,0,"subs",16);	
		OLED_ShowFontHZ(64,0,0,16,1);
		OLED_ShowFontHZ(80,0,1,16,1);
	    //指纹 
	    OLED_ShowString(64,20,"1.",12);	
		OLED_ShowFontHZ(76,20,2,12,1);
		OLED_ShowFontHZ(92,20,3,12,1);
        //刷卡
	    OLED_ShowString(64,36,"2.",12);	
		OLED_ShowFontHZ(76,36,4,12,1);
		OLED_ShowFontHZ(92,36,5,12,1);
        //密码
	    OLED_ShowString(64,52,"3.",12);	
		OLED_ShowFontHZ(76,52,6,12,1);
		OLED_ShowFontHZ(92,52,7,12,1);	
	
		switch (page_index)
	{
		case _fingerprint_Option:
			OLED_Fill(16,20,56,64,0);
			OLED_ShowString(16,20,"---->",12);
		    break;
		case _card_Option:
			OLED_Fill(16,20,56,64,0);
			OLED_ShowString(16,36,"---->",12);
		    break;
		case _password_Option:
			OLED_Fill(16,20,56,64,0);
			OLED_ShowString(16,52,"---->",12);
		    break;
		default:break;
	}
	    OLED_Refresh_Gram();  //刷新GRAM数组
}


/*
函数功能：fingerprint_Child_menu指纹的主子页面
参数：u8 page_index,u8 key_val
返回值：无
*/
void fingerprint_Child_menu(u8 page_index,u8 key_val)
{	
	
	u8 strbuf[20];
	u8 ensure=0;
equative_menu1=0;
equative_menu2=0;
		OLED_ShowString(30,0,"FingerPrint",16);
		while(PS_HandShake(&AS608Addr))//与AS608模块握手
				{
//					printf("未检测到模块，请重新连接...\r\n");
					OLED_ShowString(0,20,"AS608 Error!",16);
					OLED_Refresh_Gram();
					delay_ms(500);
					OLED_Fill_rectangle(0,20,128,16,0);
					OLED_Refresh_Gram();
					delay_ms(200);
				}
//				printf("通讯成功!\r\n");
//				printf("波特率:%d   地址:0X%X\r\n",AS608_baud,AS608Addr);
				sprintf((char *)strbuf,"Baud:%d",AS608_baud);
				OLED_ShowString(0,20,strbuf,12);
				sprintf((char *)strbuf,"Address:0X%X",AS608Addr);
				OLED_ShowString(0,35,strbuf,12);
				
				ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
				if(ensure!=0x00)
					ShowErrMessage(ensure);//显示确认码错误信息
				ensure=PS_ReadSysPara(&AS608Para);  //读参数
				if(ensure==0x00)
				{
//					printf("\r\n库容量:%d     对比等级: %d\r\n",AS608Para.PS_max-ValidN,AS608Para.PS_level);
					sprintf((char *)strbuf,"Cap:%d  Level:%d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
					OLED_ShowString(0,50,strbuf,12);
				}
				else
					ShowErrMessage(ensure);
				
		OLED_Refresh_Gram();  //刷新GRAM数组
				
		press_FR();//刷指纹
}


/*
函数功能：fingerprint_Child_menu_Option指纹功能选择页面
参数：u8 page_index,u8 key_val
返回值：无
*/
void fingerprint_Child_menu_Option(u8 page_index,u8 key_val)
{
		OLED_ShowString(30,0,"FingerPrint",16);
		OLED_ShowString(40,20,"Add Finger",16);
		OLED_ShowString(40,40,"Delete",16);
	
			switch (page_index)
			{
				case _fingerprint_Child_add_Option:
					OLED_Fill(0,20,32,64,0);
					OLED_ShowString(0,20,"-->",16);
					break;
				case _fingerprint_Child_del_Option:
					OLED_Fill(0,20,32,64,0);
					OLED_ShowString(0,40,"-->",16);
					break;
				default:break;
			}
		OLED_Refresh_Gram();  //刷新GRAM数组
						
		
}


/*
函数功能：Fingerprint_Child_add_menu指纹增加页面
参数：u8 page_index,u8 key_val
返回值：无
*/
void Fingerprint_Child_add_menu(u8 page_index,u8 key_val)
{
	Add_FR();
	func_index=4;
	key_val=4;
    OLED_Fill(0,0,127,63,0);//OLED_Clear();
	current_operation_func=table[func_index].current_operation;
	(*current_operation_func)(func_index,key_val);//执行当前索引对应的函数
}


/*
函数功能：Fingerprint_Child_dle_menu指纹减少页面
参数：u8 page_index,u8 key_val
返回值：无
*/
void Fingerprint_Child_dle_menu(u8 page_index,u8 key_val)
{
	Del_FR();
	func_index=4;
	key_val=4;
    OLED_Fill(0,0,127,63,0);//OLED_Clear();
	current_operation_func=table[func_index].current_operation;
	(*current_operation_func)(func_index,key_val);//执行当前索引对应的函数
}

/*
函数功能：card_Child选项子菜单
参数：u8 page_index,u8 key_val
返回值：无
*/
void card_Child(u8 page_index,u8 key_val)
{    
	equative_menu1=0;
		RC522_Handel();         //刷卡进程初始化
	    OLED_ShowString(44,24,num,16);
		if(num[0]=='5'  &&num[1]=='5' &&num[2]=='E' )
		{
			OLED_ShowString(44,48,"subs",16);
			subs_flag=0;
		}
		else
		{
			OLED_ShowString(44,48,"None!!!",16);
			subs_flag=0;
		}
	
		//请刷卡
		OLED_ShowFontHZ(32,0,8,16,1);
		OLED_ShowFontHZ(48,0,9,16,1);
	    OLED_ShowFontHZ(64,0,10,16,1);
	
		 //卡号：
		OLED_ShowFontHZ(0,24,11,12,1);
		OLED_ShowFontHZ(14,24,12,12,1);
		OLED_ShowFontHZ(28,24,15,12,1);
	
	    //姓名：
		OLED_ShowFontHZ(0,48,13,12,1);
		OLED_ShowFontHZ(14,48,14,12,1);
		OLED_ShowFontHZ(28,48,15,12,1);

	    OLED_Refresh_Gram();  //刷新GRAM数组
}

/*
函数功能：password_Child选项子菜单
参数：u8 page_index,u8 key_val
返回值：无
*/
void password_Child(u8 page_index,u8 key_val)
{
	       //请输入密码
		OLED_ShowFontHZ(32,0,8,16,1);
		OLED_ShowFontHZ(48,0,16,16,1);
	    OLED_ShowFontHZ(64,0,17,16,1);
		OLED_ShowFontHZ(80,0,18,16,1);
		OLED_ShowFontHZ(96,0,19,16,1);
	
		 //密码：
		OLED_ShowFontHZ(0,24,20,12,1);
		OLED_ShowFontHZ(14,24,21,12,1);
		OLED_ShowFontHZ(28,24,15,12,1);
	
	    //结果：
		OLED_ShowFontHZ(0,48,22,12,1);
		OLED_ShowFontHZ(14,48,23,12,1);
		OLED_ShowFontHZ(28,48,15,12,1);
	
		OLED_ShowString(48,24,"_",16);
	    OLED_ShowString(64,24,"_",16);
	    OLED_ShowString(80,24,"_",16);
	    OLED_ShowString(96,24,"_",16);
	
		OLED_Refresh_Gram();  //刷新GRAM数组
	
	   Input_common_password();

		func_index=3;
		key_val=4;
		OLED_Fill(0,0,127,63,0);//OLED_Clear();
		current_operation_func=table[func_index].current_operation;
		(*current_operation_func)(func_index,key_val);//执行当前索引对应的函数
}


/*
函数功能：OLED熄屏
参数：u8 page_index,u8 key_val
返回值;无
*/
void OLED_Lock(u8 page_index,u8 key_val)
{
	OLED_Clear();
}


void Input_common_password(void)
{
	int num=-1;
	while(1)
	{
		if(KEY_diy1==0)  //密码值
		{
			delay_ms(10);
			if(KEY_diy1==0)
			{
				BEEP_dd();
				num++;
				OLED_ShowNum(24+Password_count*16,24,num,1,16);
				OLED_Refresh_Gram();
				if(num>=9)
					num=-1;
				save_falg=1;
		    }
		}
		
		if(KEY_diy2==0)  //确认已输入一位密码
		{
			delay_ms(10);
			if(KEY_diy2==0)
			{
				if(save_falg==1)
				{
				BEEP_dd();
				save_PassWord_flag=1;
				save_falg=0;
				}
			}
		}
		
		if(KEY_diy3==0)   //密码存
		{
			
			delay_ms(10);//消抖
			if(KEY_diy3==0)  
			{
				if(save_PassWord_flag==1)
				{
					BEEP_dd();
					Password[Password_count-1]=num;			 //把输入密码存入数组
					Password_count++;
					delete_PassWord_flag=1;
					save_PassWord_flag=0;
					num=-1;
				}
			}
		}
		
			if(KEY_diy4==0)   //密码删
			{
				delay_ms(10);//消抖
					if(KEY_diy4==0)  
					{
						if(Password_count>1&&delete_PassWord_flag==1)		
						{		
						BEEP_dd();
						OLED_ShowString(24+Password_count*16,24,"_",16);
                        OLED_Refresh_Gram();
                        Password_count--;							
						delete_PassWord_flag=0;
						}
					}
				
			}
			
			if(Password_count>=5)
			{
				Password_count=1;
				if(pipei())
				{
					OLED_ShowString(40,48,"successful!",16);
					OLED_Refresh_Gram();
					Random_Password();   //生成随机密码
					door_ON();  //开门			
				    break;
				}				
				else
				{
					
					OLED_ShowString(40,48,"fail !",16);		
					OLED_Refresh_Gram();
					
					delay_ms(1000);					
					break;
				}
				
			}

			
    }
}

u8  pipei(void)		//进来密码匹配
{
	delay_ms(500);
	if(   ( Password[0]==Password1[0] && Password[1]==Password1[1] && Password[2]==Password1[2] && Password[3]==Password1[3] ) ||( Temp_Password[0]==Password[0] && Password[1]==Temp_Password[1] && Password[2]==Temp_Password[2] && Password[3]==Temp_Password[3] ))
		return 1;		//1代表正确，0代表错误
	else
		return 0;
}

void BEEP_dd(void)		//蜂鸣器开启
{
	BEEP=0;		//开启
	delay_ms(100);
	BEEP=1;		//关闭
}

/*
函数功能：Administrator_password_UI  管理员密码界面
参数：无
返回值;无
*/
void Administrator_password_UI(void)
{
		  //输入 管理员  密码
		OLED_ShowFontHZ(8,0,16,16,1);
	    OLED_ShowFontHZ(24,0,17,16,1);
		OLED_ShowFontHZ(40,0,36,16,1);
		OLED_ShowFontHZ(56,0,37,16,1);
		OLED_ShowFontHZ(72,0,38,16,1);
		OLED_ShowFontHZ(88,0,18,16,1);
		OLED_ShowFontHZ(104,0,19,16,1);
	
	    //密码：
		OLED_ShowFontHZ(0,24,20,12,1);
		OLED_ShowFontHZ(14,24,21,12,1);
		OLED_ShowFontHZ(28,24,15,12,1);
	
	    OLED_ShowString(48,24,"_",16);
	    OLED_ShowString(64,24,"_",16);
	    OLED_ShowString(80,24,"_",16);
	    OLED_ShowString(96,24,"_",16);
	
	    //结果：
		OLED_ShowFontHZ(0,48,22,12,1);
		OLED_ShowFontHZ(14,48,23,12,1);
		OLED_ShowFontHZ(28,48,15,12,1);
	
		OLED_Refresh_Gram();  //刷新GRAM数组
					
}

#include "rtc.h"
#include "usart.h"
#include "led.h"
#include "stdlib.h"
 /****************** 各计时单元 ***************/
extern int Distance_cm;
extern int subs_flag;           
extern unsigned char Temp_Password[4];
int password_time=0;            //密码锁随机数种子时间
int count_down_timw=300;          //有效时间倒计时
int HC_distance_time=0;         //超声波打印时间
int lamp_falg=0;                //灯光标志
int lamp_time=10;                //灯光时间
int DHT11_time=0;               //采集温湿度时间



__IO uint32_t TimeDisplay=0;

 //月份数据表                                                                       
uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表  
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};//平年的月份日期表

RTC_TIME rtc_tiemr={2023,4,3,1,19,18,00};
u8 RTC_Sec_Flag=0;  //RTC定时器秒标志位

void rtc_Init(void)
{
		/*  NVIC配置 */
		NVIC_Configuration();
		
	   if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
		{
			 /* 通过判断 BKP——DR1 是否是0xA5A5来识别是否第一次系统上电 */
		 
//			 printf(" RTC not yet configured....\r\n");
		 
			 /* 配置RTC 设置初始时间*/
			 RTC_Configuration();
		 
//			 printf(" RTC configured....\r\n");
		 			
			/* 设置RTC初值初始时间  年-月-日  时-分-秒 */
			RTC_Set(rtc_tiemr.year , rtc_tiemr.month , rtc_tiemr.day , rtc_tiemr.huor , rtc_tiemr.min , rtc_tiemr.sec );
		 
			 BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	   }
	   else
	   {
			 /* 检查是否是系统掉电 */
			 if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
			 {
//			   printf(" Power On Reset occurred....\r\n");
			 }
			 /* 检查是否是复位引脚引起的复位 */
			else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
			{
//			  printf(" External Reset occurred....\r\n");
			}

//			printf(" No need to configure RTC....\r\n");
			/* Wait for RTC registers synchronization */
			RTC_WaitForSynchro();

			/* Enable the RTC Second */
			RTC_ITConfig(RTC_IT_SEC, ENABLE);
			/* 等待 RTC 寄存器上一次操作完成 */
			RTC_WaitForLastTask();
	  }
	   
	  RCC_ClearFlag();

}

 void NVIC_Configuration(void)
 {
   NVIC_InitTypeDef NVIC_InitStructure;
 
   /* 使能RTC中断 */
   NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
 }
 
 void RTC_Configuration(void)
 {
	 uint32_t t;
	 
	   /* Enable PWR and BKP clocks */
	   RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	 
	   /* 允许访问BKP区*/
	   PWR_BackupAccessCmd(ENABLE);
	 
	   /* 复位 Backup Domain */
	   BKP_DeInit();
	 
	   /* 使能LSE */
	   RCC_LSEConfig(RCC_LSE_ON);
	   /* 等待LSE时钟稳定 */
	   while ( (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && t<5000)
	   {
		   t++;
	   }
	 
	   /* 选择LSE 作为 RTC 时钟源 */
	   RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	 
	   /* 使能 RTC 时钟 */
	   RCC_RTCCLKCmd(ENABLE);
	 
	   /* 等待 RTC 寄存器时钟同步 */
	   RTC_WaitForSynchro();
	 
	   /* 等待 RTC 寄存器上一次操作完成 */
	   RTC_WaitForLastTask();
	 
	   /* 使能 RTC 秒中断 */
	   RTC_ITConfig(RTC_IT_SEC, ENABLE);
	 
	   /* 等待 RTC 寄存器上一次操作完成 */
	   RTC_WaitForLastTask();
	 
	   /* 设置RTC预分频器，时间周期是1s */
	   RTC_SetPrescaler(32767); /* RTC period （1s） = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
	 
	   /* 等待 RTC 寄存器上一次操作完成 */
	   RTC_WaitForLastTask();
 }

 
 /*很多编程语言起源于UNIX系统，而UNIX系统认为1970年1月1日0点是时间起点，*/
/*所以我们常说的UNIX时间戳是以1970年1月1日0点为计时起点时间的*/
//获取时间	
//读出当前时间值 //返回值:0,成功;其他:错误代码.
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0;
	uint32_t temp=0;
	uint16_t temp1=0;
	timecount=RTC_GetCounter();		
	temp=timecount/86400;   //得到天数(秒钟数对应的)24*3600S
	if(daycnt!=temp){//超过一天了
		daycnt=temp;
		temp1=1970;  //从1970年开始
		while(temp>=365){
		     if(Is_Leap_Year(temp1)){//是闰年
			     if(temp>=366)temp-=366;//闰年的秒钟数
			     else {temp1++;break;} 
		     }
		     else temp-=365;       //平年
		     temp1++; 
		}  
		rtc_tiemr.year=temp1;//得到年份
		temp1=0;
		while(temp>=28){//超过了一个月
			if(Is_Leap_Year(rtc_tiemr.year)&&temp1==1){//当年是不是闰年/2月份
				if(temp>=29)temp-=29;//闰年的秒钟数
				else break;
			}else{
	            if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
	            else break;
			}
			temp1++; 
		}
		rtc_tiemr.month=temp1+1;//得到月份
		rtc_tiemr.day=temp+1;  //得到日期
	}
	temp=timecount%86400;     //得到秒钟数      
	rtc_tiemr.huor=temp/3600;     //小时
	rtc_tiemr.min=(temp%3600)/60; //分钟     
	rtc_tiemr.sec=(temp%3600)%60; //秒钟
	rtc_tiemr.week=RTC_Get_Week(rtc_tiemr.year,rtc_tiemr.month,rtc_tiemr.day);//获取星期  
	return 0;
} 


//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
uint8_t Is_Leap_Year(uint16_t year)
{                    
	if(year%4==0)  //必须能被4整除
		{ 
			if(year%100==0)
			{		
				if(year%400==0) return 1;//如果以00结尾,还要能被400整除          
				else return 0;  
			}
			else return 1;  
	    }
	else return 0;
} 


//按年月日计算星期(只允许1901-2099年)
//由RTC_Get调用 
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{    
	uint16_t temp2;
	uint8_t yearH,yearL;
	yearH=year/100;     
	yearL=year%100;
	// 如果为21世纪,年份数加100 
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的 
	temp2=yearL+yearL/4;
	temp2=temp2%7;
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7); //返回星期值
}	
 

uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{ //写入当前时间（1970~2099年有效），
	uint16_t t;
	uint32_t seccount=0;
	if(syear<2000||syear>2099)return 1;//syear范围1970-2099，此处设置范围为2000-2099       
	for(t=1970;t<syear;t++)
		{ //把所有年份的秒钟相加
			if(Is_Leap_Year(t))    seccount+=31622400;//闰年的秒钟数
			else seccount+=31536000;                    //平年的秒钟数
		}
		
	smon-=1;
		
	for(t=0;t<smon;t++)
		{         //把前面月份的秒钟数相加
		seccount+=(u32)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(syear)&&t==1)   seccount+=86400;//闰年2月份增加一天的秒钟数        
		}
		
	seccount+=(u32)(sday-1)*86400;//把前面日期的秒钟数相加
	seccount+=(u32)hour*3600;//小时秒钟数
	seccount+=(u32)min*60;      //分钟秒钟数
	seccount+=sec;//最后的秒钟加上去

	RTC_SetCounter(seccount);//把换算好的计数器值写入
	RTC_WaitForLastTask(); //等待写入完成
		
	return 0; //返回值:0,成功;其他:错误代码.    
}
 
 
 
 
 
 void Time_Display(void)
{
      RTC_Get();  //获取 年月日-时分秒 时间数据
	  RTC_Get_Week(rtc_tiemr.year,rtc_tiemr.month,rtc_tiemr.day); //获取时间第几周数据
	  //printf("DATE: %0.2d-%0.2d-%0.2d\r\n", rtc_tiemr.year, rtc_tiemr.month , rtc_tiemr.day);
	  //printf("WEEK: %d\r\n", rtc_tiemr.week);
//	  printf("Time: %0.2d:%0.2d:%0.2d\r\n", rtc_tiemr.huor, rtc_tiemr.min, rtc_tiemr.sec);
}

 void RTC_IRQHandler(void)
 {
	   if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	   {
		 /* 清除 RTC Second 中断标志位 */
		 RTC_ClearITPendingBit(RTC_IT_SEC);
	 
	 
		 /* 置1s标志位 */
		 TimeDisplay = 1;
		   
	     RTC_Sec_Flag=1;
		 RTC_Time_Base();  
		   
		 /* 等待 RTC 寄存器上一次操作完成 */
		 RTC_WaitForLastTask();
		 
	   }
 }

 void printf_RTC_clock(void)  //串口打印RTC时间
 {
		/* 秒标志位置1 */
		if (TimeDisplay == 1)
		{
		  /* 显示当前时间 */
		  Time_Display();
		  TimeDisplay = 0;
		}
}
 
void RTC_Time_Base(void)
{
	if(RTC_Sec_Flag)
	{
		RTC_Sec_Flag=0;
		
       /****************** 各计时单元 ***************/	
		password_time++;
		count_down_timw--;
		DHT11_time++;
		HC_distance_time++;
		if(password_time>180000)
			password_time=0;
		if(count_down_timw<0)
		{
			count_down_timw=300;
			Random_Password();
		}
        LED0 = !LED0;
		
		if(5<=Distance_cm && Distance_cm<=10)
		{
			lamp_falg=1;
		}
		if(lamp_falg==1)
		{
			lamp_time--;
			if(lamp_time<0)
			{
				lamp_time=10;
				lamp_falg=0;
			}
		}
		//printf("lamp_time：  %d ",lamp_time);
	}
}


void Random_Password(void)
{
	int i;
	srand(password_time);
	for(i=0;i<4;i++)
	Temp_Password[i]= rand() % 10;
}

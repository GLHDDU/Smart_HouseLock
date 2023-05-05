#include "rtc.h"
#include "usart.h"
#include "led.h"
#include "stdlib.h"
 /****************** ����ʱ��Ԫ ***************/
extern int Distance_cm;
extern int subs_flag;           
extern unsigned char Temp_Password[4];
int password_time=0;            //���������������ʱ��
int count_down_timw=300;          //��Чʱ�䵹��ʱ
int HC_distance_time=0;         //��������ӡʱ��
int lamp_falg=0;                //�ƹ��־
int lamp_time=10;                //�ƹ�ʱ��
int DHT11_time=0;               //�ɼ���ʪ��ʱ��



__IO uint32_t TimeDisplay=0;

 //�·����ݱ�                                                                       
uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�  
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};//ƽ����·����ڱ�

RTC_TIME rtc_tiemr={2023,4,3,1,19,18,00};
u8 RTC_Sec_Flag=0;  //RTC��ʱ�����־λ

void rtc_Init(void)
{
		/*  NVIC���� */
		NVIC_Configuration();
		
	   if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
		{
			 /* ͨ���ж� BKP����DR1 �Ƿ���0xA5A5��ʶ���Ƿ��һ��ϵͳ�ϵ� */
		 
//			 printf(" RTC not yet configured....\r\n");
		 
			 /* ����RTC ���ó�ʼʱ��*/
			 RTC_Configuration();
		 
//			 printf(" RTC configured....\r\n");
		 			
			/* ����RTC��ֵ��ʼʱ��  ��-��-��  ʱ-��-�� */
			RTC_Set(rtc_tiemr.year , rtc_tiemr.month , rtc_tiemr.day , rtc_tiemr.huor , rtc_tiemr.min , rtc_tiemr.sec );
		 
			 BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	   }
	   else
	   {
			 /* ����Ƿ���ϵͳ���� */
			 if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
			 {
//			   printf(" Power On Reset occurred....\r\n");
			 }
			 /* ����Ƿ��Ǹ�λ��������ĸ�λ */
			else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
			{
//			  printf(" External Reset occurred....\r\n");
			}

//			printf(" No need to configure RTC....\r\n");
			/* Wait for RTC registers synchronization */
			RTC_WaitForSynchro();

			/* Enable the RTC Second */
			RTC_ITConfig(RTC_IT_SEC, ENABLE);
			/* �ȴ� RTC �Ĵ�����һ�β������ */
			RTC_WaitForLastTask();
	  }
	   
	  RCC_ClearFlag();

}

 void NVIC_Configuration(void)
 {
   NVIC_InitTypeDef NVIC_InitStructure;
 
   /* ʹ��RTC�ж� */
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
	 
	   /* �������BKP��*/
	   PWR_BackupAccessCmd(ENABLE);
	 
	   /* ��λ Backup Domain */
	   BKP_DeInit();
	 
	   /* ʹ��LSE */
	   RCC_LSEConfig(RCC_LSE_ON);
	   /* �ȴ�LSEʱ���ȶ� */
	   while ( (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && t<5000)
	   {
		   t++;
	   }
	 
	   /* ѡ��LSE ��Ϊ RTC ʱ��Դ */
	   RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	 
	   /* ʹ�� RTC ʱ�� */
	   RCC_RTCCLKCmd(ENABLE);
	 
	   /* �ȴ� RTC �Ĵ���ʱ��ͬ�� */
	   RTC_WaitForSynchro();
	 
	   /* �ȴ� RTC �Ĵ�����һ�β������ */
	   RTC_WaitForLastTask();
	 
	   /* ʹ�� RTC ���ж� */
	   RTC_ITConfig(RTC_IT_SEC, ENABLE);
	 
	   /* �ȴ� RTC �Ĵ�����һ�β������ */
	   RTC_WaitForLastTask();
	 
	   /* ����RTCԤ��Ƶ����ʱ��������1s */
	   RTC_SetPrescaler(32767); /* RTC period ��1s�� = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
	 
	   /* �ȴ� RTC �Ĵ�����һ�β������ */
	   RTC_WaitForLastTask();
 }

 
 /*�ܶ���������Դ��UNIXϵͳ����UNIXϵͳ��Ϊ1970��1��1��0����ʱ����㣬*/
/*�������ǳ�˵��UNIXʱ�������1970��1��1��0��Ϊ��ʱ���ʱ���*/
//��ȡʱ��	
//������ǰʱ��ֵ //����ֵ:0,�ɹ�;����:�������.
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0;
	uint32_t temp=0;
	uint16_t temp1=0;
	timecount=RTC_GetCounter();		
	temp=timecount/86400;   //�õ�����(��������Ӧ��)24*3600S
	if(daycnt!=temp){//����һ����
		daycnt=temp;
		temp1=1970;  //��1970�꿪ʼ
		while(temp>=365){
		     if(Is_Leap_Year(temp1)){//������
			     if(temp>=366)temp-=366;//�����������
			     else {temp1++;break;} 
		     }
		     else temp-=365;       //ƽ��
		     temp1++; 
		}  
		rtc_tiemr.year=temp1;//�õ����
		temp1=0;
		while(temp>=28){//������һ����
			if(Is_Leap_Year(rtc_tiemr.year)&&temp1==1){//�����ǲ�������/2�·�
				if(temp>=29)temp-=29;//�����������
				else break;
			}else{
	            if(temp>=mon_table[temp1])temp-=mon_table[temp1];//ƽ��
	            else break;
			}
			temp1++; 
		}
		rtc_tiemr.month=temp1+1;//�õ��·�
		rtc_tiemr.day=temp+1;  //�õ�����
	}
	temp=timecount%86400;     //�õ�������      
	rtc_tiemr.huor=temp/3600;     //Сʱ
	rtc_tiemr.min=(temp%3600)/60; //����     
	rtc_tiemr.sec=(temp%3600)%60; //����
	rtc_tiemr.week=RTC_Get_Week(rtc_tiemr.year,rtc_tiemr.month,rtc_tiemr.day);//��ȡ����  
	return 0;
} 


//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
uint8_t Is_Leap_Year(uint16_t year)
{                    
	if(year%4==0)  //�����ܱ�4����
		{ 
			if(year%100==0)
			{		
				if(year%400==0) return 1;//�����00��β,��Ҫ�ܱ�400����          
				else return 0;  
			}
			else return 1;  
	    }
	else return 0;
} 


//�������ռ�������(ֻ����1901-2099��)
//��RTC_Get���� 
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{    
	uint16_t temp2;
	uint8_t yearH,yearL;
	yearH=year/100;     
	yearL=year%100;
	// ���Ϊ21����,�������100 
	if (yearH>19)yearL+=100;
	// ����������ֻ��1900��֮��� 
	temp2=yearL+yearL/4;
	temp2=temp2%7;
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7); //��������ֵ
}	
 

uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{ //д�뵱ǰʱ�䣨1970~2099����Ч����
	uint16_t t;
	uint32_t seccount=0;
	if(syear<2000||syear>2099)return 1;//syear��Χ1970-2099���˴����÷�ΧΪ2000-2099       
	for(t=1970;t<syear;t++)
		{ //��������ݵ��������
			if(Is_Leap_Year(t))    seccount+=31622400;//�����������
			else seccount+=31536000;                    //ƽ���������
		}
		
	smon-=1;
		
	for(t=0;t<smon;t++)
		{         //��ǰ���·ݵ����������
		seccount+=(u32)mon_table[t]*86400;//�·����������
		if(Is_Leap_Year(syear)&&t==1)   seccount+=86400;//����2�·�����һ���������        
		}
		
	seccount+=(u32)(sday-1)*86400;//��ǰ�����ڵ����������
	seccount+=(u32)hour*3600;//Сʱ������
	seccount+=(u32)min*60;      //����������
	seccount+=sec;//�������Ӽ���ȥ

	RTC_SetCounter(seccount);//�ѻ���õļ�����ֵд��
	RTC_WaitForLastTask(); //�ȴ�д�����
		
	return 0; //����ֵ:0,�ɹ�;����:�������.    
}
 
 
 
 
 
 void Time_Display(void)
{
      RTC_Get();  //��ȡ ������-ʱ���� ʱ������
	  RTC_Get_Week(rtc_tiemr.year,rtc_tiemr.month,rtc_tiemr.day); //��ȡʱ��ڼ�������
	  //printf("DATE: %0.2d-%0.2d-%0.2d\r\n", rtc_tiemr.year, rtc_tiemr.month , rtc_tiemr.day);
	  //printf("WEEK: %d\r\n", rtc_tiemr.week);
//	  printf("Time: %0.2d:%0.2d:%0.2d\r\n", rtc_tiemr.huor, rtc_tiemr.min, rtc_tiemr.sec);
}

 void RTC_IRQHandler(void)
 {
	   if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	   {
		 /* ��� RTC Second �жϱ�־λ */
		 RTC_ClearITPendingBit(RTC_IT_SEC);
	 
	 
		 /* ��1s��־λ */
		 TimeDisplay = 1;
		   
	     RTC_Sec_Flag=1;
		 RTC_Time_Base();  
		   
		 /* �ȴ� RTC �Ĵ�����һ�β������ */
		 RTC_WaitForLastTask();
		 
	   }
 }

 void printf_RTC_clock(void)  //���ڴ�ӡRTCʱ��
 {
		/* ���־λ��1 */
		if (TimeDisplay == 1)
		{
		  /* ��ʾ��ǰʱ�� */
		  Time_Display();
		  TimeDisplay = 0;
		}
}
 
void RTC_Time_Base(void)
{
	if(RTC_Sec_Flag)
	{
		RTC_Sec_Flag=0;
		
       /****************** ����ʱ��Ԫ ***************/	
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
		//printf("lamp_time��  %d ",lamp_time);
	}
}


void Random_Password(void)
{
	int i;
	srand(password_time);
	for(i=0;i<4;i++)
	Temp_Password[i]= rand() % 10;
}

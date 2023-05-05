#include "doorlock.h"
#include "delay.h"
#include "oled.h"
#include "led.h"
#include "stdlib.h"
#include "beep.h"
#include "key.h"
#include "usart.h"
#include "as608.h"
#include "usart3.h"
#include "mg995.h"

#define AS608_baud  57600//通信波特率，根据指纹模块波特率更改

u16 ValidN;//模块内有效指纹个数
SysPara AS608Para;//指纹模块AS608参数


//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
//	printf("%s\r\n",EnsureMessage(ensure));
}


//刷指纹
void press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	u8 strbuf[20];
	

		ensure=PS_GetImage();
		if(ensure==0x00)//获取图像成功 
		{	
			ensure=PS_GenChar(CharBuffer1);
			if(ensure==0x00) //生成特征成功
			{			
				ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
				if(ensure==0x00)//搜索成功
				{				
//					printf("刷指纹成功!\r\n");
//					printf("确有此人：ID:%d  匹配得分:%d\r\n",seach.pageID,seach.mathscore);
					 OLED_Fill(0,0,127,63,0);//清屏
					OLED_Clear();
					OLED_ShowString(0,0,"Finger Success!",16);
					sprintf((char *)strbuf,"ID:%d  Score:%d",seach.pageID,seach.mathscore);
					OLED_ShowString(0,20,strbuf,16);
					OLED_Refresh_Gram();
					
					door_ON();  //开门
					delay_ms(500);
					OLED_Fill(0,0,127,63,0);//清屏
				}
				else 
					ShowErrMessage(ensure);					
			}
			else
				ShowErrMessage(ensure);
		}
		

	
}

//录指纹
void Add_FR(void)
{
	u8 i,ensure ,processnum=0;
	u16 ID=0;
	
	//选择指纹ID
	OLED_Clear();
	OLED_ShowString(0,0,"Add NEW Finger...",16);
	OLED_ShowString(0,20,"ID:  0",16);
	OLED_Refresh_Gram();

   while(1)
	{
		if(KEY_diy1==0)
		{
			ID++;
			if(ID>=AS608Para.PS_max)ID=0;
			OLED_ShowNum(3*8,20,ID,3,16);
			OLED_Refresh_Gram();
//			printf("ID : %d",ID);
		}
		if(KEY_diy2==0)
		{
			ID--;
			if(ID>=AS608Para.PS_max)ID=0;
			OLED_ShowNum(3*8,20,ID,3,16);
			OLED_Refresh_Gram();
//			printf("ID : %d",ID);
		}
		   if(KEY_diy3==0)
             break;
    }

   if(KEY_diy3==0)
   {   
	   while(1)
	    {
				switch (processnum)
				{
					case 0:
						i++;
//						printf("请按指纹\r\n");
						OLED_ShowString(0,40,"Please Press Figner.",12);		
						OLED_Refresh_Gram();
						ensure=PS_GetImage();
						if(ensure==0x00) 
						{
							BEEP=0;
							ensure=PS_GenChar(CharBuffer1);//生成特征
							BEEP=1;
							if(ensure==0x00)
							{
//								printf("指纹正常\r\n");
								OLED_ShowString(0,40,"Figner1 OK...       ",12);
								OLED_Refresh_Gram();
								i=0;
								processnum=1;//跳到第二步						
							}
							else ShowErrMessage(ensure);				
						}
						else ShowErrMessage(ensure);						
						break;
					
					case 1:
						i++;
//						printf("请按再按一次指纹\r\n");
						OLED_ShowString(0,40,"Please Press Figner.",12);
						OLED_Refresh_Gram();
						ensure=PS_GetImage();
						if(ensure==0x00) 
						{
							BEEP=0;
							ensure=PS_GenChar(CharBuffer2);//生成特征
							BEEP=1;
							if(ensure==0x00)
							{
//								printf("指纹正常\r\n");
								OLED_ShowString(0,40,"Figner2 OK...       ",12);
								OLED_Refresh_Gram();
								i=0;
								processnum=2;//跳到第三步
							}
							else ShowErrMessage(ensure);	
						}
						else ShowErrMessage(ensure);		
						break;

					case 2:
//						printf("对比两次指纹\r\n");
						OLED_ShowString(0,40,"Figner1 and 2 Match.",12);
						OLED_Refresh_Gram();
						ensure=PS_Match();
						if(ensure==0x00) 
						{
//							printf("对比成功,两次指纹一样\r\n");
							OLED_ShowString(0,40,"Figner1 and 2 OK....",12);
							OLED_Refresh_Gram();
							processnum=3;//跳到第四步
						}
						else 
						{
//							printf("对比失败，请重新录入指纹\r\n");
							OLED_ShowString(0,40,"Figner1 and 2 Error.",12);
							OLED_Refresh_Gram();
							ShowErrMessage(ensure);
							i=0;
							processnum=0;//跳回第一步		
						}
						delay_ms(1000);
						break;

					case 3:
//						printf("生成指纹模板\r\n");
						ensure=PS_RegModel();
						if(ensure==0x00) 
						{
//							printf("生成指纹模板成功\r\n");
							processnum=4;//跳到第五步
						}
						else 
						{
						processnum=0;
						ShowErrMessage(ensure);
						}
						delay_ms(1000);
						break;
						
					case 4:	
//						printf("0=< ID <=299	");
//						printf("存储ID：%d\r\n",ID);
						ensure=PS_StoreChar(CharBuffer2,ID);//储存模板
						if(ensure==0x00) 
						{			
							PS_ValidTempleteNum(&ValidN);//读库指纹个数
//							printf("录入指纹成功，剩余容量数：%d\r\n",AS608Para.PS_max-ValidN);
							OLED_ShowString(0,40,"Figner Save OK!     ",12);
							OLED_ShowString(0,52,"Cap:",12);
							OLED_ShowNum(4*6,52,AS608Para.PS_max-ValidN,3,12);
							OLED_Refresh_Gram();
							delay_ms(1000);
							return;
						}
						else 
						{
						processnum=0;
						ShowErrMessage(ensure);
						}					
						break;				
				}
				delay_ms(100);

		
		}	
	}   
}
		
	



//删除指纹
void Del_FR(void)
{
	u8  ensure;
	u16 ID=0;
	
//	printf("删除指纹\r\n");
//	printf("0=< ID <=299\r\n");
	//选择指纹ID
	OLED_Clear();
	OLED_ShowString(0,0,"Delete Finger...",16);
	OLED_ShowString(0,20,"ID:  0",16);
	OLED_Refresh_Gram();
	
	while(1)
	{
		
	 if(KEY_diy1==0)//修改ID
		{
			ID++;
			if(ID>=AS608Para.PS_max)ID=0;
			OLED_ShowNum(3*8,20,ID,3,16);
			OLED_Refresh_Gram();
		}
	 if(KEY_diy2==0)//修改ID
	    {
		ID--;
		if(ID>=AS608Para.PS_max)ID=0;
		OLED_ShowNum(3*8,20,ID,3,16);
		OLED_Refresh_Gram();
	    }
	 if(KEY_diy3==0)//确定
		{
			ensure=PS_DeletChar(ID,1);//删除单个指纹
			if(ensure==0)
			{
//				printf("删除指纹成功\r\n");
				OLED_ShowString(0,40,"Delete OK!",12);
				OLED_Refresh_Gram();
			}
			else
				ShowErrMessage(ensure);
			
			delay_ms(500);
			PS_ValidTempleteNum(&ValidN);//读库指纹个数
//			printf("有效指纹总数：%d\r\n",AS608Para.PS_max-ValidN);
			OLED_ShowString(0,52,"Cap:",12);
			OLED_ShowNum(4*6,52,AS608Para.PS_max-ValidN,3,12);
			OLED_Refresh_Gram();
			return ;
		}

	}	

}




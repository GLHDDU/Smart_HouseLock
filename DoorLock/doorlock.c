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

#define AS608_baud  57600//ͨ�Ų����ʣ�����ָ��ģ�鲨���ʸ���

u16 ValidN;//ģ������Чָ�Ƹ���
SysPara AS608Para;//ָ��ģ��AS608����


//��ʾȷ���������Ϣ
void ShowErrMessage(u8 ensure)
{
//	printf("%s\r\n",EnsureMessage(ensure));
}


//ˢָ��
void press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	u8 strbuf[20];
	

		ensure=PS_GetImage();
		if(ensure==0x00)//��ȡͼ��ɹ� 
		{	
			ensure=PS_GenChar(CharBuffer1);
			if(ensure==0x00) //���������ɹ�
			{			
				ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
				if(ensure==0x00)//�����ɹ�
				{				
//					printf("ˢָ�Ƴɹ�!\r\n");
//					printf("ȷ�д��ˣ�ID:%d  ƥ��÷�:%d\r\n",seach.pageID,seach.mathscore);
					 OLED_Fill(0,0,127,63,0);//����
					OLED_Clear();
					OLED_ShowString(0,0,"Finger Success!",16);
					sprintf((char *)strbuf,"ID:%d  Score:%d",seach.pageID,seach.mathscore);
					OLED_ShowString(0,20,strbuf,16);
					OLED_Refresh_Gram();
					
					door_ON();  //����
					delay_ms(500);
					OLED_Fill(0,0,127,63,0);//����
				}
				else 
					ShowErrMessage(ensure);					
			}
			else
				ShowErrMessage(ensure);
		}
		

	
}

//¼ָ��
void Add_FR(void)
{
	u8 i,ensure ,processnum=0;
	u16 ID=0;
	
	//ѡ��ָ��ID
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
//						printf("�밴ָ��\r\n");
						OLED_ShowString(0,40,"Please Press Figner.",12);		
						OLED_Refresh_Gram();
						ensure=PS_GetImage();
						if(ensure==0x00) 
						{
							BEEP=0;
							ensure=PS_GenChar(CharBuffer1);//��������
							BEEP=1;
							if(ensure==0x00)
							{
//								printf("ָ������\r\n");
								OLED_ShowString(0,40,"Figner1 OK...       ",12);
								OLED_Refresh_Gram();
								i=0;
								processnum=1;//�����ڶ���						
							}
							else ShowErrMessage(ensure);				
						}
						else ShowErrMessage(ensure);						
						break;
					
					case 1:
						i++;
//						printf("�밴�ٰ�һ��ָ��\r\n");
						OLED_ShowString(0,40,"Please Press Figner.",12);
						OLED_Refresh_Gram();
						ensure=PS_GetImage();
						if(ensure==0x00) 
						{
							BEEP=0;
							ensure=PS_GenChar(CharBuffer2);//��������
							BEEP=1;
							if(ensure==0x00)
							{
//								printf("ָ������\r\n");
								OLED_ShowString(0,40,"Figner2 OK...       ",12);
								OLED_Refresh_Gram();
								i=0;
								processnum=2;//����������
							}
							else ShowErrMessage(ensure);	
						}
						else ShowErrMessage(ensure);		
						break;

					case 2:
//						printf("�Ա�����ָ��\r\n");
						OLED_ShowString(0,40,"Figner1 and 2 Match.",12);
						OLED_Refresh_Gram();
						ensure=PS_Match();
						if(ensure==0x00) 
						{
//							printf("�Աȳɹ�,����ָ��һ��\r\n");
							OLED_ShowString(0,40,"Figner1 and 2 OK....",12);
							OLED_Refresh_Gram();
							processnum=3;//�������Ĳ�
						}
						else 
						{
//							printf("�Ա�ʧ�ܣ�������¼��ָ��\r\n");
							OLED_ShowString(0,40,"Figner1 and 2 Error.",12);
							OLED_Refresh_Gram();
							ShowErrMessage(ensure);
							i=0;
							processnum=0;//���ص�һ��		
						}
						delay_ms(1000);
						break;

					case 3:
//						printf("����ָ��ģ��\r\n");
						ensure=PS_RegModel();
						if(ensure==0x00) 
						{
//							printf("����ָ��ģ��ɹ�\r\n");
							processnum=4;//�������岽
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
//						printf("�洢ID��%d\r\n",ID);
						ensure=PS_StoreChar(CharBuffer2,ID);//����ģ��
						if(ensure==0x00) 
						{			
							PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
//							printf("¼��ָ�Ƴɹ���ʣ����������%d\r\n",AS608Para.PS_max-ValidN);
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
		
	



//ɾ��ָ��
void Del_FR(void)
{
	u8  ensure;
	u16 ID=0;
	
//	printf("ɾ��ָ��\r\n");
//	printf("0=< ID <=299\r\n");
	//ѡ��ָ��ID
	OLED_Clear();
	OLED_ShowString(0,0,"Delete Finger...",16);
	OLED_ShowString(0,20,"ID:  0",16);
	OLED_Refresh_Gram();
	
	while(1)
	{
		
	 if(KEY_diy1==0)//�޸�ID
		{
			ID++;
			if(ID>=AS608Para.PS_max)ID=0;
			OLED_ShowNum(3*8,20,ID,3,16);
			OLED_Refresh_Gram();
		}
	 if(KEY_diy2==0)//�޸�ID
	    {
		ID--;
		if(ID>=AS608Para.PS_max)ID=0;
		OLED_ShowNum(3*8,20,ID,3,16);
		OLED_Refresh_Gram();
	    }
	 if(KEY_diy3==0)//ȷ��
		{
			ensure=PS_DeletChar(ID,1);//ɾ������ָ��
			if(ensure==0)
			{
//				printf("ɾ��ָ�Ƴɹ�\r\n");
				OLED_ShowString(0,40,"Delete OK!",12);
				OLED_Refresh_Gram();
			}
			else
				ShowErrMessage(ensure);
			
			delay_ms(500);
			PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
//			printf("��Чָ��������%d\r\n",AS608Para.PS_max-ValidN);
			OLED_ShowString(0,52,"Cap:",12);
			OLED_ShowNum(4*6,52,AS608Para.PS_max-ValidN,3,12);
			OLED_Refresh_Gram();
			return ;
		}

	}	

}




#include "networking.h"  //������Ҫ��ͷ�ļ�
#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "wifi.h"	    //������Ҫ��ͷ�ļ�
#include "delay.h"	    //������Ҫ��ͷ�ļ�
#include "usart.h"	    //������Ҫ��ͷ�ļ�
#include "usart2.h" 
#include "led.h"        //������Ҫ��ͷ�ļ�
#include "mqtt.h"       //������Ҫ��ͷ�ļ�
#include "timer.h"
#include <stdlib.h>     //������Ҫ��ͷ�ļ�

#include "dht11.h"
#include "mg995.h"
#include "beep.h"

char temp_password[16];  // ������ʱ��������
extern unsigned char Temp_Password[4];
u8 door_off_on=0;

void connect_net(void)
{
		if(Connect_flag==1)      /*���ӷ������ɹ�*/
		{   
			Handle_TxDataBuff(); /*1�������ͻ���������*/		
			Handle_RxDataBuff(); /*2��������ջ���������*/			
			Handle_CMDBuf();	 /*3�����������������*/
		}	
		else					
		{ 			
			ReconnectSever();   /*�������ӷ�����*/
			BEEP=0;
			delay_ms(200);
			BEEP=1;
			delay_ms(200);
			BEEP=0;
			delay_ms(200);
			BEEP=1;
		}
}

/*�����ͻ���������*/
void Handle_TxDataBuff(void)
{
		if(MQTT_TxDataOutPtr != MQTT_TxDataInPtr) //���ͻ�������������
		{
		//3������ɽ���if
		//��1�֣�0x10 ���ӱ���
		//��2�֣�0x82 ���ı��ģ���ConnectPack_flag��λ����ʾ���ӱ��ĳɹ�
		//��3�֣�SubcribePack_flag��λ��˵�����ӺͶ��ľ��ɹ����������Ŀɷ�
			if((MQTT_TxDataOutPtr[2]==0x10)||((MQTT_TxDataOutPtr[2]==0x82)&&(ConnectPack_flag==1))||(SubcribePack_flag==1))
				{    
//				printf("��������:0x%x\r\n",MQTT_TxDataOutPtr[2]);  //������ʾ��Ϣ
				MQTT_TxData(MQTT_TxDataOutPtr);                       //�������ݣ�����2���ͻ������е����ݣ�
				MQTT_TxDataOutPtr += BUFF_UNIT; 				//ָ������
				
					if(MQTT_TxDataOutPtr==MQTT_TxDataEndPtr)              //���ָ�뵽������β����
					MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];              //ָ���λ����������ͷ
				} 				
	      }	
}


/*������ջ���������*/
void Handle_RxDataBuff(void)
{
		if(MQTT_RxDataOutPtr != MQTT_RxDataInPtr){//���ջ�������������														
//			printf("���յ�����:\r\n");
			
			/*2.1����յ�����CONNECT����*/
			//if�жϣ������һ���ֽ���0x20����ʾ�յ�����CONNACK����
			//��������Ҫ�жϵ�4���ֽڣ�����CONNECT�����Ƿ�ɹ�
			if(MQTT_RxDataOutPtr[2]==0x20) //�յ�����CONNECT����
				{             			
					switch(MQTT_RxDataOutPtr[5]){//�ж�CONNECT�����Ƿ�ɹ�					
					case 0x00 :// printf("CONNECT���ĳɹ�\r\n");  
									ConnectPack_flag = 1;                                        //CONNECT���ĳɹ������ı��Ŀɷ�
						break;                                                                                           
					case 0x01 : //printf("�����Ѿܾ�����֧�ֵ�Э��汾��׼������\r\n");
								Connect_flag = 0;                                            //Connect_flag���㣬��������
								break;
					case 0x02 : //printf("�����Ѿܾ������ϸ�Ŀͻ��˱�ʶ����׼������\r\n");
								Connect_flag = 0;                                            //Connect_flag���㣬��������
								break;
					case 0x03 : //printf("�����Ѿܾ�������˲����ã�׼������\r\n");
								Connect_flag = 0;                                            //Connect_flag���㣬��������
								break;
					case 0x04 : //printf("�����Ѿܾ�����Ч���û��������룬׼������\r\n");
								Connect_flag = 0;                                            //Connect_flag���㣬��������						
								break;
					case 0x05 : //printf("�����Ѿܾ���δ��Ȩ��׼������\r\n");
								Connect_flag = 0;                                            //Connect_flag���㣬��������						
								break; 
					default   : //printf("�����Ѿܾ���δ֪״̬��׼������\r\n");
								Connect_flag = 0;                                            //Connect_flag���㣬��������					
								break;							
				}				
			}		
			
			/*2.2����յ�����SUBACK����*/
			//�жϣ���һ���ֽ���0x90����ʾ�յ�����SUBACK����
			//��������Ҫ�ж϶��Ļظ��������ǲ��ǳɹ�
			else if(MQTT_RxDataOutPtr[2]==0x90)//�ж�SUBACK�����Ƿ�ɹ�		
				{
					switch(MQTT_RxDataOutPtr[6])
						{					
					case 0x00 :
					case 0x01 : //printf("���ĳɹ�\r\n");
											SubcribePack_flag = 1;                //SubcribePack_flag��1����ʾ���ı��ĳɹ����������Ŀɷ���
											Ping_flag = 0;                        //Ping_flag����
											TIM4_ENABLE_30S();                    //����30s��PING��ʱ�� 
											TempHumi_State();                     //�ȷ�һ������
								break;                               
					default   : //printf("����ʧ�ܣ�׼������\r\n"); 
											Connect_flag = 0;                     //Connect_flag���㣬��������
								break;						
				}					
			}
			
			/*2.3����յ�����PINGRESP����*/
			//�жϣ���һ���ֽ���0xD0����ʾ�յ�����PINGRESP����
			else if(MQTT_RxDataOutPtr[2]==0xD0){ 
				//printf("PING���Ļظ�\r\n");
				if(Ping_flag==1){             //���Ping_flag=1����ʾ��һ�η���
					 Ping_flag = 0;    				  //Ҫ���Ping_flag��־
				}else if(Ping_flag>1){ 				//���Ping_flag>1����ʾ�Ƕ�η����ˣ�������2s����Ŀ��ٷ���
					Ping_flag = 0;     				  //Ҫ���Ping_flag��־
					TIM4_ENABLE_30S(); 				  //PING��ʱ���ػ�30s��ʱ��
				}				
			}	
			
			/*2.4����յ����Ƿ�������������������*/	
                //�жϣ������һ���ֽ���0x30����ʾ�յ����Ƿ�������������������
				//����Ҫ��ȡ��������			
			else if((MQTT_RxDataOutPtr[2]==0x30)){ //����Ҫ��ȡ��������
				//printf("�������ȼ�0����\r\n");
				MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr);  //����ȼ�0��������
			}				
							
			MQTT_RxDataOutPtr += BUFF_UNIT;                   //ָ������
			if(MQTT_RxDataOutPtr==MQTT_RxDataEndPtr)          //���ָ�뵽������β����
				MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];          //ָ���λ����������ͷ                        
		}	
}

/*���������������*/
void Handle_CMDBuf(void)
{
			if(MQTT_CMDOutPtr != MQTT_CMDInPtr)  //if�����Ļ���˵�����������������
				{                             			       
				   //printf("����:%s\r\n",&MQTT_CMDOutPtr[2]);                 //���������Ϣ
					
					//��������
				   if(strstr((char *)MQTT_CMDOutPtr+2,"\"params\":{\"AlarmSwitch\":1}"))
					{                 //�򵥲����ͽ������
					BEEP=0;//�򿪷�����
					//printf("AlarmSwitch_ON��         \r\n");
                    Device_State();
                    }
					
					else if(strstr((char *)MQTT_CMDOutPtr+2,"\"params\":{\"AlarmSwitch\":0}"))
					{
					BEEP=1;//�رշ�����	
					//printf("AlarmSwitch_OFF��         \r\n");
                    Device_State();
                    }
					
					
					//�Ŵ���
				  if(strstr((char *)MQTT_CMDOutPtr+2,"\"params\":{\"ELock\":1}"))
					{                 //�򵥲����ͽ������
					door_off_on=1;
					door_ON();
					//printf("ELock_ON��         \r\n");
                    Device_State();
                    }
					
					else if(strstr((char *)MQTT_CMDOutPtr+2,"\"params\":{\"ELock\":0}"))
					{
					door_off_on=0;
					door_OFF();	
					//printf("ELock_OFF��         \r\n");
                    Device_State();
                    }


	           /*************************/
					if(strstr((char *)MQTT_CMDOutPtr+2, (const char *)"\"params\":{\"addOTP\"") != NULL)//רҵ���ݷ���������Ӣ�ĺ�����
					{int i;		 
					  USER_GetSteeringJsonValue((char *)MQTT_CMDOutPtr+2, (char *)"addOTP");
					  //printf("��ʱ����:%s\r\n",temp_password);
							for( i=0;i<4;i++)
						{
							Temp_Password[i]=temp_password[i]-'0';
						}
//						char temp2[16];
//                      sscanf((char *)MQTT_CMDOutPtr+2, "%*[^/]/%[^@]", temp2);		
//					  printf("����������ʱ�������ݣ�%s\r\n",temp2);  
						
							
					}
						  MQTT_CMDOutPtr += BUFF_UNIT;                               	 //ָ������
						    if(MQTT_CMDOutPtr==MQTT_CMDEndPtr)           	             //���ָ�뵽������β����
							   MQTT_CMDOutPtr = MQTT_CMDBuf[0];          	             //ָ���λ����������ͷ				
				}
}

/*����������*/
void ReconnectSever(void)
{
		//printf("��Ҫ���ӷ�����\r\n");                    //���������Ϣ
		TIM_Cmd(TIM1,DISABLE);                          //�ر�TIM1 
		TIM_Cmd(TIM4,DISABLE);                          //�ر�TIM4  
		WiFi_RxCounter=0;                               //WiFi������������������                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ����� 
		if(WiFi_Connect_IoTServer()==0)
		{   			     		                  //���WiFi�����Ʒ�������������0����ʾ��ȷ������if
			//printf("����TCP���ӳɹ�\r\n");             //���������Ϣ
			Connect_flag = 1;                             //Connect_flag��1����ʾ���ӳɹ�	
			WiFi_RxCounter=0;                             //WiFi������������������                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);       //���WiFi���ջ����� 		
			MQTT_Buff_ReInit();                           //���³�ʼ�����ͻ�����                    
		}			
}

/*--------------��ʪ�Ȳɼ�������-------------------*/
extern u8 humidityH;	  //ʪ����������
extern u8 humidityL;	  //ʪ��С������
extern u8 temperatureH;   //�¶���������
extern u8 temperatureL;   //�¶�С������
/*-------------------------------------------------*/
/*���������ɼ���ʪ�ȣ���������������               */
/*��  ������                                       */
/*����ֵ����                    4sһ��               */
/*-------------------------------------------------*/
void TempHumi_State(void)    
{  	
	char temp[128];  
	
	DHT11_Read_Data(&humidityH,&humidityL,&temperatureH,&temperatureL);
	
	//printf("�¶ȣ�%d  ʪ�ȣ�%d\r\n",temperatureH,humidityH);

	sprintf(temp,"{\"method\":\"thing.event.property.post\",\"id\":\"203302322\",\"params\":{\"CuTemperature\":%2d,\"CurrentHumidity\":%2d},\"version\":\"1.0.0\"}",temperatureH,humidityH);  //�����ظ�ʪ���¶�����
	MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //������ݣ�������������	
	
}


void Device_State(void)    
{  	
	char temp[128];  

	sprintf(temp,"{\"method\":\"thing.event.property.post\",\"id\":\"203302322\",\"params\":{\"AlarmSwitch\":%d,\"ELock\":%d},\"version\":\"1.0.0\"}",alarm,door_off_on);  //�����ظ�AlarmSwitch����
	MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //������ݣ�������������	
	
}

void Reporting_data_flow(void)
{
	TempHumi_State(); 
}

/*******************************************************************
*������unsigned char *USER_GetSteeringJsonValue(char *cJson, char *Tag)
*���ܣ�jsonΪ�ַ������У���json��ʽ�е�Ŀ�����Tag��Ӧ��ֵ�ַ���ת��Ϊ�ַ���
*���룺
		char *cJson json�ַ���
		char *Tag Ҫ�����Ķ����ǩ
*�����������ֵ
*����˵�����û������ڴ˻����ϸ������չ�ú���������ֻ�Ǹ��򵥵�DEMO
*******************************************************************/
void USER_GetSteeringJsonValue(char *cJson, char *Tag)
{ 
  char *target = NULL;
	int8_t i=0;
	memset(temp_password, 0x00, 128);
	sprintf(temp_password,"\"%s\":\"",Tag);
	target=strstr((const char *)cJson, (const char *)temp_password);
	if(target == NULL)
	{
		//printf("���ַ���\r\n");
		return ;
	}
	i=strlen((const char *)temp_password);
	target=target+i;   //��ַΪ���ݵ�ַ
	memset(temp_password, 0x00, 128);
	for(i=0; i<12; i++, target++)//��ֵ����10��λΪ�Ƿ�������2^32=4294967296
	{
		if((*target!='"'))
		{
			temp_password[i]=*target;
		}
		else
		{
			break;
		}
	}
	temp_password[i+1] = '\0';
	//printf("��ʱ����1111111111:%s\r\n",temp_password);
}


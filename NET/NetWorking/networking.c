#include "networking.h"  //包含需要的头文件
#include "stm32f10x.h"  //包含需要的头文件
#include "wifi.h"	    //包含需要的头文件
#include "delay.h"	    //包含需要的头文件
#include "usart.h"	    //包含需要的头文件
#include "usart2.h" 
#include "led.h"        //包含需要的头文件
#include "mqtt.h"       //包含需要的头文件
#include "timer.h"
#include <stdlib.h>     //包含需要的头文件

#include "dht11.h"
#include "mg995.h"
#include "beep.h"

char temp_password[16];  // 解析临时密码数组
extern unsigned char Temp_Password[4];
u8 door_off_on=0;

void connect_net(void)
{
		if(Connect_flag==1)      /*连接服务器成功*/
		{   
			Handle_TxDataBuff(); /*1、处理发送缓冲区数据*/		
			Handle_RxDataBuff(); /*2、处理接收缓冲区数据*/			
			Handle_CMDBuf();	 /*3、处理命令缓冲区数据*/
		}	
		else					
		{ 			
			ReconnectSever();   /*重启连接服务器*/
			BEEP=0;
			delay_ms(200);
			BEEP=1;
			delay_ms(200);
			BEEP=0;
			delay_ms(200);
			BEEP=1;
		}
}

/*处理发送缓冲区数据*/
void Handle_TxDataBuff(void)
{
		if(MQTT_TxDataOutPtr != MQTT_TxDataInPtr) //发送缓冲区有数据了
		{
		//3种情况可进入if
		//第1种：0x10 连接报文
		//第2种：0x82 订阅报文，且ConnectPack_flag置位，表示连接报文成功
		//第3种：SubcribePack_flag置位，说明连接和订阅均成功，其他报文可发
			if((MQTT_TxDataOutPtr[2]==0x10)||((MQTT_TxDataOutPtr[2]==0x82)&&(ConnectPack_flag==1))||(SubcribePack_flag==1))
				{    
//				printf("发送数据:0x%x\r\n",MQTT_TxDataOutPtr[2]);  //串口提示信息
				MQTT_TxData(MQTT_TxDataOutPtr);                       //发送数据（串口2发送缓冲区中的数据）
				MQTT_TxDataOutPtr += BUFF_UNIT; 				//指针下移
				
					if(MQTT_TxDataOutPtr==MQTT_TxDataEndPtr)              //如果指针到缓冲区尾部了
					MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];              //指针归位到缓冲区开头
				} 				
	      }	
}


/*处理接收缓冲区数据*/
void Handle_RxDataBuff(void)
{
		if(MQTT_RxDataOutPtr != MQTT_RxDataInPtr){//接收缓冲区有数据了														
//			printf("接收到数据:\r\n");
			
			/*2.1如果收到的是CONNECT报文*/
			//if判断，如果第一个字节是0x20，表示收到的是CONNACK报文
			//接着我们要判断第4个字节，看看CONNECT报文是否成功
			if(MQTT_RxDataOutPtr[2]==0x20) //收到的是CONNECT报文
				{             			
					switch(MQTT_RxDataOutPtr[5]){//判断CONNECT报文是否成功					
					case 0x00 :// printf("CONNECT报文成功\r\n");  
									ConnectPack_flag = 1;                                        //CONNECT报文成功，订阅报文可发
						break;                                                                                           
					case 0x01 : //printf("连接已拒绝，不支持的协议版本，准备重启\r\n");
								Connect_flag = 0;                                            //Connect_flag置零，重启连接
								break;
					case 0x02 : //printf("连接已拒绝，不合格的客户端标识符，准备重启\r\n");
								Connect_flag = 0;                                            //Connect_flag置零，重启连接
								break;
					case 0x03 : //printf("连接已拒绝，服务端不可用，准备重启\r\n");
								Connect_flag = 0;                                            //Connect_flag置零，重启连接
								break;
					case 0x04 : //printf("连接已拒绝，无效的用户名或密码，准备重启\r\n");
								Connect_flag = 0;                                            //Connect_flag置零，重启连接						
								break;
					case 0x05 : //printf("连接已拒绝，未授权，准备重启\r\n");
								Connect_flag = 0;                                            //Connect_flag置零，重启连接						
								break; 
					default   : //printf("连接已拒绝，未知状态，准备重启\r\n");
								Connect_flag = 0;                                            //Connect_flag置零，重启连接					
								break;							
				}				
			}		
			
			/*2.2如果收到的是SUBACK报文*/
			//判断，第一个字节是0x90，表示收到的是SUBACK报文
			//接着我们要判断订阅回复，看看是不是成功
			else if(MQTT_RxDataOutPtr[2]==0x90)//判断SUBACK报文是否成功		
				{
					switch(MQTT_RxDataOutPtr[6])
						{					
					case 0x00 :
					case 0x01 : //printf("订阅成功\r\n");
											SubcribePack_flag = 1;                //SubcribePack_flag置1，表示订阅报文成功，其他报文可发送
											Ping_flag = 0;                        //Ping_flag清零
											TIM4_ENABLE_30S();                    //启动30s的PING定时器 
											TempHumi_State();                     //先发一次数据
								break;                               
					default   : //printf("订阅失败，准备重启\r\n"); 
											Connect_flag = 0;                     //Connect_flag置零，重启连接
								break;						
				}					
			}
			
			/*2.3如果收到的是PINGRESP报文*/
			//判断，第一个字节是0xD0，表示收到的是PINGRESP报文
			else if(MQTT_RxDataOutPtr[2]==0xD0){ 
				//printf("PING报文回复\r\n");
				if(Ping_flag==1){             //如果Ping_flag=1，表示第一次发送
					 Ping_flag = 0;    				  //要清除Ping_flag标志
				}else if(Ping_flag>1){ 				//如果Ping_flag>1，表示是多次发送了，而且是2s间隔的快速发送
					Ping_flag = 0;     				  //要清除Ping_flag标志
					TIM4_ENABLE_30S(); 				  //PING定时器重回30s的时间
				}				
			}	
			
			/*2.4如果收到的是服务器发来的推送数据*/	
                //判断，如果第一个字节是0x30，表示收到的是服务器发来的推送数据
				//我们要提取控制命令			
			else if((MQTT_RxDataOutPtr[2]==0x30)){ //我们要提取控制命令
				//printf("服务器等级0推送\r\n");
				MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr);  //处理等级0推送数据
			}				
							
			MQTT_RxDataOutPtr += BUFF_UNIT;                   //指针下移
			if(MQTT_RxDataOutPtr==MQTT_RxDataEndPtr)          //如果指针到缓冲区尾部了
				MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];          //指针归位到缓冲区开头                        
		}	
}

/*处理命令缓冲区数据*/
void Handle_CMDBuf(void)
{
			if(MQTT_CMDOutPtr != MQTT_CMDInPtr)  //if成立的话，说明命令缓冲区有数据了
				{                             			       
				   //printf("命令:%s\r\n",&MQTT_CMDOutPtr[2]);                 //串口输出信息
					
					//警报处理
				   if(strstr((char *)MQTT_CMDOutPtr+2,"\"params\":{\"AlarmSwitch\":1}"))
					{                 //简单布尔型结果分析
					BEEP=0;//打开蜂鸣器
					//printf("AlarmSwitch_ON！         \r\n");
                    Device_State();
                    }
					
					else if(strstr((char *)MQTT_CMDOutPtr+2,"\"params\":{\"AlarmSwitch\":0}"))
					{
					BEEP=1;//关闭蜂鸣器	
					//printf("AlarmSwitch_OFF！         \r\n");
                    Device_State();
                    }
					
					
					//门处理
				  if(strstr((char *)MQTT_CMDOutPtr+2,"\"params\":{\"ELock\":1}"))
					{                 //简单布尔型结果分析
					door_off_on=1;
					door_ON();
					//printf("ELock_ON！         \r\n");
                    Device_State();
                    }
					
					else if(strstr((char *)MQTT_CMDOutPtr+2,"\"params\":{\"ELock\":0}"))
					{
					door_off_on=0;
					door_OFF();	
					//printf("ELock_OFF！         \r\n");
                    Device_State();
                    }


	           /*************************/
					if(strstr((char *)MQTT_CMDOutPtr+2, (const char *)"\"params\":{\"addOTP\"") != NULL)//专业数据分析，包括英文和数字
					{int i;		 
					  USER_GetSteeringJsonValue((char *)MQTT_CMDOutPtr+2, (char *)"addOTP");
					  //printf("临时密码:%s\r\n",temp_password);
							for( i=0;i<4;i++)
						{
							Temp_Password[i]=temp_password[i]-'0';
						}
//						char temp2[16];
//                      sscanf((char *)MQTT_CMDOutPtr+2, "%*[^/]/%[^@]", temp2);		
//					  printf("分析到的临时密码数据：%s\r\n",temp2);  
						
							
					}
						  MQTT_CMDOutPtr += BUFF_UNIT;                               	 //指针下移
						    if(MQTT_CMDOutPtr==MQTT_CMDEndPtr)           	             //如果指针到缓冲区尾部了
							   MQTT_CMDOutPtr = MQTT_CMDBuf[0];          	             //指针归位到缓冲区开头				
				}
}

/*重连服务器*/
void ReconnectSever(void)
{
		//printf("需要连接服务器\r\n");                    //串口输出信息
		TIM_Cmd(TIM1,DISABLE);                          //关闭TIM1 
		TIM_Cmd(TIM4,DISABLE);                          //关闭TIM4  
		WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区 
		if(WiFi_Connect_IoTServer()==0)
		{   			     		                  //如果WiFi连接云服务器函数返回0，表示正确，进入if
			//printf("建立TCP连接成功\r\n");             //串口输出信息
			Connect_flag = 1;                             //Connect_flag置1，表示连接成功	
			WiFi_RxCounter=0;                             //WiFi接收数据量变量清零                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);       //清空WiFi接收缓冲区 		
			MQTT_Buff_ReInit();                           //重新初始化发送缓冲区                    
		}			
}

/*--------------温湿度采集的数据-------------------*/
extern u8 humidityH;	  //湿度整数部分
extern u8 humidityL;	  //湿度小数部分
extern u8 temperatureH;   //温度整数部分
extern u8 temperatureL;   //温度小数部分
/*-------------------------------------------------*/
/*函数名：采集温湿度，并发布给服务器               */
/*参  数：无                                       */
/*返回值：无                    4s一次               */
/*-------------------------------------------------*/
void TempHumi_State(void)    
{  	
	char temp[128];  
	
	DHT11_Read_Data(&humidityH,&humidityL,&temperatureH,&temperatureL);
	
	//printf("温度：%d  湿度：%d\r\n",temperatureH,humidityH);

	sprintf(temp,"{\"method\":\"thing.event.property.post\",\"id\":\"203302322\",\"params\":{\"CuTemperature\":%2d,\"CurrentHumidity\":%2d},\"version\":\"1.0.0\"}",temperatureH,humidityH);  //构建回复湿度温度数据
	MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //添加数据，发布给服务器	
	
}


void Device_State(void)    
{  	
	char temp[128];  

	sprintf(temp,"{\"method\":\"thing.event.property.post\",\"id\":\"203302322\",\"params\":{\"AlarmSwitch\":%d,\"ELock\":%d},\"version\":\"1.0.0\"}",alarm,door_off_on);  //构建回复AlarmSwitch数据
	MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //添加数据，发布给服务器	
	
}

void Reporting_data_flow(void)
{
	TempHumi_State(); 
}

/*******************************************************************
*函数：unsigned char *USER_GetSteeringJsonValue(char *cJson, char *Tag)
*功能：json为字符串序列，将json格式中的目标对象Tag对应的值字符串转换为字符串
*输入：
		char *cJson json字符串
		char *Tag 要操作的对象标签
*输出：返回数值
*特殊说明：用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
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
		//printf("空字符！\r\n");
		return ;
	}
	i=strlen((const char *)temp_password);
	target=target+i;   //地址为数据地址
	memset(temp_password, 0x00, 128);
	for(i=0; i<12; i++, target++)//数值超过10个位为非法，由于2^32=4294967296
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
	//printf("临时密码1111111111:%s\r\n",temp_password);
}


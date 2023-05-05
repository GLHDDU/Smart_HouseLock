#ifndef __NETWORKING_H
#define __NETWORKING_H

#include "usart.h"	    //������Ҫ��ͷ�ļ�

#define alarm  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)      //��ȡ����alarm

void connect_net(void);
void TempHumi_State(void);
void Device_State(void);
void Handle_TxDataBuff(void);
void Handle_RxDataBuff(void);
void Handle_CMDBuf(void);
void ReconnectSever(void);
void USER_GetSteeringJsonValue(char *cJson, char *Tag);

void Reporting_data_flow(void);

#endif



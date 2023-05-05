

#ifndef _doorlock_H
#define _doorlock_H

#include "sys.h"


//函数声明


void ShowErrMessage(u8 ensure);//显示确认码错误信息
void press_FR(void);           //刷指纹
void Add_FR(void);             //录指纹
void Del_FR(void);             //删除指纹

#endif

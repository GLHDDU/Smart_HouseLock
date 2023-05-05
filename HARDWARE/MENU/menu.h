#ifndef _GUI_H_
#define	_GUI_H_

#include "stm32f10x.h"

#define ON 1
#define OFF 0
typedef struct
{
	u8 Cur_Index;//当前索引项
	u8 previous;//上一页
	u8 next;//下一页
	u8 enter;//确认
	u8 back;//返回
	void (*current_operation)(u8,u8);//	当前索引执行的函数(界面)
}Main_Menu;

//各界面的索引值
enum
{
	_Main_UI=0,//主界面0
	
	_fingerprint_Option,//选择指纹界面1
	_card_Option,//选择刷卡界面2
	_password_Option,//选择矩阵按键界面3
	
	_fingerprint_Child,//指纹的主子界面4
	
	_fingerprint_Child_add_Option,//指纹子界面-选择增加--菜单5
	_fingerprint_Child_del_Option,//指纹子界面-选择删除--菜单6
	
	_fingerprint_Child_add_menu,  //指纹子界面-增加指纹--界面7
	_fingerprint_Child_del_menu,  //指纹子界面-删除指纹--界面8
	
	_card_Child,//刷卡子界面--是否成功9
	_password_Child,//矩阵按键子界面--是否成功10
	_OLED_Lock,//OLED熄屏11
	
};

//按键索引值
enum
{
	KEY_PREVIOUS=1,
	KEY_NEXT,
	KEY_ENTER,	
	KEY_BACK
};

void GUI_Refresh(void);//函数功能：刷新界面

void Main_UI(u8 page_index,u8 key_val);//函数功能：显示主界面
void Main_Menu_Func(u8 page_index,u8 key_val);//函数功能：主菜单显示函数

void fingerprint_Child_menu(u8 page_index,u8 key_val);//函数功能：fingerprint_Child_menu指纹的主子页面
void fingerprint_Child_menu_Option(u8 page_index,u8 key_val);//函数功能：fingerprint_Child_menu_Option指纹功能选择页面
void Fingerprint_Child_add_menu(u8 page_index,u8 key_val);//函数功能：Fingerprint_Child_add_menu指纹增加页面
void Fingerprint_Child_dle_menu(u8 page_index,u8 key_val);//函数功能：Fingerprint_Child_dle_menu指纹减少页面


void card_Child(u8 page_index,u8 key_val);//函数功能：card_Child选项子菜单
void password_Child(u8 page_index,u8 key_val);//函数功能：password_Child选项子菜单


void OLED_Lock(u8 page_index,u8 key_val);//函数功能：OLED熄屏

void Administrator_password_UI(void);  //管理员密码界面
void Input_common_password(void); //输入密码
void BEEP_dd(void);	//蜂鸣器开启
u8  pipei(void);	//进来密码匹配
#endif

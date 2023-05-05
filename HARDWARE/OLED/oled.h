/*
		  OLED（IIC）模块-->单片机IO
		  VCC-->5V
		  GND-->GND
		  SCL-->PB6
		  SDA-->PB7
*/
#ifndef _oled_H
#define _oled_H

#include "sys.h"
//旋转角度显示
#define USE_HORIZONTAL 270

#define SIZE 16
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


//OLED控制用函数
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Set_Pos(unsigned char x, unsigned char y);	   							   		    
void OLED_Init(void);
void OLED_Refresh_Gram(void); //刷新屏幕显示
void OLED_Clear(void);//清屏
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);//对角填充
void OLED_Fill_rectangle(u8 x0,u8 y0,u8 width,u8 height,u8 color);
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2,u8 color);
void OLED_DrawRectangle(u8 x1, u8 y1, u8 x2, u8 y2,u8 color);
void OLED_Draw_Circle(u8 x0,u8 y0,u8 r,u8 color);
void OLED_Draw_Bigpoint(u8 x0,u8 y0,u8 color);
void OLED_Draw_vline(u8 x0,u8 y0,u8 len,u8 color);
void OLED_Draw_hline(u8 x0,u8 y0,u8 len,u8 color);
void OLED_Fill_circle(u8 x0,u8 y0,u8 r,u8 color);
void OLED_Draw_ellipse(u8 x0, u8 y0, u8 rx, u8 ry,u8 color);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size); //显示字符串
void OLED_ShowFontHZ(u8 x,u8 y,u8 pos,u8 size,u8 mode);//显示汉字
void OLED_DrawBMP(u8 x0, u8 y0,u8 x1, u8 y1,u8 BMP[]); //显示图片

#endif

#ifndef _LCD_H_
#define  _LCD_H_
#include"stm32f4xx_conf.h"
#define SCK_SETB GPIOB->BSRRL = GPIO_Pin_11
#define SCK_CLR GPIOB->BSRRH = GPIO_Pin_11
#define SDI_SETB GPIOB->BSRRL = GPIO_Pin_12
#define SDI_CLR GPIOB->BSRRH = GPIO_Pin_12
#define DC_SETB  GPIOB->BSRRL=GPIO_Pin_13
#define DC_CLR    GPIOB->BSRRH=GPIO_Pin_13
#define REST_SETB GPIOB->BSRRL=GPIO_Pin_14
#define REST_CLR   GPIOB->BSRRH=GPIO_Pin_14
#define CS_SETB   GPIOB->BSRRL=GPIO_Pin_15
#define CS_CLR     GPIOB->BSRRH=GPIO_Pin_15
#define BLACK 0
#define WHITE 0xffff
#define FONT 16
#define START 0
#define MAX_SHOW_X 320
#define MAX_SHOW_Y 240
#define PROJECT_COLOR 0xff00
extern u8 const ASCII[1520];
extern u16 BACK_COLOR,FONT_COLOR;
typedef struct
{
	char Index[2];
	u8 Msk[32];
}GB_TypeDef;
extern GB_TypeDef const GB2312[];
void LCD_Init(void);
void LCD_Clear(u16 Color);
void LCD_WR_BYTE(u8 byte);
void LCD_WR_REG(u8 byte);
void LCD_WR_DATA8(u8 byte);
void LCD_WR_DATA16(u16 byte);
void LCD_SetAdd(u16 x,u16 y,u16 x1,u16 y1);
void LCD_ASCII(u16 x,u16 y,u16 dat,u16 Color);
void LCD_Chine(u16 x,u16 y,char *dat,u16 Color);
void LCD_Font(u16 x,u16 y,void const *dd,u16 Color);
void LCD_Addr_Clear(u16 x,u16 y,u16 x1,u16 y1,u16 Color);//清除制定位置
void LCD_Chine(u16 x,u16 y,char *dat,u16 Color);//中文显示
void LCD_ASCII(u16 x,u16 y,u16 dat,u16 Color);//ASCII显示
void LCD_Font(u16 x,u16 y,void const *dd,u16 Color);//中文ASCII连续显示
void LCD_Line(u16 x,u16 y,u16 x1,u16 y1,u16 Color);//画线
void LCD_Point(u16 x,u16 y,u16 Color);//画点
void LCD_Line_Rectlar(u16 x,u16 y,u16 x1,u16 y1,u16 Color);//画矩形
#endif

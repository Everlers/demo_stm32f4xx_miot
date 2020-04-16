#ifndef _ILI9325_H_
#define _ILI9325_H_
#include "stm32f4xx_conf.h"
#define Memalloc 0 //1:使用动态内存 0:不使用动态内存 创建数组
#define GBKADDR 0  //1:使用SPIFLASH 0:使用FATFS GBK16.fon文件
#define   BLACK                0x0000                // 黑色：    0,   0,   0 //
#define   BLUE                 0x001F                // 蓝色：    0,   0, 255 //
#define   GREEN                0x07E0                // 绿色：    0, 255,   0 //
#define   CYAN                 0x07FF                // 青色：    0, 255, 255 //
#define   RED                  0xF800                // 红色：  255,   0,   0 //
#define   MAGENTA              0xF81F                // 品红：  255,   0, 255 //
#define   YELLOW               0xFFE0                // 黄色：  255, 255, 0   //
#define   WHITE                0xFFFF                // 白色：  255, 255, 255 //
#define   NAVY                 0x000F                // 深蓝色：  0,   0, 128 //
#define   DGREEN               0x03E0                // 深绿色：  0, 128,   0 //
#define   DCYAN                0x03EF                // 深青色：  0, 128, 128 //
#define   MAROON               0x7800                // 深红色：128,   0,   0 //
#define   PURPLE               0x780F                // 紫色：  128,   0, 128 //
#define   OLIVE                0x7BE0                // 橄榄绿：128, 128,   0 //
#define   LGRAY                0xC618                // 灰白色：192, 192, 192 //
#define   DGRAY                0x7BEF                // 深灰色：128, 128, 128 //
#define Bank1_LCD_D    ((u32)0x6Cff0000)    //Disp Data ADDR
#define Bank1_LCD_C    ((u32)0x6Cf10000)	   //Disp Reg ADDR
#define LCD_RAM        *((u16 *)0x6cff0000)
#define LED_ON GPIOB->BSRRL = GPIO_Pin_11
#define LED_OFF GPIOB->BSRR = GPIO_Pin_11
void LCD_Init(void);
void WriteData(u16 tem_data);
void LCD_Setaddr(u16 XStart,u16 YStart,u16 XEnd,u16 YEnd);
void LCD_Clear(u16 XStart,u16 YStart,u16 XEnd,u16 YEnd,u16 Color);
void LCD_Chine(u16 x,u16 y,char *dat,u16 Color);//????
void LCD_ASCII(u16 x,u16 y,u16 dat,u16 Color);//ASCII??
void LCD_Font(u16 x,u16 y,void const *dd,u16 Color);//??ASCII????
void LCD_Line(u16 x,u16 y,u16 x1,u16 y1,u16 Color);//画线
void LCD_Point(u16 x,u16 y,u16 Color);//画点
void LCD_Line_Rectlar(u16 x,u16 y,u16 x1,u16 y1,u16 Color);//画矩形
u16 GetPoint(u16 x,u16 y);//读一个像素
extern u16 FONT_COLOR,BACK_COLOR;
extern u8 const ASCII[1520];
/*
PD14 -----FSMC_D0  ----D0
PD15 -----FSMC_D1  ----D1
PD0   -----FSMC_D2  ----D2
PD1   -----FSMC_D3  ----D3
PE7    -----FSMC_D4  ---D4
PE8    -----FSMC_D5  ---D5
PE9    -----FSMC_D6  ---D6
PE10  -----FSMC_D7   ----D7
PE11  -----FSMC_D8   ----D8
PE12  -----FSMC_D9   ----D9
PE13  -----FSMC_D10   ----D10
PE14  -----FSMC_D11   ----D11
PE15  -----FSMC_D12   ----D12
PD8   -----FSMC_D13   ----D13
PD9   -----FSMC_D14   ----D14
PD10 -----FSMC_D15   ----D15
PD4   -----FSMC_NOE -----RD
PD5   -----FSMC_NWE ----WR
PG12    -----FSMC_NE1  ----CS
PD11 -----FSMC_A16   ----RS

PB5-------------------LCD_BL8
*/
#endif

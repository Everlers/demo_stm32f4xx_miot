#include"ILI9325.h"
#include"delay.h"
#include"ff.h"
#include"diskio.h"
#include"FSMC_Memalloc.h"
#include"bsp_spi_Flash.h"
u16 FONT_COLOR,BACK_COLOR;
void LCD_Point(u16 x,u16 y,u16 Color)
{
	LCD_Setaddr(x,y,x,y);
	WriteData(Color);
}
void LCD_Line_Rectlar(u16 x,u16 y,u16 x1,u16 y1,u16 Color)
{
	LCD_Line(x,y,x1,y,Color);
	LCD_Line(x,y,x,y1,Color);
	LCD_Line(x,y1,x1,y1,Color);
	LCD_Line(x1,y,x1,y1,Color);
}
__inline void swap_int(u16 *a, u16 *b)   
{    
    *a ^= *b;    
    *b ^= *a;    
    *a ^= *b;    
} 
void LCD_Line(u16 x,u16 y,u16 x1,u16 y1,u16 Color)
{
	   u16 dx =( x1 - x), // ????   
        dy =( y1 - y), // ????   
        yy = 0;         // ??????45??
   
 int ix ,    
        iy,    
        cx ,    
        cy ,    
        n2dy,    
        n2dydx ,    
        d ; 
	FONT_COLOR = Color;
    if(dx < dy)  // ????? 1 ?,dx?dy ??, ?????45?
    {    
        yy = 1;              // ?????  
        swap_int(&x, &y);  // ????  
        swap_int(&x1, &y1);    
        swap_int(&dx, &dy);    
    }        
    ix = (x1 - x) > 0 ? 1 : -1; // ?????1????    
    iy = (y1 - y) > 0 ? 1 : -1;    
    cx = x;  //  x??  
    cy = y;  //  y??  
    n2dy = dy * 2;       // ?????????
     n2dydx = (dy - dx) * 2;    
    d = dy * 2 - dx;     

// ????? x ??????45?    
    if(yy)  
    {   
        while(cx != x1) // ????????  
        {    
            if(d < 0)  
            {    
                d += n2dy;    
            }   
            else   
            {    
                cy += iy;    
                d += n2dydx;    
            }    
            LCD_Point( cy, cx,Color); // ??   
            cx += ix;    
        }    
    }  
  
// ????? x ??????45?    
    else   
    {   
        while(cx != x1)   
        {    
            if(d < 0)  
            {    
                d += n2dy;    
            }  
            else   
            {    
                cy += iy;    
                d += n2dydx;    
            }    
            LCD_Point( cx, cy,Color);    
            cx += ix;    
        }    
    }    
}
FIL GBK_File;
u32 GBK_Len;
void LCD_Font(u16 x,u16 y,void const *dd,u16 Color)
{
	u8 *dat;
	#if Memalloc==1
	u8 *GBK_Addr;
	GBK_Addr=Malloc(32);
	#else 
	u32 GBK_Addr[32];
	#endif
	dat=(u8 *)dd;
	
	
	while(*dat!=0)
	{
		if(*dat=='\r')y+=16;
		else if(*dat<127)
		{
			LCD_ASCII(x,y,*dat,Color);
			x+=8;
			dat+=1;
		}
		else
		{
			u32 Data_Place;
			if(dat[1]<0x7f)
				Data_Place=((dat[0]-0x81)*190+(dat[1]-0x40))*32;
			if(dat[1]>0x80)
				Data_Place=((dat[0]-0x81)*190+(dat[1]-0x41))*32;
			#if GBKADDR==1
			sf_ReadBuffer(GBK_Addr,Data_Place,32);
			#else
			f_open(&GBK_File,"GBK16.fon",FA_READ);
			f_lseek(&GBK_File,Data_Place);
			f_read(&GBK_File,GBK_Addr,32,&GBK_Len);
			#endif
			LCD_Chine(x,y,(char *)GBK_Addr,Color);
			#if Malloc==1
			Delete_Malloc(GBK_Addr);
			#endif
			x+=16;
			dat+=2;
		}
	}
}
void LCD_Chine(u16 x,u16 y,char *dat,u16 Color)
{
	u8 Buff;
	u8 xl,yl,i;
	LCD_Setaddr(x,y,x+16-1,y+16-1);
	for(xl=0;xl<16;xl++)
	{
		for(yl=0;yl<2;yl++)
		{
			Buff=*dat;dat++;
			for(i=0;i<8;i++)
			{
				if(Buff&0x80)WriteData(Color);
				else WriteData(BACK_COLOR);
				Buff<<=1;
			}
		}
	}
}
void LCD_ASCII(u16 x,u16 y,u16 dat,u16 Color)
{
	u32 i,z;
	u8 Buff;
	LCD_Setaddr(x,y,x+8-1,y+16-1);
	dat=((dat-' ')*16);
	for(i=0;i<16;i++)
	{
		Buff=ASCII[dat+i];
		for(z=0;z<8;z++)
		{
			if(Buff&0x01)
			WriteData(Color);
			else WriteData(BACK_COLOR);
			Buff>>=1;
		}
	}
} 
__inline void WriteComm(u16 CMD)
{	
	*(__IO u16 *) (Bank1_LCD_C) = CMD;
}
__inline void WriteData(u16 tem_data)
{		
	*(__IO u16 *) (Bank1_LCD_D) = tem_data;
}
u16 GetPoint(u16 x,u16 y)
{
	 u16 a,b;
	WriteComm(0x2a);   
	WriteData(x>>8);
	WriteData(x);
	WriteData(x>>8);
	WriteData(x);

	WriteComm(0x2b);   
	WriteData(y>>8);
	WriteData(y);
	WriteData(y>>8);
	WriteData(y);

	WriteComm(0x2e);
	
	a = *(__IO u16 *) (Bank1_LCD_D);a=1;while(--a);
	a = *(__IO u16 *) (Bank1_LCD_D);
	b = *(__IO u16 *) (Bank1_LCD_D);
// 	printf("RIN=%04x\r\n",b);

	return (a&0xf800)|((a&0x00fc)<<3)|(b>>11);
}
void LCD_Setaddr(u16 XStart,u16 YStart,u16 XEnd,u16 YEnd)
{
	WriteComm(0x2a);
	WriteData(XStart>>8);
	WriteData(XStart&0x00ff);
	WriteData(XEnd>>8);
	WriteData(XEnd&0x00ff);
	WriteComm(0x2b);
	WriteData(YStart>>8);
	WriteData(YStart&0x00ff);
	WriteData(YEnd>>8);
	WriteData(YEnd&0x00ff);
	WriteComm(0x2c);
}
void LCD_Clear(u16 XStart,u16 YStart,u16 XEnd,u16 YEnd,u16 Color)
{
	u32 res;
	LCD_Setaddr(XStart,YStart,XEnd-1,YEnd-1);
	for(res=0;res<((XEnd-XStart)*(YEnd-YStart));res++)
	*(__IO u16 *) (Bank1_LCD_D) = Color;
}
static void LCD_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_Struct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOB
	|RCC_AHB1Periph_GPIOG,ENABLE);
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);
	GPIO_Struct.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8
	|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Struct.GPIO_Mode =  GPIO_Mode_AF;
	GPIO_Struct.GPIO_OType = GPIO_OType_PP;
	GPIO_Struct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Struct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD,&GPIO_Struct);
	
	GPIO_Struct.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12
	|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Struct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Struct.GPIO_OType = GPIO_OType_PP;
	GPIO_Struct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Struct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE,&GPIO_Struct);
	
	GPIO_Struct.GPIO_Pin = GPIO_Pin_12;
	GPIO_Struct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Struct.GPIO_OType = GPIO_OType_PP;
	GPIO_Struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Struct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOG,&GPIO_Struct);
	
	GPIO_Struct.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_Struct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Struct.GPIO_OType = GPIO_OType_PP;
	GPIO_Struct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Struct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_Struct);
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);
	
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource12,GPIO_AF_FSMC);
	
	GPIOG->BSRRL = GPIO_Pin_12;
	GPIOD->BSRRL = GPIO_Pin_4;
	GPIOD->BSRRL = GPIO_Pin_5;
	GPIOD->BSRRL = GPIO_Pin_11;
	
	GPIOB->BSRRH = GPIO_Pin_10|GPIO_Pin_11;
	GPIOB->BSRRL = GPIO_Pin_10;
	
}
static void LCD_FSMC_Config(void)
{    
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  	FSMC_NORSRAMTimingInitTypeDef  p;
	  LCD_GPIO_Config();                                                     //LCD????
  	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);
  	p.FSMC_AddressSetupTime = 03;                                           //??????
  	p.FSMC_AddressHoldTime = 0;                                            //??????
  	p.FSMC_DataSetupTime = 5;                                              //??????
  	p.FSMC_BusTurnAroundDuration = 0;
  	p.FSMC_CLKDivision = 0;
  	p.FSMC_DataLatency = 0;
  	p.FSMC_AccessMode = FSMC_AccessMode_A;                                  // ???A??LCD
  	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
  	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;
  	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);   
  	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE); 
		
}
static void LCD_Rst(void)
{			
	GPIO_ResetBits(GPIOB, GPIO_Pin_10);
	delay_ms(100);					   
	GPIO_SetBits(GPIOB, GPIO_Pin_10);		 	 
	delay_ms(100);	
}
void LCD_Init(void)
{	
	LCD_FSMC_Config();
	delay_ms(1);
	LCD_Rst();
	delay_ms(1);
	WriteComm(0x11);
	delay_ms(1);

	WriteComm(0xB0);
	WriteData(0x04);

	WriteComm(0xB3);//Frame Memory Access and Interface Setting
	WriteData(0x02);
	WriteData(0x00);

	WriteComm(0xC1);//Panel Driving Setting
	WriteData(0x23);
	WriteData(0x31);//NL
	WriteData(0x99);
	WriteData(0x21);
	WriteData(0x20);
	WriteData(0x00);
	WriteData(0x10);//DIVI
	WriteData(0x28);//RTN
	WriteData(0x0C);//BP
	WriteData(0x0A);//FP
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x21);
	WriteData(0x01);

	WriteComm(0xC2);//Display V-Timing Setting
	WriteData(0x00);
	WriteData(0x06);
	WriteData(0x06);
	WriteData(0x01);
	WriteData(0x03);
	WriteData(0x00);

	WriteComm(0xC8);//GAMMA
	WriteData(0x01);
	WriteData(0x0A);
	WriteData(0x12);
	WriteData(0x1C);
	WriteData(0x2B);
	WriteData(0x45);
	WriteData(0x3F);
	WriteData(0x29);
	WriteData(0x17);
	WriteData(0x13);
	WriteData(0x0F);
	WriteData(0x04);

	WriteData(0x01);
	WriteData(0x0A);
	WriteData(0x12);
	WriteData(0x1C);
	WriteData(0x2B);
	WriteData(0x45);
	WriteData(0x3F);
	WriteData(0x29);
	WriteData(0x17);
	WriteData(0x13);
	WriteData(0x0F);
	WriteData(0x04);

	WriteComm(0xC9);//GAMMA
	WriteData(0x01);
	WriteData(0x0A);
	WriteData(0x12);
	WriteData(0x1C);
	WriteData(0x2B);
	WriteData(0x45);
	WriteData(0x3F);
	WriteData(0x29);
	WriteData(0x17);
	WriteData(0x13);
	WriteData(0x0F);
	WriteData(0x04);

	WriteData(0x01);
	WriteData(0x0A);
	WriteData(0x12);
	WriteData(0x1C);
	WriteData(0x2B);
	WriteData(0x45);
	WriteData(0x3F);
	WriteData(0x29);
	WriteData(0x17);
	WriteData(0x13);
	WriteData(0x0F);
	WriteData(0x04);

	WriteComm(0xCA);//GAMMA
	WriteData(0x01);
	WriteData(0x0A);
	WriteData(0x12);
	WriteData(0x1C);
	WriteData(0x2B);
	WriteData(0x45);
	WriteData(0x3F);
	WriteData(0x29);
	WriteData(0x17);
	WriteData(0x13);
	WriteData(0x0F);
	WriteData(0x04);

	WriteData(0x01);
	WriteData(0x0A);
	WriteData(0x12);
	WriteData(0x1C);
	WriteData(0x2B);
	WriteData(0x45);
	WriteData(0x3F);
	WriteData(0x29);
	WriteData(0x17);
	WriteData(0x13);
	WriteData(0x0F);
	WriteData(0x04);

	WriteComm(0xD0);//Power Setting (Charge Pump Setting)
	WriteData(0x99);//DC
	WriteData(0x03);
	WriteData(0xCE);
	WriteData(0xA6);
	WriteData(0x00);//CP or SR
	WriteData(0x43);//VC3, VC2
	WriteData(0x20);
	WriteData(0x10);
	WriteData(0x01);
	WriteData(0x00);
	WriteData(0x01);
	WriteData(0x01);
	WriteData(0x00);
	WriteData(0x03);
	WriteData(0x01);
	WriteData(0x00);

	WriteComm(0xD3);//Power Setting for Internal Mode
	WriteData(0x33);//AP

	WriteComm(0xD5);//VPLVL/VNLVL Setting
	WriteData(0x2A);
	WriteData(0x2A);

	WriteComm(0xD6);//
	WriteData(0xA8);//

	WriteComm(0xD6);//
	WriteData(0x01);//

	WriteComm(0xDE);//VCOMDC Setting
	WriteData(0x01);
	WriteData(0x4F);

	WriteComm(0xE6);//VCOMDC Setting
	WriteData(0x4F);

	WriteComm(0xFA);//VDC_SEL Setting
	WriteData(0x03);

	delay_ms(1);

	WriteComm(0x2A);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x01);
	WriteData(0xDF);

	WriteComm(0x2B);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x03);
	WriteData(0x1F);

	WriteComm(0x36);
	WriteData(0x60);

	WriteComm(0x3A);
	WriteData(0x55);

	WriteComm(0x29);
	delay_ms(1);

	WriteComm(0x2C); 
	delay_ms(1);
	bsp_InitSFlash();
	LCD_Clear(0,0,800,480,BLACK);
	LED_ON;
}

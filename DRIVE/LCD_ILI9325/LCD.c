#include"stm32f4xx_conf.h"
#include"delay.h"
#include"LCD.h"
#include"ff.h"
#include"diskio.h"
u16 BACK_COLOR,FONT_COLOR;
void LCD_Point(u16 x,u16 y,u16 Color)
{
	LCD_SetAdd(x,y,x,y);
	LCD_WR_DATA16(Color);
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
        yy = 0;         // ??????45°??
   
 int ix ,    
        iy,    
        cx ,    
        cy ,    
        n2dy,    
        n2dydx ,    
        d ; 
	FONT_COLOR = Color;
    if(dx < dy)  // ????? 1 ?,dx?dy ??, ?????45° 
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
void LCD_Font(u16 x,u16 y,void const *dd,u16 Color)
{
	u8 i;
	u8 *dat;
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
			for(i=0;i<10;i++)
			{
				if((GB2312[i].Index[0]==dat[0])&&(GB2312[i].Index[1]==dat[1]))
				{
					LCD_Chine(x,y,(char *)&GB2312[i].Msk,Color);
					x+=16;
					dat+=2;
					break;
				}
			}
		}
	}
}
void LCD_Chine(u16 x,u16 y,char *dat,u16 Color)
{
	u8 Buff;
	u8 xl,yl,i;
	LCD_SetAdd(x,y,x+16-1,y+16-1);
	for(xl=0;xl<16;xl++)
	{
		for(yl=0;yl<2;yl++)
		{
			Buff=*dat;dat++;
			for(i=0;i<8;i++)
			{
				if(Buff&0x01)LCD_WR_DATA16(Color);
				else LCD_WR_DATA16(BACK_COLOR);
				Buff>>=1;
			}
		}
	}
}
void LCD_ASCII(u16 x,u16 y,u16 dat,u16 Color)
{
	u32 i,z;
	u8 Buff;
	LCD_SetAdd(x,y,x+8-1,y+16-1);
	dat=((dat-' ')*16);
	for(i=0;i<16;i++)
	{
		Buff=ASCII[dat+i];
		for(z=0;z<8;z++)
		{
			if(Buff&0x01)
			LCD_WR_DATA16(Color);
			else LCD_WR_DATA16(BACK_COLOR);
			Buff>>=1;
		}
	}
}
void LCD_Addr_Clear(u16 x,u16 y,u16 x1,u16 y1,u16 Color)
{
	u32 i;
	u32 Len;
	LCD_SetAdd(x,y,x1,y1);
	Len=((x1-x+1)*(y1-y+1));
	DC_SETB;
	BACK_COLOR=Color;
	for(i=0;i<Len;i++)
	{
		LCD_WR_DATA16(Color);
	}
}
void LCD_SetAdd(u16 x,u16 y,u16 x1,u16 y1)
{
	LCD_WR_REG(0x2a);
	LCD_WR_DATA16(x);
	LCD_WR_DATA16(x1);
	LCD_WR_REG(0x2b);
	LCD_WR_DATA16(y);
	LCD_WR_DATA16(y1);
	LCD_WR_REG(0x2c);
}
void LCD_Clear(u16 Color)
{
	u16 x,y;
	LCD_SetAdd(0,0,319,239);
	for(x=0;x<240;x++)
	{
		for(y=0;y<320;y++)
		LCD_WR_DATA16(Color);
	}
}
__inline void LCD_WR_DATA16(u16 byte)
{
	u8 i;
	DC_SETB;
	for(i=0;i<16;i++)
	{
		if(byte&0x8000)SDI_SETB;
		else SDI_CLR;
		SCK_CLR;
		byte<<=1;
		SCK_SETB;
	}
}
__inline void LCD_WR_DATA8(u8 byte)
{
	u8 i;
	DC_SETB;
	for(i=0;i<8;i++)
	{
		if(byte&0x80)SDI_SETB;
		else SDI_CLR;
		SCK_CLR;
		byte<<=1;
		SCK_SETB;
	}
}
__inline void LCD_WR_REG(u8 byte)
{
	u8 i;
	DC_CLR;
	for(i=0;i<8;i++)
	{
		if(byte&0x80)SDI_SETB;
		else SDI_CLR;
		SCK_CLR;
		byte<<=1;
		SCK_SETB;
	}
}
void LCD_Init(void)
{
  GPIO_InitTypeDef GPIO_Struct;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	GPIO_Struct.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Struct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Struct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Struct.GPIO_OType = GPIO_OType_PP;
	GPIO_Struct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB,&GPIO_Struct);
	REST_CLR;
	delay_ms(20);
	REST_SETB;
	delay_ms(20);
	CS_CLR;

				LCD_WR_REG(0xCB);  
        LCD_WR_DATA8(0x39); 
        LCD_WR_DATA8(0x2C); 
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0x34); 
        LCD_WR_DATA8(0x02); 

        LCD_WR_REG(0xCF);  
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0XC1); 
        LCD_WR_DATA8(0X30); 
 
        LCD_WR_REG(0xE8);  
        LCD_WR_DATA8(0x85); 
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0x78); 
 
        LCD_WR_REG(0xEA);  
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0x00); 
 
        LCD_WR_REG(0xED);  
        LCD_WR_DATA8(0x64); 
        LCD_WR_DATA8(0x03); 
        LCD_WR_DATA8(0X12); 
        LCD_WR_DATA8(0X81); 

        LCD_WR_REG(0xF7);  
        LCD_WR_DATA8(0x20); 
  
        LCD_WR_REG(0xC0);    //Power control 
        LCD_WR_DATA8(0x23);   //VRH[5:0] 
 
        LCD_WR_REG(0xC1);    //Power control 
        LCD_WR_DATA8(0x10);   //SAP[2:0];BT[3:0] 
 
        LCD_WR_REG(0xC5);    //VCM control 
        LCD_WR_DATA8(0x3e); //¶Ô±È¶Èµ÷½Ú
        LCD_WR_DATA8(0x28); 
 
        LCD_WR_REG(0xC7);    //VCM control2 
        LCD_WR_DATA8(0x86);  //--
 
        LCD_WR_REG(0x36);    // Memory Access Control 
        LCD_WR_DATA8(0x28); //C8	   //48 68ÊúÆÁ//28 E8 ºáÆÁ

        LCD_WR_REG(0x3A);    
        LCD_WR_DATA8(0x55); 

        LCD_WR_REG(0xB1);    
        LCD_WR_DATA8(0x00);  
        LCD_WR_DATA8(0x18); 
 
        LCD_WR_REG(0xB6);    // Display Function Control 
        LCD_WR_DATA8(0x08); 
        LCD_WR_DATA8(0x82);
        LCD_WR_DATA8(0x27);  
 
        LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
        LCD_WR_DATA8(0x00); 
 
        LCD_WR_REG(0x26);    //Gamma curve selected 
        LCD_WR_DATA8(0x01); 
 
        LCD_WR_REG(0xE0);    //Set Gamma 
        LCD_WR_DATA8(0x0F); 
        LCD_WR_DATA8(0x31); 
        LCD_WR_DATA8(0x2B); 
        LCD_WR_DATA8(0x0C); 
        LCD_WR_DATA8(0x0E); 
        LCD_WR_DATA8(0x08); 
        LCD_WR_DATA8(0x4E); 
        LCD_WR_DATA8(0xF1); 
        LCD_WR_DATA8(0x37); 
        LCD_WR_DATA8(0x07); 
        LCD_WR_DATA8(0x10); 
        LCD_WR_DATA8(0x03); 
        LCD_WR_DATA8(0x0E); 
        LCD_WR_DATA8(0x09); 
        LCD_WR_DATA8(0x00); 

        LCD_WR_REG(0XE1);    //Set Gamma 
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0x0E); 
        LCD_WR_DATA8(0x14); 
        LCD_WR_DATA8(0x03); 
        LCD_WR_DATA8(0x11); 
        LCD_WR_DATA8(0x07); 
        LCD_WR_DATA8(0x31); 
        LCD_WR_DATA8(0xC1); 
        LCD_WR_DATA8(0x48); 
        LCD_WR_DATA8(0x08); 
        LCD_WR_DATA8(0x0F); 
        LCD_WR_DATA8(0x0C); 
        LCD_WR_DATA8(0x31); 
        LCD_WR_DATA8(0x36); 
        LCD_WR_DATA8(0x0F); 
 
        LCD_WR_REG(0x11);    //Exit Sleep 
        delay_ms(120); 
				
        LCD_WR_REG(0x29);    //Display on 
        LCD_WR_REG(0x2c); 
}

#include "lcd_tft.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ili9341.h"
#include "ascii.c"
#include "16_32.c"
#include "32_64.c"
#include "FreeRTOS.h"
#include "task.h"

//��ʼ��
void lcd_init(void)
{
	ili_blk_off();//�رձ���
	ili_init();//��ʼ��������ʾоƬ
	lcd_clear(BLACK);//�����ɫ
}
//���ô���
void lcd_set_win(u16 x,u16 y,u16 xend,u16 yend)
{
	ili_addr_set(x,y,xend,yend);
}
//�����ɫ
void lcd_clear(u16 color)
{
	u32 i;
	ili_addr_set(0,0,LCD_H,LCD_W);
	for(i=0;i<320*240;i++)
		ili_write_short(color);
}
u16 RGB888_565(u32 rgb)
{
	return RGB565(rgb>>24,rgb>>16,rgb);
}
//RGB888-RGB565
u16 RGB565 (u8 r,u8 g,u8 b)
{
	u16 rgb;
	rgb  = (u16)(r&0xF8) << 8;
	rgb |= (u16)(g&0xFC) << 3;
	rgb |= (u16)(b&0xF8) >> 3; 
	return rgb;
}
//����
void lcd_draw_point(u16 x,u16 y,u16 color)
{
	ili_addr_set(x,y,x,y);
	ili_write_short(color);
}
//��ָ�����������ָ����ɫ
//�����С:
//  (xend-xsta)*(yend-ysta)
void lcd_fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 i,j; 
	ili_addr_set(xsta,ysta,xend,yend);      //���ù��λ�� 
	for(i=ysta;i<=yend;i++)
	{													   	 	
		for(j=xsta;j<=xend;j++)
			ili_write_short(color);//���  
	} 					  	    
}
//����
//x1,y1:�������
//x2,y2:�յ�����  
void lcd_draw_line(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		lcd_draw_point(uRow,uCol,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}  
//������
void lcd_draw_rectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	lcd_draw_line(x1,y1,x2,y1,color);
	lcd_draw_line(x1,y1,x1,y2,color);
	lcd_draw_line(x1,y2,x2,y2,color);
	lcd_draw_line(x2,y1,x2,y2,color);
}
//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void lcd_draw_circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
	while(a<=b)
	{
		lcd_draw_point(x0-b,y0-a,color);             //3           
		lcd_draw_point(x0+b,y0-a,color);             //0           
		lcd_draw_point(x0-a,y0+b,color);             //1       
		lcd_draw_point(x0-b,y0-a,color);             //7           
		lcd_draw_point(x0-a,y0-b,color);             //2             
		lcd_draw_point(x0+b,y0+a,color);             //4               
		lcd_draw_point(x0+a,y0-b,color);             //5
		lcd_draw_point(x0+a,y0+b,color);             //6 
		lcd_draw_point(x0-b,y0+a,color);             
		a++;
		//ʹ��Bresenham�㷨��Բ     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		lcd_draw_point(x0+a,y0+b,color);
	}
}
//��ָ��λ����ʾһ���ַ�
void lcd_show_char (u16 x,u16 y,char c,u16 color,painting_mode mode)
{
	u8 pos,tmp,t;
	c -= ' ';
	ili_addr_set(x,y,x+8-1,y+16-1);//���ù��
	for(pos=0;pos<16;pos++)
	{
		tmp=ascii[(u16)c*16+pos];		 //����1608����
		for(t=0;t<8;t++)
		{                 
			if(mode == POINT_MODE)//ʹ�û��㺯��
			{
				if(tmp&0x80)lcd_draw_point(x+t,y+pos,color);//��һ����     
			}
			else
			{
				if(tmp & 0x80)ili_write_short(color);
				else ili_write_short(0);
			}
			tmp<<=1; 
		}
	}
}

void lcd_show_num_32 (u16 x,u16 y,u8 c,u16 color)
{
	u16 i,pos,tmp,t;
	for(pos=0;pos<64;pos++)
	{
		for(i=0;i<4;i++)
		{
			tmp=ASCII32_64[(u16)c*(64*4)+(pos*4)+i];		 //����32*64����
			for(t=0;t<8;t++)
			{
				if(tmp & 0x80)lcd_draw_point(x+t+(i*8),y+pos,color);
				tmp<<=1; 
			}
		}
	}
}

void lcd_show_num_16 (u16 x,u16 y,u8 c,u16 color)
{
	u16 i,pos,tmp,t;
	for(pos=0;pos<32;pos++)
	{
		for(i=0;i<2;i++)
		{
			tmp=NUM16_32[(u16)c*(32*2)+(pos*2)+i];		 //����32*64����
			for(t=0;t<8;t++)
			{
				if(tmp & 0x80)lcd_draw_point(x+t+(i*8),y+pos,color);
				tmp<<=1; 
			}
		}
	}
}

//��ָ��λ����ʾ�ַ���
void lcd_show_string(u16 x,u16 y,char *c,u16 color,painting_mode mode)
{
	if(c == NULL)return;
	while(*c != 0)
	{
		if(*c > 127)
			*c = '?';
		lcd_show_char(x,y,*c,color,mode);
		c++;
		x+=8;
		if(x >= 320)
		{
			x = RESET;
			y+=16;
		}
	}
}

//��ʾͼƬ
void lcd_show_picture(u16 x,u16 y,u16 xend,u16 yend,u8 *gImage)
{
	u32 i;
	ili_addr_set(x,y,xend,yend);
	for(i=0;i<(yend-y)*(xend-x);i++)
	{
		ili_write_byte(gImage[i]);
	}
}


u16 *lcd_buf_addr;

void lcd_show_buf(u16 x,u16 y,u16 xend,u16 yend,u16 *buf)
{
	u32 i;
	ili_addr_set(x,y,xend,yend);
	for(i=0;i<(xend-x)*(yend-y);i++)
		ili_write_short(buf[i]);
}

//����д��Ļ����ַ
void lcd_set_buf_addr(u16 *addr)
{
	lcd_buf_addr = addr;
}

void lcd_write_buf(u16 x,u16 y,u16 color)
{
	lcd_buf_addr[(y*240)+x] = color;
}

void lcd_set_fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 i,j; 
	for(i=ysta;i<=yend;i++)
	{													   	 	
		for(j=xsta;j<=xend;j++)
			lcd_write_buf(j,i,color);//���ù��λ�� 	    
	} 					  	    
}

void lcd_set_num_32_64 (u16 x,u16 y,u8 c,u16 color)
{
	u16 i,pos,tmp,t;
	for(pos=0;pos<64;pos++)
	{
		for(i=0;i<4;i++)
		{
			tmp=ASCII32_64[(u16)c*(64*4)+(pos*4)+i];		 //����32*64����
			for(t=0;t<8;t++)
			{
				if(tmp & 0x80)lcd_write_buf(x+t+(i*8),y+pos,color);
				tmp<<=1; 
			}
		}
	}
}

void lcd_set_num_16_32 (u16 x,u16 y,u8 c,u16 color)
{
	u16 i,pos,tmp,t;
	for(pos=0;pos<32;pos++)
	{
		for(i=0;i<2;i++)
		{
			tmp=NUM16_32[(u16)c*(32*2)+(pos*2)+i];		 //����32*64����
			for(t=0;t<8;t++)
			{
				if(tmp & 0x80)lcd_write_buf(x+t+(i*8),y+pos,color);
				tmp<<=1; 
			}
		}
	}
}

char *lp_char;//LCD��ӡ�ַ�
u16 lp_point_x,lp_point_y;//���

//��ʼ��LCD��ӡ����
void lcd_printf_init(void)
{
	lp_char = pvPortMalloc(((40+1)*(15+1)));//ÿ����ʾ30���ַ� ������ʾ20��
	MALLOC_CHECK(lp_char);
	memset(lp_char,' ',40*15);
	lcd_clear(BLACK);
}
//�˳�LCD��ӡ����
void lcd_printf_exit(void)
{
	vPortFree(lp_char);
	lp_char = NULL;
}

void lcd_printf_task(void)
{
	if(lp_char)
		lcd_show_string(0,0,lp_char,WHITE,FILL_MODE);
}

static void lcd_printf_string(char *c)
{
	u16 i;
	u16 len = strlen(c);
	if(lp_char != 0)
	for(i=0;i<len;i++)
	{
		if((lp_point_x >= 40 && lp_point_y >= 15) || lp_point_y >= 15)//������
		{
			u16 x;
			for(x=0;x<14;x++)
				memcpy(lp_char+(x*40),lp_char+((x+1)*40),40);
			memset(lp_char+(14*40),' ',40);
			lp_point_y = 14;
			lp_point_x = RESET;
		}
		if(c[i] == '\r')
			lp_point_x = RESET;
		if(c[i] == '\n')
			lp_point_y ++;
		if(c[i] != '\r' && c[i] != '\n')
			lp_char[(lp_point_y*40)+lp_point_x++] = c[i];
		if(lp_point_x >= 40){
			lp_point_y++;
			lp_point_x = RESET;
		}
	}
}

void lcd_log(char *fmt,...)  
{
	va_list ap;  
	char *string;
	char *d;
	string = pvPortMalloc(512);
	MALLOC_CHECK(string);
	memset(string,' ',512);
	va_start(ap,fmt);  
	vsprintf(string,fmt,ap);
	d = strstr(string,"\r\n");
	if(d)
		memset(d,' ',2);
	string[strlen(string)] = ' ';
	string[40] = 0;
	lcd_show_string(0,240-16,string,WHITE,FILL_MODE);  
	vPortFree(string);
	va_end(ap);  
}

void lcd_printf(char *fmt,...)  
{
	va_list ap;  
	char *string;  
	string = pvPortMalloc(512);
	MALLOC_CHECK(string);
	memset(string,0,256);
	va_start(ap,fmt);  
	vsprintf(string,fmt,ap);  
	lcd_printf_string(string);
	lcd_printf_task();
	vPortFree(string);
	va_end(ap);  
}

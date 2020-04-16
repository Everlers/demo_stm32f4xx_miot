#ifndef _FSMC_MEMALLOC_H_
#define _FSMC_MEMALLOC_H_
#include "stm32f4xx_conf.h"
#define SRAM_ADDR    (0x68000000)
#define MEM_TABLE_SIZE 64
#define MAX_MEM_BASE_SIZE 1024*1000
#define MAX_MEM_TABLE_SIZE (MAX_MEM_BASE_SIZE/MEM_TABLE_SIZE)
typedef struct
{
	u8 *Mem_Base;
	u8 Mem_Ment[MAX_MEM_TABLE_SIZE];
	u8 Mem_Flag;
	u8 Mem_Percent[8];
}Memalloc_TypeDef;
void Mem_Init(void);
void *Malloc(u32 mem_size);
void Delete_Malloc(void *ptr);
void FSMC_Configuration(void);
void Malloc_Percent(u8 *Percent,char *Ascii);
#endif

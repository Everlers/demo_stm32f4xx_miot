#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_
#include"stm32f10x.h"
#define MEM_BLOCK_SIZE 32
#define MAX_MEM_BASE_SIZE 1024*10
#define MAX_ALLOC_TABLE_SIZE (MAX_MEM_BASE_SIZE/MEM_BLOCK_SIZE)
typedef struct
{
	u8 Mem_Base[MAX_MEM_BASE_SIZE];
	u8 Mem_Map[MAX_ALLOC_TABLE_SIZE];
	u8 Mem_Flag;
}Memalloc;
void Mem_Init(void);
void *Malloc(u32 mem_size);
void Delete_Malloc(void *ptr);
#endif

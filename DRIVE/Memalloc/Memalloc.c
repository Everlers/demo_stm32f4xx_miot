#include"Memalloc.h"
Memalloc Mem_Struct;
void Mem_Init(void)
{
	u32 Mem_Len;
	Mem_Len=MAX_MEM_BASE_SIZE;
	while(Mem_Len--)Mem_Struct.Mem_Base[Mem_Len]=0;
	Mem_Len=MAX_ALLOC_TABLE_SIZE;
	while(Mem_Len--)Mem_Struct.Mem_Map[Mem_Len] =0;
	Mem_Struct.Mem_Flag=1;
}
void *Malloc(u32 mem_size)
{
	u32 len,i;
	u32 mem_block;
	u32 Mem_Block;
	if(!Mem_Struct.Mem_Flag)Mem_Init();
	if(mem_size==0)return ((void*)0);
	mem_block=mem_size/MEM_BLOCK_SIZE;
	if(mem_size%MEM_BLOCK_SIZE)mem_block+=1;
	for(len=MAX_ALLOC_TABLE_SIZE;len>0;len--)
	{
		if(Mem_Struct.Mem_Map[len]==0)Mem_Block++;
		else Mem_Block=0;
		if(Mem_Block==mem_block)
		{
			for(i=0;i<Mem_Block;i++)
			Mem_Struct.Mem_Map[i+len]=Mem_Block;
			return ((void*)((len*MEM_BLOCK_SIZE)+(u32)&Mem_Struct.Mem_Base));
		}
	}
	return 0;
}
void Delete_Malloc(void *ptr)
{
	u32 Base_addr;
	u32 len;
	u32 map_addr;
	Base_addr=(u32)ptr;
	Base_addr-=(u32)&Mem_Struct.Mem_Base;
	if(Base_addr<MAX_MEM_BASE_SIZE)
	{
		Base_addr/=MEM_BLOCK_SIZE;
		map_addr = Mem_Struct.Mem_Map[Base_addr];
		for(len=0;len<map_addr;len++)
		Mem_Struct.Mem_Map[len+Base_addr] = 0;
	}
}

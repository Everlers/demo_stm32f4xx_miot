#ifndef _MY_FAT_H_
#define _MY_FAT_H_
#include"stm32f4xx.h"
#include"ff.h"
#include "fsmc_Memalloc.h"
u8 mf_readdir(DIR *dir,FILINFO *fileinfo);

#endif

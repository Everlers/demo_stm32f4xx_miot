#include"My_FAT.h"
//打读取文件夹
//返回值:执行结果
FILINFO fileinfo;
DIR dir;
u8 mf_readdir(DIR *dir,FILINFO *fileinfo)
{	
	u8 volatile res;
 	fileinfo->lfsize = _MAX_LFN;
	fileinfo->lfname = Malloc(fileinfo->lfsize);	  
	res = f_readdir(dir,fileinfo);//读取一个文件的信息
	fileinfo->lfname=(fileinfo->lfname[0]!=0)? fileinfo->lfname:fileinfo->fname;
	return res;
}			 


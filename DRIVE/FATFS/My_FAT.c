#include"My_FAT.h"
//���ȡ�ļ���
//����ֵ:ִ�н��
FILINFO fileinfo;
DIR dir;
u8 mf_readdir(DIR *dir,FILINFO *fileinfo)
{	
	u8 volatile res;
 	fileinfo->lfsize = _MAX_LFN;
	fileinfo->lfname = Malloc(fileinfo->lfsize);	  
	res = f_readdir(dir,fileinfo);//��ȡһ���ļ�����Ϣ
	fileinfo->lfname=(fileinfo->lfname[0]!=0)? fileinfo->lfname:fileinfo->fname;
	return res;
}			 


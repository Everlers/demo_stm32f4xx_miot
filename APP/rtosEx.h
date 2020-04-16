#ifndef _RTOSEX_H_
#define _RTOSEX_H_
#include "hal_sys.h"

#define RTOSEX_DEBUG								debug	//RTOS��չ������ӡ
#define RTOSEX_LCD_SHOW							0//��LCD��ʾ�����б�
#define RTOSEX_MAX_TASK_LIST_NUM		10//��������б����ʾ����
#define RTOSEX_TASK_LIST_PARAM_LEN	28//�����б�Ĳ�������
#define RTOSEX_SHOW_OTHER_INFO_NUM	2	//��ʾ������Ϣ������ ��Ҫ���n��������ʾ��������Ϣ

#define tskRUNNING_CHAR		( 'X' )//����
#define tskBLOCKED_CHAR		( 'B' )//����
#define tskREADY_CHAR			( 'R' )//����
#define tskDELETED_CHAR		( 'D' )//ɾ��
#define tskSUSPENDED_CHAR	( 'S' )//����

void rtosShowStatus(void);
void rtosExConfigureRunTime (void);
u32 rtosExGetRunTimeCounterValue (void);

#endif


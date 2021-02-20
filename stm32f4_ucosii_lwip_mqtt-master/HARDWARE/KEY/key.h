#ifndef _KEY_H
#define _KEY_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//����������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	



#define DOOR PFin(7)
#define WARN1 PFin(6)
#define WARN2 PBin(15)
#define DOOR_OPEN	1
#define WARN1_ACTIVE 2//�͵�ƽ��Ч
#define WARN2_ACTIVE	3

#define RESTAR PEout(2)

void KEY_Init(void);  //IO��ʼ��
u8 KEY_Scan(u8);    //����ɨ�躯��
#endif 

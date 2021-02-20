#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H
#include "sys.h"
#include "includes.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//NETCONN API��̷�ʽ��TCP�ͻ��˲��Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   
 
 extern char get_passidbuf[50];
#define TCP_CLIENT_RX_BUFSIZE	2000	//���ջ���������
#define REMOTE_PORT				9989	//����Զ��������IP��ַ
#define LWIP_SEND_DATA			0X80    //���������ݷ���
#define rec_data_len sizeof(get_passidbuf)
#define SIZE rec_data_len/4+((rec_data_len%4)?1:0)

extern char tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//TCP�ͻ��˽������ݻ�����
extern u8 tcp_client_flag;		//TCP�ͻ������ݷ��ͱ�־λ

INT8U tcp_client_init(void);  //tcp�ͻ��˳�ʼ��(����tcp�ͻ����߳�)
#endif


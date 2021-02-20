#ifndef __COMMIT_H
#define __COMMIT_H

#include "stdio.h"	
#include "sys.h"
#include "string.h"
#include "stm32f4xx.h"


#define USART2_REC_LEN  			200  	//�����������ֽ��� 200

	  	
extern char  USART2_RX_BUF[USART2_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
//extern u16 USART2_RX_STA;         		//����״̬���	



#define false 0
#define true 1

//�������鳤��
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2 
extern	int year;
extern	int mouth;
extern	int date;
extern	int hour;
extern	int min;
extern	int sec;
typedef struct SaveData 
{
	char GPS_Buffer[GPS_Buffer_Length];
	char isGetData;		//�Ƿ��ȡ��GPS����
	char isParseData;	//�Ƿ�������
	char UTCTime[UTCTime_Length];		//UTCʱ��
	char latitude[latitude_Length];		//γ��
	char N_S[N_S_Length];		//N/S
	char longitude[longitude_Length];		//����
	char E_W[E_W_Length];		//E/W
	char isUsefull;		//��λ��Ϣ�Ƿ���Ч
	char UTCDate[UTCTime_Length];
	char rate[2];
	char direction[2];
} _SaveData;




void uart2_init(uint32_t bound);
extern char rxdatabufer;
extern uint16_t point1;
extern _SaveData Save_Data;

void Get_Gps(void);
void errorLog(int num);

void CLR_Buf(void);
uint8_t Hand(char *a);
void clrStruct(void);


#endif

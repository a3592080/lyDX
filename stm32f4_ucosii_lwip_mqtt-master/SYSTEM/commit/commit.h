#ifndef __COMMIT_H
#define __COMMIT_H

#include "stdio.h"	
#include "sys.h"
#include "string.h"
#include "stm32f4xx.h"


#define USART2_REC_LEN  			200  	//定义最大接收字节数 200

	  	
extern char  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
//extern u16 USART2_RX_STA;         		//接收状态标记	



#define false 0
#define true 1

//定义数组长度
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
	char isGetData;		//是否获取到GPS数据
	char isParseData;	//是否解析完成
	char UTCTime[UTCTime_Length];		//UTC时间
	char latitude[latitude_Length];		//纬度
	char N_S[N_S_Length];		//N/S
	char longitude[longitude_Length];		//经度
	char E_W[E_W_Length];		//E/W
	char isUsefull;		//定位信息是否有效
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

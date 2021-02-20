#ifndef __USART2_H
#define __USART2_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.csom
//修改日期:2011/6/14
//版本：V1.4
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
////////////////////////////////////////////////////////////////////////////////// 	
#define USART3_REC_LEN  			1024  	//定义最大接收字节数 200

#define MCLOCK_TXIO			RCC_AHB1Periph_GPIOB
#define MGPTXIO				GPIOB	  
#define MTXD1             GPIO_Pin_1                 //IO-TXD	PB1
#define TX_L()      GPIO_ResetBits(MGPTXIO, MTXD1)  //TX:0
#define TX_H()      GPIO_SetBits(MGPTXIO, MTXD1)    //TX:1
#define RX_LEN_GSM 1024
extern char  USART3_RX_BUF[USART3_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART3_RX_STA;         		//接收状态标记	


//如果想串口中断接收，请不要注释以下宏定义
void MUSART1_TX_init(void);
void MUSART1_SendData(u8 data);
void MUSART1_SendStr(char *str);
void usart3_init(u32 bound);
void UART_SendStr(USART_TypeDef* USARTx, char* str, u16 legth);
#endif



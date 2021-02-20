#ifndef __USART2_H
#define __USART2_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2011/6/14
//�汾��V1.4
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
////////////////////////////////////////////////////////////////////////////////// 	
#define USART3_REC_LEN  			1024  	//�����������ֽ��� 200

#define MCLOCK_TXIO			RCC_AHB1Periph_GPIOB
#define MGPTXIO				GPIOB	  
#define MTXD1             GPIO_Pin_1                 //IO-TXD	PB1
#define TX_L()      GPIO_ResetBits(MGPTXIO, MTXD1)  //TX:0
#define TX_H()      GPIO_SetBits(MGPTXIO, MTXD1)    //TX:1
#define RX_LEN_GSM 1024
extern char  USART3_RX_BUF[USART3_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART3_RX_STA;         		//����״̬���	


//����봮���жϽ��գ��벻Ҫע�����º궨��
void MUSART1_TX_init(void);
void MUSART1_SendData(u8 data);
void MUSART1_SendStr(char *str);
void usart3_init(u32 bound);
void UART_SendStr(USART_TypeDef* USARTx, char* str, u16 legth);
#endif



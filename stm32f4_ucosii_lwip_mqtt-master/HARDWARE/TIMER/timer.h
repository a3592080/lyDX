#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/16
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//#define fan PAout(15)
/* �߼����ƶ�ʱ��PWM���벶�� */
/* PWM���벶������ */
#define ADVANCE_ICPWM_PIN             GPIO_Pin_11            
#define ADVANCE_ICPWM_GPIO_PORT       GPIOA                      
#define ADVANCE_ICPWM_GPIO_CLK        RCC_AHB1Periph_GPIOA
#define ADVANCE_ICPWM_PINSOURCE				GPIO_PinSource11
#define ADVANCE_ICPWM_AF							GPIO_AF_TIM1
#define ADVANCE_IC1PWM_CHANNEL        TIM_Channel_4
#define ADVANCE_IC2PWM_CHANNEL        TIM_Channel_3

/* �߼����ƶ�ʱ�� */
#define ADVANCE_TIM           		    TIM1
#define ADVANCE_TIM_CLK       		    RCC_APB2Periph_TIM1

/* ����/�Ƚ��ж� */
#define ADVANCE_TIM_IRQn					    TIM1_CC_IRQn
#define ADVANCE_TIM_IRQHandler        TIM1_CC_IRQHandler
void TIM1_Int_Init();
#endif

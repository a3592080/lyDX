#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/6/16
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//#define fan PAout(15)
/* 高级控制定时器PWM输入捕获 */
/* PWM输入捕获引脚 */
#define ADVANCE_ICPWM_PIN             GPIO_Pin_11            
#define ADVANCE_ICPWM_GPIO_PORT       GPIOA                      
#define ADVANCE_ICPWM_GPIO_CLK        RCC_AHB1Periph_GPIOA
#define ADVANCE_ICPWM_PINSOURCE				GPIO_PinSource11
#define ADVANCE_ICPWM_AF							GPIO_AF_TIM1
#define ADVANCE_IC1PWM_CHANNEL        TIM_Channel_4
#define ADVANCE_IC2PWM_CHANNEL        TIM_Channel_3

/* 高级控制定时器 */
#define ADVANCE_TIM           		    TIM1
#define ADVANCE_TIM_CLK       		    RCC_APB2Periph_TIM1

/* 捕获/比较中断 */
#define ADVANCE_TIM_IRQn					    TIM1_CC_IRQn
#define ADVANCE_TIM_IRQHandler        TIM1_CC_IRQHandler
void TIM1_Int_Init();
#endif

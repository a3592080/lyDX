#include "exit.h"
#include "delay.h"
#include "key.h"
#include "usart.h"

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif

u8 door_flag = 0;
void EXTI9_5_IRQHandler(void)
{
	#if SYSTEM_SUPPORT_UCOS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	delay_ms(10);	//消抖
	if(EXTI_GetITStatus(EXTI_Line7)!=RESET)//判断某个线上的中断是否发生
	{
		door_flag = 1;
		printf("进入中断");
	}		 
	 EXTI_ClearITPendingBit(EXTI_Line7); //清除LINE0上的中断标志位 
#if SYSTEM_SUPPORT_UCOS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}	


void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
		KEY_Init(); //按键对应的IO口初始化
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource7|EXTI_PinSource6);//PA0 连接到中断线0
	 /* 配置EXTI_Line0 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line7|EXTI_Line6;//LINE0
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //上升沿触发 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE0
  EXTI_Init(&EXTI_InitStructure);//配置
	
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//外部中断0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	
}


	



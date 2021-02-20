#include "exit.h"
#include "delay.h"
#include "key.h"
#include "usart.h"

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif

u8 door_flag = 0;
void EXTI9_5_IRQHandler(void)
{
	#if SYSTEM_SUPPORT_UCOS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	delay_ms(10);	//����
	if(EXTI_GetITStatus(EXTI_Line7)!=RESET)//�ж�ĳ�����ϵ��ж��Ƿ���
	{
		door_flag = 1;
		printf("�����ж�");
	}		 
	 EXTI_ClearITPendingBit(EXTI_Line7); //���LINE0�ϵ��жϱ�־λ 
#if SYSTEM_SUPPORT_UCOS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
}	


void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
		KEY_Init(); //������Ӧ��IO�ڳ�ʼ��
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//ʹ��SYSCFGʱ��
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource7|EXTI_PinSource6);//PA0 ���ӵ��ж���0
	 /* ����EXTI_Line0 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line7|EXTI_Line6;//LINE0
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //�����ش��� 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE0
  EXTI_Init(&EXTI_InitStructure);//����
	
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//�ⲿ�ж�0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
	
}


	



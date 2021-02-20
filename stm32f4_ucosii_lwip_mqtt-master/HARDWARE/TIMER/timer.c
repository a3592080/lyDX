#include "timer.h"
#include "led.h"
#include "lwip_comm.h"
#include "usart.h"
#include "includes.h"					//ucos ʹ��	 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 


//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��1!
u8 TIM1CH4_CAPTURE_STA;
u16 TIM1CH4_CAPTURE_VAL;
int time_l;
void TIM1_Int_Init()
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	 NVIC_InitTypeDef NVIC_InitStructure; 
	// ����TIMx_CLK,x[1,8] 
  RCC_APB2PeriphClockCmd(ADVANCE_TIM_CLK, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //ʹ��PORTAʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //GPIOA0
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //�ٶ� 100MHz
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //����
GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ�� PA0
GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_TIM1); //PA0 ����λ��ʱ�� 5
	
	
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF-1; 	
	// �߼����ƶ�ʱ��ʱ��ԴTIMxCLK = HCLK=168MHz 
	// �趨��ʱ��Ƶ��Ϊ=TIMxCLK/(TIM_Prescaler+1)=100KHz
  TIM_TimeBaseStructure.TIM_Prescaler = 168-1;	
  // ������ʽ
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	
	// ��ʼ����ʱ��TIMx, x[1,8]
	TIM_TimeBaseInit(ADVANCE_TIM, &TIM_TimeBaseStructure);
	
	/* IC1���������ش��� TI1FP1 */
  TIM_ICInitStructure.TIM_Channel = ADVANCE_IC1PWM_CHANNEL;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;
  TIM_ICInit(ADVANCE_TIM, &TIM_ICInitStructure);
	
//	/* IC2�����½��ش��� TI1FP2 */	
//	TIM_ICInitStructure.TIM_Channel = ADVANCE_IC2PWM_CHANNEL;
//  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
//  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
//  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
//  TIM_ICInitStructure.TIM_ICFilter = 0x0;
//  TIM_PWMIConfig(ADVANCE_TIM, &TIM_ICInitStructure);
	
	/* ѡ��ʱ�����봥��: TI1FP1 */
  //TIM_SelectInputTrigger(ADVANCE_TIM, TIM_TS_TI2FP2);		

  /* ѡ���ģʽ: ��λģʽ */
  //TIM_SelectSlaveMode(ADVANCE_TIM, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(ADVANCE_TIM,TIM_MasterSlaveMode_Disable);

  /* ʹ�ܸ߼����ƶ�ʱ�� */
  TIM_Cmd(ADVANCE_TIM, ENABLE);

  /* ʹ�ܲ���/�Ƚ�2�ж����� */
  TIM_ITConfig(ADVANCE_TIM, TIM_IT_CC4|TIM_IT_Update, ENABLE);
	
    // �����ж���Ϊ0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
		// �����ж���Դ
    NVIC_InitStructure.NVIC_IRQChannel = ADVANCE_TIM_IRQn; 	
		// ������ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	 
	  // ���������ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
}


//��ʱ��5�жϷ������	 
void TIM1_CC_IRQHandler(void)
{
		OSIntEnter();    
	if((TIM1CH4_CAPTURE_STA&0X80)==0)//��δ�ɹ�����
    {
        if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
        {
            if(TIM1CH4_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
            {
                if((TIM1CH4_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
                {
                    TIM1CH4_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
                    TIM1CH4_CAPTURE_VAL=0XFFFF;
                }else TIM1CH4_CAPTURE_STA++;
            }
        }
        if (TIM_GetITStatus(TIM1, TIM_IT_CC4) != RESET)//���� 1 ���������¼�
        {
            if(TIM1CH4_CAPTURE_STA&0X40) //����һ���½���
            {
                TIM1CH4_CAPTURE_STA|=0X80; //��ǳɹ�����һ��������
                TIM1CH4_CAPTURE_VAL=TIM_GetCapture4(TIM1);
							printf("the value is : %d\r\n",TIM1CH4_CAPTURE_VAL);
                TIM_OC4PolarityConfig(TIM1,TIM_ICPolarity_Rising);
                //CC1P=0 ����Ϊ�����ز���
            }else //��δ��ʼ,��һ�β���������
            {
                TIM1CH4_CAPTURE_STA=0; //���
                TIM1CH4_CAPTURE_VAL=0;
							TIM_Cmd(TIM1,DISABLE ); 	//�رն�ʱ��5
                TIM_SetCounter(TIM1,0);
                TIM1CH4_CAPTURE_STA|=0X40; //��ǲ�����������
                TIM_OC4PolarityConfig(TIM1,TIM_ICPolarity_Falling);
							TIM_Cmd(TIM1,ENABLE ); 	//ʹ�ܶ�ʱ��5
                //CC1P=1 ����Ϊ�½��ز���
            }
        }
    }
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC4|TIM_IT_Update); //����жϱ�־λ


///* �����ʱ������/�Ƚ�1�ж� */
//  TIM_ClearITPendingBit(ADVANCE_TIM, TIM_IT_CC4);

//  /* ��ȡ���벶��ֵ */
//  IC1Value = TIM_GetCapture3(ADVANCE_TIM);
//  IC2Value = TIM_GetCapture4(ADVANCE_TIM);	
//  printf("IC1Value = %d  IC2Value = %d ",IC1Value,IC2Value);
//	
//  if (IC1Value != 0)
//  {
//    /* ռ�ձȼ��� */
//    DutyCycle = (float)(IC2Value * 100) / IC1Value;

//    /* Ƶ�ʼ��� */
//    Frequency = 168000000/1680/(float)IC1Value;
//		printf("ռ�ձȣ�%0.2f%%   Ƶ�ʣ�%0.2fHz\n",DutyCycle,Frequency);
//  }
//  else
//  {
//    DutyCycle = 0;
//    Frequency = 0;
//  }
OSIntExit();    	//�˳��ж�
	}

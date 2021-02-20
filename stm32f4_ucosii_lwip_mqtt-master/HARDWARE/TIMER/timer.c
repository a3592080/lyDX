#include "timer.h"
#include "led.h"
#include "lwip_comm.h"
#include "usart.h"
#include "includes.h"					//ucos 使用	 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/4
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 


//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器1!
u8 TIM1CH4_CAPTURE_STA;
u16 TIM1CH4_CAPTURE_VAL;
int time_l;
void TIM1_Int_Init()
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	 NVIC_InitTypeDef NVIC_InitStructure; 
	// 开启TIMx_CLK,x[1,8] 
  RCC_APB2PeriphClockCmd(ADVANCE_TIM_CLK, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能PORTA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //GPIOA0
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //速度 100MHz
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //下拉
GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化 PA0
GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_TIM1); //PA0 复用位定时器 5
	
	
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF-1; 	
	// 高级控制定时器时钟源TIMxCLK = HCLK=168MHz 
	// 设定定时器频率为=TIMxCLK/(TIM_Prescaler+1)=100KHz
  TIM_TimeBaseStructure.TIM_Prescaler = 168-1;	
  // 计数方式
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	
	// 初始化定时器TIMx, x[1,8]
	TIM_TimeBaseInit(ADVANCE_TIM, &TIM_TimeBaseStructure);
	
	/* IC1捕获：上升沿触发 TI1FP1 */
  TIM_ICInitStructure.TIM_Channel = ADVANCE_IC1PWM_CHANNEL;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;
  TIM_ICInit(ADVANCE_TIM, &TIM_ICInitStructure);
	
//	/* IC2捕获：下降沿触发 TI1FP2 */	
//	TIM_ICInitStructure.TIM_Channel = ADVANCE_IC2PWM_CHANNEL;
//  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
//  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
//  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
//  TIM_ICInitStructure.TIM_ICFilter = 0x0;
//  TIM_PWMIConfig(ADVANCE_TIM, &TIM_ICInitStructure);
	
	/* 选择定时器输入触发: TI1FP1 */
  //TIM_SelectInputTrigger(ADVANCE_TIM, TIM_TS_TI2FP2);		

  /* 选择从模式: 复位模式 */
  //TIM_SelectSlaveMode(ADVANCE_TIM, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(ADVANCE_TIM,TIM_MasterSlaveMode_Disable);

  /* 使能高级控制定时器 */
  TIM_Cmd(ADVANCE_TIM, ENABLE);

  /* 使能捕获/比较2中断请求 */
  TIM_ITConfig(ADVANCE_TIM, TIM_IT_CC4|TIM_IT_Update, ENABLE);
	
    // 设置中断组为0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
		// 设置中断来源
    NVIC_InitStructure.NVIC_IRQChannel = ADVANCE_TIM_IRQn; 	
		// 设置抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	 
	  // 设置子优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
}


//定时器5中断服务程序	 
void TIM1_CC_IRQHandler(void)
{
		OSIntEnter();    
	if((TIM1CH4_CAPTURE_STA&0X80)==0)//还未成功捕获
    {
        if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
        {
            if(TIM1CH4_CAPTURE_STA&0X40)//已经捕获到高电平了
            {
                if((TIM1CH4_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
                {
                    TIM1CH4_CAPTURE_STA|=0X80;//标记成功捕获了一次
                    TIM1CH4_CAPTURE_VAL=0XFFFF;
                }else TIM1CH4_CAPTURE_STA++;
            }
        }
        if (TIM_GetITStatus(TIM1, TIM_IT_CC4) != RESET)//捕获 1 发生捕获事件
        {
            if(TIM1CH4_CAPTURE_STA&0X40) //捕获到一个下降沿
            {
                TIM1CH4_CAPTURE_STA|=0X80; //标记成功捕获到一次上升沿
                TIM1CH4_CAPTURE_VAL=TIM_GetCapture4(TIM1);
							printf("the value is : %d\r\n",TIM1CH4_CAPTURE_VAL);
                TIM_OC4PolarityConfig(TIM1,TIM_ICPolarity_Rising);
                //CC1P=0 设置为上升沿捕获
            }else //还未开始,第一次捕获上升沿
            {
                TIM1CH4_CAPTURE_STA=0; //清空
                TIM1CH4_CAPTURE_VAL=0;
							TIM_Cmd(TIM1,DISABLE ); 	//关闭定时器5
                TIM_SetCounter(TIM1,0);
                TIM1CH4_CAPTURE_STA|=0X40; //标记捕获到了上升沿
                TIM_OC4PolarityConfig(TIM1,TIM_ICPolarity_Falling);
							TIM_Cmd(TIM1,ENABLE ); 	//使能定时器5
                //CC1P=1 设置为下降沿捕获
            }
        }
    }
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC4|TIM_IT_Update); //清除中断标志位


///* 清除定时器捕获/比较1中断 */
//  TIM_ClearITPendingBit(ADVANCE_TIM, TIM_IT_CC4);

//  /* 获取输入捕获值 */
//  IC1Value = TIM_GetCapture3(ADVANCE_TIM);
//  IC2Value = TIM_GetCapture4(ADVANCE_TIM);	
//  printf("IC1Value = %d  IC2Value = %d ",IC1Value,IC2Value);
//	
//  if (IC1Value != 0)
//  {
//    /* 占空比计算 */
//    DutyCycle = (float)(IC2Value * 100) / IC1Value;

//    /* 频率计算 */
//    Frequency = 168000000/1680/(float)IC1Value;
//		printf("占空比：%0.2f%%   频率：%0.2fHz\n",DutyCycle,Frequency);
//  }
//  else
//  {
//    DutyCycle = 0;
//    Frequency = 0;
//  }
OSIntExit();    	//退出中断
	}

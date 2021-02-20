#include "key.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//按键输入驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

//按键初始化函数
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOE,ENABLE);  //使能GPIOA GPIOE时钟
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_6;  //PE2,3,4引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //输入
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //上拉输入
	GPIO_Init(GPIOF,&GPIO_InitStructure); 			//初始化GPIOE
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;  //PA0引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //输入
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //下拉输入
	GPIO_Init(GPIOB,&GPIO_InitStructure);     //初始化GPIOA
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;  //PA0引脚//重启
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;  //输入
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_DOWN;  //下拉输入
	GPIO_Init(GPIOE,&GPIO_InitStructure);     //初始化GPIOA
}

//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，WKUP按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{
	static u8 key_up=1;   //按键松开标志
	if(mode) key_up=1;  //支持连按
	if(key_up&&(DOOR==0||WARN1==0||WARN2==0)) //有按键按下
	{
		delay_ms(10);  //按键去抖
		key_up=0;
		if(DOOR==0) return 1;//门打开
		else if(WARN1==0) return 2;
		else if(WARN2==0) return 3;
	}else if(DOOR==1&&WARN1==1&&WARN2==1)key_up=1; 
	return 0; //无按键按下
}


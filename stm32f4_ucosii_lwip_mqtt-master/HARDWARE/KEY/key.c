#include "key.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//����������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

//������ʼ������
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOE,ENABLE);  //ʹ��GPIOA GPIOEʱ��
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_6;  //PE2,3,4����
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //��������
	GPIO_Init(GPIOF,&GPIO_InitStructure); 			//��ʼ��GPIOE
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;  //PA0����
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //��������
	GPIO_Init(GPIOB,&GPIO_InitStructure);     //��ʼ��GPIOA
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;  //PA0����//����
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;  //����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_DOWN;  //��������
	GPIO_Init(GPIOE,&GPIO_InitStructure);     //��ʼ��GPIOA
}

//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��KEY0����
//2��KEY1����
//3��KEY2���� 
//4��WKUP���� WK_UP
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{
	static u8 key_up=1;   //�����ɿ���־
	if(mode) key_up=1;  //֧������
	if(key_up&&(DOOR==0||WARN1==0||WARN2==0)) //�а�������
	{
		delay_ms(10);  //����ȥ��
		key_up=0;
		if(DOOR==0) return 1;//�Ŵ�
		else if(WARN1==0) return 2;
		else if(WARN2==0) return 3;
	}else if(DOOR==1&&WARN1==1&&WARN2==1)key_up=1; 
	return 0; //�ް�������
}


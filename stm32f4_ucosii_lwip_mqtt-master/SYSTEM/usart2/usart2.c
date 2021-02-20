#include "sys.h"
#include "usart2.h"	
#include "delay.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F4̽���߿�����
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/6/10
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART3_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 #if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  

 u32 delayTime = 104;	//1000000/115200=8.6us  
extern OS_EVENT *box_msg_ptr;

//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
char USART3_RX_BUF[USART3_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
extern char * Rx_Buf_Gsm;

//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART3_RX_STA=0;       //����״̬���	

//��ʼ��IO ����1 
//bound:������
void MUSART1_TX_init(void)
{  
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(MCLOCK_TXIO,ENABLE); //ʹ��GPIOAʱ��

    GPIO_InitStructure.GPIO_Pin = MTXD1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(MGPTXIO, &GPIO_InitStructure);

    GPIO_SetBits(MGPTXIO, MTXD1);//TXD ����״̬�Ǹߵ�ƽ
}

/*!
 * @brief ģ�⴮��1����һ���ֽ�
 * @param
 * @return	none
 * @note	���ݵ�λ��ǰ��λ�ں�
 */
void MUSART1_SendData(u8 data)//����һ������
{

    u8 i = 0;
    TX_L();	//!<��ʼλ
    delay_us(delayTime);
    for(i = 0; i < 8; i++) 
    {
        if(data & 0x01)
            TX_H();
        else
            TX_L();
        delay_us(delayTime);
        data >>= 1;
    }
    TX_H();	//!<ֹͣλ
    delay_us(delayTime);
}

// ����һ���ַ���
void MUSART1_SendStr(char *str)
{
    while(*str != 0)
    {
        MUSART1_SendData(*str);
        str++;
    }
}

void usart3_init(u32 bound){
	//GPIO�˿�����
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//ʹ��USART1ʱ��

	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); 
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); 
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	//USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx ;	//�շ�ģʽ
	USART_Init(USART3, &USART_InitStructure); //��ʼ������1
	USART_Cmd(USART3, ENABLE);  //ʹ�ܴ���1 
	
	USART_ClearFlag(USART3, USART_FLAG_TC);


	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//��������ж�

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����


	
}


void USART3_IRQHandler(void)                	//����1�жϷ������
{
	char Res;

#if SYSTEM_SUPPORT_UCOS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		
		Res =(char)USART_ReceiveData(USART3);//(USART1->DR);	//��ȡ���յ�������	
		if((USART3_RX_STA&0x8000)==0)//����δ���
		{
			if(USART3_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a)USART3_RX_STA=0;//���մ���,���¿�ʼ
				else {
					OSMboxPost(box_msg_ptr,USART3_RX_BUF);
					
					USART3_RX_STA|=0x8000;	//��������� 
				}
			}
			else //��û�յ�0X0D
			{	
				if(Res==0x0d)USART3_RX_STA|=0x4000;
				else
				{
					USART3_RX_BUF[USART3_RX_STA&0X3FFF]=Res ;
					USART3_RX_STA++;
					if(USART3_RX_STA>(USART3_REC_LEN-1))USART3_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}
	USART_ClearITPendingBit(USART3,USART_IT_RXNE);		
	}

		  		 
   
#if SYSTEM_SUPPORT_UCOS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 

void UART_SendStr(USART_TypeDef* USARTx, char* str, u16 legth)
{	
	u16 im;
	for(im = 0; im < legth; im++ )
	{		
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
		USART_SendData(USARTx, *str);
		str++;
	}
}
 




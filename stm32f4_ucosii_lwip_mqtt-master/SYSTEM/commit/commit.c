#include "commit.h"


char rxdatabufer;
u16 point1 = 0;
	int year;
	int mouth;
	int date;
	int hour;
	int min;
	int sec;
_SaveData Save_Data;
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif 
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  

 

//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
char USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
//u16 USART3_RX_STA=0;       //����״̬���	  
  
void uart2_init(u32 bound)
{
 
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA11����ΪUSART3
	
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //GPIOA11��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode =  USART_Mode_Rx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(USART2, ENABLE);  //ʹ�ܴ���1 
	
//	USART_ClearFlag(USART3, USART_FLAG_TC);
	

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//��������ж�

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
	CLR_Buf();//��ջ���
}

void USART2_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	{
		Res =USART_ReceiveData(USART2);//(USART3->DR);	//��ȡ���յ�������
	
	if(Res == '$')
	{
		point1 = 0;	
	}
		

	  USART2_RX_BUF[point1++] = Res;

	if(USART2_RX_BUF[0] == '$' && USART2_RX_BUF[4] == 'M' && USART2_RX_BUF[5] == 'C')			//ȷ���Ƿ��յ�"GPRMC/GNRMC"��һ֡����
	{
		if(Res == '\n')									   
		{
			memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //���
			memcpy(Save_Data.GPS_Buffer, USART2_RX_BUF, point1); 	//��������
			Save_Data.isGetData = true;
			point1 = 0;
			memset(USART2_RX_BUF, 0, USART2_REC_LEN);      //���				
		}	
				
	}
	
	if(point1 >= USART2_REC_LEN)
	{
		point1 = USART2_REC_LEN;
	}	
		
		
		
		
		
		
// 		USART3_RX_STA|=0x8000;	//���������
// 		if((USART3_RX_STA&0x8000)==0)//����δ���
// 		{
// 			if(USART3_RX_STA&0x4000)//���յ���0x0d
// 			{
// 				if(Res!=0x0a)USART3_RX_STA=0;//���մ���,���¿�ʼ
// 				else USART3_RX_STA|=0x8000;	//��������� 			//bit31�����Ƿ���յ�0x0a(\n)
// 			}
// 			else //��û�յ�0X0D
// 			{	
// 				if(Res==0x0d)USART3_RX_STA|=0x4000;						//bit30�����Ƿ���յ�0x0d(\r)
// 				else
// 				{
// 					USART2_RX_BUF[USART3_RX_STA&0X3FFF]=Res ;
// 					USART3_RX_STA++;
// 					if(USART3_RX_STA>(USART_REC_LEN-1))USART3_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
// 				}		 
// 			}
// 		}   		 
   } 
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();  											 
#endif
}


u8 Hand(char *a)                   // ��������ʶ����
{ 
    if(strstr(USART2_RX_BUF,a)!=NULL)
	    return 1;
	else
		return 0;
}

void CLR_Buf(void)                           // ���ڻ�������
{
	memset(USART2_RX_BUF, 0, USART2_REC_LEN);      //���
  point1 = 0;                    
}

void clrStruct()
{
	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
	memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //���
	memset(Save_Data.UTCTime, 0, UTCTime_Length);
	memset(Save_Data.latitude, 0, latitude_Length);
	memset(Save_Data.N_S, 0, N_S_Length);
	memset(Save_Data.longitude, 0, longitude_Length);
	memset(Save_Data.E_W, 0, E_W_Length);
	
}

void Get_Gps(void)
	
{
	char *subString;
	char *subStringNext;
	char i = 0;
	
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		printf("**************\r\n");
		printf("%s",Save_Data.GPS_Buffer);

		
		for (i = 0 ; i <= 9 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					continue;
					//errorLog(1);	//��������
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//��ȡUTCʱ��
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//��ȡUTCʱ��
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//��ȡγ����Ϣ
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//��ȡN/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//��ȡ������Ϣ
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//��ȡE/W
						case 7:memcpy(Save_Data.rate, subString, subStringNext - subString);break;	
						case 8:memcpy(Save_Data.direction, subString, subStringNext - subString);break;	
						case 9:memcpy(Save_Data.UTCDate , subString, subStringNext - subString);break;	//��ȡE/W
						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
					{
						Save_Data.isUsefull = true;
					
							date = atoi(Save_Data.UTCDate)/10000;
					mouth = atoi(Save_Data.UTCDate)/100%100;
					year=atoi(Save_Data.UTCDate)%100;
					
					hour = atoi(Save_Data.UTCTime)/10000+8;
					min = atoi(Save_Data.UTCTime)/100%100;
					sec	= atoi(Save_Data.UTCTime)%100;
					
					}
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;
				
				}
				else
				{
				//	errorLog(2);	//��������
				}
			}


		}
	}
	
}
void errorLog(int num)
{
	
	while (1)
	{
	  	printf("ERROR%d\r\n",num);
	}
}

	

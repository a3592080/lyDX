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
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif 
 

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  

 

//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
char USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
//u16 USART3_RX_STA=0;       //接收状态标记	  
  
void uart2_init(u32 bound)
{
 
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA11复用为USART3
	
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //GPIOA11与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode =  USART_Mode_Rx;	//收发模式
	USART_Init(USART2, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(USART2, ENABLE);  //使能串口1 
	
//	USART_ClearFlag(USART3, USART_FLAG_TC);
	

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	CLR_Buf();//清空缓存
}

void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	{
		Res =USART_ReceiveData(USART2);//(USART3->DR);	//读取接收到的数据
	
	if(Res == '$')
	{
		point1 = 0;	
	}
		

	  USART2_RX_BUF[point1++] = Res;

	if(USART2_RX_BUF[0] == '$' && USART2_RX_BUF[4] == 'M' && USART2_RX_BUF[5] == 'C')			//确定是否收到"GPRMC/GNRMC"这一帧数据
	{
		if(Res == '\n')									   
		{
			memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
			memcpy(Save_Data.GPS_Buffer, USART2_RX_BUF, point1); 	//保存数据
			Save_Data.isGetData = true;
			point1 = 0;
			memset(USART2_RX_BUF, 0, USART2_REC_LEN);      //清空				
		}	
				
	}
	
	if(point1 >= USART2_REC_LEN)
	{
		point1 = USART2_REC_LEN;
	}	
		
		
		
		
		
		
// 		USART3_RX_STA|=0x8000;	//接收完成了
// 		if((USART3_RX_STA&0x8000)==0)//接收未完成
// 		{
// 			if(USART3_RX_STA&0x4000)//接收到了0x0d
// 			{
// 				if(Res!=0x0a)USART3_RX_STA=0;//接收错误,重新开始
// 				else USART3_RX_STA|=0x8000;	//接收完成了 			//bit31表明是否接收到0x0a(\n)
// 			}
// 			else //还没收到0X0D
// 			{	
// 				if(Res==0x0d)USART3_RX_STA|=0x4000;						//bit30表明是否接收到0x0d(\r)
// 				else
// 				{
// 					USART2_RX_BUF[USART3_RX_STA&0X3FFF]=Res ;
// 					USART3_RX_STA++;
// 					if(USART3_RX_STA>(USART_REC_LEN-1))USART3_RX_STA=0;//接收数据错误,重新开始接收	  
// 				}		 
// 			}
// 		}   		 
   } 
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  											 
#endif
}


u8 Hand(char *a)                   // 串口命令识别函数
{ 
    if(strstr(USART2_RX_BUF,a)!=NULL)
	    return 1;
	else
		return 0;
}

void CLR_Buf(void)                           // 串口缓存清理
{
	memset(USART2_RX_BUF, 0, USART2_REC_LEN);      //清空
  point1 = 0;                    
}

void clrStruct()
{
	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
	memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
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
					//errorLog(1);	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W
						case 7:memcpy(Save_Data.rate, subString, subStringNext - subString);break;	
						case 8:memcpy(Save_Data.direction, subString, subStringNext - subString);break;	
						case 9:memcpy(Save_Data.UTCDate , subString, subStringNext - subString);break;	//获取E/W
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
				//	errorLog(2);	//解析错误
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

	

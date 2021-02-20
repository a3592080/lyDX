 /**********************************************************************************************************
** �ļ���		:main.c
** ����			:maxlicheng<licheng.chn@outlook.com>
** ����github	:https://github.com/maxlicheng
** ���߲���		:https://www.maxlicheng.com/	
** ��������		:2018-08-08
** ����			:��Ŀ���ļ���ɾ���������������޸ģ�ע������������С
************************************************************************************************************/
#include "sys.h"
#include "delay.h"
#include "usart.h"

#include "key.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "timer.h"

#include "sram.h"
#include "malloc.h"
#include "lwip_comm.h"
#include "includes.h"
#include "lwipopts.h"
#include "transport.h"

#include "math.h"	
#include "led.h"
#include "timer.h"
#include "exit.h"

#include "cJSON.h"
#include "mqtt_app.h"
#include "commit.h"

#include "tcp_client.h"

#include "w25qxx.h"
#include "task_stat.h"
#include "stmflash.h"
#include "sht30.h"
#include "myiic.h"
#include "httpd.h"
#include "usart2.h"
#include "cjson.h"

//START����
//�������ȼ�
#define START_TASK_PRIO		10
//�����ջ��С
#define START_STK_SIZE		128
//�����ջ
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata); 

//mqtt ����
#define mqtt_task_prio	11
#define mqtt_stk_size   256
__align(8) static OS_STK mqtt_task_stk[mqtt_stk_size];
void mqtt_task(void *pdata);

#define dhcpselect_task_prio 13
#define dhcpselect_stk_size 128
__align(8) static OS_STK dhcpselect_task_stk[dhcpselect_stk_size];
void dhcpselct_task(void *pdata);

#define dht11_task_prio	14
#define dht11_stk_size   256
__align(8) static OS_STK dht11_task_stk[dht11_stk_size];
void dht11_task(void *pdata);



#define fan_task_prio 8
#define fan_stk_size 128
__align(8) static OS_STK fan_task_stk[fan_stk_size];
void fan_task(void *pdata);


#define task_stat_info_prio OS_LOWEST_PRIO-2
#define task_stat_info_size 64
__align(8) static OS_STK task_stat_info_stk[task_stat_info_size];
void task_stat_info(void *pdata);

//------ע�Ტ��ѯ����---------------------



OS_EVENT *regist_msg_ptr;//�ź����¼����ƿ�
OS_EVENT *box_msg_ptr,* BOX_TIMING_PTR;



//-------------------------------------------------------------



extern u16 TIM1CH4_CAPTURE_STA;
extern u8 TIM1CH4_CAPTURE_VAL;
extern u8 mode;
u8 status=0;
u8 fan_warn = 0;
char get_passidbuf[50];

float 	temperature;        // temperature [��C]
 float	 humidity;	        // relative humidity [%RH]
char *buf2=NULL;
char *buf3=NULL;	
char * ss=NULL;	
char  _payload_out[516];
char TIME_BUF[25];
u8 sonboardloadover;
u8 commit_ok(char * );
void Get_Gps(void);
void errorLog(int num);
void errorlog(int num)
{
	while(1)
	{
		printf("eeror %d\r\n", num);
	}
}

int main(void)
{

	char regist_re_buf[50];
	delay_init(168);       	//��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϷ�������
	uart_init(115200);    	//���ڲ���������
	uart2_init(9600);//��GPSģ��ͨ��
	usart3_init(115200);//��װ��stm32RCͨ��

	W25QXX_Init();
	IIC_Init();
	
	
	FAN_Init();
	EXTIX_Init();       //��ʼ���ⲿ�ж����� 

	TIM1_Int_Init();

	FSMC_SRAM_Init();		//SRAM��ʼ��
	
	clrStruct();
	mymem_init(SRAMIN);  	//��ʼ���ڲ��ڴ��
	mymem_init(SRAMEX);  	//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM); 	//��ʼ��CCM�ڴ��
	buf2=mymalloc(SRAMCCM,1024);
	buf3=mymalloc(SRAMCCM,1024);
	while(buf2==0)
	{
		printf("buf2 malloc failed\r\n");
	}
	buf3=mymalloc(SRAMCCM,1024);
	while(buf3==0)
	{
		printf("buf3 malloc failed\r\n");
	}

//-------------------------------------------------------------------------------------------------------
	OSInit(); 					//UCOS��ʼ��
	clrStruct();
while(Save_Data.isUsefull==0)//���GPS����
	
	{
		Get_Gps();
		
		printf("gps data is not usefull\n");
	}
	Save_Data.isUsefull=0;
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//�ر�GPS����ж�
	sprintf(TIME_BUF,"20%d.%d.%d %d:%d:%d\r\n",year,mouth,date,hour,min,sec);
	while(sonboardloadover==0)
	{
			delay_ms(100);
			printf("�ط��Ӱ崮��\r\n");
			W25QXX_Read((u8*)buf2,0x1010,1024);
			sprintf(buf3,"%s\r\n",buf2);
			UART_SendStr(USART3, buf3, strlen(buf3));
			printf("%s",buf3);
			if(USART3_RX_STA&0x8000)
			{
				if(strstr(USART3_RX_BUF,"sonboard_ok"))sonboardloadover=1;
				else sonboardloadover=0;
				USART3_RX_STA=0;
			}
				
	}
while(lwip_comm_init()) 	//lwip��ʼ��
	{
		printf("Lwip Init failed!"); 	//lwip��ʼ��ʧ��
		delay_ms(500);
	}

	
		while(tcp_client_init()) 									//��ʼ��tcp_client(����tcp_client�߳�)
		{
		printf("TCP Client failed!!"); //tcp�ͻ��˴���ʧ��
		delay_ms(500);
		}
	

		
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);//�ڿ���UCOS֮ǰ�������ٽ���һ������//

	OSStart(); //����UCOS
}

//start����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	INT8U err;
	pdata = pdata ;
	
	printf("now at start task\n");
	OSStatInit();  			//��ʼ��ͳ������
	OS_ENTER_CRITICAL();  	//���ж�
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //����DHCP����
	OSTimeDlyHMSM(0,0,2,500);	
#endif
	
	regist_msg_ptr=OSSemCreate(0);
	box_msg_ptr=OSMboxCreate(NULL);
	BOX_TIMING_PTR = OSMboxCreate(NULL);
	//IntBuf = OSMemCreate(IntPart,8,516,&ERR);
	//printf("�ź����������\n");
	
	err=OSTaskCreateExt(dhcpselct_task,(void*)0,(OS_STK*)&dhcpselect_task_stk[dhcpselect_stk_size-1],dhcpselect_task_prio,dhcpselect_task_prio,(OS_STK*)&dhcpselect_task_stk[0],dhcpselect_stk_size,(void *)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); //mqtt����
	if(OS_ERR_NONE != err)
	{
		printf("create task failed %d\r\n",err);
	}
	err=OSTaskCreateExt(mqtt_task,(void*)0,(OS_STK*)&mqtt_task_stk[mqtt_stk_size-1],mqtt_task_prio,mqtt_task_prio,(OS_STK*)&mqtt_task_stk[0],mqtt_stk_size,(void *)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); //mqtt����
	if(OS_ERR_NONE != err)
	{
		printf("create task failed %d\r\n",err);
	}
	//	OSTaskCreate(temperature_task,(void *)0,(OS_STK*)&temperature_task_stk[temperature_stk_size-1],temperature_task_prio);
	
	err=OSTaskCreateExt(fan_task,(void *)0,(OS_STK*)&fan_task_stk[fan_stk_size-1],fan_task_prio,fan_task_prio,(OS_STK*)&fan_task_stk[0],fan_stk_size,(void *)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	if(OS_ERR_NONE != err)
	{
		printf("create task failed %d\r\n",err);
	}
	err=OSTaskCreateExt(dht11_task,(void*)0,(OS_STK*)&dht11_task_stk[dht11_stk_size-1],dht11_task_prio,dht11_task_prio,(OS_STK*)&dht11_task_stk[0],dht11_stk_size,(void *)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); //mqtt����
	if(OS_ERR_NONE != err)
	{
		printf("create task failed %d\r\n",err);
	}
	err=OSTaskCreateExt(task_stat_info,(void *)0,(OS_STK*)&task_stat_info_stk[task_stat_info_size-1],task_stat_info_prio,task_stat_info_prio,(OS_STK*)&task_stat_info_stk[0],task_stat_info_size,(void*)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	//OSTaskSuspend(OS_PRIO_SELF); 	//����start_task����
	OSTaskDel(OS_PRIO_SELF);
	
	OS_EXIT_CRITICAL();  			//���ж�
}


void mqtt_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	INT8U err;
	pdata = pdata ;
	
	printf("\r\ncJSON Version: %s\r\n", cJSON_Version());
	OSTaskSuspend(OS_PRIO_SELF); 	
	OSSemPend(regist_msg_ptr,0,&err);
	OSTimeDlyHMSM(0,0,0,500);  //
	mqtt_thread();
	while(1)
	{
		OSTimeDlyHMSM(0,0,0,500);  //
	}
}

void dht11_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	INT8U err;
	OS_STK_DATA StackBytes;

	
	pdata = pdata ;
	//_payload_out=mymalloc(SRAMIN,516);
	while(1)
	{
		
		ss=OSMboxPend(box_msg_ptr,0,&err);
//		OSTaskStkChk(OS_PRIO_SELF, &StackBytes);
//		printf("dhtll task freed stack is %d, used is %d\r\n",StackBytes.OSFree,StackBytes.OSUsed);
	
		if (strstr(ss,"event")==NULL)  
		{  
				printf("not the event\r\n");  
			
				USART3_RX_STA=0;
				USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�ر�GPS����ж�
		
				while(Save_Data.isUsefull==0)//���GPS����
			
					{
						Get_Gps();
					}
				
					USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//�ر�GPS����ж�
					
					Save_Data.isUsefull=0;
					
					sprintf(TIME_BUF,"20%d.%d.%d %d:%d:%d",year,mouth,date,hour,min,sec);
					if(mode==1)
						sprintf((char*)_payload_out,"{\"messageType\":\"state_timing\",\"deviceNo\":%d,\"time\":\"%s\",\"CurrentTemperature\":%0.1f,\"RelativeHumidity\":%0.1f,\"workState\":1,\"chnls\":%s}",CLIENT_ID,TIME_BUF,temperature, humidity,ss);
					if(mode==0)
					sprintf((char*)_payload_out,"{\"messageType\":\"state_timing\",\"deviceNo\":%d,\"time\":\"%s\",\"CurrentTemperature\":%0.1f,\"RelativeHumidity\":%0.1f,\"workState\":0,\"chnls\":%s}",CLIENT_ID,TIME_BUF,temperature, humidity,ss);
					if(mode==3)
					{
						mode = 0;
						sprintf((char*)_payload_out,"{\"messageType\":\"state_feedback\",\"deviceNo\":%d,\"time\":\"%s\",\"CurrentTemperature\":%0.1f,\"RelativeHumidity\":%0.1f,\"workState\":0,\"chnls\":%s}",CLIENT_ID,TIME_BUF,temperature, humidity,ss);
					}
					memset(USART3_RX_BUF,0,USART3_REC_LEN);
					OSMboxPost(BOX_TIMING_PTR,_payload_out);
					//myfree(SRAMIN,_payload_out);
					printf("temperature=%.1f��\thumidity=%.1f%%RH\t\r\n",(float)temperature,(float)humidity);
						printf("ss�ڴ�������%d\r\n",mem_perused(SRAMIN));
					printf("sramccm�ڴ�������%d\r\n",mem_perused(SRAMCCM));
					OSTimeDlyHMSM(0,0,0,500);  
		}
	else 
		{
			memcpy(_payload_out,ss,516);
			
			USART3_RX_STA=0;
			memset(USART3_RX_BUF,0,USART3_REC_LEN);
	
			OSMboxPost(BOX_TIMING_PTR,_payload_out);
	
		
		}
		OSTaskStkChk(OS_PRIO_SELF, &StackBytes);
		printf("dhtll task freed stack is %d, used is %d\r\n",StackBytes.OSFree,StackBytes.OSUsed);
		OSTimeDlyHMSM(0,0,0,500); 
	}
	
}

void dhcpselct_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	pdata = pdata ;
	while (1)
		{
			if( lwipdev.dhcpstatus==0XFF)
			{
				httpd_init(); 
				printf("����http web service\r\n");
				OSTaskSuspend(OS_PRIO_SELF); 	//����start_task����
			
			}else if(lwipdev.dhcpstatus==2)
			{
			
				OSTaskResume(12);//tcp_task
				OSTaskResume(mqtt_task_prio);
				OSTaskResume(dht11_task_prio);
//				OSTaskResume(temperature_task_prio);
				OSTaskResume(fan_task_prio);
				OSTaskSuspend(OS_PRIO_SELF); 	
			}
		}
}

//void temperature_task(void * pdata)
//{
//			
//				OS_CPU_SR cpu_sr;
//				INT8U rc;
//				pdata = pdata ;
//	OSTaskSuspend(OS_PRIO_SELF); 	
//			while(1){
//						delay_ms(1000);
//				
//				while (rc==0|rc==0xfb|rc==0xfd|rc==0xfc|rc==0xfe)
//				{
//				rc=ReadSht3xMeasure(&temperature,&humidity);
//				}
//			
//				printf("tttt temperature=%.1f��\thumidity=%.1f%%RH\t\r\n",(float)temperature,(float)humidity);
//					OSTimeDlyHMSM(0,0,0,5000);  
//			}
//	
//}

void fan_task(void * pdata)
{
				OS_CPU_SR cpu_sr;
				int temp=0;  
	OS_STK_DATA StackBytes;
				pdata = pdata ;
	OSTaskSuspend(OS_PRIO_SELF); 	
			while(1)
			{
			
				if(temperature>10)
				{	
						FAN=1;
	
					if(TIM1CH4_CAPTURE_STA&0x80)
					{
						temp=TIM1CH4_CAPTURE_STA&0X3F;
						temp*=0XFFFF; //���ʱ���ܺ�
						temp+=TIM1CH4_CAPTURE_VAL; //�õ��ܵĸߵ�ƽʱ��
						printf("HIGH:%d us\r\n",temp);//��ӡ�ܵĸߵ�ƽʱ��
						TIM1CH4_CAPTURE_STA=0; //������һ�β���
					}
						OSTimeDlyHMSM(0,0,0,20);  
					if(temp==0)
						fan_warn =1;
					else fan_warn =0;
	
				}
				else FAN=0;
//						OSTaskStkChk(OS_PRIO_SELF, &StackBytes);
//		printf("fan task freed stack is %d, used is %d\r\n",StackBytes.OSFree,StackBytes.OSUsed);
					OSTimeDlyHMSM(0,0,0,5000);  
				
						
		
				}
	
}




	
	
		
	
	
	




 /**********************************************************************************************************
** 文件名		:main.c
** 作者			:maxlicheng<licheng.chn@outlook.com>
** 作者github	:https://github.com/maxlicheng
** 作者博客		:https://www.maxlicheng.com/	
** 生成日期		:2018-08-08
** 描述			:项目主文件，删减功能请在这里修改，注意任务数量大小
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

//START任务
//任务优先级
#define START_TASK_PRIO		10
//任务堆栈大小
#define START_STK_SIZE		128
//任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata); 

//mqtt 任务
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

//------注册并查询任务---------------------



OS_EVENT *regist_msg_ptr;//信号量事件控制块
OS_EVENT *box_msg_ptr,* BOX_TIMING_PTR;



//-------------------------------------------------------------



extern u16 TIM1CH4_CAPTURE_STA;
extern u8 TIM1CH4_CAPTURE_VAL;
extern u8 mode;
u8 status=0;
u8 fan_warn = 0;
char get_passidbuf[50];

float 	temperature;        // temperature [°C]
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
	delay_init(168);       	//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断分组配置
	uart_init(115200);    	//串口波特率设置
	uart2_init(9600);//与GPS模块通信
	usart3_init(115200);//与底板的stm32RC通信

	W25QXX_Init();
	IIC_Init();
	
	
	FAN_Init();
	EXTIX_Init();       //初始化外部中断输入 

	TIM1_Int_Init();

	FSMC_SRAM_Init();		//SRAM初始化
	
	clrStruct();
	mymem_init(SRAMIN);  	//初始化内部内存池
	mymem_init(SRAMEX);  	//初始化外部内存池
	mymem_init(SRAMCCM); 	//初始化CCM内存池
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
	OSInit(); 					//UCOS初始化
	clrStruct();
while(Save_Data.isUsefull==0)//获得GPS数据
	
	{
		Get_Gps();
		
		printf("gps data is not usefull\n");
	}
	Save_Data.isUsefull=0;
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//关闭GPS相关中断
	sprintf(TIME_BUF,"20%d.%d.%d %d:%d:%d\r\n",year,mouth,date,hour,min,sec);
	while(sonboardloadover==0)
	{
			delay_ms(100);
			printf("重发子板串口\r\n");
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
while(lwip_comm_init()) 	//lwip初始化
	{
		printf("Lwip Init failed!"); 	//lwip初始化失败
		delay_ms(500);
	}

	
		while(tcp_client_init()) 									//初始化tcp_client(创建tcp_client线程)
		{
		printf("TCP Client failed!!"); //tcp客户端创建失败
		delay_ms(500);
		}
	

		
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);//在开启UCOS之前必须至少建立一个任务//

	OSStart(); //开启UCOS
}

//start任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	INT8U err;
	pdata = pdata ;
	
	printf("now at start task\n");
	OSStatInit();  			//初始化统计任务
	OS_ENTER_CRITICAL();  	//关中断
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //创建DHCP任务
	OSTimeDlyHMSM(0,0,2,500);	
#endif
	
	regist_msg_ptr=OSSemCreate(0);
	box_msg_ptr=OSMboxCreate(NULL);
	BOX_TIMING_PTR = OSMboxCreate(NULL);
	//IntBuf = OSMemCreate(IntPart,8,516,&ERR);
	//printf("信号量创建完成\n");
	
	err=OSTaskCreateExt(dhcpselct_task,(void*)0,(OS_STK*)&dhcpselect_task_stk[dhcpselect_stk_size-1],dhcpselect_task_prio,dhcpselect_task_prio,(OS_STK*)&dhcpselect_task_stk[0],dhcpselect_stk_size,(void *)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); //mqtt任务
	if(OS_ERR_NONE != err)
	{
		printf("create task failed %d\r\n",err);
	}
	err=OSTaskCreateExt(mqtt_task,(void*)0,(OS_STK*)&mqtt_task_stk[mqtt_stk_size-1],mqtt_task_prio,mqtt_task_prio,(OS_STK*)&mqtt_task_stk[0],mqtt_stk_size,(void *)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); //mqtt任务
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
	err=OSTaskCreateExt(dht11_task,(void*)0,(OS_STK*)&dht11_task_stk[dht11_stk_size-1],dht11_task_prio,dht11_task_prio,(OS_STK*)&dht11_task_stk[0],dht11_stk_size,(void *)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); //mqtt任务
	if(OS_ERR_NONE != err)
	{
		printf("create task failed %d\r\n",err);
	}
	err=OSTaskCreateExt(task_stat_info,(void *)0,(OS_STK*)&task_stat_info_stk[task_stat_info_size-1],task_stat_info_prio,task_stat_info_prio,(OS_STK*)&task_stat_info_stk[0],task_stat_info_size,(void*)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	//OSTaskSuspend(OS_PRIO_SELF); 	//挂起start_task任务
	OSTaskDel(OS_PRIO_SELF);
	
	OS_EXIT_CRITICAL();  			//开中断
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
				USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//关闭GPS相关中断
		
				while(Save_Data.isUsefull==0)//获得GPS数据
			
					{
						Get_Gps();
					}
				
					USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//关闭GPS相关中断
					
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
					printf("temperature=%.1f℃\thumidity=%.1f%%RH\t\r\n",(float)temperature,(float)humidity);
						printf("ss内存利用率%d\r\n",mem_perused(SRAMIN));
					printf("sramccm内存利用率%d\r\n",mem_perused(SRAMCCM));
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
				printf("启动http web service\r\n");
				OSTaskSuspend(OS_PRIO_SELF); 	//挂起start_task任务
			
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
//				printf("tttt temperature=%.1f℃\thumidity=%.1f%%RH\t\r\n",(float)temperature,(float)humidity);
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
						temp*=0XFFFF; //溢出时间总和
						temp+=TIM1CH4_CAPTURE_VAL; //得到总的高电平时间
						printf("HIGH:%d us\r\n",temp);//打印总的高点平时间
						TIM1CH4_CAPTURE_STA=0; //开启下一次捕获
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




	
	
		
	
	
	




/**********************************************************************************************************
** 文件名		:mqtt_app.c
** 作者			:maxlicheng<licheng.chn@outlook.com>
** 作者github	:https://github.com/maxlicheng
** 作者博客		:https://www.maxlicheng.com/	
** 生成日期		:2018-08-08
** 描述			:mqtt服务程序
************************************************************************************************************/
#include "mqtt_app.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "ucos_ii.h"
#include "led.h"
#include "ds18b20.h"
#include "malloc.h"
#include "lcd.h"
#include "usart2.h"
#include <string.h>
#include <stdio.h>
#include "usart2.h"
#include "hmac.h"
#include "cJSON.h"

#include "U2GBK.h"
#include "text.h"	
#include "stmflash.h"
#include "sht30.h"
#include "commit.h"
#include "w25qxx.h"
#include "key.h"
#define send_duration	10	//温度发送周期（ms）

extern    float 	temperature;        // temperature [°C]
extern  float	 humidity;	        // relative humidity [%RH]
extern char  _payload_out[516];
extern  u8 fan_warn;
extern OS_EVENT * BOX_TIMING_PTR;
char *rdbuf=NULL;
u8 mode=0;
char rdfromflash_regist_buf[64] ;
char rdfromflash_query_buf[64];
 char *payload_out=NULL;
	int payload_out_len = 0;
#define regist_data_len sizeof(rdfromflash_regist_buf)
#define _REGIST_SIZE regist_data_len/4+((regist_data_len%4)?1:0) 
#define query_data_len sizeof(rdfromflash_query_buf)
#define _QUERY_SIZE query_data_len/4+((query_data_len%4)?1:0) 

void mcuRestart(void)
{
  __set_FAULTMASK(1); //关闭所有中断
 NVIC_SystemReset(); //复位
}


void mqtt_thread(void)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	MQTTString receivedTopic;
	MQTTString topicString = MQTTString_initializer;
	
	//volatile char *payload_out;
	//int payload_out_len = 0;
	

	OS_CPU_SR cpu_sr=0;
	uint32_t curtick  =	OSTimeGet();
	uint32_t sendtick = OSTimeGet();
	OS_STK_DATA StackBytes;
	INT8U err;
	int rc = 0;
	unsigned char buf[1024];
	int buflen = sizeof(buf);
	int	mysock = 0;
	int sjtime=0;
	int payloadlen_in;
	unsigned char* payload_in;
	unsigned short msgid = 1;
	int subcount;
	int granted_qos =0;
	unsigned char sessionPresent, connack_rc;
	unsigned short submsgid;
	int len = 0;
	int req_qos = 1;
	int publish_req_qos =2;
	unsigned char dup;
	int qos;
	int ti=1;
	char sendbuf[25];
	char tempbuf[64];
	char tempbuf1[64];
	unsigned char retained = 0;
	
	cJSON *json ,*json_command,*json_position,*json_state;
	cJSON *json_reset;
	cJSON *json_init,*json_devicename,*json_devicevalue;
	
	uint8_t connect_flag = 0;		//连接标志
	uint8_t msgtypes = CONNECT;		//消息状态初始化
	uint8_t t=0;
	char usartsendbuf[100];
	printf("socket connect to server\r\n");
	//-------MQTT服务器的IP和PORT-------//
	mysock = transport_open((char *)HOST_NAME,HOST_PORT);//
	printf("Sending to hostname %s port %d\r\n", HOST_NAME, HOST_PORT);
	
//	len = MQTTSerialize_disconnect((unsigned char*)buf,buflen);
//	rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
//	if(rc == len)															//
//		printf("send DISCONNECT Successfully\r\n");
//	else
//		printf("send DISCONNECT failed\r\n"); 
//	
//	OSTimeDlyHMSM(0,0,2,500);
//	
//	printf("socket connect to server\r\n");
//	mysock = transport_open((char *)HOST_NAME,HOST_PORT);
//	printf("Sending to hostname %s port %d\r\n", HOST_NAME, HOST_PORT);
	//--------------------------------------------------------------------
	//-----------------------------------------------------------------
	//----------------------------读flash，得到用户产品ID和密码-----------------------
	W25QXX_Read((u8 *)rdfromflash_regist_buf,100,40);
	W25QXX_Read((u8 *)rdfromflash_query_buf,200,40);
	printf("the id is %s and the passcode is %s\r\n",rdfromflash_regist_buf,rdfromflash_query_buf);
	sprintf(tempbuf,"%d",CLIENT_ID);
	sprintf(tempbuf1,"box_%d",CLIENT_ID);
	data.clientID.cstring = tempbuf;//rdfromflash_regist_buf;
	data.keepAliveInterval = 60;		//心跳时长
	data.cleansession = 1;
	data.username.cstring = rdfromflash_regist_buf;//rdfromflash_regist_buf;
	data.password.cstring = rdfromflash_query_buf;//rdfromflash_query_buf;
	data.MQTTVersion = 4;
	//myfree(SRAMEX, PASSWORD);
	
	
	
	while(1)
	{
		if((OSTimeGet() - curtick) >(data.keepAliveInterval*200))		//uCosII 每秒200次tick
		{
			if(msgtypes == 0)
			{
				curtick = OSTimeGet();
				msgtypes = PINGREQ;
			}

		}
		if(connect_flag == 1)
		{
			if((OSTimeGet() - sendtick) >= (send_duration*200))
			{
				
				sendtick = OSTimeGet();
//					OS_ENTER_CRITICAL();	//进入临界区(无法被中断打断)
				rc =ReadSht3xMeasure(&temperature,&humidity);	
//			OS_EXIT_CRITICAL();		//退出临界区(可以被中断打断)
					printf("mqtt temperature=%.1f℃\thumidity=%.1f%%RH\t\r\n",(float)temperature,(float)humidity);
				printf("in 定时上报\r\n");
			//	MUSART1_SendStr("displaynew 220v_ch1\r\n");
				sprintf(sendbuf,"displaynew\r\n");
				UART_SendStr(USART3,sendbuf,strlen(sendbuf));
				
				
				if(door_flag)
				{
					rdbuf=mymalloc(SRAMIN,256);
					if(rdbuf){
					sprintf(rdbuf,"{\"messageType\":\"event\",\"deviceNo\":%d,\"chnl\":-1,\"eventCode\":0,\"eventMessage\":\"open the door\"}",CLIENT_ID);
						printf("the door buf is ok\t,%s\r\n",rdbuf);
					}
					topicString.cstring = DEVICE_PUBLISH;		//属性上报 发布
					len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, publish_req_qos, retained, msgid, topicString, (unsigned char*)rdbuf, rdbuf_len);
					rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
					if(rc == len)															//
					printf("send door publish Successfully\r\n");
					else
					printf("send door publish failed\r\n");  
					door_flag=0;
					myfree(SRAMIN,rdbuf);
					memset(buf,0,1024);
				}
				if(fan_warn)
				{
						rdbuf=mymalloc(SRAMIN,256);
					if(rdbuf)
					{
					sprintf(rdbuf,"{\"messageType\":\"event\",\"deviceNo\":%d,\"chnl\":-1,\"eventCode\":0,\"eventMessage\":\"fan is not work\"}",CLIENT_ID);
					}
					topicString.cstring = DEVICE_PUBLISH;		//属性上报 发布
					len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, publish_req_qos, retained, msgid, topicString, (unsigned char*)rdbuf, rdbuf_len);
					rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
					if(rc == len)															//
					printf("send fan publish Successfully\r\n");
					else
					printf("send fan publish failed\r\n");  
					fan_warn=0;
					myfree(SRAMIN,rdbuf);
					memset(buf,0,1024);
					
				}
				
			
				if(ti==1){
				rdbuf=mymalloc(SRAMIN,1024);
				
				//STMFLASH_Read(0X080B0100,(u32*)rdbuf,256);
				W25QXX_Read((u8 *)rdbuf,0x1010,1024);
				topicString.cstring = DEVICE_PUBLISH;		//属性上报 发布
				len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, publish_req_qos, retained, msgid, topicString, (unsigned char*)rdbuf, rdbuf_len);
				rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
				if(rc == len)															//
					printf("send init PUBLISH Successfully\r\n");
				else
					printf("send init PUBLISH failed\r\n");  
				ti=0;
				myfree(SRAMIN,rdbuf);
				memset(buf,0,1024);
				
				}
	//			payload_out=mymalloc(SRAMIN,1024);
	
			
			//-------------------------------发送到MQTT---------------------------------------
				payload_out=OSMboxPend(BOX_TIMING_PTR,1,&err);
				//memset(_payload_out,0,516);
				if(payload_out)
				{	
						printf("mqtt内存利用率%d\r\n",mem_perused(SRAMIN));
					payload_out_len = strlen((char*)payload_out);
					topicString.cstring = DEVICE_PUBLISH;		//属性上报 发布
					len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, publish_req_qos, retained, msgid, topicString, (unsigned char*)payload_out, payload_out_len);
					rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
					if(rc == len)															//
						printf("send PUBLISH Successfully\r\n");
					else
						printf("send PUBLISH failed\r\n");  
				//	memset();
					//----------------------------------------------------------------------------------------
					myfree(SRAMIN,payload_out);
						printf("mqtt内存利用率%d\r\n",mem_perused(SRAMIN));
				}	
			
				
			}
			
		}
		switch(msgtypes)
		{

			case CONNECT:	len = MQTTSerialize_connect((unsigned char*)buf, buflen, &data); 						//获取数据组长度		发送连接信息     
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);		//发送 返回发送数组长度
							if(rc == len)															//
								printf("send CONNECT Successfully\r\n");
							else
								printf("send CONNECT failed\r\n");               
							printf("MQTT concet to server!\r\n");
							msgtypes = 0;
							break;

			case CONNACK:   if(MQTTDeserialize_connack(&sessionPresent, &connack_rc, (unsigned char*)buf, buflen) != 1 || connack_rc != 0)	//收到回执
							{
								printf("Unable to connect, return code %d\r\n", connack_rc);		//回执不一致，连接失败
								msgtypes = CONNECT;	
							}
							else
							{
								printf("MQTT is concet OK!\r\n");									//连接成功
								connect_flag = 1;
								msgtypes = SUBSCRIBE;													//连接成功 执行 订阅 操作
							}
							
							break;
			case SUBSCRIBE: topicString.cstring = DEVICE_SUBSCRIBE;
							len = MQTTSerialize_subscribe((unsigned char*)buf, buflen, 0, msgid, 1, &topicString, &req_qos);
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
							if(rc == len)
								printf("send SUBSCRIBE Successfully\r\n");
							else
							{
								printf("send SUBSCRIBE failed\r\n"); 
								t++;
								if(t >= 10)
								{
									t = 0;
									msgtypes = CONNECT;
								}
								else
									msgtypes = SUBSCRIBE;
								break;
							}
							printf("client subscribe:[%s]\r\n",topicString.cstring);
							msgtypes = 0;
							break;
			case SUBACK: 	rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, (unsigned char*)buf, buflen);	//有回执  QoS                                                     
							printf("granted qos is %d\r\n", granted_qos);         								//打印 Qos                                                       
							msgtypes = 0;
							break;
			case PUBLISH:	payload_in=mymalloc(SRAMIN,125);
								rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,&payload_in, &payloadlen_in, (unsigned char*)buf, buflen);	//服务器有推送信息
							printf("message arrived : %s\r\n", payload_in);
							
								if(strstr((char*)payload_in,"switch"))
								{
									if(strstr((char *)payload_in,"\"position\":\"0\""))
									{
										printf("position0 is option\r\n");
										if(strstr((char *)payload_in,"\"state\":\"1\""))	sprintf(usartsendbuf,"open 220v_ch1\r\n");
										else sprintf(usartsendbuf,"close 220v_ch1\r\n");
									}
									if(strstr((char *)payload_in,"\"position\":\"1\""))
									{
										if(strstr((char *)payload_in,"\"state\":\"1\""))	sprintf(usartsendbuf,"open 220v_ch4\r\n");
										else sprintf(usartsendbuf,"close 220v_ch4\r\n");
									}
									if(strstr((char *)payload_in,"\"position\":\"2\""))
									{
										if(strstr((char *)payload_in,"\"state\":\"1\""))	sprintf(usartsendbuf,"open 24v_ch1\r\n");
										else sprintf(usartsendbuf,"close 24v_ch1\r\n");
									}
										if(strstr((char *)payload_in,"\"position\":\"3\""))
									{
										if(strstr((char *)payload_in,"\"state\":\"1\""))	sprintf(usartsendbuf,"open 24v_ch2\r\n");
										else sprintf(usartsendbuf,"close 24v_ch2\r\n");
									}
										if(strstr((char *)payload_in,"\"position\":\"4\""))
									{
										if(strstr((char *)payload_in,"\"state\":\"1\""))	sprintf(usartsendbuf,"open 12v_ch1\r\n");
										else sprintf(usartsendbuf,"close 12v_ch1\r\n");
									}
									if(strstr((char *)payload_in,"\"position\":\"5\""))
									{
										if(strstr((char *)payload_in,"\"state\":\"1\""))	sprintf(usartsendbuf,"open 12v_ch2\r\n");
										else sprintf(usartsendbuf,"close 12v_ch2\r\n");
									}
									if(strstr((char *)payload_in,"\"position\":\"6\""))
									{
										if(strstr((char *)payload_in,"\"state\":\"1\""))	sprintf(usartsendbuf,"open 5v_ch\r\n");
										else sprintf(usartsendbuf,"close 5v_ch\r\n");
									}
									if(strstr((char *)payload_in,"\"position\":\"7\""))
									{
										if(strstr((char *)payload_in,"\"state\":\"1\""))	sprintf(usartsendbuf,"open 9v_ch\r\n");
										else sprintf(usartsendbuf,"close 9v_ch\r\n");
									}
									UART_SendStr(USART3,usartsendbuf,strlen(usartsendbuf));
										memset(usartsendbuf,0,100);
									mode=3;
									msgtypes = 0;
								}
								if(strstr((char*)payload_in,"reset")){mcuRestart();msgtypes = 0;}
								if(strstr((char*)payload_in,"info"))msgtypes=15;
								if(strstr((char*)payload_in,"setMode"))
								{
									if(strstr((char*)payload_in,"operation"))mode = 1;
									else mode = 0;
									msgtypes = 0;
								}
								if(strstr((char*)payload_in,"restart")){RESTAR=1;msgtypes=0;}
								
								myfree(SRAMIN,payload_in);
								//memset(usartsendbuf,0,100);
	//-----------------------------解析数据包------------------------------------------------------------------//
							//json = cJSON_Parse((char *)payload_in);			//
						
//							if (!json)  
//							{  
//								printf("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
//								
//							} 
//							else
//							{
//								json_command = cJSON_GetObjectItem(json , "command"); 
//								
//								if(json_command->type == cJSON_String)
//								{
//									printf("command:%s\r\n", json_command->valuestring);  
//								}
//								if(strcmp(json_command->valuestring,"switch")==0)
//								{
//									json_position = cJSON_GetObjectItem(json , "position");
//									json_state = cJSON_GetObjectItem(json , "state");
//									switch(atoi(json_position->valuestring))
//										{
//											case 0 : if(strcmp(json_state->valuestring,"1")==0)
//														sprintf(usartsendbuf,"open 220v_ch1\r\n");
//														else sprintf(usartsendbuf,"close 220v_ch1\r\n");
//													break;
//											case 1 : if(strcmp(json_state->valuestring,"1")==0)
//														sprintf(usartsendbuf,"open 220v_ch4\r\n");
//														else sprintf(usartsendbuf,"close 220v_ch4\r\n");
//													break;
//											case 2 : if(strcmp(json_state->valuestring,"1")==0)
//														sprintf(usartsendbuf,"open 24v_ch1\r\n");
//														else sprintf(usartsendbuf,"close 24v_ch1\r\n");
//													break;
//											case 3 : if(strcmp(json_state->valuestring,"1")==0)
//														sprintf(usartsendbuf,"open 24v_ch2\r\n");
//														else sprintf(usartsendbuf,"close 24v_ch2\r\n");
//													break;
//											case 4 : if(strcmp(json_state->valuestring,"1")==0)
//														sprintf(usartsendbuf,"open 12v_ch1\r\n");
//														else sprintf(usartsendbuf,"close 12v_ch1\r\n");
//													break;
//											case 5 : if(strcmp(json_state->valuestring,"1")==0)
//														sprintf(usartsendbuf,"open 12v_ch2\r\n");
//														else sprintf(usartsendbuf,"close 12v_ch2\r\n");
//													break;
//											case 6 : if(strcmp(json_state->valuestring,"1")==0)
//														sprintf(usartsendbuf,"open 5v_ch\r\n");
//														else sprintf(usartsendbuf,"close 5v_ch\r\n");
//													break;
//											case 7 : if(strcmp(json_state->valuestring,"1")==0)
//														sprintf(usartsendbuf,"open 9v_ch\r\n");
//														else sprintf(usartsendbuf,"close 9v_ch\r\n");
//													break;
//											}
//										UART_SendStr(USART3,usartsendbuf,strlen(usartsendbuf));
//										memset(usartsendbuf,0,100);
//									
//									}
//								cJSON_Delete(json);
//									
//										
//								}
//								if(strcmp(json_command->valuestring,"reset")==0)
//								{
//									mcuRestart();
//										cJSON_Delete(json);
//									
//								}
//								if(strcmp(json_command->valuestring,"info")==0)
//								{
//									msgtypes=15;
//									cJSON_Delete(json);	
//								}										
							if(qos == 1)
							{
								printf("publish qos is 1,send publish ack.\r\n");							//Qos为1，进行回执 响应
								memset(buf,0,buflen);
								len = MQTTSerialize_ack((unsigned char*)buf,buflen,PUBACK,dup,msgid);   					//publish ack                       
								rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);			//
								if(rc == len)
									printf("send PUBACK Successfully\r\n");
								else
									printf("send PUBACK failed\r\n");                                       
							}
							
							break;
			case 15:
									rdbuf=mymalloc(SRAMIN,1024);
									 W25QXX_Read((u8*)rdbuf,0x1010,700);
									//sprintf(rdbuf,"%s\r\n",rdbuf);
									
									topicString.cstring = DEVICE_PUBLISH;		//属性上报 发布
									len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, publish_req_qos, retained, msgid, topicString, (unsigned char*)rdbuf, rdbuf_len);
									rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
									if(rc == len)															//
										printf("send init PUBLISH Successfully\r\n");
									else
										printf("send init PUBLISH failed\r\n");  
									ti=0;
										myfree(SRAMIN,rdbuf);
									msgtypes = 0;
							break;
			
									
			case PUBACK:    printf("PUBACK!\r\n");					//发布成功
							msgtypes = 0;
							break;

			case PUBREC:    printf("PUBREC!\r\n");     				//just for qos2
							msgtypes=PUBREL;
							break;
			case PUBREL:    printf("PUBREL!\r\n");        			//just for qos2
							memset(buf,0,buflen);
								len = MQTTSerialize_ack((unsigned char*)buf,buflen,PUBREL,dup,msgid);   					//publish ack                       
								rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);			//
								if(rc == len)
									printf("send PUBREL Successfully\r\n");
								else
									printf("send PUBREL failed\r\n");  
								msgtypes = 0;
							break;
			case PUBCOMP:   printf("PUBCOMP!\r\n");        			//just for qos2
							msgtypes = 0;
							break;
			case PINGREQ:   
								printf("time to ping mqtt server to take alive!\r\n");
							len = MQTTSerialize_pingreq((unsigned char*)buf, buflen);							//心跳
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
							if(rc == len)
							{
								printf("send PINGREQ Successfully\r\n");
								msgtypes = 0;
							}
							else 
								{
									printf("send PINGREQ failed\r\n");   
									printf("重新连接MQTT服务器\r\n");
									connect_flag = 0;
									transport_close(mysock);
									mysock = 0;
									mysock = transport_open((char *)HOST_NAME,HOST_PORT);
									msgtypes = CONNECT;
									
								}
														
							
								//    
							
								
								break;
			case PINGRESP:	printf("mqtt server Pong\r\n");  			//心跳回执，服务有响应                                                     
							msgtypes = 0;
							break;
			default:
							break;

		}
		memset(buf,0,buflen);
		rc=MQTTPacket_read((unsigned char*)buf, buflen, transport_getdata);       	//轮询，读MQTT返回数据，
		if(rc >0)													//如果有数据，进入相应状态。
		{
			msgtypes = rc;
			printf("MQTT is get recv:\r\n");
		}
			OSTaskStkChk(OS_PRIO_SELF, &StackBytes);
		printf("mqtt task freed stack is %d, used is %d\r\n",StackBytes.OSFree,StackBytes.OSUsed);
		OSTimeDlyHMSM(0,0,0,500); 
		
	}
	transport_close(mysock);
    printf("mqtt thread exit.\r\n");
    OSTaskDel(NULL);
}



/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 存放目标字符串
// [IN] pbSrc - 输入16进制数的起始地址
// [IN] nLen - 16进制数的字节数
// return value: 
// remarks : 将16进制数转化为字符串
*/
void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen)
{
	char ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = '\0';
}

//通过hmac_sha1算法获取password
void getPassword(const char *device_secret, const char *content, char *password)
{
	char buf[256] = {0};
	int len = sizeof(buf);

//	printf("\r\nlen = %d\r\n\r\n", len);

	hmac_sha1(device_secret, strlen(device_secret), content, strlen(content), buf, &len);
	HexToStr(password, buf, len);
}





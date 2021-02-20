#include "tcp_client.h"
#include "cJSON.h"
#include "lwip/opt.h"
#include "lwip_comm.h"
#include "lwip/lwip_sys.h"
#include "lwip/api.h"
#include "includes.h"
#include "commit.h"
#include "cJSON.h"
#include "stmflash.h"
#include "w25qxx.h"


 #define RECV_STEP	1
 #define SEND_STEP	2
 #define REGISTER_FLASH_STEP 3
 #define QUERY_FALSH_STEP 4
 #define PARSE_STEP 5
 #define Suspend_STEP 6
 #define CONNECT_STEP 7
 #define COMPETE_STEP 8
 #define NEXT_STEP_QUE 1
 #define NEXT_STEP_REG 0
 #define REMAIN_CONNTIME 3
 extern OS_EVENT *regist_msg_ptr;//信号量事件控制块
 
u8_t TCP_STEP=RECV_STEP;

char  regist_buf[64]={0};//账号buf
char  passcode_buf[64]={0};//密码buf
	#define REGIST_LENTH sizeof(regist_buf)	 		  	//数组长度	
	#define SIZE_REGIST REGIST_LENTH/4+((REGIST_LENTH%4)?1:0)
	#define PASSCODE_LENTH sizeof(passcode_buf)	 		  	//数组长度	
	#define SIZE_PASSCODE PASSCODE_LENTH/4+((PASSCODE_LENTH%4)?1:0)

struct netconn *tcp_clientconn;					//TCP CLIENT网络连接结构体
char tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//TCP客户端接收数据缓冲区
//char tcp_client_sendbuf[100]={0};	//TCP客户端发送数据缓冲区
u8 tcp_client_flag;		//TCP客户端数据发送标志位
//char *get_passidbuf;
//TCP客户端任务

#define TCPCLIENT_PRIO		12
//任务堆栈大小
#define TCPCLIENT_STK_SIZE	300
//任务堆栈
OS_STK TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE];
u8_t TCP_STEP;

u8 check_flash(u8 * buf)
{
		int i;
		u8 temp=buf[0];
		for( i = 0 ; i<40; i++)
	{
		if(buf[i]^temp) goto err;
		temp=buf[i];
	}
	return 0;
	err:
	return 1;
}	

cJSON * root;
char*page_buf1 ;
char * sendbuf;
char * add_json(u8_t arg,void * pbuf)
{
	u32 sn0;
	sn0=*(vu32*)(0x1FFF7A10);//获取STM32的唯一ID的前24位作为MAC地址后三字节
	root = cJSON_CreateObject();
	if(!root)
	{
		printf("get root failed\n");
	}
	if(arg==NULL)
	{
	printf("register pbuf is using\n");
	cJSON_AddStringToObject(root,"method","register");
	cJSON_AddNumberToObject(root,"no",sn0);
	cJSON_AddStringToObject(root,"longitude",Save_Data.longitude);
	cJSON_AddStringToObject(root,"latitude",Save_Data.latitude);
		
	}
	else 
	{
	printf("query pbuf is using\n");
	cJSON_AddStringToObject(root,"method","query");
	cJSON_AddNumberToObject(root,"no",sn0);
	cJSON_AddStringToObject(root,"code",(char *) pbuf);
	}
	sendbuf = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	sprintf(sendbuf,"%s\r\n",sendbuf);
	//printf("%s",sendbuf);
	return sendbuf;
	
}



//tcp客户端任务函数
static void tcp_client_thread(void *arg)
{
	OS_CPU_SR cpu_sr;
	u32 data_len = 0;
	struct pbuf *q;
	err_t err,recv_err;
	static ip_addr_t server_ipaddr,loca_ipaddr;
	static u16_t 		 server_port,loca_port;
	
	cJSON * get_r;
	cJSON * json_result;
	cJSON * json_value;
	char * tcp_client_sendbuf;
	u8_t counter;

	u8_t TCP_STEP=CONNECT_STEP;
	u8_t fin_step=0;
	struct netbuf *recvbuf;
	LWIP_UNUSED_ARG(arg);
	server_port = REMOTE_PORT;
	
	OSTaskSuspend(OS_PRIO_SELF); 	
	printf("now in tcp client task\n");

	W25QXX_Read((u8*)regist_buf,100,(u16)40);
	
	printf("username %s\r\n",regist_buf);

	W25QXX_Read((u8*)passcode_buf,200,(u16)40);
	
	printf("password is %s\r\n",passcode_buf);
	
	
	if(check_flash((u8 *)regist_buf)==0||check_flash((u8 *)passcode_buf)==0)//如果flash数据为空，说明是新设备，则TCP注册
	{	
	
	IP4_ADDR(&server_ipaddr,47,105,94,219);//lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]);
	tcp_client_sendbuf =add_json(NULL,NULL);
	fin_step 	= 	NEXT_STEP_REG;
	counter 	=		REMAIN_CONNTIME;
	while (1)
	{
	
		switch(TCP_STEP)
		{
		case CONNECT_STEP:	tcp_clientconn = netconn_new(NETCONN_TCP);  //创建一个TCP链接
												err = netconn_connect(tcp_clientconn,&server_ipaddr,server_port);//连接服务器
												if(err != ERR_OK) 
												{
													netconn_delete(tcp_clientconn); //返回值不等于ERR_OK,删除tcp_clientconn连接
													TCP_STEP = (counter==0)?Suspend_STEP:CONNECT_STEP;//counter 初始值为3,连接3次，不成挂起
													counter --;
													if(counter ==0)break;
												}							
												else if (err == ERR_OK)    //处理新连接的数据
												{								
													tcp_clientconn->recv_timeout = 10;
													counter =3;
													netconn_getaddr(tcp_clientconn,&loca_ipaddr,&loca_port,1); //获取本地IP主机IP地址和端口号
													printf("连接上服务器%d.%d.%d.%d,本机端口号为:%d\r\n",47,105,94,219,loca_port);
													TCP_STEP = RECV_STEP;
						
													tcp_client_sendbuf =(fin_step==0)?add_json(NULL,NULL):add_json(1,regist_buf);								
													break;
												}
		case RECV_STEP: if((recv_err = netconn_recv(tcp_clientconn,&recvbuf)) == ERR_OK)  //接收到数据
						{	
							printf("now RECV_STEP\n");
							OS_ENTER_CRITICAL(); //关中断
							memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //数据接收缓冲区清零
							for(q=recvbuf->p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
							{
							//判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
							//的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
								if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//拷贝数据
								else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
								data_len += q->len;  	
								if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
							}
							OS_EXIT_CRITICAL();  //开中断
							TCP_STEP = PARSE_STEP;
							printf("%s\r\n",tcp_client_recvbuf);
							data_len=0;  //复制完成后data_len要清零。					
							netbuf_delete(recvbuf);
							break;
						
						}	
						else if(recv_err == ERR_CLSD)  //关闭连接
						{
							netconn_close(tcp_clientconn);
							netconn_delete(tcp_clientconn);
							TCP_STEP = CONNECT_STEP;
							counter 	=		REMAIN_CONNTIME;
							printf("服务器%d.%d.%d.%d断开连接\r\n",47,105,94,219);
							break;
						}
						else 
						{
							TCP_STEP = RECV_STEP;
							break;
						}
				
				
					
	case PARSE_STEP	:	printf("now PARSE_STEP\n");		
										if(strstr(tcp_client_recvbuf,"hi,this is MessageServer"))
										{
											TCP_STEP = SEND_STEP;
											break;
										}
										get_r = cJSON_Parse(tcp_client_recvbuf);//解析收到的ACK
										if(!get_r)
										{
											printf("get the ack failed \n");
											cJSON_Delete(get_r);
											TCP_STEP = SEND_STEP;
											break;
								
										}
										json_result = cJSON_GetObjectItem(get_r,"code");
										if(!json_result)
										{
											printf("no result\n");
											cJSON_Delete(get_r);
											TCP_STEP = SEND_STEP;
											break;
										}
										if(json_result->type == cJSON_Number)
										{
											printf("the ack result is %d \n",json_result->valueint);
											switch(json_result->valueint)
											{
											case	0: 
															json_value = cJSON_GetObjectItem(get_r, "value");
															if(json_value->type == cJSON_String)
															{	
																printf("the pass code is %s \n", json_value->valuestring);
																sprintf(get_passidbuf,"%s",json_value->valuestring);
																TCP_STEP =(fin_step==1)?QUERY_FALSH_STEP: REGISTER_FLASH_STEP;	
															}
														
															break;
									
											case 1:	printf("注册参数错误\n"); TCP_STEP=Suspend_STEP;break;
											case 2: if(fin_step== NEXT_STEP_REG)
														{	
															printf("设备已经注册过了\r\n");//说明已经注册过，并且已经将注册的username写入flash中
															
															printf("regist_buf:\r%s\n",(char*)regist_buf);//已经注册过的，但没有查询的设备，意外断电后，如果刚上电，就读取flash中的username，进行查询。
															
															tcp_client_sendbuf=add_json(1,regist_buf); 
															
															fin_step = NEXT_STEP_QUE; 
													
															TCP_STEP=RECV_STEP; //由于服务器发完一个指令就会断开，所以到接收步骤，等待断开，然后重新连接
															break;	//挂起任务
															
														}
													if(fin_step == NEXT_STEP_QUE)
													{
														printf("未注册\r\n");//没有被注册，要先注册
													
														TCP_STEP=RECV_STEP; //由于服务器发完一个指令就会断开，所以到接收步骤，等待断开，然后重新连接
														tcp_client_sendbuf=add_json(NULL,NULL); 
														fin_step = NEXT_STEP_REG ;
														break;
														
													}
											case 3:	printf("正在审核\r\n");TCP_STEP=RECV_STEP;fin_step=NEXT_STEP_QUE;break;
											case 4: printf("设备已经激活\r\n");TCP_STEP=Suspend_STEP;break;
											
											default:break;
											}
											break;
										}
									
									
						
						
						
						
	case REGISTER_FLASH_STEP:			printf("now REGISTER_FLASH_STEP\n");	//紧前是注册命令下，回的code=0，才会到这一步
																OS_ENTER_CRITICAL(); //关中断
																W25QXX_Write((u8 *)get_passidbuf,100,(u16)40);
																OS_EXIT_CRITICAL();  //开中断
															
																printf("flash1 ok\n");
						
																W25QXX_Read((u8*)regist_buf,REGISTER_FLASH_ADDR,(u16)40);
														
																printf("page_buf1:\r%s\n",(char*)regist_buf);
																tcp_client_sendbuf=add_json(1,regist_buf);
																//FLASH_ADDR = 0X080E0104;
																TCP_STEP =	RECV_STEP;
																fin_step = NEXT_STEP_QUE;
																
																break;
																
									
	case QUERY_FALSH_STEP:			printf("now QUERY_FALSH_STEP\n");
															OS_ENTER_CRITICAL(); //关中断
															W25QXX_Write((u8 *)get_passidbuf,QUERY_FALSH_ADDR,(u16)40);
															
															OS_EXIT_CRITICAL();  //开中断
															printf("flash2 ok\n");
															W25QXX_Read((u8*)passcode_buf,200,(u16)40);

															printf("page_buf2:\r%s\n",(char*)passcode_buf);
															fin_step = 0;
															TCP_STEP=COMPETE_STEP;
															break;
			
										
	case SEND_STEP:						printf("now SEND_STEP\n");
														err = netconn_write(tcp_clientconn ,
														tcp_client_sendbuf,
														strlen((char*)tcp_client_sendbuf),
														NETCONN_COPY); //发送tcp_server_sentbuf中的数据
														if(err != ERR_OK)
														{
															printf("发送失败\r\n");
														}
														TCP_STEP=RECV_STEP;
														break;
	case Suspend_STEP:	
											printf("挂起TCP任务\r\n");
											netconn_close(tcp_clientconn);
											netconn_delete(tcp_clientconn);			
											OSTaskSuspend(OS_PRIO_SELF); 
											break;
	
	case COMPETE_STEP:	OSSemPost(regist_msg_ptr);
											printf("完成注册\r\n");
											netconn_close(tcp_clientconn);
											netconn_delete(tcp_clientconn);			
											OSTaskSuspend(OS_PRIO_SELF); 
											break;
	default:break;
	}
}
}else
{
	printf("the flash is not empty\n");
	
	
	printf("username is %s\r\n",regist_buf);
	
	printf("password is %s\r\n",regist_buf);
	OSTimeDlyHMSM(0,0,2,500);
	OSSemPost(regist_msg_ptr);
	OSTaskSuspend(OS_PRIO_SELF);

}	
	
	
}
//创建TCP客户端线程
//返回值:0 TCP客户端创建成功
//		其他 TCP客户端创建失败
INT8U tcp_client_init(void)
{
	INT8U res;
	OS_CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();	//关中断
	
	res = OSTaskCreate(tcp_client_thread,(void*)0,(OS_STK*)&TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE-1],TCPCLIENT_PRIO); //创建TCP客户端线程
	
	OS_EXIT_CRITICAL();		//开中断
	
	return res;
}

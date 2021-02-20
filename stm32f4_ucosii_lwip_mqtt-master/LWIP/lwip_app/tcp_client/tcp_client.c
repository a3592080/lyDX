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
 extern OS_EVENT *regist_msg_ptr;//�ź����¼����ƿ�
 
u8_t TCP_STEP=RECV_STEP;

char  regist_buf[64]={0};//�˺�buf
char  passcode_buf[64]={0};//����buf
	#define REGIST_LENTH sizeof(regist_buf)	 		  	//���鳤��	
	#define SIZE_REGIST REGIST_LENTH/4+((REGIST_LENTH%4)?1:0)
	#define PASSCODE_LENTH sizeof(passcode_buf)	 		  	//���鳤��	
	#define SIZE_PASSCODE PASSCODE_LENTH/4+((PASSCODE_LENTH%4)?1:0)

struct netconn *tcp_clientconn;					//TCP CLIENT�������ӽṹ��
char tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//TCP�ͻ��˽������ݻ�����
//char tcp_client_sendbuf[100]={0};	//TCP�ͻ��˷������ݻ�����
u8 tcp_client_flag;		//TCP�ͻ������ݷ��ͱ�־λ
//char *get_passidbuf;
//TCP�ͻ�������

#define TCPCLIENT_PRIO		12
//�����ջ��С
#define TCPCLIENT_STK_SIZE	300
//�����ջ
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
	sn0=*(vu32*)(0x1FFF7A10);//��ȡSTM32��ΨһID��ǰ24λ��ΪMAC��ַ�����ֽ�
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



//tcp�ͻ���������
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
	
	
	if(check_flash((u8 *)regist_buf)==0||check_flash((u8 *)passcode_buf)==0)//���flash����Ϊ�գ�˵�������豸����TCPע��
	{	
	
	IP4_ADDR(&server_ipaddr,47,105,94,219);//lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]);
	tcp_client_sendbuf =add_json(NULL,NULL);
	fin_step 	= 	NEXT_STEP_REG;
	counter 	=		REMAIN_CONNTIME;
	while (1)
	{
	
		switch(TCP_STEP)
		{
		case CONNECT_STEP:	tcp_clientconn = netconn_new(NETCONN_TCP);  //����һ��TCP����
												err = netconn_connect(tcp_clientconn,&server_ipaddr,server_port);//���ӷ�����
												if(err != ERR_OK) 
												{
													netconn_delete(tcp_clientconn); //����ֵ������ERR_OK,ɾ��tcp_clientconn����
													TCP_STEP = (counter==0)?Suspend_STEP:CONNECT_STEP;//counter ��ʼֵΪ3,����3�Σ����ɹ���
													counter --;
													if(counter ==0)break;
												}							
												else if (err == ERR_OK)    //���������ӵ�����
												{								
													tcp_clientconn->recv_timeout = 10;
													counter =3;
													netconn_getaddr(tcp_clientconn,&loca_ipaddr,&loca_port,1); //��ȡ����IP����IP��ַ�Ͷ˿ں�
													printf("�����Ϸ�����%d.%d.%d.%d,�����˿ں�Ϊ:%d\r\n",47,105,94,219,loca_port);
													TCP_STEP = RECV_STEP;
						
													tcp_client_sendbuf =(fin_step==0)?add_json(NULL,NULL):add_json(1,regist_buf);								
													break;
												}
		case RECV_STEP: if((recv_err = netconn_recv(tcp_clientconn,&recvbuf)) == ERR_OK)  //���յ�����
						{	
							printf("now RECV_STEP\n");
							OS_ENTER_CRITICAL(); //���ж�
							memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
							for(q=recvbuf->p;q!=NULL;q=q->next)  //����������pbuf����
							{
							//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
							//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
								if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
								else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
								data_len += q->len;  	
								if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
							}
							OS_EXIT_CRITICAL();  //���ж�
							TCP_STEP = PARSE_STEP;
							printf("%s\r\n",tcp_client_recvbuf);
							data_len=0;  //������ɺ�data_lenҪ���㡣					
							netbuf_delete(recvbuf);
							break;
						
						}	
						else if(recv_err == ERR_CLSD)  //�ر�����
						{
							netconn_close(tcp_clientconn);
							netconn_delete(tcp_clientconn);
							TCP_STEP = CONNECT_STEP;
							counter 	=		REMAIN_CONNTIME;
							printf("������%d.%d.%d.%d�Ͽ�����\r\n",47,105,94,219);
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
										get_r = cJSON_Parse(tcp_client_recvbuf);//�����յ���ACK
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
									
											case 1:	printf("ע���������\n"); TCP_STEP=Suspend_STEP;break;
											case 2: if(fin_step== NEXT_STEP_REG)
														{	
															printf("�豸�Ѿ�ע�����\r\n");//˵���Ѿ�ע����������Ѿ���ע���usernameд��flash��
															
															printf("regist_buf:\r%s\n",(char*)regist_buf);//�Ѿ�ע����ģ���û�в�ѯ���豸������ϵ��������ϵ磬�Ͷ�ȡflash�е�username�����в�ѯ��
															
															tcp_client_sendbuf=add_json(1,regist_buf); 
															
															fin_step = NEXT_STEP_QUE; 
													
															TCP_STEP=RECV_STEP; //���ڷ���������һ��ָ��ͻ�Ͽ������Ե����ղ��裬�ȴ��Ͽ���Ȼ����������
															break;	//��������
															
														}
													if(fin_step == NEXT_STEP_QUE)
													{
														printf("δע��\r\n");//û�б�ע�ᣬҪ��ע��
													
														TCP_STEP=RECV_STEP; //���ڷ���������һ��ָ��ͻ�Ͽ������Ե����ղ��裬�ȴ��Ͽ���Ȼ����������
														tcp_client_sendbuf=add_json(NULL,NULL); 
														fin_step = NEXT_STEP_REG ;
														break;
														
													}
											case 3:	printf("�������\r\n");TCP_STEP=RECV_STEP;fin_step=NEXT_STEP_QUE;break;
											case 4: printf("�豸�Ѿ�����\r\n");TCP_STEP=Suspend_STEP;break;
											
											default:break;
											}
											break;
										}
									
									
						
						
						
						
	case REGISTER_FLASH_STEP:			printf("now REGISTER_FLASH_STEP\n");	//��ǰ��ע�������£��ص�code=0���Żᵽ��һ��
																OS_ENTER_CRITICAL(); //���ж�
																W25QXX_Write((u8 *)get_passidbuf,100,(u16)40);
																OS_EXIT_CRITICAL();  //���ж�
															
																printf("flash1 ok\n");
						
																W25QXX_Read((u8*)regist_buf,REGISTER_FLASH_ADDR,(u16)40);
														
																printf("page_buf1:\r%s\n",(char*)regist_buf);
																tcp_client_sendbuf=add_json(1,regist_buf);
																//FLASH_ADDR = 0X080E0104;
																TCP_STEP =	RECV_STEP;
																fin_step = NEXT_STEP_QUE;
																
																break;
																
									
	case QUERY_FALSH_STEP:			printf("now QUERY_FALSH_STEP\n");
															OS_ENTER_CRITICAL(); //���ж�
															W25QXX_Write((u8 *)get_passidbuf,QUERY_FALSH_ADDR,(u16)40);
															
															OS_EXIT_CRITICAL();  //���ж�
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
														NETCONN_COPY); //����tcp_server_sentbuf�е�����
														if(err != ERR_OK)
														{
															printf("����ʧ��\r\n");
														}
														TCP_STEP=RECV_STEP;
														break;
	case Suspend_STEP:	
											printf("����TCP����\r\n");
											netconn_close(tcp_clientconn);
											netconn_delete(tcp_clientconn);			
											OSTaskSuspend(OS_PRIO_SELF); 
											break;
	
	case COMPETE_STEP:	OSSemPost(regist_msg_ptr);
											printf("���ע��\r\n");
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
//����TCP�ͻ����߳�
//����ֵ:0 TCP�ͻ��˴����ɹ�
//		���� TCP�ͻ��˴���ʧ��
INT8U tcp_client_init(void)
{
	INT8U res;
	OS_CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();	//���ж�
	
	res = OSTaskCreate(tcp_client_thread,(void*)0,(OS_STK*)&TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE-1],TCPCLIENT_PRIO); //����TCP�ͻ����߳�
	
	OS_EXIT_CRITICAL();		//���ж�
	
	return res;
}

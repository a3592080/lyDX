#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "lwip_comm.h"
#include "stmflash.h"
#include "malloc.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include "w25qxx.h"
#include "mqtt_app.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//NETCONN API��̷�ʽ��WebServer���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   


#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define BURSIZE 2048
	


//����LED��CGI handler
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

char* DeviceName_buf;
char* DevicePower_buf;
char Device_buf[1024];
#define  buf_len sizeof(Device_buf)
#define bufsize buf_len/4+((buf_len%4)?1:0)
//char rdbuf[512];
//#define rdbuf_len sizeof(rdbuf)
//#define rdbufsize rdbuf_len/4+((rdbuf_len%4)?1:0)
static const tCGI ppcURLs[]= //cgi����
{
	{"/init.cgi",LEDS_CGI_Handler}
//......
};
int hex2dec(char c)
{
    if ('0' <= c && c <= '9') 
    {
        return c - '0';
    } 
    else if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    } 
    else if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    } 
    else 
    {
        return -1;
    }
}

char dec2hex(short int c)
{
    if (0 <= c && c <= 9) 
    {
        return c + '0';
    } 
    else if (10 <= c && c <= 15) 
    {
        return c + 'A' - 10;
    } 
    else 
    {
        return -1;
    }
}
void urldecode(char url[])
{
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BURSIZE];
    for (i = 0; i < len; ++i) 
    {
        char c = url[i];
        if (c != '%') 
        {
            res[res_len++] = c;
        }
        else 
        {
            char c1 = url[++i];
            char c0 = url[++i];
            int num = 0;
            num = hex2dec(c1) * 16 + hex2dec(c0);
            res[res_len++] = num;
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}
//��web�ͻ��������������ʱ��,ʹ�ô˺�����CGI handler����
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop); //����iLOOP
		}
	}
	return (-1);
}



//CGI LED���ƾ��
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  int i=0;  //ע������Լ���GET�Ĳ����Ķ�����ѡ��iֵ��Χ
	int intd=0;
	cJSON *http_root;

	cJSON *h_array,*v_array;
	cJSON *item_name,*item_power,*list;
	char * intdchar;
//	DeviceName_buf=mymalloc(SRAMIN,100);
	DevicePower_buf=mymalloc(SRAMIN,200);
	
	
	
	
	iIndex = FindCGIParameter("device",pcParam,iNumParams);  //�ҵ�led��������
	//ֻ��һ��CGI��� iIndex=0
	h_array=cJSON_CreateArray();
	if (iIndex != -1)
	{
			
		for (i=0; i<iNumParams; i++) //���CGI����: example GET /leds.cgi?led=2&led=4 */
		{
		  if (strcmp(pcParam[i] , "device")==0)  //������"�豸����"
		  {
			  printf("get the device\r\n");
			//  urldecode(pcValue[i]);
			  cJSON_AddItemToArray(h_array,cJSON_CreateString(pcValue[i]));
			//Device_buf[sizeof(Device_buf)*i]= pcValue[i];
		  }
		}
	 }

	 
	
	 iIndex = FindCGIParameter("power",pcParam,iNumParams);  //�ҵ�led��������
	v_array=cJSON_CreateArray();
	 if (iIndex != -1)
	{
			
		for (i=0; i<iNumParams; i++) //���CGI����: example GET /leds.cgi?led=2&led=4 */
		{
		  if (strcmp(pcParam[i] , "power")==0)  //������"led"
		  {
			
			 cJSON_AddItemToArray(v_array,cJSON_CreateString(pcValue[i]));
			//Device_buf[sizeof(Device_buf)*i]= pcValue[i];
		  }
		}
	 }
	intd=0;
	 
	http_root = cJSON_CreateArray();
	 for(intd=0;intd<14;intd++){
		sprintf(intdchar,"%d",intd);
		 
		item_name=cJSON_GetArrayItem(h_array,intd);
		 item_power=cJSON_GetArrayItem(v_array,intd);
		 cJSON_AddItemToArray(http_root, list = cJSON_CreateObject());
		 cJSON_AddStringToObject(list,"index",intdchar);
		 
		 cJSON_AddStringToObject(list,"name",item_name->valuestring);
		  cJSON_AddStringToObject(list,"power",item_power->valuestring);
		 
	 }
	 DevicePower_buf = cJSON_PrintUnformatted(http_root);
	
	 printf("the device power is %s\r\n",DevicePower_buf);
	 sprintf(Device_buf,"{\"messageType\":\"device_info\",\"deviceNo\":%d,\"chnls\":%s}",CLIENT_ID,DevicePower_buf);
	cJSON_Delete(h_array);
	cJSON_Delete(v_array);
	cJSON_Delete(http_root);
	 cJSON_free(DevicePower_buf);
	 rdbuf=mymalloc(SRAMIN,1024);
	 W25QXX_Write((u8 *)Device_buf,DEVICE_INIT_FLASH_ADDR,1024);
	
	//STMFLASH_Read(0X080B0100,(u32*)rdbuf,rdbufsize);
	 printf("%s\r\n",rdbuf);
	 free(Device_buf);
	return "/success.shtml";  	//LED1��,BEEP��
			
}


//CGI�����ʼ��
void httpd_cgi_init(void)
{ 
  //����CGI���LEDs control CGI) */
  http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}



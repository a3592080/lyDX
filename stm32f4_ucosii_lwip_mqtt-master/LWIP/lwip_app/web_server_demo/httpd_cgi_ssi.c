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
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//NETCONN API编程方式的WebServer测试代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   


#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define BURSIZE 2048
	


//控制LED的CGI handler
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

char* DeviceName_buf;
char* DevicePower_buf;
char Device_buf[1024];
#define  buf_len sizeof(Device_buf)
#define bufsize buf_len/4+((buf_len%4)?1:0)
//char rdbuf[512];
//#define rdbuf_len sizeof(rdbuf)
//#define rdbufsize rdbuf_len/4+((rdbuf_len%4)?1:0)
static const tCGI ppcURLs[]= //cgi程序
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
//当web客户端请求浏览器的时候,使用此函数被CGI handler调用
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop); //返回iLOOP
		}
	}
	return (-1);
}



//CGI LED控制句柄
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  int i=0;  //注意根据自己的GET的参数的多少来选择i值范围
	int intd=0;
	cJSON *http_root;

	cJSON *h_array,*v_array;
	cJSON *item_name,*item_power,*list;
	char * intdchar;
//	DeviceName_buf=mymalloc(SRAMIN,100);
	DevicePower_buf=mymalloc(SRAMIN,200);
	
	
	
	
	iIndex = FindCGIParameter("device",pcParam,iNumParams);  //找到led的索引号
	//只有一个CGI句柄 iIndex=0
	h_array=cJSON_CreateArray();
	if (iIndex != -1)
	{
			
		for (i=0; i<iNumParams; i++) //检查CGI参数: example GET /leds.cgi?led=2&led=4 */
		{
		  if (strcmp(pcParam[i] , "device")==0)  //检查参数"设备名称"
		  {
			  printf("get the device\r\n");
			//  urldecode(pcValue[i]);
			  cJSON_AddItemToArray(h_array,cJSON_CreateString(pcValue[i]));
			//Device_buf[sizeof(Device_buf)*i]= pcValue[i];
		  }
		}
	 }

	 
	
	 iIndex = FindCGIParameter("power",pcParam,iNumParams);  //找到led的索引号
	v_array=cJSON_CreateArray();
	 if (iIndex != -1)
	{
			
		for (i=0; i<iNumParams; i++) //检查CGI参数: example GET /leds.cgi?led=2&led=4 */
		{
		  if (strcmp(pcParam[i] , "power")==0)  //检查参数"led"
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
	return "/success.shtml";  	//LED1开,BEEP关
			
}


//CGI句柄初始化
void httpd_cgi_init(void)
{ 
  //配置CGI句柄LEDs control CGI) */
  http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}



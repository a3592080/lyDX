#ifndef _KEY_H
#define _KEY_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//按键输入驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	



#define DOOR PFin(7)
#define WARN1 PFin(6)
#define WARN2 PBin(15)
#define DOOR_OPEN	1
#define WARN1_ACTIVE 2//低电平有效
#define WARN2_ACTIVE	3

#define RESTAR PEout(2)

void KEY_Init(void);  //IO初始化
u8 KEY_Scan(u8);    //按键扫描函数
#endif 

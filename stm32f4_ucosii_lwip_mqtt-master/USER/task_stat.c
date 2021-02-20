#include "task_stat.h"
 void task_stat_info(void *pdata)
{
	
	OS_TCB *ptcb;	   
	OS_STK_DATA stkDat;	  
pdata =pdata;	
	ptcb = &OSTCBTbl[0];//Table TCB

	printf("************************************ App Task Debug Info ***********************************\r\n");
	printf("  Prio     Used    Free    Per      TaskName\r\n");
	while (ptcb != NULL)
	{
		OSTaskStkChk(ptcb->OSTCBPrio, &stkDat);//Check task stack
		printf("   %2d    %5d    %5d    %02d%%     %s\r\n", ptcb->OSTCBPrio, stkDat.OSUsed, stkDat.OSFree, (stkDat.OSUsed * 100)/(stkDat.OSUsed + stkDat.OSFree), ptcb->OSTCBTaskName);		
		ptcb = ptcb->OSTCBPrev;//Previous TCB list
	}
	printf("\r\n");
}
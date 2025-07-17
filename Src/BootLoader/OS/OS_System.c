/****************************************************
  * @Name	OS_System.c
  * @Brief	OS kernal
  ***************************************************/

/*-------------Header Files Include----------------*/
#include "OS_System.h"


/*-------------Internal Functions Declaration-------*/


/*-------------Module Variables Declaration---------*/
volatile OS_TaskTypeDef OS_Task[OS_TASK_SUM];


/*-----Module Call-Back function pointer Declaration----*/
CPUInterrupt_CallBack_t CPUInterrupptCtrlCBS;


/*-------------Module Functions Definition----------*/
/********************************************************************************************************
	@Name		: OS_CPUInterruptCBSRegister
	@Function	: register interrupt control function
		@pCPUInterruptCtrlCBS: CPU interrupt control call-back function's address
********************************************************************************************************/
void OS_CPUInterruptCBSRegister(CPUInterrupt_CallBack_t pCPUInterruptCtrlCBS)
{
	if(CPUInterrupptCtrlCBS == 0)
	{
		CPUInterrupptCtrlCBS = pCPUInterruptCtrlCBS;
	}
}

/********************************************************************************************************
	@Name		: OS_TaskInit                                                         
	@Function	: System task initial				                                     
********************************************************************************************************/
void OS_TaskInit(void)
{
	unsigned char i;
	for(i=0; i<OS_TASK_SUM; i++)
	{
		OS_Task[i].task = 0;
		OS_Task[i].RunFlag = OS_SLEEP;
		OS_Task[i].RunPeriod = 0;
		OS_Task[i].RunTimer = 0;
	}	
}


/*******************************************************************************
	@Name		: OS_CreatTask
	@Function	: Creat task
	@Para		  ID: task ID
				  proc: polling function
				  Period: running time
				  OS_TaskStatusTypeDef
*******************************************************************************/
void OS_CreatTask(unsigned char ID, void (*proc)(void), unsigned short Period, OS_TaskStatusTypeDef flag)
{	
	if(!OS_Task[ID].task)
	{
		OS_Task[ID].task = proc;
		OS_Task[ID].RunFlag = OS_SLEEP;
		OS_Task[ID].RunPeriod = Period;
		OS_Task[ID].RunTimer = 0;
	}
}


/********************************************************************************************************
	@Name		: OS_ClockInterruptHandle						                                                           
	@Function	: System task handler for each Systick Interrupt			                                     
********************************************************************************************************/
void OS_ClockInterruptHandle(void)
{
	unsigned char i;
	for(i=0; i<OS_TASK_SUM; i++)	
	{
		if(OS_Task[i].task)	
		{					
			OS_Task[i].RunTimer++;
			if(OS_Task[i].RunTimer >= OS_Task[i].RunPeriod)	
			{
				OS_Task[i].RunTimer = 0;
				OS_Task[i].RunFlag = OS_RUN;
			}
			
		}
	}
	
}

/*******************************************************************************
	@Name		: OS_Start
	@Function	: Start task
*******************************************************************************/
void OS_Start(void)
{
	unsigned char i;
	
	while(1)
	{
		for(i=0; i<OS_TASK_SUM; i++)
		{
			if(OS_Task[i].RunFlag == OS_RUN)
			{
				OS_Task[i].RunFlag = OS_SLEEP;
		 
				(*(OS_Task[i].task))();	
			}
		}	
	}
}

/*******************************************************************************
	@Name		: OS_TaskGetUp
	@Function	: wake up a task
		@taskID		: ID of task need to be wake up
*******************************************************************************/
void OS_TaskGetUp(OS_TaskIDTypeDef taskID)
{	
	unsigned char IptStatus;
	if(CPUInterrupptCtrlCBS != 0)
	{
		CPUInterrupptCtrlCBS(CPU_ENTER_CRITICAL,&IptStatus);
	}
	OS_Task[taskID].RunFlag = OS_RUN;	
	if(CPUInterrupptCtrlCBS != 0)
	{
		CPUInterrupptCtrlCBS(CPU_EXIT_CRITICAL,&IptStatus);
	}
}

/*******************************************************************************
	@Name		: void OS_TaskSleep(OS_TaskIDTypeDef taskID)
	@Function	: 
*******************************************************************************/
void OS_TaskSleep(OS_TaskIDTypeDef taskID)
{
	unsigned char IptStatus;
	if(CPUInterrupptCtrlCBS != 0)
	{
		CPUInterrupptCtrlCBS(CPU_ENTER_CRITICAL,&IptStatus);
	}
	OS_Task[taskID].RunFlag = OS_SLEEP;
	if(CPUInterrupptCtrlCBS != 0)
	{
		CPUInterrupptCtrlCBS(CPU_EXIT_CRITICAL,&IptStatus);
	}
}


/* Queue Buffer functions: */
/********************************************************************************************************
	@Name		: S_QueueEmpty	                                                           
	@Function	: Clear queue							                                     
	@Para		: Head
				  Tail
				  HBuff
********************************************************************************************************/
void S_QueueEmpty(unsigned char **Head, unsigned char **Tail, unsigned char *HBuff)
{
		*Head = HBuff;
		*Tail = HBuff;
}

/********************************************************************************************************
	@Name		: S_QueueDataIn	                                                           
	@Function	: Queue in data							                                     
	@Para		: Head
				  Tail
				  HBuff
				  Len
				  HData
				  DataLen
********************************************************************************************************/
void S_QueueDataIn(unsigned char **Head, unsigned char **Tail, unsigned char *HBuff, unsigned short Len, unsigned char *HData, unsigned short DataLen)
{	
	unsigned short num;
	unsigned char IptStatus;
	
	if(CPUInterrupptCtrlCBS != 0)
	{
		CPUInterrupptCtrlCBS(CPU_ENTER_CRITICAL,&IptStatus);
	}
	for(num = 0; num < DataLen; num++, HData++)
	{
			**Tail = *HData;
			(*Tail)++;
			if(*Tail == HBuff+Len)
				*Tail = HBuff;
			if(*Tail == *Head)
			{
					if(++(*Head) == HBuff+Len)
						*Head = HBuff;		
			}
	}	
	if(CPUInterrupptCtrlCBS != 0)
	{
		CPUInterrupptCtrlCBS(CPU_EXIT_CRITICAL,&IptStatus);
	}
}

/********************************************************************************************************
	@Name		: S_QueueDataOut	                                                           
	@Function	: Queue out data							                                     
	@Para		: Head
				  Tail
				  HBuff
				  Len
				  Data
********************************************************************************************************/
unsigned char S_QueueDataOut(unsigned char **Head, unsigned char **Tail, unsigned char *HBuff, unsigned short Len, unsigned char *Data)
{					   
	unsigned char back = 0;
	unsigned char IptStatus;
	if(CPUInterrupptCtrlCBS != 0)
	{
		CPUInterrupptCtrlCBS(CPU_ENTER_CRITICAL,&IptStatus);
	}
	*Data = 0;
	if(*Tail != *Head)
	{
			*Data = **Head;
			back = 1; 				
			if(++(*Head) == HBuff+Len)
				*Head = HBuff;
	}
	if(CPUInterrupptCtrlCBS != 0)
	{
		CPUInterrupptCtrlCBS(CPU_EXIT_CRITICAL,&IptStatus);
	}
	return back;	
}

/********************************************************************************************************
	@Name		: S_QueueDataLen	                                                           
	@Function	: Get data length of queue							                                     
	@Para		: Head
				  Tail
				  Len
	@Retval		: Datalength
********************************************************************************************************/
unsigned short S_QueueDataLen(unsigned char **Head, unsigned char **Tail, unsigned short Len)
{
		if(*Tail > *Head)
			return *Tail-*Head;
		if(*Tail < *Head)
			return *Tail+Len-*Head;
		return 0;
}


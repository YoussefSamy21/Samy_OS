/*
 * Scheduler.c
 *
 *  Created on: Feb 23, 2024
 *      Author: Youssef Samy
 */


#include "Scheduler.h"
#include "Samy_OS_FIFO.h"


uint8 Idle_Task_LED, SysTick_LED; // For Testing

FIFO_BUFF Ready_FIFO; // Queue for the Ready Tasks
Tasks_ref *Ready_QUEUE[MAX_TASKS_NUM];
static Tasks_ref Idle_Task;


struct
{
	/********** Scheduling Table **********/
	Tasks_ref *OS_Tasks[MAX_TASKS_NUM]; // Array of Pointers to Struct , each struct which is the task :)

	uint8  Tasks_Num;

	uint32 _S_MSP_OS;
	uint32 _E_MSP_OS;
	uint32 _PSP_Task_Locator; // Holding Address of the Next Task which will be Allocated in its Stack

	Tasks_ref *Current_Task;
	Tasks_ref *Next_Task;

	enum
	{
		OS_Suspended,
		OS_Running,
		OS_ERROR
	}OS_Mode_ID;

}OS_Control;


typedef enum
{
	SVC_Activate_Task,
	SVC_Terminate_Task,
	SVC_Waiting_Task,
	SVC_Acquire_Mutex,
	SVC_Release_Mutex
}SVC_ID;

/****************************************************************************************************************************/

// STATIC APIs
static void Samy_OS_Create_MainSTACK(void);
static void Samy_OS_Idle_Task(void); // this task has the lowest priority(255), Runs only in case of no other Running Tasks
static void Samy_OS_Task_Stack_Init(Tasks_ref *new_task);
static void Samy_OS_SVC_Call(SVC_ID service_id);

static void Samy_OS_Update_Scheduler_Table_and_Ready_QUEUE(void);
static void Samy_OS_Bubble_Sort_Scheduler_Table(void);

static void Samy_OS_Decide_Next_Task(void);
static void Samy_OS_Update_Tasks_Waiting_Time(void);

/****************************************************************************************************************************/


//==========================> OS Initialization <==========================

Samy_OS_Status Samy_OS_Init(void)
{
	Samy_OS_Status L_OS_status = No_Error;

	OS_Control.OS_Mode_ID = OS_Suspended;

	// Creating the Main Stack Boundaries for OS
	Samy_OS_Create_MainSTACK();

	// Create Ready Queue for OS
	if(QUEUE_init(&Ready_FIFO, Ready_QUEUE, MAX_TASKS_NUM) != QUEUE_NO_ERROR)
	{
		L_OS_status = Ready_QUEUE_Init_ERROR;
	}

	// Configure the IDLE Task (in Case no running Tasks)
	String_Copy((uint8*)&(Idle_Task.Task_Name), (uint8*)"IdleTask"); // Task Name
	Idle_Task.Task_Priority = 255; // Lowest Priority uint8 = 255; , (Runs in case of no any Running Tasks)
	Idle_Task.pf_Task_Entry = Samy_OS_Idle_Task;
	Idle_Task.Task_Stack_Size = 300;

	L_OS_status = Samy_OS_Create_Task(&Idle_Task);

	return L_OS_status;
}

Samy_OS_Status Samy_OS_Create_Task(Tasks_ref *new_task)
{
	Samy_OS_Status L_OS_status = No_Error;

	// creating the Task Stack Boundaries (_S_PSP_Task & _E_PSP_Task)
	new_task->_S_PSP_Task = OS_Control._PSP_Task_Locator;
	new_task->_E_PSP_Task = (new_task->_S_PSP_Task) - (new_task->Task_Stack_Size); // Remember: ARM Cortex-M stack is descending stack

	// Check if Task_Stack_Size entered by the user exceed the PSP Stack or not
	if(new_task->_E_PSP_Task < (uint32)&_eheap)
	{
		L_OS_status = Task_Exceeded_Stack_Size;
	}

	// Initialize the Stack Region for the Task (Initializing all the CPU Registers in the Task Stack)
	Samy_OS_Task_Stack_Init(new_task);

	// Align 8Bytes between this task and the Next one
	OS_Control._PSP_Task_Locator = (new_task->_E_PSP_Task) - 8;

	// Adding the Task in the Scheduling Table (which is an array of Pointers to struct)
	OS_Control.OS_Tasks[OS_Control.Tasks_Num] = new_task;
	OS_Control.Tasks_Num++;

	// Initially, The created task is in the Suspended State
	new_task->Task_State = Suspended;

	return L_OS_status;
}



void Samy_OS_Activate_Task(Tasks_ref *p_Task)
{
	p_Task->Task_State = Waiting;
	Samy_OS_SVC_Call(SVC_Activate_Task);

}



void Samy_OS_Terminate_Task(Tasks_ref *p_Task)
{
	p_Task->Task_State = Suspended;
	Samy_OS_SVC_Call(SVC_Terminate_Task);
}

void Samy_OS_Start_OS(void)
{
	OS_Control.OS_Mode_ID = OS_Running;

	// Set the Default Current Task (idle task),This is in Case no tasks had been created (the idle task has the least Priority)
	OS_Control.Current_Task = &Idle_Task;
	Samy_OS_Activate_Task(&Idle_Task);

	// Start the Systick Timer
	Start_Ticker();

	// Set PSP
	OS_SET_PSP(OS_Control.Current_Task->Current_PSP);
	OS_SWITCH_SP_to_PSP();

	// Switch to Thread Mode and Unprivileged Access
	Switch_CPU_Access_Level_to_Unprivileged();
	OS_Control.Current_Task->pf_Task_Entry();
}


void Samy_OS_Task_Wait(uint32 ticks , Tasks_ref *p_Task)
{
	p_Task->Task_Waiting_Time.Task_Blocking_State = enabled;
	p_Task->Task_Waiting_Time.ticks = ticks;

	// Here , We implement the Task waiting state "as if" it is in the Suspended mode until the waiting ticks expires , then removing it from the Suspended state again
	p_Task->Task_State = Suspended;
	Samy_OS_SVC_Call(SVC_Terminate_Task);
}


Samy_OS_Status Samy_OS_Acquire_Mutex(Tasks_ref *p_Task , Mutex_ref *p_Mutex)
{
	Samy_OS_Status L_OS_status = No_Error;
	if(p_Mutex->Current_User == NULL) // Not Taken by any User
	{
		p_Mutex->Current_User = p_Task;

		// Check for Priority Ceiling
		if(p_Mutex->Mutex_Priority_Ceiling.State == Priority_Ceiling_Enabled)
		{
			// Save Current Task Priority , to return it to the task after releasing the Mutex
			p_Mutex->Mutex_Priority_Ceiling.Old_Priority = p_Task->Task_Priority; // or (p_Mutex->Current_User->Task_Priority)

			// Promote the Priority of the Task Required the Mutex ===> to the Ceiling Priority
			p_Mutex->Current_User->Task_Priority = p_Mutex->Mutex_Priority_Ceiling.Ceiling_Priority;
		}
	}
	else if(p_Mutex->Current_User == p_Task) // the task already Acquired the Mutex Before
	{
		L_OS_status = Mutex_Already_Acquired;
	}
	else if(p_Mutex->Next_User == NULL) // Mutex Acquired , but not Pending Request by other next tasks
	{
		p_Mutex->Next_User = p_Task;
		p_Mutex->Next_User->Task_State == Suspended; // Suspend Task Until the Mutex is released by the Other Task
		Samy_OS_SVC_Call(SVC_Terminate_Task);
	}
	else // Mutex Acquired , and Also Pending Request by other Task
	{
		L_OS_status = Mutex_Reached_Max_No_Of_Users;
	}

	return L_OS_status;
}

void Samy_OS_Release_Mutex(Mutex_ref *p_Mutex)
{
	if(p_Mutex->Current_User != NULL)
	{
		// Check for Priority Ceiling
		if(p_Mutex->Mutex_Priority_Ceiling.State == Priority_Ceiling_Enabled)
		{
			// Restore the Original Priority to the Promoted task when mutex was acquired
			p_Mutex->Current_User->Task_Priority = p_Mutex->Mutex_Priority_Ceiling.Old_Priority;
		}


		if(p_Mutex->Next_User == NULL) // No Any Pending Requests for the Mutex
		{
			p_Mutex->Current_User = NULL;
		}
		else // p_Mutex->Next_User != NULL , So the Pending Task now can Acquire the Mutex and be Activated
		{
			p_Mutex->Current_User = p_Mutex->Next_User;
			p_Mutex->Next_User = NULL;

			p_Mutex->Current_User->Task_State = Waiting;

			if(p_Mutex->Mutex_Priority_Ceiling.State == Priority_Ceiling_Enabled)
			{
				// Save Current Task Priority , to return it to the task after releasing the Mutex
				p_Mutex->Mutex_Priority_Ceiling.Old_Priority = p_Mutex->Current_User->Task_Priority;

				// Promote the Priority of the Task Required the Mutex ===> to the Ceiling Priority
				p_Mutex->Current_User->Task_Priority = p_Mutex->Mutex_Priority_Ceiling.Ceiling_Priority;
			}

			Samy_OS_SVC_Call(SVC_Activate_Task);
		}
	}
}


/****************************************************************************************************************************/


// ==========================>     STATIC APIs     <==========================

static void Samy_OS_Create_MainSTACK(void)
{
	OS_Control._S_MSP_OS = (uint32)&_estack;
	OS_Control._E_MSP_OS = OS_Control._S_MSP_OS - MAIN_Stack_SIZE;

	// Aligning 8 Bytes between the Main Stack & the Process Stack
	OS_Control._PSP_Task_Locator = OS_Control._E_MSP_OS -  8;
}


static void Samy_OS_Idle_Task(void)
{
	while(1)
	{
		Idle_Task_LED ^= 1;
		// Wait for event , to let Processor Enters Sleep Mode ==> to enhance CPU Utilization (Power Consumption)
		__asm("WFE");
	}
}

static void Samy_OS_Task_Stack_Init(Tasks_ref *new_task)
{
	/*
		1- we need initially when creating each task to initialize ALL the CPU Registers from the start_psp of the task
		   where it must be in order , for the Context Switching (Save/Restore) Operations ....

		2- the Cortex-M Processor "Automatically" saves (xPSR , PC , LR , R12 , R3 , R2 , R1 , R0) when Interrupt Occurs

		3- So we need "Manually" to save all Other CPU Registers which are (R4,R5,R6,R7,R8,R9,R10,R11)

		4- But Now, in this API , we are just "initializing" all the CPU Registers for Further Context Switching Operations
		   (we can initialize by any dummy value , since the task is not in the running state yet)

	  	======> TASK FRAME <======
	 * CPU Automatically Saved/Restored registers
	 * ------------------------------------------
	 * xPSR
	 * PC (Next instruction to be executed in the task)
	 * LR (return register which is saved in CPU before switching)
	 * R12
	 * R3
	 * R2
	 * R1
	 * R0
	 *
	 * Manually Saved/Restored registers
	 * ----------------------------------
	 * R5
	 * R5
	 * R6
	 * R7
	 * R8
	 * R9
	 * R10
	 * R11
	 */

	uint8 i;

	new_task->Current_PSP = (uint32*) new_task->_S_PSP_Task;

	// xPSR dummy value -> T = 1 to avoid bus faults
	new_task->Current_PSP--;
	*(new_task->Current_PSP) = 0x01000000;

	// PC dummy value -> Start at task entry point
	new_task->Current_PSP--;
	*(new_task->Current_PSP) = (uint32)new_task->pf_Task_Entry;

	// LR dummy value -> Return to thread mode with PSP
	new_task->Current_PSP--;
	*(new_task->Current_PSP) = 0xFFFFFFFD;

	// CPU General Purpose Registers -> initialized by ZERO
	for(i = 0 ; i<13 ; i++)
	{
		new_task->Current_PSP--;
		*(new_task->Current_PSP) = 0;
	}

}

static void Samy_OS_SVC_Call(SVC_ID service_id)
{
	switch(service_id)
	{
		case SVC_Activate_Task:
			__asm ("SVC #0x00"); // SVC_Handler (exceptional interrupt) in CortexMX_OS_Porting.c
			break;

		case SVC_Terminate_Task:
			__asm ("SVC #0x01");
			break;

		case SVC_Waiting_Task:
			__asm ("SVC #0x02");
			break;

		case SVC_Acquire_Mutex:
			__asm ("SVC #0x03");
			break;

		case SVC_Release_Mutex:
			__asm ("SVC #0x04");
			break;

	}
}

static void Samy_OS_Update_Scheduler_Table_and_Ready_QUEUE(void)
{
	Tasks_ref *temp = NULL;
	uint8 i = 0;
	Tasks_ref *p_task , *p_next_task;

	// Sorting the Scheduler table from the Highest Priority to the Lowest Priority
	Samy_OS_Bubble_Sort_Scheduler_Table();

	// Free Ready QUEUE
	while(Dequeue_item(&Ready_FIFO, &temp) != QUEUE_EMPTY);

	// Update Ready QUEUE
	while(i < OS_Control.Tasks_Num)
	{
		p_task = OS_Control.OS_Tasks[i];
		p_next_task = OS_Control.OS_Tasks[i+1];

		if( (p_task->Task_State) != Suspended )
		{
			// In case we reached to the end of available OSTasks
			if( (p_next_task->Task_State) == Suspended)
			{
				Enqueue_item(&Ready_FIFO, p_task);
				p_task->Task_State = Ready;
				break;
			}
			// Check for Priority
			if((p_task->Task_Priority) < (p_next_task->Task_Priority))
			{
				Enqueue_item(&Ready_FIFO, p_task);
				p_task->Task_State = Ready;
				break;
			}
			else if((p_task->Task_Priority) == (p_next_task->Task_Priority))
			{
				Enqueue_item(&Ready_FIFO, p_task);
				p_task->Task_State = Ready;
			}
			else if((p_task->Task_Priority) > (p_next_task->Task_Priority))
			{
				// NOT Allowed , because we already Sorted the Tasks
				break;
			}

		}

		i++;
	}

}

static void Samy_OS_Bubble_Sort_Scheduler_Table(void)
{
	uint8 num = OS_Control.Tasks_Num;
	uint8 i , j;
	Tasks_ref *temp = NULL;
	uint8 sorted_flag;
	for(i=0 ; i < (num-1) ; i++)
	{
		sorted_flag = 1;
		for(j=0 ; j < (num-i-1) ; j++)
		{
			if((OS_Control.OS_Tasks[j+1]->Task_Priority) < (OS_Control.OS_Tasks[j]->Task_Priority))
			{
				// Swap
				temp = OS_Control.OS_Tasks[j];
				OS_Control.OS_Tasks[j] = OS_Control.OS_Tasks[j+1];
				OS_Control.OS_Tasks[j+1] = temp;
				sorted_flag = 0;
			}
		}
		if(sorted_flag == 1)
		{
			break;
		}
	}

}


static void Samy_OS_Decide_Next_Task(void)
{
	// If Ready Queue is empty && OS_Control->CurrentTask != Suspended
	if( (Ready_FIFO.count == 0) && (OS_Control.Current_Task->Task_State != Suspended) )
	{
		OS_Control.Current_Task->Task_State = Running;

		// Add the Current Task Again (Round-Robin)
		Enqueue_item(&Ready_FIFO, OS_Control.Current_Task);
		OS_Control.Next_Task = OS_Control.Current_Task;
	}
	else
	{
		Dequeue_item(&Ready_FIFO, &OS_Control.Next_Task);
		OS_Control.Next_Task->Task_State = Running;

		// Update Ready Queue(To keep the Round-Robin Algorithm)
		if(((OS_Control.Current_Task->Task_Priority) == (OS_Control.Next_Task->Task_Priority)) && (OS_Control.Current_Task->Task_State != Suspended) )
		{
			Enqueue_item(&Ready_FIFO, OS_Control.Current_Task);
			OS_Control.Current_Task->Task_State = Ready;
		}
	}

}

static void Samy_OS_Update_Tasks_Waiting_Time(void)
{
	// Note: This API is called by the Systick Handler every Sys-Tick
	uint8 i;
	for(i = 0 ; i < OS_Control.Tasks_Num ; i++)
	{
		if(OS_Control.OS_Tasks[i]->Task_State == Suspended)
		{
			if(OS_Control.OS_Tasks[i]->Task_Waiting_Time.Task_Blocking_State == enabled)
			{
				OS_Control.OS_Tasks[i]->Task_Waiting_Time.ticks--;

				if(OS_Control.OS_Tasks[i]->Task_Waiting_Time.ticks == 0)
				{
					OS_Control.OS_Tasks[i]->Task_Waiting_Time.Task_Blocking_State = disabled;
					OS_Control.OS_Tasks[i]->Task_State = Waiting;

					Samy_OS_SVC_Call(SVC_Waiting_Task);
				}
			}

		}
	}
}
/****************************************************************************************************************************/


/*
	This API (Samy_OS_SVC_Services) executes specific OS Services , Branched by the SVC_Handler Exceptional Interrupt.
	REMARK:
	------
	We didn't put this Code in the SVC_Handler
	===> to Abstract the Processor Architecture (Vector table) or the MCAL Layer (which is found in the CortexMX_OS_Porting.c)
		 with the OS Layer
*/
void Samy_OS_SVC_Services(uint32 *StackFrame_ptr)
{
	/*
		We will extract the SVC_Number in this API to be able to execute the required service ,
		This is done by starting from R0 which is the default argument , which is holding the stack frame pointer,
		So ===> R0 ===> Argument 0 ===> StackFrame_ptr
		Stack Frame:
		-----------
		* xPSR
		* PC <======== StackFrame_ptr[6]
		* LR
		* R12
		* R3
		* R2
		* R1
		* R0 <======== StackFrame_ptr

		So PC = Instruction which is immediately after the SVC Instruction
		So (StackFrame_ptr[6]- 2 ) = SVC #SVC_Number
		===> SVC instruction is 16-Bit , where SVC_Number is stored in the Least Byte)
	*/
	uint8 SVC_Number;
	SVC_Number = *((uint8*)((uint8*)(StackFrame_ptr[6])) - 2) ; // it can also be written as: SVC_Number =  ((uint8*)StackFrame_ptr[6]) [-2];
	switch(SVC_Number)
	{
		case SVC_Activate_Task:
		case SVC_Terminate_Task:
		case SVC_Waiting_Task:
		case SVC_Acquire_Mutex:
		case SVC_Release_Mutex:

		// Update Scheduler Table and Ready Queue
		Samy_OS_Update_Scheduler_Table_and_Ready_QUEUE();

		// If OS is in running state -> Decide what next task
		if(OS_Control.OS_Mode_ID == OS_Running)
		{
			if(String_Compare((uint8*)OS_Control.Current_Task->Task_Name, (uint8*)"IdleTask") != 0)
			{
				Samy_OS_Decide_Next_Task();

				// =====> CONTEXT SWITCHING <=====
				Trigger_OS_PendSV(); // Macro for Triggering PendSV Exceptional Interrupt
			}
		}
		break;
	}

}

__attribute ((naked)) void PendSV_Handler()
{
	/*
		================>	1- Context SAVE 	<================
		As mentioned above (in the Samy_OS_SVC_Services API) ...
		that the Processor saves by default some registers which are (xPSR,PC,LR,R12,R3,R2,R1,R0)
		So Now we will Save the Others registers in the Task Control Block (TCB).
		These Registers are (R4,R5,R6,R7,R8,R9,R10,R11)
	*/

	//Save the Current PSP Register for the Task in the OS_Control.Current_Task->Current_PSP
	OS_GET_PSP(OS_Control.Current_Task->Current_PSP);

	// Small Note: We decrement the Pointer first as the Cortex-M Stack is "FULL" Descending Stack (Points to the last pushed data not to the empty region)
	OS_Control.Current_Task->Current_PSP--;
	__asm volatile("MOV %0, R4" : "=r" (*(OS_Control.Current_Task->Current_PSP)));

	OS_Control.Current_Task->Current_PSP--;
	__asm volatile("MOV %0, R5" : "=r" (*(OS_Control.Current_Task->Current_PSP)));

	OS_Control.Current_Task->Current_PSP--;
	__asm volatile("MOV %0, R6" : "=r" (*(OS_Control.Current_Task->Current_PSP)));

	OS_Control.Current_Task->Current_PSP--;
	__asm volatile("MOV %0, R7" : "=r" (*(OS_Control.Current_Task->Current_PSP)));

	OS_Control.Current_Task->Current_PSP--;
	__asm volatile("MOV %0, R8" : "=r" (*(OS_Control.Current_Task->Current_PSP)));

	OS_Control.Current_Task->Current_PSP--;
	__asm volatile("MOV %0, R9" : "=r" (*(OS_Control.Current_Task->Current_PSP)));

	OS_Control.Current_Task->Current_PSP--;
	__asm volatile("MOV %0, R10" : "=r" (*(OS_Control.Current_Task->Current_PSP)));

	OS_Control.Current_Task->Current_PSP--;
	__asm volatile("MOV %0, R11" : "=r" (*(OS_Control.Current_Task->Current_PSP)));


	// ================>	2- Context RESTORE of Next Task (same idea) 	<================

	if(OS_Control.Next_Task != NULL)
	{
		OS_Control.Current_Task = OS_Control.Next_Task;
		OS_Control.Next_Task = NULL;
	}
	__asm volatile("MOV R11, %0" : : "r" (*(OS_Control.Current_Task->Current_PSP)));
	OS_Control.Current_Task->Current_PSP++;

	__asm volatile("MOV R10, %0" : : "r" (*(OS_Control.Current_Task->Current_PSP)));
	OS_Control.Current_Task->Current_PSP++;

	__asm volatile("MOV R9, %0" : : "r" (*(OS_Control.Current_Task->Current_PSP)));
	OS_Control.Current_Task->Current_PSP++;

	__asm volatile("MOV R8, %0" : : "r" (*(OS_Control.Current_Task->Current_PSP)));
	OS_Control.Current_Task->Current_PSP++;

	__asm volatile("MOV R7, %0" : : "r" (*(OS_Control.Current_Task->Current_PSP)));
	OS_Control.Current_Task->Current_PSP++;

	__asm volatile("MOV R6, %0" : : "r" (*(OS_Control.Current_Task->Current_PSP)));
	OS_Control.Current_Task->Current_PSP++;

	__asm volatile("MOV R5, %0" : : "r" (*(OS_Control.Current_Task->Current_PSP)));
	OS_Control.Current_Task->Current_PSP++;

	__asm volatile("MOV R4, %0" : : "r" (*(OS_Control.Current_Task->Current_PSP)));
	OS_Control.Current_Task->Current_PSP++;

	// Update PSP and EXIT
	OS_SET_PSP(OS_Control.Current_Task->Current_PSP);
	__asm volatile("BX LR");

}

void SysTick_Handler()
{
	SysTick_LED ^= 1;

	Samy_OS_Update_Tasks_Waiting_Time();

	// Determine Current and Next Tasks
	Samy_OS_Decide_Next_Task();

	//Context Switching
	Trigger_OS_PendSV();
}




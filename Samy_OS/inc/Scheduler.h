/*
 * Scheduler.h
 *
 *  Created on: Feb 23, 2024
 *      Author: Youssef Samy
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_


#include "CortexMX_OS_Porting.h"
#include "Samy_String_Library.h"

#define 	MAX_TASKS_NUM		100

typedef enum
{
	No_Error,
	Ready_QUEUE_Init_ERROR,
	Task_Exceeded_Stack_Size,
	Mutex_Reached_Max_No_Of_Users,
	Mutex_Already_Acquired
}Samy_OS_Status;


typedef struct
{
	uint8 Task_Name[30];
	uint8 Task_Priority;
	void (*pf_Task_Entry)(void); // Pointer to the Entry of the Task Function
	uint32 Task_Stack_Size;

	uint32 _S_PSP_Task; 	// Not entered by the user
	uint32 _E_PSP_Task; 	// Not entered by the user
	uint32 *Current_PSP; 	// Not entered by the user

	enum
	{	// Task States: Suspended , Waiting , Ready , Running
		Suspended,
		Running,
		Waiting,
		Ready,
	}Task_State;

	struct
	{
		enum
		{
			enabled,
			disabled
		}Task_Blocking_State;

		uint32 ticks;
	}Task_Waiting_Time;

}Tasks_ref;


typedef struct
{
	uint8 *ptr_PayLoad;
	uint32 PayLoad_Size;

	Tasks_ref *Current_User;
	Tasks_ref *Next_User;

	uint8 Mutex_Name[30];

	struct
	{
		enum
		{
			Priority_Ceiling_Enabled,
			Priority_Ceiling_Disabled
		}State;

		uint8 Ceiling_Priority;
		uint8 Old_Priority;

	}Mutex_Priority_Ceiling;

}Mutex_ref;

/****************************************************************************************************************************/

/************************** User Supported APIs **************************/

Samy_OS_Status Samy_OS_Init(void);

Samy_OS_Status Samy_OS_Create_Task(Tasks_ref *new_task);

void Samy_OS_Activate_Task(Tasks_ref *p_Task);

void Samy_OS_Terminate_Task(Tasks_ref *p_Task);

void Samy_OS_Start_OS(void);

void Samy_OS_Task_Wait(uint32 ticks , Tasks_ref *p_Task);

Samy_OS_Status Samy_OS_Acquire_Mutex(Tasks_ref *p_Task , Mutex_ref *p_Mutex);

void Samy_OS_Release_Mutex(Mutex_ref *p_Mutex);


#endif /* SCHEDULER_H_ */

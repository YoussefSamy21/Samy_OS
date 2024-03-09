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


/*==========================================================
* @Fn			- Samy_OS_Init
* @brief 		- Initializes the OS Main Stack Boundaries , Buffer and Configuring the Idle-Task.
* @return 		- Returns No_Error if no error occured, or specific Error if occured.
* Note			- Must be called firstly, before calling any APIs.
*/
Samy_OS_Status Samy_OS_Init(void);


/*==========================================================
* @Fn			- Samy_OS_Create_Task
* @brief 		- Create the Task's Stack Boundaries Region and Adds the Task in the Scheduling table.
* @param[in]    - new_task: Pointer to the Task's Configuration Struct.
* @return 		- Returns No_Error if no error occured, or specific Error if occured.
* Note			- The Created Task is initially in the Suspended State after calling this API.
*/
Samy_OS_Status Samy_OS_Create_Task(Tasks_ref *new_task);


/*==========================================================
* @Fn			- Samy_OS_Activate_Task
* @brief 		- Moves the Task to the Ready State.
* @param[in]    - p_Task: Pointer to the Task's Configuration Struct.
* @return 		- None
* Note			- The Activated Task would Run directly if it has the Highest priority among the other tasks. Otherwise, it is added to the Ready QUEUE to be Scheduled.
*/
void Samy_OS_Activate_Task(Tasks_ref *p_Task);


/*==========================================================
* @Fn			- Samy_OS_Terminate_Task
* @brief 		- Moves the Task to the Suspended State.
* @param[in]    - p_Task: Pointer to the Task's Configuration Struct.
* @return 		- None
* Note			- None
*/
void Samy_OS_Terminate_Task(Tasks_ref *p_Task);

/*==========================================================
* @Fn			- Samy_OS_Start_OS
* @brief 		- Starts the OS Systick timer, to begin scheduling the tasks.
* @param[in]    - None
* @return 		- None
* Note			- Set the idle task(least priority) as the default running task, in case of no other running tasks. 
*/
void Samy_OS_Start_OS(void);


/*==========================================================
* @Fn			- Samy_OS_Task_Wait
* @brief 		- Sends the Task to the Waiting state, according to the desired ticks time. 
* @param[in]    - ticks: amount of ticks for task waiting, until moving to the ready state. 
* @param[in]    - p_Task: Pointer to the Task's Configuration Struct. 
* @return 		- None
* Note			- None.
*/
void Samy_OS_Task_Wait(uint32 ticks , Tasks_ref *p_Task);


/*==========================================================
* @Fn			- Samy_OS_Acquire_Mutex
* @brief 		- Acquire Mutex if available
* @param[in]    - p_Task:  Pointer to the Task's  Configuration Struct.
* @param[in]    - p_Mutex: Pointer to the Mutex's Configuration Struct. 
* @return 		- Returns No_Error if no error occured, or specific Error if occured.
* Note			- Mutex Can be hold by a Maximum of 2 Tasks only (a task already acquires it, while the other is waiting until released).
*/
Samy_OS_Status Samy_OS_Acquire_Mutex(Tasks_ref *p_Task , Mutex_ref *p_Mutex);


/*==========================================================
* @Fn			- Samy_OS_Release_Mutex
* @brief 		- Release Mutex and acquires it to the other task waiting for that Mutex (if found).
* @param[in]    - p_Mutex: Pointer to the Mutex's Configuration Struct. 
* @return 		- Returns No_Error if no error occured, or specific Error if occured.
* Note			- Mutex is Released if and only if it was acquired by the Same task, not any other tasks. 
*/

void Samy_OS_Release_Mutex(Mutex_ref *p_Mutex);


#endif /* SCHEDULER_H_ */

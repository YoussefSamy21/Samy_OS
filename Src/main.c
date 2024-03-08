/**
 ******************************************************************************
 *
 * @file           : main.c
 * @author         : Youssef Samy
 *
 */


#include "Scheduler.h"


Tasks_ref Task1 , Task2 , Task3;

uint8 Task1_LED = 0 , Task2_LED = 0 ,  Task3_LED = 0;

void Task1_func(void)
{
	while(1)
	{
		Task1_LED ^= 1;
	}
}

void Task2_func(void)
{
	while(1)
	{
		Task2_LED ^= 1;
	}
}

void Task3_func(void)
{
	while(1)
	{
		Task3_LED ^= 1;
	}
}

int main(void)
{

	HW_Init();


	if(Samy_OS_Init() != No_Error)
	{
		while(1);
	}

	Task1.Task_Stack_Size = 1024;
	Task1.pf_Task_Entry = Task1_func;
	Task1.Task_Priority = 3;
	String_Copy(Task1.Task_Name, (uint8*)"TASK1");

	Task2.Task_Stack_Size = 1024;
	Task2.pf_Task_Entry = Task2_func;
	Task2.Task_Priority = 3;
	String_Copy(Task2.Task_Name, (uint8*)"TASK2");

	Task3.Task_Stack_Size = 1024;
	Task3.pf_Task_Entry = Task3_func;
	Task3.Task_Priority = 3;
	String_Copy(Task3.Task_Name, (uint8*)"TASK3");

	Samy_OS_Create_Task(&Task1);
	Samy_OS_Create_Task(&Task2);
	Samy_OS_Create_Task(&Task3);

	Samy_OS_Activate_Task(&Task1);
	Samy_OS_Activate_Task(&Task2);
	Samy_OS_Activate_Task(&Task3);

	Samy_OS_Start_OS();

	while(1)
	{

	}

}

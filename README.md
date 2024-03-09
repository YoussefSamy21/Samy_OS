# SamyOS
Samy OS is a Real-Time Operating System written from scratch, targeting
Embedded Systems Applications.It is Built on the ARM CMSIS V5 Library for easy porting and simple software re-use. 

```💡 Please Note that this Project is made for educational and learning purposes, trying to better my understanding of Real-Time Operating Systems.```

``` As such, it may contain imperfections or faults that I have yet to discover.So if you spot an issue or bug fixes, feel free to submit a pull request.```

``` 📌 Looking ahead, I am planning to continuously refine and enhance this project, adding new features and addressing any identified faults. ```


## OS Feautres:
1) **Pre-emptive Scheduler:** SamyOS is a Priority based scheduler, supporting **Round-Robin** scheduling algorithm for same priority tasks. 

2) **Reduced Power Consumption:** SamyOS Consumes **8%** of the Cortex-M3 CPU Utilization in the idle State, where no tasks are running. <br /> 
This is mainly achieved by the "WFE" (wait for event) ARM Instruction which causes CPU entry to Sleep Mode when no tasks are running (Idle State).

3) **Tasks Synchronization:** SamyOS supports Mutex, which allows a task to enter in the waiting state for an acquired Mutex, and directly be Activated once the Mutex is released.

4) **Priority Ceiling:** SamyOS implements a simple Priority Ceiling technique, which helps in avoiding Priority Inversion Problem.

5) **Flexible Tasks Activation/Termination:** SamyOS supports running the tasks periodically, it also allows the user to Activate/Terminate tasks in run-time.  

## Test Cases:
Several test cases were covered on ARM Keil uvision, with Systick = 1ms and STM32F103C8 (Cortex-M3 microcontroller). Here are some of them:

#### 1) Round-Robin Scheduling 3 Tasks having the Same Priority:

![Round-Robin](https://github.com/YoussefSamy21/Samy_OS/blob/main/Keil%20Simulation/Case_1_Round_Robin.gif)

#### 2) Running 3 Tasks **Periodically**: (task1 every 100ms), (task2 every 300ms), (task3 every 500ms).

![Run-Tasks-Periodically](https://github.com/YoussefSamy21/Samy_OS/blob/main/Keil%20Simulation/Case_2_Tasks_Waiting_Time.gif)

#### 3) CPU Utilization = 8% , in the Idle State. (No running tasks)
![CPU-Utilization](https://github.com/YoussefSamy21/Samy_OS/blob/main/Keil%20Simulation/Case_3_CPU_Utilization.gif)


## User Supported APIs:
- **`Samy_OS_Init`**: Creates the OS Main Stack Boundaries, Buffer, and Configure the Idle-Task.

- **`Samy_OS_Create_Task`**: Create the Task's Stack Boundaries Region and Adds the Task in the Scheduling table.

- **`Samy_OS_Start_OS`**: Starts the OS Systick timer, to begin scheduling the tasks, starts running the idle task.

- **`Samy_OS_Activate_Task`**: Moves the Task to the Ready State (added to Ready QUEUE).

- **`Samy_OS_Terminate_Task`**: Moves the Task to the Suspended State.

- **`Samy_OS_Task_Wait`**: Sends the Task to the Waiting state, according to the desired time.

- **`Samy_OS_Acquire_Mutex`**: Acquire Mutex if available.

- **`Samy_OS_Release_Mutex`**: Release Mutex and acquires it to the other task waiting for that Mutex (if found).

```c
/*==========================================================
* @Fn			- Samy_OS_Init
* @brief 		- Initializes the OS Main Stack Boundaries , Buffer and Configuring the Idle-Task.
* @return 		- Returns No_Error if no error occured, or specific Error if occured.
* Note			- Must be called firstly, before calling any APIs.
*/
Samy_OS_Status Samy_OS_Init(void);
```
```c
/*==========================================================
* @Fn			- Samy_OS_Create_Task
* @brief 		- Create the Task's Stack Boundaries Region and Adds the Task in the Scheduling table.
* @param[in]    - new_task: Pointer to the Task's Configuration Struct.
* @return 		- Returns No_Error if no error occured, or specific Error if occured.
* Note			- The Created Task is initially in the Suspended State after calling this API.
*/
Samy_OS_Status Samy_OS_Create_Task(Tasks_ref *new_task);
```
```c
/*==========================================================
* @Fn			- Samy_OS_Start_OS
* @brief 		- Starts the OS Systick timer, to begin scheduling the tasks.
* @param[in]    - None
* @return 		- None
* Note			- Set the idle task(least priority) as the default running task, in case of no other running tasks. 
*/
void Samy_OS_Start_OS(void);
```
```c
/*==========================================================
* @Fn			- Samy_OS_Activate_Task
* @brief 		- Moves the Task to the Ready State.
* @param[in]    - p_Task: Pointer to the Task's Configuration Struct.
* @return 		- None
* Note			- The Activated Task would Run directly if it has the Highest priority among the other tasks. Otherwise, it is added to the Ready QUEUE to be Scheduled.
*/
void Samy_OS_Activate_Task(Tasks_ref *p_Task);
```
```c
/*==========================================================
* @Fn			- Samy_OS_Terminate_Task
* @brief 		- Moves the Task to the Suspended State.
* @param[in]    - p_Task: Pointer to the Task's Configuration Struct.
* @return 		- None
* Note			- None
*/
void Samy_OS_Terminate_Task(Tasks_ref *p_Task);
```
```c
/*==========================================================
* @Fn			- Samy_OS_Task_Wait
* @brief 		- Sends the Task to the Waiting state, according to the desired ticks time. 
* @param[in]    - ticks: amount of ticks for task waiting, until moving to the ready state. 
* @param[in]    - p_Task: Pointer to the Task's Configuration Struct. 
* @return 		- None
* Note			- None.
*/
void Samy_OS_Task_Wait(uint32 ticks , Tasks_ref *p_Task);
```
```c
/*==========================================================
* @Fn			- Samy_OS_Acquire_Mutex
* @brief 		- Acquire Mutex if available
* @param[in]    - p_Task:  Pointer to the Task's  Configuration Struct.
* @param[in]    - p_Mutex: Pointer to the Mutex's Configuration Struct. 
* @return 		- Returns No_Error if no error occured, or specific Error if occured.
* Note			- Mutex Can be hold by a Maximum of 2 Tasks only (a task already acquires it, while the other is waiting until released).
*/
Samy_OS_Status Samy_OS_Acquire_Mutex(Tasks_ref *p_Task , Mutex_ref *p_Mutex);
```
```c
/*==========================================================
* @Fn			- Samy_OS_Release_Mutex
* @brief 		- Release Mutex and acquires it to the other task waiting for that Mutex (if found).
* @param[in]    - p_Mutex: Pointer to the Mutex's Configuration Struct. 
* @return 		- Returns No_Error if no error occured, or specific Error if occured.
* Note			- Mutex is Released if and only if it was acquired by the Same task, not any other tasks. 
*/
void Samy_OS_Release_Mutex(Mutex_ref *p_Mutex);
```
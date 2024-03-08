/*
 * CortexMX_OS_Porting.h
 *
 *  Created on: Feb 23, 2024
 *      Author: Youssef Samy
 */

#ifndef CORTEXMX_OS_PORTING_H_
#define CORTEXMX_OS_PORTING_H_

#include "core_cm3.h"
#include "ARMCM3.h"
#include "Platform_Types.h"

extern uint32 _estack; // the address of the Top of the Stack (Note the ARM Stack is Full Descending Stack)
extern uint32 _eheap;

#define MAIN_Stack_SIZE		3072 // 3Kb * 1024 = 3072 Bytes


/********************************************* OS Macros *********************************************/

#define Trigger_OS_PendSV() (SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk)


#define OS_SET_PSP(address)		__asm volatile("MOV R0,%0  \n\t   MSR PSP,R0"  :   :"r" (address))
#define OS_GET_PSP(address)		__asm volatile("MRS R0,PSP  \n\t   MOV %0,R0"  : "=r" (address))

#define OS_SWITCH_SP_to_PSP()  	__asm volatile("MRS R0,CONTROL  \n\t   ORR R0,R0,#0x02  \n\t   MSR CONTROL,R0") // Set SP to Shadow on PSP instead of MSP (SET Bit1 in CONTROL Register)
#define OS_SWITCH_SP_to_MSP()  	__asm volatile("MRS R0,CONTROL  \n\t   AND R0,R0,#0xFFFFFFFD  \n\t   MSR CONTROL,R0") // Set SP to Shadow on MSP instead of PSP (Clear Bit1 in CONTROL Register)

#define Switch_CPU_Access_Level_to_Privileged()	 	__asm("MRS R0,CONTROL \n\t   AND R0,R0,#0xFFFFFFFE \n\t MSR CONTROL,R0")
#define Switch_CPU_Access_Level_to_Unprivileged() 	__asm("MRS R0,CONTROL  \n\t  ORR R0,R0,#0x1         \n\t MSR CONTROL,R0")

/************************************************************************************************/

void HW_Init(void);
void Start_Ticker(void);


#endif /* CORTEXMX_OS_PORTING_H_ */

/*
 * CortexMX_OS_Porting.c
 *
 *  Created on: Feb 23, 2024
 *      Author: Youssef Samy
 */

#include "CortexMX_OS_Porting.h"

void NMI_Handler(void)
{

}
void HardFault_Handler(void)
{

}
void MemManage_Handler(void)
{

}
void BusFault_Handler(void)
{

}
void UsageFault_Handler(void)
{

}


__attribute ((naked)) void SVC_Handler()
{
	__asm( "TST    LR, #0x04  \n\t"
			"ITE    EQ        \n\t"
			"MRSEQ  R0, MSP   \n\t"
			"MRSNE  R0, PSP   \n\t" // R0 now holds the PSP (StackFramePointer)
			"B      Samy_OS_SVC_Services"); // We will extract the SVC ID_number in this API to be able to execute the required service

}

void HW_Init(void)
{
	/* Initialize clock tree (RCC -> SysTick Timer & CPU 8MHz)
	 * 8MHz
	 * 1 Count -> 0.125us
	 * X count -> 1ms
	 * X = 8000 count
	 */

	// Decrease PendSV interrupt priority to be smaller or equal to Systick Timer
	// This is done to make sure the Systick Comes first , then triggering the PendSV for Context Switching
	__NVIC_SetPriority(PendSV_IRQn, 15);

	// SVC is by default the Highest Priority than Systick and PendSV
}

void Start_Ticker(void)
{
	SysTick_Config(8000); // Periodic Tick = 1msec
}



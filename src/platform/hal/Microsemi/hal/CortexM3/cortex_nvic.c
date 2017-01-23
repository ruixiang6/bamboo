/*******************************************************************************
 * (c) Copyright 2007-2013 Microsemi SoC Products Group.  All rights reserved.
 * 
 * Legacy Actel HAL Cortex NVIC control functions.
 * The use of these functions should be replaced by calls to the equivalent
 * CMSIS function in your application code.
 *
 * SVN $Revision: 5259 $
 * SVN $Date: 2013-03-21 12:58:05 +0000 (Thu, 21 Mar 2013) $
 */
#include <stdint.h>
#include "cortex_nvic.h"
#include "../../CMSIS/m2sxxx.h"
#include "../../CMSIS/mss_assert.h"


/* Private Macros ------------------------------------------------------------- */
/** @defgroup NVIC_Private_Macros NVIC Private Macros
 * @{
 */

/* Vector table offset bit mask */
#define NVIC_VTOR_MASK              0x3FFFFF80

/**
 * @}
 */


/* Public Functions ----------------------------------------------------------- */
/** @addtogroup NVIC_Public_Functions
 * @{
 */


/*****************************************************************************//**
 * @brief		De-initializes the NVIC peripheral registers to their default
 * 				reset values.
 * @param		None
 * @return      None
 *
 * These following NVIC peripheral registers will be de-initialized:
 * - Disable Interrupt (32 IRQ interrupt sources that matched with LPC178X)
 * - Clear all Pending Interrupts (32 IRQ interrupt source that matched with LPC178X)
 * - Clear all Interrupt Priorities (32 IRQ interrupt source that matched with LPC178X)
 *******************************************************************************/
void NVIC_DeInit(void)
{
	uint8_t tmp;

	/* Disable all interrupts */
	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0x00000001;
	/* Clear all pending interrupts */
	NVIC->ICPR[0] = 0xFFFFFFFF;
	NVIC->ICPR[1] = 0x00000001;

	/* Clear all interrupt priority */
	for (tmp = 0; tmp < 32; tmp++) {
		NVIC->IP[tmp] = 0x00;
	}
}

/*****************************************************************************//**
 * @brief			De-initializes the SCB peripheral registers to their default
 *                  reset values.
 * @param			none
 * @return 			none
 *
 * These following SCB NVIC peripheral registers will be de-initialized:
 * - Interrupt Control State register
 * - Interrupt Vector Table Offset register
 * - Application Interrupt/Reset Control register
 * - System Control register
 * - Configuration Control register
 * - System Handlers Priority Registers
 * - System Handler Control and State Register
 * - Configurable Fault Status Register
 * - Hard Fault Status Register
 * - Debug Fault Status Register
 *******************************************************************************/
void NVIC_SCBDeInit(void)
{
	uint8_t tmp;

	SCB->ICSR = 0x0A000000;
	SCB->VTOR = 0x00000000;
	SCB->AIRCR = 0x05FA0000;
	SCB->SCR = 0x00000000;
	SCB->CCR = 0x00000000;

	for (tmp = 0; tmp < 32; tmp++) {
		SCB->SHP[tmp] = 0x00;
	}

	SCB->SHCSR = 0x00000000;
	SCB->CFSR = 0xFFFFFFFF;
	SCB->HFSR = 0xFFFFFFFF;
	SCB->DFSR = 0xFFFFFFFF;
}


/*****************************************************************************//**
 * @brief		Set Vector Table Offset value
 * @param		offset Offset value
 * @return      None
 *******************************************************************************/
void NVIC_SetVTOR(uint32_t offset)
{
//	SCB->VTOR  = (offset & NVIC_VTOR_MASK);
	SCB->VTOR  = offset;
}

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */

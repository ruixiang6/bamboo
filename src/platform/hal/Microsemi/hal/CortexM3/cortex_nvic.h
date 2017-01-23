/*******************************************************************************
 * (c) Copyright 2007-2013 Microsemi SoC Products Group.  All rights reserved.
 * 
 * Legacy Actel HAL Cortex NVIC control functions.
 * The use of these functions should be replaced by calls to the equivalent
 * CMSIS function in your application code.
 *
 * SVN $Revision: 5257 $
 * SVN $Date: 2013-03-21 12:24:10 +0000 (Thu, 21 Mar 2013) $
 */
#ifndef CORTEX_NVIC_H_
#define CORTEX_NVIC_H_

#ifdef __cplusplus
extern "C"
{
#endif


/* Public Functions ----------------------------------------------------------- */
/** @defgroup NVIC_Public_Functions NVIC Public Functions
 * @{
 */

void NVIC_DeInit(void);
void NVIC_SCBDeInit(void);
void NVIC_SetVTOR(uint32_t offset);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /*CORTEX_NVIC_H_*/

/**
 * OSEL Kernel Control Utility Routines
 *
 * @file osel_kernel.h
 * @author qhw(2013-3-11)
 *
 * @addtogroup OSEL_KERNEL	OSEL Kernel Control Utility Routines
 * @ingroup OSEL
 * @{
 */
#ifndef __OSEL_KERNEL_H
#define __OSEL_KERNEL_H

#define OSEL_DECL_CRITICAL()	 uint32_t cpu_sr
#define OSEL_ENTER_CRITICAL()	OS_ENTER_CRITICAL()
#define OSEL_EXIT_CRITICAL()	OS_EXIT_CRITICAL()

/**
 * Initialize operating system
 */
void osel_init(void);

/**
 * Start operating system scheduling
 */
void osel_start(void);

/**
 * Start operating system systick
 */
void osel_systick_init(void);

/**
 * delay tick and yield proc right
 */
void osel_systick_delay(uint32_t tick);

/**
 * get system tick
 */
uint32_t osel_systick_get(void);

#endif
/**
 * @}
 */

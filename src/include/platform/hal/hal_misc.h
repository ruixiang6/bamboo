 /**
 * provides an abstraction for hardware.
 *
 * @file hal_misc.h
 * @author qhw
 *
 * @addtogroup HAL_MISC HAL miscellaneous
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_MISC_H
#define __HAL_MISC_H

/**
 * 	board initilazie nvic register mainly enable all irq
 */
void hal_board_init();

void hal_board_reset(void);

#endif

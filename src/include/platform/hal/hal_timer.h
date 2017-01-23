/**
 * provides an abstraction for timer and sytem frequency
 *
 * @file hal_timer.h
 * @author qhw
 *
 * @addtogroup HAL_TIMER HAL Timer and MCU Frequency
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_TIMER_H
#define __HAL_TIMER_H

#define MAX_TIMER_NUM   2

/* normal block delay function us uint, pls note the range!(0~4294967us)*/
void delay_us(uint32_t us);

/* normal block delay function ms uint, pls note the range!(0~4294967ms)*/
void delay_ms(uint32_t ms);

/**
 * initialize the timer
 */
void hal_timer_init(void);

/**
 * This call allows you to specify the tick time in us
 *
 * @param: start_cnt if pass NULL,used now_cnt
 * @param: tick timing tick us
 * @param: func executed when time_us expired
 *
 * @return TRUE or FALSE
 */
uint8_t hal_timer_alloc(uint64_t time_us, fpv_t func);

/*
 * free specific tick timer
 *
 * @Param: timer_id  the id of hardware clock to be stopped
 */
uint8_t hal_timer_free(uint8_t timer_id);

#endif

/**
 * @}
 */

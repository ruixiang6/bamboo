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

typedef struct
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t weekday;
	uint8_t week;
}hal_rtc_block_t;

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


void hal_rtc_get(hal_rtc_block_t *rtc_block);

void hal_rtc_set(hal_rtc_block_t *rtc_block);

//////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#define HAL_FPGA_TIM_NUM						8

#define HAL_FPGA_TIM0_BASE					(0x33000000)
#define HAL_FPGA_TIM1_BASE					(0x34000000)
#define HAL_FPGA_TIM2_BASE					(0x35000000)
#define HAL_FPGA_TIM3_BASE					(0x36000000)
#define HAL_FPGA_TIM4_BASE					(0x37000000)
#define HAL_FPGA_TIM5_BASE					(0x38000000)
#define HAL_FPGA_TIM6_BASE					(0x39000000)
#define HAL_FPGA_TIM7_BASE					(0x3a000000)

typedef struct
{
	uint32_t load;
	uint32_t value;
	uint32_t control;
	uint32_t prescale;
	uint32_t int_clr;
	uint32_t int_raw_status;
	uint32_t int_mask_status;
}hal_fpga_timer_t;

#define HAL_FPGA_TIM0     ((hal_fpga_timer_t *) HAL_FPGA_TIM0_BASE)
#define HAL_FPGA_TIM1     ((hal_fpga_timer_t *) HAL_FPGA_TIM1_BASE)
#define HAL_FPGA_TIM2     ((hal_fpga_timer_t *) HAL_FPGA_TIM2_BASE)
#define HAL_FPGA_TIM3     ((hal_fpga_timer_t *) HAL_FPGA_TIM3_BASE)
#define HAL_FPGA_TIM4     ((hal_fpga_timer_t *) HAL_FPGA_TIM4_BASE)
#define HAL_FPGA_TIM5     ((hal_fpga_timer_t *) HAL_FPGA_TIM5_BASE)
#define HAL_FPGA_TIM6     ((hal_fpga_timer_t *) HAL_FPGA_TIM6_BASE)
#define HAL_FPGA_TIM7     ((hal_fpga_timer_t *) HAL_FPGA_TIM7_BASE)


void hal_fpga_tim_init(void);

bool_t hal_fpga_tim_exist(void);

void hal_fpga_tim_enable(uint8_t index);

void hal_fpga_tim_disable(uint8_t index);

void hal_fpga_tim_int_enable(uint8_t index);

void hal_fpga_tim_int_disable(uint8_t index);

void hal_fpga_tim_int_reg(uint8_t index, fpv_t func);

void hal_fpga_tim_int_unreg(uint8_t index);

void hal_fpga_tim_int_clear(uint8_t index);

uint32_t hal_fpga_tim_get_value(uint8_t index);

void hal_fpga_tim_set_value(uint8_t index, uint32_t value);


#endif

/**
 * @}
 */

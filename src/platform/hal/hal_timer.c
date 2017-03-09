#include <platform.h>
#include <m2sxxx.h>
#include <cortex_nvic.h>
#include <mss_timer.h>
#include <mss_watchdog.h>
#include <mss_rtc.h>
#include <hal_timer.h>

#define HAL_DELAY_TIMER_MHZ	50

#define HAL_TIMER_HZ        100000000
#define HAL_TICK_PER_US     100
#define HAL_TICK_PER_MS     100000
#define HAL_TICK_PER_S      100000000

#define TIMER_HARDWARE_THRESHOLD	0xffffffff

typedef uint32_t hs_cnt_t; /* hardware time counter */
typedef uint32_t ss_cnt_t; /* software time counter */
typedef uint64_t w_cnt_t;  /* wide time counter */

typedef struct
{
	hs_cnt_t hs_cnt;
 	ss_cnt_t ss_cnt;
} s_cnt_t;

typedef union
{
	s_cnt_t s;
	w_cnt_t w;
} hal_cnt_t;

#pragma pack(4)
typedef struct
{
	bool_t used;
	hal_cnt_t cnt;
    fpv_t func;
}timer_reg_t;
#pragma pack()

static timer_reg_t timer_reg[MAX_TIMER_NUM];

void delay_us(uint32_t us)
{
    uint32_t during_cnt = us*HAL_DELAY_TIMER_MHZ;

	if (during_cnt>WATCHDOG->WDOGLOAD || during_cnt == 0)
	{
		return;
	}
	during_cnt = WATCHDOG->WDOGLOAD - during_cnt;
	
	MSS_WD_reload();
		
	while(MSS_WD_current_value()>during_cnt);	
}

void delay_ms(uint32_t ms)
{
	uint32_t i;
	
	for (i=0; i<ms; i++)
	{
		delay_us(1000);
	}    
}

void hal_timer_init(void)
{
	//start wathdog timer used for block time delay
	MSS_WD_init();
	//
	MSS_TIM1_init(MSS_TIMER_ONE_SHOT_MODE);
	//
	MSS_TIM2_init(MSS_TIMER_ONE_SHOT_MODE);
	//
	mem_clr(timer_reg, sizeof(timer_reg));
#if RTC_EN	
#define PO_RESET_DETECT_MASK	0x00000001u
//#define RTC_PRESCALER	(32768u - 1u)	  /* 32KHz crystal is RTC clock source. */
//#define RTC_PRESCALER (1000000u - 1u)        /* 1MHz clock is RTC clock source. */
//#define RTC_PRESCALER	(25000000u - 1u)  /* 25MHz clock is RTC clock source. */
#define RTC_PRESCALER	(50000000u - 1u)  /* 50MHz clock is RTC clock source. */	
    if(SYSREG->RESET_SOURCE_CR & PO_RESET_DETECT_MASK)
    {
        MSS_RTC_init(MSS_RTC_CALENDAR_MODE, RTC_PRESCALER);
        SYSREG->RESET_SOURCE_CR = PO_RESET_DETECT_MASK;
    }	
#endif
}

uint8_t hal_timer_alloc(uint64_t time_us, fpv_t func)
{
	uint8_t id;

	if (time_us == 0 || func == PLAT_NULL)
	{
		return PLAT_FALSE;
	}
	
	for (id = 0; id<MAX_TIMER_NUM; id++)
	{
		if (timer_reg[id].used == PLAT_FALSE) break;		
	}
	
	if (id == MAX_TIMER_NUM) return PLAT_FALSE;		

	timer_reg[id].used = PLAT_TRUE;
	timer_reg[id].cnt.s.ss_cnt = (time_us*HAL_TICK_PER_US)/TIMER_HARDWARE_THRESHOLD;
	timer_reg[id].cnt.s.hs_cnt = (time_us*HAL_TICK_PER_US)%TIMER_HARDWARE_THRESHOLD;
	timer_reg[id].func = func;
	
	if (id == 0)
	{
		if (timer_reg[id].cnt.s.ss_cnt == 0)
		{
			MSS_TIM1_load_immediate(timer_reg[id].cnt.s.hs_cnt);
			timer_reg[id].cnt.s.hs_cnt = 0;
		}
		else
		{
			MSS_TIM1_load_immediate(TIMER_HARDWARE_THRESHOLD);
			timer_reg[id].cnt.s.ss_cnt--;
		}
		MSS_TIM1_clear_irq();
		MSS_TIM1_enable_irq();
		MSS_TIM1_start();
		
	}
	else if (id == 1)
	{
		if (timer_reg[id].cnt.s.ss_cnt == 0)
		{
			MSS_TIM2_load_immediate(timer_reg[id].cnt.s.hs_cnt);
			timer_reg[id].cnt.s.hs_cnt = 0;
		}
		else
		{
			MSS_TIM2_load_immediate(TIMER_HARDWARE_THRESHOLD);
			timer_reg[id].cnt.s.ss_cnt--;
		}
		MSS_TIM2_clear_irq();
		MSS_TIM2_enable_irq();
		MSS_TIM2_start();
	}
	else
	{
		return PLAT_FALSE;
	}
	
	return id+1;
}

uint8_t hal_timer_free(uint8_t timer_id)
{
	uint8_t id = timer_id - 1;
	
	if (timer_reg[id].used == PLAT_FALSE)
	{
		return 0;		
	}
	
	if (id == 0)
	{
		MSS_TIM1_stop();
		MSS_TIM1_disable_irq();
		MSS_TIM1_clear_irq();
		
		timer_reg[id].used = PLAT_FALSE;		
		timer_reg[id].func = PLAT_NULL;
	}
	else if (id == 1)
	{
		MSS_TIM2_stop();
		MSS_TIM2_disable_irq();
		MSS_TIM2_clear_irq();
		
		timer_reg[id].used = PLAT_FALSE;		
		timer_reg[id].func = PLAT_NULL;
	}
	
	return 0;	
}

void Timer1_IRQHandler(void)
{
	MSS_TIM1_clear_irq();
	MSS_TIM1_stop();
		
	if (timer_reg[0].used)
	{
		if (timer_reg[0].cnt.s.ss_cnt)
		{
			MSS_TIM1_load_immediate(TIMER_HARDWARE_THRESHOLD);
			timer_reg[0].cnt.s.ss_cnt--;			
		}
		else 
		{
			if (timer_reg[0].cnt.s.hs_cnt)
			{
				MSS_TIM1_load_immediate(timer_reg[0].cnt.s.hs_cnt);				
				timer_reg[0].cnt.s.hs_cnt = 0;
			}
			else
			{
				timer_reg[0].used = PLAT_FALSE;
				//execute
				if (timer_reg[0].func)
				{
					timer_reg[0].func();
				}			
				
				return;
			}
		}
		MSS_TIM1_start();
	}
}

void Timer2_IRQHandler(void)
{
	MSS_TIM2_clear_irq();
	MSS_TIM2_stop();
		
	if (timer_reg[1].used)
	{
		if (timer_reg[1].cnt.s.ss_cnt)
		{
			MSS_TIM2_load_immediate(TIMER_HARDWARE_THRESHOLD);
			timer_reg[1].cnt.s.ss_cnt--;			
		}
		else 
		{
			if (timer_reg[1].cnt.s.hs_cnt)
			{
				MSS_TIM2_load_immediate(timer_reg[1].cnt.s.hs_cnt);				
				timer_reg[1].cnt.s.hs_cnt = 0;
			}
			else
			{
				timer_reg[1].used = PLAT_FALSE;
				//execute
				if (timer_reg[1].func)
				{
					timer_reg[1].func();
				}			
				
				return;
			}
		}
		MSS_TIM2_start();
	}
}

void hal_rtc_set(hal_rtc_block_t *rtc_block)
{
#if RTC_EN
	MSS_RTC_set_calendar_count((mss_rtc_calendar_t *)rtc_block);
	MSS_RTC_start();
#endif
}

void hal_rtc_get(hal_rtc_block_t *rtc_block)
{
#if RTC_EN
	MSS_RTC_get_calendar_count((mss_rtc_calendar_t *)rtc_block);
#endif
}

static struct
{
	bool_t used_flag;
	hal_fpga_timer_t *hw[HAL_FPGA_TIM_NUM];
	fpv_t tmr_handler[HAL_FPGA_TIM_NUM];
}tim_handler = 
{
	.used_flag = PLAT_FALSE,
	.hw[0] = HAL_FPGA_TIM0,
	.hw[1] = HAL_FPGA_TIM1,
	.hw[2] = HAL_FPGA_TIM2,
	.hw[3] = HAL_FPGA_TIM3,
	.hw[4] = HAL_FPGA_TIM4,
	.hw[5] = HAL_FPGA_TIM5,
	.hw[6] = HAL_FPGA_TIM6,
	.hw[7] = HAL_FPGA_TIM7,
	.tmr_handler[0] = PLAT_NULL,
	.tmr_handler[1] = PLAT_NULL,
	.tmr_handler[2] = PLAT_NULL,
	.tmr_handler[3] = PLAT_NULL,
	.tmr_handler[4] = PLAT_NULL,
	.tmr_handler[5] = PLAT_NULL,
	.tmr_handler[6] = PLAT_NULL,
	.tmr_handler[7] = PLAT_NULL
};

void hal_fpga_tim_init(void)
{
	NVIC_ClearPendingIRQ(FabricIrq3_IRQn);
    NVIC_EnableIRQ(FabricIrq3_IRQn);

	NVIC_ClearPendingIRQ(FabricIrq4_IRQn);
    NVIC_EnableIRQ(FabricIrq4_IRQn);

	NVIC_ClearPendingIRQ(FabricIrq5_IRQn);
    NVIC_EnableIRQ(FabricIrq5_IRQn);

	NVIC_ClearPendingIRQ(FabricIrq6_IRQn);
    NVIC_EnableIRQ(FabricIrq6_IRQn);

	NVIC_ClearPendingIRQ(FabricIrq7_IRQn);
    NVIC_EnableIRQ(FabricIrq7_IRQn);

	NVIC_ClearPendingIRQ(FabricIrq8_IRQn);
    NVIC_EnableIRQ(FabricIrq8_IRQn);

	NVIC_ClearPendingIRQ(FabricIrq9_IRQn);
    NVIC_EnableIRQ(FabricIrq9_IRQn);

	NVIC_ClearPendingIRQ(FabricIrq10_IRQn);
    NVIC_EnableIRQ(FabricIrq10_IRQn);

	tim_handler.used_flag = PLAT_TRUE;	
}

bool_t hal_fpga_tim_exist(void)
{
	return tim_handler.used_flag;
}

void hal_fpga_tim_enable(uint8_t index)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		tim_handler.hw[index]->control |= (1u<<0);
		tim_handler.hw[index]->control |= (1u<<2);
	}
}

void hal_fpga_tim_disable(uint8_t index)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		tim_handler.hw[index]->control &= ~(1u<<2);
		tim_handler.hw[index]->control &= ~(1u<<0);
	}
}

void hal_fpga_tim_int_enable(uint8_t index)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		tim_handler.hw[index]->control |= (1u<<1);
	}
}

void hal_fpga_tim_int_disable(uint8_t index)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		tim_handler.hw[index]->control &= ~(1u<<1);
	}
}

void hal_fpga_tim_int_reg(uint8_t index, fpv_t func)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		tim_handler.tmr_handler[index] = func;
	}
}

void hal_fpga_tim_int_unreg(uint8_t index)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		tim_handler.tmr_handler[index] = PLAT_NULL;
	}
}

void hal_fpga_tim_int_clear(uint8_t index)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		tim_handler.hw[index]->int_clr |= (1u<<0);
	}
}

uint32_t hal_fpga_tim_get_value(uint8_t index)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		return tim_handler.hw[index]->value;
	}
    else
    {
        return 0;
    }
}

void hal_fpga_tim_set_value(uint8_t index, uint32_t value)
{
	if (index<HAL_FPGA_TIM_NUM)
	{
		tim_handler.hw[index]->load = value;
	}
}


//timer0
void FabricIrq3_IRQHandler(void)
{
	hal_fpga_tim_int_clear(0);

	if (tim_handler.tmr_handler[0])
	{
		(*(tim_handler.tmr_handler[0]))();
	}
}
//timer1
void FabricIrq4_IRQHandler(void)
{
	hal_fpga_tim_int_clear(1);

	if (tim_handler.tmr_handler[1])
	{
		(*(tim_handler.tmr_handler[1]))();
	}
}
//timer2
void FabricIrq5_IRQHandler(void)
{
	hal_fpga_tim_int_clear(2);

	if (tim_handler.tmr_handler[2])
	{
		(*(tim_handler.tmr_handler[2]))();
	}
}
//timer3
void FabricIrq6_IRQHandler(void)
{
	hal_fpga_tim_int_clear(3);

	if (tim_handler.tmr_handler[3])
	{
		(*(tim_handler.tmr_handler[3]))();
	}
}
//timer4
void FabricIrq7_IRQHandler(void)
{
	hal_fpga_tim_int_clear(4);

	if (tim_handler.tmr_handler[4])
	{
		(*(tim_handler.tmr_handler[4]))();
	}
}
//timer5
void FabricIrq8_IRQHandler(void)
{
	hal_fpga_tim_int_clear(5);

	if (tim_handler.tmr_handler[5])
	{
		(*(tim_handler.tmr_handler[5]))();
	}
}
//timer6
void FabricIrq9_IRQHandler(void)
{
	hal_fpga_tim_int_clear(6);

	if (tim_handler.tmr_handler[6])
	{
		(*(tim_handler.tmr_handler[6]))();
	}
}
//timer7
void FabricIrq10_IRQHandler(void)
{
	hal_fpga_tim_int_clear(7);

	if (tim_handler.tmr_handler[7])
	{
		(*(tim_handler.tmr_handler[7]))();
	}
}



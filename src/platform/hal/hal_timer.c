#include <platform.h>
#include <m2sxxx.h>
#include <cortex_nvic.h>
#include <mss_timer.h>
#include <mss_watchdog.h>
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


#include <ucos_ii.h>
#include <platform.h>

extern uint32_t SystemCoreClock;         /*!< System Clock Frequency (Core Clock) */

void osel_init(void)
{
    OSInit();
    osel_task_init();
    osel_event_init();
}

void osel_start(void)
{
	OSStart();
}

void osel_systick_init(void)
{
	OS_CPU_SysTickInit(SystemCoreClock/OS_TICKS_PER_SEC);
}

void osel_systick_delay(uint32_t tick)
{
    OSTimeDly(tick);
}

uint32_t osel_systick_get(void)
{
    return OSTimeGet();
}

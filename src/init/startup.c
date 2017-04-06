#include <platform.h>
#include <kbuf.h>
#include <device.h>
#include <test.h>
#include <nwk.h>
#include <nwk_eth.h>
#include <mac.h>
#include "Control_IO.h"
#include <app.h>

#define INIT_TASK_STK_SIZE			256
#define INIT_TASK_PRIO				OSEL_TASK_PRIO(0)

OSEL_DECLARE_TASK(INIT_TASK, param);
osel_task_t *init_task_h;

extern void app_init(void);
extern void gui_proc(void);

void main(void)
{	
	DBG_SET_LEVEL(DBG_LEVEL_PRINTF);
	/* 板级初始化 */
	hal_board_init();
	/* 系统HEAP内存初始化 */
	memory_init(NULL);
    /* 定时器初始化 */
	hal_timer_init();
    /* 调试串口初始化 */
	hal_uart_init(UART_DEBUG, 115200);
    DBG_TRACE("Start!\r\n");
	/* GPIO初始化 */
	hal_gpio_init(PLAT_TRUE);
	//开机上电检测
	ControlIO_Init();	
	/* 北斗串口初始化 */
	hal_uart_init(UART_BD, 9600);	
	/* 操作系统初始化 */
    osel_init();
	/* 创建 INIT 任务 */
	init_task_h = osel_task_create(INIT_TASK, 
    								PLAT_NULL, 
    								INIT_TASK_STK_SIZE, 
    								INIT_TASK_PRIO);
    DBG_ASSERT(init_task_h != PLAT_NULL);

    	
	/*操作系统启动*/
	osel_start();

	while(1);
}

OSEL_DECLARE_TASK(INIT_TASK, param)
{
	(void)param;
	bool_t res;
	
	DBG_TRACE("INIT_TASK!\r\n");
	
	//pbuf贯串整个通信模块，故在这边进行了初始化
	res = kbuf_init();
	DBG_ASSERT(res == PLAT_TRUE);	
	//系统tick时钟
	osel_systick_init();
	//设备信息初始化
	device_info_init();	
	/* TEST */
	test_init();
	/* APP Task */
	app_init();
	/* NWK Task */
	//nwk_init();
    /* MAC Task */
	//mac_init();
	/* GUI */
   	GUI_Init();

	osel_task_idle_hook_reg(2, nwk_idle_hook);//nwk

	osel_task_idle_hook_reg(1, mac_idle_hook);//mac

	osel_task_idle_hook_reg(5, gui_proc);//mac
	
    while(1)
	{
		//触发test事件，进入测试程序
		test_handler();
	}

}

void HardFault_Handler(void)
{
	DBG_TRACE("HardFault_Handler!\r\n");
}

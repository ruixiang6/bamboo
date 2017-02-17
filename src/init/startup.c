#include <platform.h>
#include <kbuf.h>
#include <device.h>
#include <test.h>
#include <nwk.h>
#include <mac.h>
#include "Control_IO.h"

#define INIT_TASK_STK_SIZE			256
#define INIT_TASK_PRIO				OSEL_TASK_PRIO(0)

OSEL_DECLARE_TASK(INIT_TASK, param);
osel_task_t *init_task_h;

extern void app_init(void);

void main(void)
{	
	DBG_SET_LEVEL(DBG_LEVEL_PRINTF);
	/* �弶��ʼ�� */
	hal_board_init();
	/* ϵͳHEAP�ڴ��ʼ�� */
	memory_init(NULL);
    /* ��ʱ����ʼ�� */
	hal_timer_init();
    /* ���Դ��ڳ�ʼ�� */
	hal_uart_init(UART_DEBUG, 115200);
	/* GPIO��ʼ�� */
	hal_gpio_init(PLAT_TRUE);
	//�����ϵ���
	ControlIO_Init();	
	/* �������ڳ�ʼ�� */
	hal_uart_init(UART_BD, 9600);	
	/* ����ϵͳ��ʼ�� */
    osel_init();
	/* ���� INIT ���� */
	init_task_h = osel_task_create(INIT_TASK, 
    								PLAT_NULL, 
    								INIT_TASK_STK_SIZE, 
    								INIT_TASK_PRIO);
    DBG_ASSERT(init_task_h != PLAT_NULL);

    	
	/*����ϵͳ����*/
	osel_start();

	while(1);
}

OSEL_DECLARE_TASK(INIT_TASK, param)
{
	(void)param;
	bool_t res;
	
	DBG_TRACE("INIT_TASK!\r\n");
	
	//pbuf�ᴮ����ͨ��ģ�飬������߽����˳�ʼ��
	res = kbuf_init();
	DBG_ASSERT(res == PLAT_TRUE);	
	//ϵͳtickʱ��
	osel_systick_init();
	//�豸��Ϣ��ʼ��
	device_info_init();	
	/* TEST */
	test_init();
	/* APP Task */
	app_init();
	/* NWK Task */
	nwk_init();
    /* MAC Task */
	mac_init();

	osel_task_idle_hook_reg(nwk_idle_hook);
	
    while(1)
	{
		//����test�¼���������Գ���
		test_handler();
	}

}

void HardFault_Handler(void)
{
	DBG_TRACE("HardFault_Handler!\r\n");
}

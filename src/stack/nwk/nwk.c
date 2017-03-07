#include <platform.h>
#include <device.h>
#include <nwk.h>
#include <nwk_eth.h>
#include <nwk_mesh.h>


#define NWK_TASK_STK_SIZE			256
#define NWK_TASK_PRIO				OSEL_TASK_PRIO(2)

OSEL_DECLARE_TASK(NWK_TASK, param);

osel_task_t *nwk_task_h;
osel_event_t *nwk_event_h;


OSEL_DECLARE_TASK(NWK_TASK, param)
{
	(void)param;
	osel_event_res_t res;
	
	DBG_TRACE("NWK_TASK!\r\n");

	osel_systick_delay(100);
	
	while (1)
	{		
		res = osel_event_wait(nwk_event_h, OSEL_WAIT_FOREVER);
		if (res == OSEL_EVENT_NONE)
		{
			nwk_handler(OSEL_EVENT_GET(nwk_event_h, uint16_t));
		}
	}
}


void nwk_deinit(void)
{
	nwk_mesh_deinit();
	
	nwk_eth_deinit();
	
	osel_task_delete(nwk_task_h);
	osel_event_delete(nwk_event_h);
}


void nwk_init(void)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	
	nwk_task_h = osel_task_create(NWK_TASK, 
    								NULL, 
    								NWK_TASK_STK_SIZE, 
    								NWK_TASK_PRIO);
	DBG_ASSERT(nwk_task_h != PLAT_NULL);
	nwk_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(nwk_event_h != PLAT_NULL);	
	//启动以太网和tcpip协议栈，网卡的初始化放在网线有连接的时候
	nwk_eth_init();
	//在普通模式下启动mesh，其余如监听模式则不需要启动mesh
	if (GET_MODE_ID(p_device_info->id) == MODE_NORMAL)
	{
		nwk_mesh_init();
	}
	DBG_TRACE("nwk_init ok\r\n");	
}


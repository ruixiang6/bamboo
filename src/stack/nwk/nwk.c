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
	nwk_task_h = osel_task_create(NWK_TASK, 
    								NULL, 
    								NWK_TASK_STK_SIZE, 
    								NWK_TASK_PRIO);
	DBG_ASSERT(nwk_task_h != PLAT_NULL);
	nwk_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(nwk_event_h != PLAT_NULL);	

	nwk_eth_init();

	nwk_mesh_init();

	DBG_TRACE("nwk_init ok\r\n");
}


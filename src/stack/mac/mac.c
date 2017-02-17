#include <mac.h>
#include <device.h>
#include <phy.h>

#define MAC_TASK_STK_SIZE			256
#define MAC_TASK_PRIO				OSEL_TASK_PRIO(1)

OSEL_DECLARE_TASK(MAC_TASK, param);

osel_task_t *mac_task_h;
osel_event_t *mac_event_h;

static list_t mac_ofdm_send_q0_list;
static list_t mac_ofdm_send_q1_list;
static list_t mac_ofdm_send_q2_list;
static list_t mac_ofdm_send_q3_list;
static list_t mac_ofdm_send_q4_list;

void mac_init(void)
{
  	/*创建 MAC 任务 */   
	mac_task_h = osel_task_create(MAC_TASK, 
    								NULL, 
    								MAC_TASK_STK_SIZE, 
    								MAC_TASK_PRIO);
    DBG_ASSERT(mac_task_h != PLAT_NULL);
	mac_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(mac_event_h != PLAT_NULL);
}

void mac_deinit(void)
{
	//OFDM进入IDLE态
	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);
	
	osel_event_delete(mac_event_h);
	osel_task_delete(mac_task_h);
	mac_event_h = PLAT_NULL;
	mac_task_h = PLAT_NULL;
}

OSEL_DECLARE_TASK(MAC_TASK, param)
{
    (void)param;
	osel_event_res_t res;
	
	DBG_TRACE("MAC_TASK!\r\n");

	list_init(&mac_ofdm_recv_list);
	list_init(&mac_ofdm_send_list);
	
	mac_ofdm_send_multi_list[0] = &mac_ofdm_send_q0_list;
	mac_ofdm_send_multi_list[1] = &mac_ofdm_send_q1_list;
	mac_ofdm_send_multi_list[2] = &mac_ofdm_send_q2_list;
	mac_ofdm_send_multi_list[3] = &mac_ofdm_send_q3_list;
	mac_ofdm_send_multi_list[4] = &mac_ofdm_send_q4_list;

	for (uint8_t i=0; i<MAC_QOS_LIST_MAX_NUM; i++)
	{
		list_init(mac_ofdm_send_multi_list[i]);
	}

	phy_init();
	
	while(1)
	{
		res = osel_event_wait(mac_event_h, OSEL_WAIT_FOREVER);
		if (res == OSEL_EVENT_NONE)
		{
			mac_handler(OSEL_EVENT_GET(mac_event_h, uint16_t));
		}
	}	
}

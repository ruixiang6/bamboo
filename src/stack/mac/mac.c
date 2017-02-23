#include <mac.h>
#include <device.h>
#include <phy.h>

#define MAC_TASK_STK_SIZE			256
#define MAC_TASK_PRIO				OSEL_TASK_PRIO(1)

OSEL_DECLARE_TASK(MAC_TASK, param);

osel_task_t *mac_task_h;
osel_event_t *mac_event_h;
mac_timer_t mac_timer;
kbuf_t *mac_rdy_snd_kbuf = PLAT_NULL;

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

	phy_tmr_free(mac_timer.send_id);
	phy_tmr_free(mac_timer.csma_id);
	phy_tmr_free(mac_timer.live_id);
}

OSEL_DECLARE_TASK(MAC_TASK, param)
{
    (void)param;
	osel_event_res_t res;
	
	DBG_TRACE("MAC_TASK!\r\n");

	list_init(&mac_ofdm_recv_list);
	list_init(&mac_ofdm_send_list);

	mem_set(&mac_timer, 0, sizeof(mac_timer_t));
	
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
	phy_ofdm_init(mac_ofdm_send_cb, mac_ofdm_recv_cb);
	phy_tmr_init();

	mac_timer.send_id = phy_tmr_alloc(mac_tx_cb);
	mac_timer.csma_id = phy_tmr_alloc(mac_csma_cb);
	mac_timer.live_id = phy_tmr_alloc(mac_rdy_kbuf_live_cb);

	phy_tmr_start(mac_timer.send_id, MAC_SEND_INTERVAL_US);
	
	while(1)
	{
		res = osel_event_wait(mac_event_h, OSEL_WAIT_FOREVER);
		if (res == OSEL_EVENT_NONE)
		{
			mac_handler(OSEL_EVENT_GET(mac_event_h, uint16_t));
		}
	}	
}

void mac_tx_cb(void)
{
	uint16_t object = MAC_EVENT_OF_TX;
    DBG_PRINTF("{");
	
	phy_tmr_repeat(mac_timer.send_id);
	
	osel_event_set(mac_event_h, &object);
}

void mac_csma_cb(void)
{
	uint16_t object = MAC_EVENT_CSMA;

	osel_event_set(mac_event_h, &object);	
}

void mac_ofdm_recv_cb(void)
{
	uint32_t crc32 = 0;
	uint16_t object = MAC_EVENT_OF_RX;
	mac_frm_head_t *p_mac_head = PLAT_NULL;
	
	kbuf_t *kbuf = kbuf_alloc(KBUF_BIG_TYPE);

	if (kbuf == PLAT_NULL)
	{
		return;
	}

	phy_ofdm_read(kbuf->base, sizeof(mac_frm_head_t));

	p_mac_head = (mac_frm_head_t *)kbuf->base;

	crc32 = p_mac_head->crc32;
	p_mac_head->crc32 = 0;

	if (crc32 != crc32_tab((uint8_t *)p_mac_head, 0, sizeof(mac_frm_head_t)-sizeof(uint32_t)))
	{
		kbuf_free(kbuf);
		return;
	}
   
	if (p_mac_head->phy+1<=MAX_PHY_OFDM_FRM_MULTI)//max = 1888Bytes
	{
		kbuf->valid_len = (p_mac_head->phy+1)*HAL_RF_OF_REG_MAX_RAM_SIZE;
		p_mac_head->phy = (uint8_t)phy_ofdm_snr();
		phy_ofdm_read(kbuf->base, kbuf->valid_len);
		list_behind_put(&kbuf->list, &mac_ofdm_recv_list);
		osel_event_set(mac_event_h, &object);
	}
	else
	{
		kbuf_free(kbuf);
	}
}

void mac_ofdm_send_cb(void)
{
	if (mac_rdy_snd_kbuf)
	{
		kbuf_free(mac_rdy_snd_kbuf);
		mac_rdy_snd_kbuf = PLAT_NULL;		
	}
	
	phy_tmr_stop(mac_timer.live_id);		
	
    DBG_PRINTF("&");
	//进入接收状态
	phy_ofdm_recv();
}

void mac_rdy_kbuf_live_cb(void)
{
	uint8_t state;

	state = hal_rf_of_get_state();

	if (state == HAL_RF_OF_SEND_M)
	{
		//如果状态还在发送阶段，则等待1ms后
		phy_tmr_start(mac_timer.live_id, 1000);
		return;
	}
	
	if (mac_rdy_snd_kbuf)
	{		
		kbuf_free(mac_rdy_snd_kbuf);
		mac_rdy_snd_kbuf = PLAT_NULL;
	}
	
	phy_tmr_stop(mac_timer.live_id);		
		
	phy_tmr_stop(mac_timer.csma_id);		
    
    DBG_PRINTF("-");
	phy_ofdm_recv();
}


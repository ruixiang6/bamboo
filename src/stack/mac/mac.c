#include <mac.h>
#include <device.h>
#include <phy.h>
#include <app.h>
#include <nwk.h>

#define MAC_TASK_STK_SIZE			256
#define MAC_TASK_PRIO				OSEL_TASK_PRIO(1)

OSEL_DECLARE_TASK(MAC_TASK, param);

osel_task_t *mac_task_h;
osel_event_t *mac_event_h;
mac_timer_t mac_timer;
kbuf_t *mac_rdy_snd_kbuf = PLAT_NULL;

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
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	
	DBG_TRACE("MAC_TASK!\r\n");

	list_init(&mac_ofdm_recv_list);	
	
	for (uint8_t i=0; i<MAC_QOS_LIST_MAX_NUM; i++)
	{
		list_init(&mac_send_entity[i].tx_list);		
		mac_send_entity[i].total_num = 0;
		mac_send_entity[i].total_size = 0;		
	}

	phy_init();
	phy_ofdm_init(mac_ofdm_send_cb, mac_ofdm_recv_cb);
	phy_tmr_init();

	mem_set(&mac_timer, 0, sizeof(mac_timer_t));
	mac_timer.send_id = phy_tmr_alloc(mac_send_cb);
	mac_timer.csma_id = phy_tmr_alloc(mac_csma_cb);
	mac_timer.live_id = phy_tmr_alloc(mac_live_cb);
	mac_timer.idle_id = phy_tmr_alloc(mac_idle_cb);
	mac_timer.idle_state = PLAT_FALSE;

	//在普通模式下通过此定时器触发发送，其余如监听模式则不需要启动此定时器
	if (GET_MODE_ID(p_device_info->id) == MODE_NORMAL)
	{
		phy_tmr_start(mac_timer.send_id, MAC_SEND_INTERVAL_US);
		//phy_tmr_start(mac_timer.live_id, MAC_PKT_LIVE_US);
		//phy_tmr_start(mac_timer.csma_id, MAC_PKT_DIFS_US);
	}
	
	while(1)
	{
		res = osel_event_wait(mac_event_h, OSEL_WAIT_FOREVER);
		if (res == OSEL_EVENT_NONE)
		{
			mac_handler(OSEL_EVENT_GET(mac_event_h, uint16_t));
		}
	}	
}

void mac_send_cb(void)
{
	uint16_t object = MAC_EVENT_OF_TX;
    
	osel_event_set(mac_event_h, &object);
}

void mac_idle_cb(void)
{
	uint16_t object = MAC_EVENT_OF_IDLE;
    
	osel_event_set(mac_event_h, &object);
}


void mac_csma_cb(void)
{
	uint16_t object = MAC_EVENT_CSMA;

	osel_event_set(mac_event_h, &object);

	//DBG_PRINTF("-");
}

void mac_ofdm_recv_cb(void)
{
	uint16_t chksum = 0;
	uint16_t object = MAC_EVENT_OF_RX;
	mac_frm_head_t *p_mac_head = PLAT_NULL;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	kbuf_t *kbuf = kbuf_alloc(KBUF_BIG_TYPE);

	if (kbuf == PLAT_NULL)
	{
		return;
	}
	//这边从alloc中分配的kbuf，offset和base是一致的
	//如果是监听设备，则保留4字节作为应用帧
	if (GET_MODE_ID(p_device_info->id) == MODE_SINFFER)
	{
		kbuf->offset += sizeof(app_sniffer_frm_head_t);
	}
	
	phy_ofdm_read(kbuf->offset, sizeof(mac_frm_head_t));

	p_mac_head = (mac_frm_head_t *)kbuf->offset;

	chksum = p_mac_head->chksum;
	p_mac_head->chksum = 0;

	if (chksum != check16_sum((uint8_t *)p_mac_head, sizeof(mac_frm_head_t)))
	{
		kbuf_free(kbuf);
		return;
	}
   
	if (p_mac_head->phy+1<=MAX_PHY_OFDM_FRM_MULTI)//max = 1888Bytes
	{
		kbuf->valid_len = (p_mac_head->phy+1)*HAL_RF_OF_REG_MAX_RAM_SIZE;
		p_mac_head->phy = (uint8_t)phy_ofdm_snr();
		phy_ofdm_read(kbuf->offset+sizeof(mac_frm_head_t), kbuf->valid_len-sizeof(mac_frm_head_t));

		if (GET_MODE_ID(p_device_info->id) == MODE_SINFFER)
		{
			//只要是监听设备则发到app层处理
			object = APP_EVENT_SNIFFER;
			list_behind_put(&kbuf->list, &app_sniffer.kbuf_rx_list);
			osel_event_set(app_event_h, &object);
		}
		else
		{
			list_behind_put(&kbuf->list, &mac_ofdm_recv_list);
			osel_event_set(mac_event_h, &object);
		}
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
	//进入接收状态
	phy_ofdm_recv();
}

void mac_live_cb(void)
{
	//uint16_t object = MAC_EVENT_OF_LIVE;
	uint8_t state;
	
	state = hal_rf_of_get_state();

	if (state == HAL_RF_OF_SEND_M)
	{
		return;
	}
	
	if (mac_rdy_snd_kbuf)
	{		
		kbuf_free(mac_rdy_snd_kbuf);
		mac_rdy_snd_kbuf = PLAT_NULL;
	}

	DBG_PRINTF("[%d]", mac_timer.csma_difs_cnt);

	mac_timer.csma_difs_cnt = 0;
	mac_timer.csma_slot_cnt = 0;
	phy_tmr_stop(mac_timer.csma_id);
    mac_timer.csma_type = MAC_CSMA_FREE;

	phy_ofdm_recv();
	
	//osel_event_set(mac_event_h, &object);
}


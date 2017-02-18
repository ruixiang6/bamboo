#include <mac.h>
#include <phy.h>
#include <device.h>

list_t *mac_ofdm_send_multi_list[MAC_QOS_LIST_MAX_NUM];
list_t mac_ofdm_recv_list;
list_t mac_ofdm_send_list;

static bool_t mac_ofdm_frame_parse(kbuf_t *kbuf);

bool_t mac_send(kbuf_t *kbuf)
{
	OSEL_DECL_CRITICAL();
	uint16_t object = MAC_EVENT_OF_TX;
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	
	if (kbuf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	if (kbuf->valid_len==0 || kbuf->valid_len>=MAX_PHY_OFDM_FRM_LEN)
	{
		return PLAT_FALSE;
	}

	p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
	//填写MESH_ID
	p_mac_frm_head->mesh_id = GET_MESH_ID(p_device_info->id);
	//填写SRC_DEV_ID
	p_mac_frm_head->src_dev_id = GET_DEV_ID(p_device_info->id);
	//kbuf赋予总长度
	kbuf->valid_len = sizeof(mac_frm_head_t) + p_mac_frm_head->frm_len;
	//最大的长度倍数
	p_mac_frm_head->phy = (kbuf->valid_len-1)/HAL_RF_OF_REG_MAX_RAM_SIZE;
	//前导码+发射时间+切换时间+offset
	p_mac_frm_head->duration = 360+720*(p_mac_frm_head->phy+1)+100+200;
	//计算CRC32
	p_mac_frm_head->crc32 = 0;
	p_mac_frm_head->crc32 = crc32_tab(kbuf->base, 0, sizeof(mac_frm_head_t)-sizeof(uint32_t));

	if (p_mac_frm_head->frm_ctrl.type == MAC_FRM_MGMT_TYPE)
	{
		OSEL_ENTER_CRITICAL();
		list_behind_put(&kbuf->list, mac_ofdm_send_multi_list[0]);
		OSEL_EXIT_CRITICAL();
	}
	else if (p_mac_frm_head->frm_ctrl.type == MAC_FRM_DATA_TYPE)
	{
		OSEL_ENTER_CRITICAL();
		list_behind_put(&kbuf->list, mac_ofdm_send_multi_list[1]);
		OSEL_EXIT_CRITICAL();
	}
	else
	{
		return PLAT_FALSE;
	}

	osel_event_set(mac_event_h, &object);
	
	return PLAT_TRUE;
}

static void mac_of_rx_handler(void)
{
	OSEL_DECL_CRITICAL();
	kbuf_t *kbuf = PLAT_NULL;	

	while (1)
	{
		OSEL_ENTER_CRITICAL();
		kbuf = (kbuf_t *)list_front_get(&mac_ofdm_recv_list);
		OSEL_EXIT_CRITICAL();
		
		if (kbuf)
		{
			mac_ofdm_frame_parse(kbuf);
		}
		else
		{
			return;
		}
	}
}

static void mac_of_tx_handler(void)
{
	OSEL_DECL_CRITICAL();
	static kbuf_t *skbuf = PLAT_NULL;
	static uint8_t delay_time_cnt = 0;
	kbuf_t *kbuf = PLAT_NULL;
	uint8_t loop;
	int8_t cca;
	uint16_t object;
	
	if (skbuf)
	{
		if (delay_time_cnt>=5)
		{
			kbuf_free(skbuf);
			skbuf = PLAT_NULL;
		}
	}
	
	if (skbuf == PLAT_NULL)
	{
		for (loop=0; loop<MAC_QOS_LIST_MAX_NUM; loop++)
		{
			OSEL_ENTER_CRITICAL();
			kbuf = (kbuf_t *)list_front_get(mac_ofdm_send_multi_list[loop]);
			OSEL_EXIT_CRITICAL();
			if (kbuf)
			{
				skbuf = kbuf;
				delay_time_cnt = 0;
				break;
			}
		}

		if (kbuf == PLAT_NULL) return;
	}
	
	if (hal_rf_of_get_state() == HAL_RF_OF_SEND_M)
	{
		object = MAC_EVENT_OF_TX;
		osel_event_set(mac_event_h, &object);
		return;
	}
	//尝试发送
	for(loop=0; loop<5; loop++)
	{
		cca = phy_ofdm_cca();
		if (cca>-68)
		{
			delay_time_cnt++;
			return;
		}
	}
	//可以发送
	if (phy_ofdm_send(skbuf))
	{		
		skbuf = PLAT_NULL;
	}
	else
	{
		delay_time_cnt++;
	}
}

static bool_t mac_ofdm_frame_parse(kbuf_t *kbuf)
{
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	uint16_t object;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	OSEL_DECL_CRITICAL();
	
	if (kbuf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	p_mac_frm_head = (mac_frm_head_t *)kbuf->base;

	//找到NAV位置
	//TODO
	
	if (p_mac_frm_head->mesh_id != GET_MESH_ID(p_device_info->id))
	{
		kbuf_free(kbuf);
		return PLAT_FALSE;
	}

	if (p_mac_frm_head->dest_dev_id == GET_DEV_ID(p_device_info->id)
		|| p_mac_frm_head->dest_dev_id == BROADCAST_ID)
	{
		if (p_mac_frm_head->frm_ctrl.type == MAC_FRM_DATA_TYPE
			|| p_mac_frm_head->frm_ctrl.type == MAC_FRM_MGMT_TYPE)
		{
			object = NWK_EVENT_MESH_RX;
			//把数据偏移到网络层
			kbuf->offset = kbuf->base + sizeof(mac_frm_head_t);
			//kbuf的长度为网络层的长度
			kbuf->valid_len = p_mac_frm_head->frm_len;
			OSEL_ENTER_CRITICAL();
			list_behind_put(&kbuf->list, &nwk_mesh_rx_list);
			OSEL_EXIT_CRITICAL();
			//上至NWK
			osel_event_set(nwk_event_h, &object);
		}
		else
		{
			kbuf_free(kbuf);
			return PLAT_FALSE;
		}
	}
	else
	{
		kbuf_free(kbuf);
		return PLAT_FALSE;
	}
	
	return PLAT_TRUE;
}

void mac_handler(uint16_t event_type)
{
	uint16_t object;
	
	if (event_type & MAC_EVENT_OF_RX) 
	{		
		object = MAC_EVENT_OF_RX;
		osel_event_clear(mac_event_h, &object);		
		mac_of_rx_handler();				
	}
	else if (event_type & MAC_EVENT_OF_TX) 
	{		
		object = MAC_EVENT_OF_TX;
		osel_event_clear(mac_event_h, &object);		
		mac_of_tx_handler();
	}
	else
	{
		//
	}
}

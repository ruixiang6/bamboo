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
	//��дMESH_ID
	p_mac_frm_head->mesh_id = GET_MESH_ID(p_device_info->id);
	//��дSRC_DEV_ID
	p_mac_frm_head->src_dev_id = GET_DEV_ID(p_device_info->id);
	//kbuf�����ܳ���
	kbuf->valid_len = sizeof(mac_frm_head_t) + p_mac_frm_head->frm_len;
	//���ĳ��ȱ���
	p_mac_frm_head->phy = (kbuf->valid_len-1)/HAL_RF_OF_REG_MAX_RAM_SIZE;
	//ǰ����+����ʱ��+�л�ʱ��+offset
	p_mac_frm_head->duration = 360+720*(p_mac_frm_head->phy+1)+100+200;
	//����CRC32
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
		//DBG_PRINTF("+");
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
	kbuf_t *kbuf = PLAT_NULL;
	uint8_t loop;
	int8_t cca;
	
	if (skbuf)
	{
		if (mac_csma_tmr.send_cnt>=5)
		{
			DBG_PRINTF("#");
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
				//DBG_PRINTF("-");
				skbuf = kbuf;
				mac_csma_tmr.send_cnt = 0;
				break;
			}
		}

		if (kbuf == PLAT_NULL) return;
	}
	
	if (hal_rf_of_get_state() == HAL_RF_OF_SEND_M)
	{
		DBG_PRINTF("*");
		return;
	}

	if (mac_csma_tmr.type == MAC_CSMA_FREE)
	{
		if (phy_ofdm_send(skbuf) == PLAT_TRUE)
		{
            DBG_PRINTF("S");
			skbuf = PLAT_NULL;
			return;
		}		

		DBG_PRINTF(".");
		mac_csma_tmr.type = MAC_CSMA_DIFS;
		mac_csma_tmr.send_cnt++;
		phy_tmr_start(2000);//2ms		
	}
    else
    {
        DBG_PRINTF("%d", mac_csma_tmr.type);
    }
}

void mac_csma_handler(void)
{
	uint32_t random_slot;
	uint16_t object = 0;
		
	if (mac_csma_tmr.type == MAC_CSMA_DIFS)
	{
		if (mac_csma_tmr.send_cnt == 0) mac_csma_tmr.send_cnt = 1;
		random_slot = (rand()%(mac_csma_tmr.send_cnt*10))+1;	//����һ��ʱ϶��		
		mac_csma_tmr.type = MAC_CSMA_SLOT;
		phy_tmr_start(random_slot*50);						//50us����һ��ʱ϶
		
		//DBG_PRINTF("D=%d%d|", mac_csma_tmr.send_cnt, random_slot);
	}
	else if (mac_csma_tmr.type == MAC_CSMA_SLOT)
	{
        //DBG_PRINTF("L");
		object = MAC_EVENT_OF_TX;
		osel_event_set(mac_event_h, &object);
		mac_csma_tmr.type = MAC_CSMA_FREE;
	}	
	else
	{
        DBG_PRINTF("T");
		mac_csma_tmr.type = MAC_CSMA_DIFS;		
		phy_tmr_start(2000);//2ms
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

	//�ҵ�NAVλ��
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
			//������ƫ�Ƶ������
			kbuf->offset = kbuf->base + sizeof(mac_frm_head_t);
			//kbuf�ĳ���Ϊ�����ĳ���
			kbuf->valid_len = p_mac_frm_head->frm_len;
			OSEL_ENTER_CRITICAL();
			list_behind_put(&kbuf->list, &nwk_mesh_rx_list);
			OSEL_EXIT_CRITICAL();
			//����NWK
			osel_event_set(nwk_event_h, &object);
			DBG_PRINTF("R");
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
	else if (event_type & MAC_EVENT_CSMA)
	{
		object = MAC_EVENT_CSMA;
		osel_event_clear(mac_event_h, &object);		
		mac_csma_handler();
	}
}

#include <mac.h>
#include <phy.h>
#include <device.h>
#include <app.h>

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
	//kbuf�����ܳ���HAL_RF_OF_REG_MAX_RAM_SIZE����
	kbuf->valid_len = (p_mac_frm_head->phy+1)*HAL_RF_OF_REG_MAX_RAM_SIZE;

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
		DBG_PRINTF("+");
	}
	else if (p_mac_frm_head->frm_ctrl.type == MAC_FRM_TEST_TYPE)
	{
		OSEL_ENTER_CRITICAL();
		list_behind_put(&kbuf->list, mac_ofdm_send_multi_list[2]);
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
	kbuf_t *kbuf = PLAT_NULL;
	uint8_t loop;	
	int8_t cca;
	
	if (mac_rdy_snd_kbuf != PLAT_NULL)
	{
		return;
	}

	for (loop=0; loop<MAC_QOS_LIST_MAX_NUM; loop++)
	{
		OSEL_ENTER_CRITICAL();
		kbuf = (kbuf_t *)list_front_get(mac_ofdm_send_multi_list[loop]);
		OSEL_EXIT_CRITICAL();
		if (kbuf)
		{
			DBG_PRINTF("N");
			mac_rdy_snd_kbuf = kbuf;				
			phy_ofdm_write(mac_rdy_snd_kbuf->base, mac_rdy_snd_kbuf->valid_len);
			OSEL_ENTER_CRITICAL();
			mac_timer.live_id = phy_tmr_start(MAC_PKT_LIVE_US, mac_rdy_kbuf_live_cb);
			OSEL_EXIT_CRITICAL();
			break;
		}
	}
	
	if (kbuf == PLAT_NULL) return;

	cca = phy_ofdm_cca();

	if (cca>MAC_CCA_THREDHOLD)
	{
		DBG_PRINTF("*");
		OSEL_ENTER_CRITICAL();
		mac_timer.csma_type = MAC_CSMA_DIFS;
		mac_timer.csma_id = phy_tmr_start(MAC_PKT_DIFS_US+rand()%MAC_PKT_DIFS_US, mac_csma_cb);
		OSEL_EXIT_CRITICAL();
		mac_timer.csma_difs_cnt = 1;
		return;
	}
	else
	{
		//DBG_PRINTF("L");
		OSEL_ENTER_CRITICAL();
        mac_timer.csma_type = MAC_CSMA_SLOT;
		mac_timer.csma_id = phy_tmr_start(MAC_PKT_SLOT_UNIT_US, mac_csma_cb);
		OSEL_EXIT_CRITICAL();
		mac_timer.csma_slot_cnt = 1;
		return;
	}
}

void mac_csma_handler(void)
{
	OSEL_DECL_CRITICAL();
	int8_t cca;

	switch(mac_timer.csma_type)
	{
		case MAC_CSMA_DIFS:
            //DBG_PRINTF("D");
			cca = phy_ofdm_cca();
			if (cca>MAC_CCA_THREDHOLD)
			{
				DBG_PRINTF("*");
				OSEL_ENTER_CRITICAL();
				mac_timer.csma_type = MAC_CSMA_DIFS;
				phy_tmr_add(mac_timer.csma_id, MAC_PKT_DIFS_US+rand()%MAC_PKT_DIFS_US);				
				OSEL_EXIT_CRITICAL();
			}
			else
			{
				mac_timer.csma_slot_cnt = pow(2, mac_timer.csma_difs_cnt);
				mac_timer.csma_slot_cnt = rand()%mac_timer.csma_slot_cnt;
				OSEL_ENTER_CRITICAL();
				mac_timer.csma_type = MAC_CSMA_SLOT;
				phy_tmr_add(mac_timer.csma_id, MAC_PKT_SLOT_UNIT_US*mac_timer.csma_slot_cnt);				
				OSEL_EXIT_CRITICAL();
			}
			mac_timer.csma_difs_cnt++;
			break;
		case MAC_CSMA_SLOT:            
			cca = phy_ofdm_cca();
			if (cca>MAC_CCA_THREDHOLD)
			{
				DBG_PRINTF("*");
				OSEL_ENTER_CRITICAL();
                mac_timer.csma_type = MAC_CSMA_DIFS;
				phy_tmr_add(mac_timer.csma_id, +rand()%MAC_PKT_DIFS_US);
				OSEL_EXIT_CRITICAL();
				mac_timer.csma_difs_cnt++;
			}
			else
			{
				phy_ofdm_idle();
				OSEL_ENTER_CRITICAL();
                mac_timer.csma_type = MAC_CSMA_RDY;				
				phy_tmr_add(mac_timer.csma_id, MAC_IDLE_TO_SEND_US);
				OSEL_EXIT_CRITICAL();
			}
			break;
		case MAC_CSMA_RDY:
			//DBG_PRINTF("S");
			phy_ofdm_send();
			OSEL_ENTER_CRITICAL();
			phy_tmr_stop(mac_timer.csma_id);
			mac_timer.csma_type = MAC_CSMA_FREE;
			mac_timer.csma_id = 0;
			OSEL_EXIT_CRITICAL();
			break;
		default:
			DBG_TRACE("mac_timer.csma_type=%d\r\n", mac_timer.csma_type);
			break;
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
		else if (p_mac_frm_head->frm_ctrl.type == MAC_FRM_TEST_TYPE)
		{
			object = APP_EVENT_TEST_MAC;
			OSEL_ENTER_CRITICAL();
			list_behind_put(&kbuf->list, &app_recv_list);
			OSEL_EXIT_CRITICAL();
			//����APP
			osel_event_set(app_event_h, &object);
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

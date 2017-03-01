#include <mac.h>
#include <nwk.h>
#include <nwk_eth.h>
#include <nwk_mesh.h>
#include <phy.h>
#include <device.h>
#include <app.h>

mac_send_t mac_send_entity[MAC_QOS_LIST_MAX_NUM];
list_t mac_ofdm_recv_list;

static bool_t mac_ofdm_frame_parse(kbuf_t *kbuf);

bool_t mac_send(kbuf_t *kbuf, mac_send_info_t *p_send_info)
{
	OSEL_DECL_CRITICAL();
	uint16_t object = MAC_EVENT_OF_TX;
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;    
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);    
	
	if (kbuf == PLAT_NULL || p_send_info == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	if (kbuf->valid_len==0 || kbuf->valid_len>=MAX_PHY_OFDM_FRM_LEN-sizeof(mac_frm_head_t))
	{
		return PLAT_FALSE;
	}

	p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
	//��дMESH_ID
	p_mac_frm_head->mesh_id = GET_MESH_ID(p_device_info->id);
	//��дSRC_DEV_ID
	p_mac_frm_head->src_dev_id = p_send_info->src_id;
	//��дDST_DEV_ID
	p_mac_frm_head->dest_dev_id = p_send_info->dest_id;
	//��дSender_ID
	p_mac_frm_head->sender_id = p_send_info->sender_id;
	//��дTarget_ID
	p_mac_frm_head->target_id = p_send_info->target_id;
	//��д���
	p_mac_frm_head->seq_ctrl.seq_num = p_send_info->seq_num;
	//����غɳ���
	p_mac_frm_head->frm_len = kbuf->valid_len;
	//kbuf�����ܳ���
	kbuf->valid_len = sizeof(mac_frm_head_t) + p_mac_frm_head->frm_len;
	//���ĳ��ȱ���
	p_mac_frm_head->phy = (kbuf->valid_len-1)/HAL_RF_OF_REG_MAX_RAM_SIZE;
	//ǰ����+����ʱ��+�л�ʱ��+offset
	//p_mac_frm_head->duration = 360+720*(p_mac_frm_head->phy+1)+100+200;
	p_mac_frm_head->duration = 0;
	//����checksum
	p_mac_frm_head->chksum = 0;
	p_mac_frm_head->chksum = check16_sum(kbuf->base, sizeof(mac_frm_head_t));
	//kbuf�����ܳ���HAL_RF_OF_REG_MAX_RAM_SIZE����
	kbuf->valid_len = (p_mac_frm_head->phy+1)*HAL_RF_OF_REG_MAX_RAM_SIZE;
	//��дQos��Ӧ�Ķ��к�֡����todo
	p_mac_frm_head->frm_ctrl.type = p_send_info->type;
    
	if (MAC_FRM_TYPE_QOS(p_mac_frm_head->frm_ctrl.type) == QOS_H)
	{
		OSEL_ENTER_CRITICAL();
		list_behind_put(&kbuf->list, &mac_send_entity[0].tx_list);
		mac_send_entity[0].total_size += kbuf->valid_len;
		mac_send_entity[0].total_num++;
		OSEL_EXIT_CRITICAL();
	}
	else if (MAC_FRM_TYPE_QOS(p_mac_frm_head->frm_ctrl.type) == QOS_M)
	{
        OSEL_ENTER_CRITICAL();
        list_behind_put(&kbuf->list, &mac_send_entity[1].tx_list);
        mac_send_entity[1].total_size += kbuf->valid_len;
        mac_send_entity[1].total_num++;
        OSEL_EXIT_CRITICAL();	
	}
	else if (MAC_FRM_TYPE_QOS(p_mac_frm_head->frm_ctrl.type) == QOS_L)
	{
		OSEL_ENTER_CRITICAL();
		list_behind_put(&kbuf->list, &mac_send_entity[2].tx_list);
		mac_send_entity[2].total_size += kbuf->valid_len;
		mac_send_entity[2].total_num++;
		OSEL_EXIT_CRITICAL();
	}
	else
	{
		return PLAT_FALSE;
	}

	if (mac_timer.idle_state == PLAT_FALSE)
	{
		osel_event_set(mac_event_h, &object);
	}
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
	bool_t cca_flag;
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	uint32_t tmp;

	if (mac_timer.idle_state == PLAT_FALSE)
	{
    	phy_tmr_repeat(mac_timer.send_id);
	}
	
	if (mac_rdy_snd_kbuf != PLAT_NULL)
	{
		mac_timer.csma_difs_cnt++;
		if (mac_timer.csma_difs_cnt>(MAC_PKT_LIVE_US/MAC_PKT_DIFS_US))
		{
			phy_tmr_stop(mac_timer.live_id);
			DBG_PRINTF("(%d)", mac_timer.csma_difs_cnt);
			kbuf_free(mac_rdy_snd_kbuf);
			mac_rdy_snd_kbuf = PLAT_NULL;
			mac_timer.csma_difs_cnt = 0;
			mac_timer.csma_slot_cnt = 0;
			phy_tmr_stop(mac_timer.csma_id);
			mac_timer.csma_type = MAC_CSMA_FREE;
    		phy_ofdm_recv();
		}
		return;
	}

	for (loop=0; loop<MAC_QOS_LIST_MAX_NUM; loop++)
	{
		OSEL_ENTER_CRITICAL();
		kbuf = (kbuf_t *)list_front_get(&mac_send_entity[loop].tx_list);			
		if (kbuf)
		{
			mac_send_entity[loop].total_size -= kbuf->valid_len;
			mac_send_entity[loop].total_num--;			
			OSEL_EXIT_CRITICAL();				
			mac_rdy_snd_kbuf = kbuf;				
			phy_ofdm_write(mac_rdy_snd_kbuf->base, mac_rdy_snd_kbuf->valid_len);			
			phy_tmr_start(mac_timer.live_id, MAC_PKT_LIVE_US);
			//DBG_PRINTF("+");
			break;
		}
		OSEL_EXIT_CRITICAL();
	}
	
	if (kbuf == PLAT_NULL) return;

	cca_flag = phy_ofdm_cca();

	if (cca_flag == PLAT_FALSE)
	{				
		mac_timer.csma_type = MAC_CSMA_DIFS;
		tmp = MAC_PKT_DIFS_US*(rand()%4)+MAC_PKT_DIFS_US;
		//DBG_PRINTF("!=%d,", tmp);
		phy_tmr_start(mac_timer.csma_id, tmp);
		//DBG_PRINTF("0");
		mac_timer.csma_difs_cnt = 1;
		mac_timer.csma_slot_cnt = 0;
		return;
	}
	else
	{ 
        mac_timer.csma_type = MAC_CSMA_SLOT;
		phy_tmr_start(mac_timer.csma_id, MAC_PKT_SLOT_UNIT_US);
		//DBG_PRINTF("1");
		mac_timer.csma_difs_cnt = 0;
		mac_timer.csma_slot_cnt = 1;
		return;
	}
}

void mac_csma_handler(void)
{
	bool_t cca_flag;	
	uint32_t tmp;
	
	switch(mac_timer.csma_type)
	{
		case MAC_CSMA_DIFS:             
			cca_flag = phy_ofdm_cca();
			if (cca_flag == PLAT_FALSE)
			{				
				mac_timer.csma_type = MAC_CSMA_DIFS;
				tmp = MAC_PKT_DIFS_US*(rand()%4)+MAC_PKT_DIFS_US;
				//DBG_PRINTF("@=%d,", tmp);
				phy_tmr_start(mac_timer.csma_id, tmp);
				//DBG_PRINTF("2");
			}
			else
			{
                tmp = pow(2, mac_timer.csma_difs_cnt);
				mac_timer.csma_slot_cnt = rand() % tmp;
				mac_timer.csma_type = MAC_CSMA_SLOT;
				tmp = MAC_PKT_SLOT_UNIT_US*mac_timer.csma_slot_cnt+MAC_PKT_SLOT_UNIT_US;
				//DBG_PRINTF("$=%d", tmp);
				phy_tmr_start(mac_timer.csma_id, tmp);
				//DBG_PRINTF("3");
			}
			mac_timer.csma_difs_cnt++;
			break;
		case MAC_CSMA_SLOT:            
			cca_flag = phy_ofdm_cca();
			if (cca_flag == PLAT_FALSE)
			{						
                mac_timer.csma_type = MAC_CSMA_DIFS;
				tmp = MAC_PKT_DIFS_US*(rand()%4)+MAC_PKT_DIFS_US;
				//DBG_PRINTF("#=%d", tmp);		
				phy_tmr_start(mac_timer.csma_id, tmp);
				//DBG_PRINTF("4");
				mac_timer.csma_difs_cnt++;
			}
			else
			{
				phy_ofdm_idle();				
                mac_timer.csma_type = MAC_CSMA_RDY;
				phy_tmr_start(mac_timer.csma_id, MAC_IDLE_TO_SEND_US);
				//DBG_PRINTF("5");
			}
			break;
		case MAC_CSMA_RDY:
#if 0
			//����idle̬			 
			if (mac_timer.idle_state == PLAT_FALSE && mac_rdy_snd_kbuf)
			{
				p_mac_frm_head = (mac_frm_head_t *)mac_rdy_snd_kbuf->base;
				if (p_mac_frm_head->duration)
				{
					//idleʱ����ڣ��ɷ�������
					phy_tmr_start(mac_timer.idle_id, p_mac_frm_head->duration);
					mac_timer.idle_us = p_mac_frm_head->duration;
				}
			}
#endif
			phy_tmr_stop(mac_timer.live_id);
			phy_ofdm_send();			
			phy_tmr_stop(mac_timer.csma_id);
			//DBG_PRINTF("S%d", mac_timer.csma_difs_cnt);
			mac_timer.csma_type = MAC_CSMA_FREE;			
			break;
		default:
			//DBG_TRACE("mac_timer.csma_type=%d\r\n", mac_timer.csma_type);
			break;
	}
}

static void mac_of_idle_handler(void)
{
	uint16_t object = MAC_EVENT_OF_TX;
	
	if (mac_timer.idle_state == PLAT_FALSE)
	{
		//idleʱ����ڣ����ɷ�������
		mac_timer.idle_state = PLAT_TRUE;
		phy_tmr_start(mac_timer.idle_id, mac_timer.idle_us);
	}
	else
	{
		//idleʱ����ڣ��ɷ�������
		mac_timer.idle_state = PLAT_FALSE;
		osel_event_set(mac_event_h, &object);
	}
}

static void mac_of_live_handler(void)
{
	
}


static bool_t mac_ofdm_frame_parse(kbuf_t *kbuf)
{
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	uint16_t object;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	kbuf_t *kbuf_copy = PLAT_NULL;
	uint8_t qos;
	OSEL_DECL_CRITICAL();
	
	if (kbuf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
	//������ƫ�Ƶ������
	kbuf->offset = kbuf->base + sizeof(mac_frm_head_t);
#if 0
	//�ҵ�NAVλ��
	if (p_mac_frm_head->duration && mac_timer.idle_state == PLAT_FALSE)
	{
		phy_tmr_start(mac_timer.idle_id, p_mac_frm_head->duration);
		OSEL_ENTER_CRITICAL();
		mac_timer.idle_us = p_mac_frm_head->duration;
		mac_timer.idle_state = PLAT_TRUE;
		OSEL_EXIT_CRITICAL();
	}
#endif
	if (p_mac_frm_head->mesh_id != GET_MESH_ID(p_device_info->id))
	{
		kbuf_free(kbuf);
		return PLAT_FALSE;
	}

	if (p_mac_frm_head->dest_dev_id == GET_DEV_ID(p_device_info->id)
		|| p_mac_frm_head->dest_dev_id == BROADCAST_ID)
	{
		if (MAC_FRM_TYPE_PROB(p_mac_frm_head->frm_ctrl.type) == PROB)
		{
			kbuf_copy = kbuf_alloc(KBUF_BIG_NUM);
			if (kbuf_copy)
			{
				p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
				kbuf_copy->offset = kbuf_copy->base+sizeof(mac_frm_head_t);
				kbuf_copy->valid_len = sizeof(probe_data_t);
				
				mem_cpy(kbuf_copy->base, kbuf->base, sizeof(mac_frm_head_t));
				//��ȥ
				p_mac_frm_head->frm_len = p_mac_frm_head->frm_len-sizeof(probe_data_t);
				//copy��ctrl_data_t�������� 	
				mem_cpy(kbuf_copy->offset, kbuf->offset+p_mac_frm_head->frm_len-1, sizeof(probe_data_t));
				//��֡���ͱ�Ϊ��PROB�����ݰ�
				p_mac_frm_head->frm_ctrl.type = MAC_FRM_TYPE_ASM(0,PROB,0,0);
				nwk_mesh_recv_put(kbuf_copy);
			}
		}

		p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
		//������ƫ�Ƶ������
		kbuf->offset = kbuf->base + sizeof(mac_frm_head_t);
		
		if (MAC_FRM_TYPE_QOS(p_mac_frm_head->frm_ctrl.type) >= QOS_L
			&& MAC_FRM_TYPE_QOS(p_mac_frm_head->frm_ctrl.type) <= QOS_H)
		{
            if (MAC_FRM_TYPE_PROB(p_mac_frm_head->frm_ctrl.type) == PROB)
			{
				qos = MAC_FRM_TYPE_QOS(p_mac_frm_head->frm_ctrl.type);
				//��֡���ͱ�Ϊ��QOS�����ݰ�	
				p_mac_frm_head->frm_ctrl.type = MAC_FRM_TYPE_ASM(0,0,0,qos);
				//��֡����Ϊ��ͨ���ݳ���
				p_mac_frm_head->frm_len -= sizeof(probe_data_t);
			}
			//kbuf�ĳ���Ϊ�����ĳ���
			kbuf->valid_len = p_mac_frm_head->frm_len;
			nwk_mesh_recv_put(kbuf);
		}
		else if (MAC_FRM_TYPE_TEST(p_mac_frm_head->frm_ctrl.type) == TEST)
		{
			object = APP_EVENT_TEST_MAC;
			OSEL_ENTER_CRITICAL();
			list_behind_put(&kbuf->list, &app_recv_list);
			OSEL_EXIT_CRITICAL();
			//����APP
			osel_event_set(app_event_h, &object);
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
	else if (event_type & MAC_EVENT_OF_IDLE)
	{
		object = MAC_EVENT_OF_IDLE;
		osel_event_clear(mac_event_h, &object);		
		mac_of_idle_handler();
	}
	else if (event_type & MAC_EVENT_OF_LIVE)
	{
		object = MAC_EVENT_OF_LIVE;
		osel_event_clear(mac_event_h, &object);		
		mac_of_live_handler();
	}
}

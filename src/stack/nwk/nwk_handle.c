#include <platform.h>
#include <kbuf.h>
#include <nwk.h>
#include <nwk_eth.h>
#include <nwk_mesh.h>
#include <route.h>
#include <mac.h>
#include <device.h>



//return 1:send to mesh
//return 2:send to local_ip
//return 4:send to eth
uint8_t nwk_pkt_transfer(uint8_t src_type, kbuf_t *kbuf, mac_send_info_t *p_msi)
{
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	eth_hdr_t *p_eth_hdr = PLAT_NULL;
	etharp_hdr_t *p_etharp_hdr = PLAT_NULL;
	ip_hdr_t *p_ip_hdr = PLAT_NULL;
	ip_addr_t ipaddr, netmask;
    uint16_t ipaddr2_0, ipaddr2_1;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);

	//���ݰ������������豸����PC
	if (src_type == SRC_ETH)
	{
		p_eth_hdr = (eth_hdr_t *)kbuf->offset;
		
		//�������յ����ݣ���Դmac��pc�ض����ӱ��ڵ㣬���Ե�ַ������Ӵ�����Ӧ��
		addr_table_add(p_eth_hdr->src.addr, GET_DEV_ID(p_device_info->id));
			
		//�ȹ��ˣ����Զ��mac��Ҫ����
		if (p_device_info->remote_eth_mac_addr[0] != 0xFF
			&& p_device_info->remote_eth_mac_addr[1] != 0xFF
			&& p_device_info->remote_eth_mac_addr[2] != 0xFF
			&& p_device_info->remote_eth_mac_addr[3] != 0xFF
			&& p_device_info->remote_eth_mac_addr[4] != 0xFF
			&& p_device_info->remote_eth_mac_addr[5] != 0xFF)
		{
			if (p_eth_hdr->src.addr[0] != p_device_info->remote_eth_mac_addr[0]
				|| p_eth_hdr->src.addr[1] != p_device_info->remote_eth_mac_addr[1]
				|| p_eth_hdr->src.addr[2] != p_device_info->remote_eth_mac_addr[2]
				|| p_eth_hdr->src.addr[3] != p_device_info->remote_eth_mac_addr[3]
				|| p_eth_hdr->src.addr[4] != p_device_info->remote_eth_mac_addr[4]
				|| p_eth_hdr->src.addr[5] != p_device_info->remote_eth_mac_addr[5])
			{
				return 0;
			}			
		}
		///////////////////////////////////////
		if (p_eth_hdr->dest.addr[0] == p_device_info->local_eth_mac_addr[0]
			&& p_eth_hdr->dest.addr[1] == p_device_info->local_eth_mac_addr[1]
			&& p_eth_hdr->dest.addr[2] == p_device_info->local_eth_mac_addr[2]
			&& p_eth_hdr->dest.addr[3] == p_device_info->local_eth_mac_addr[3]
			&& p_eth_hdr->dest.addr[4] == p_device_info->local_eth_mac_addr[4]
			&& p_eth_hdr->dest.addr[5] == p_device_info->local_eth_mac_addr[5])

		{
			return DEST_IP;
		}
		else if (p_eth_hdr->dest.addr[0] == 0xFF
			&& p_eth_hdr->dest.addr[1] == 0xFF
			&& p_eth_hdr->dest.addr[2] == 0xFF
			&& p_eth_hdr->dest.addr[3] == 0xFF
			&& p_eth_hdr->dest.addr[4] == 0xFF
			&& p_eth_hdr->dest.addr[5] == 0xFF)
		{
			switch(htons(p_eth_hdr->type))
			{
				case ETHTYPE_ARP:
					p_etharp_hdr = (etharp_hdr_t *)((uint8_t *)p_eth_hdr+sizeof(eth_hdr_t));
                    ipaddr2_0 = p_device_info->local_ip_addr[1]<<8|p_device_info->local_ip_addr[0];
                    ipaddr2_1 = p_device_info->local_ip_addr[3]<<8|p_device_info->local_ip_addr[2];
					if (p_etharp_hdr->dipaddr.addrw[0] == ipaddr2_0
						&& p_etharp_hdr->dipaddr.addrw[1] == ipaddr2_1)
					{                       
						return DEST_IP;
					}
					else
					{
                         //���ARP�����ȼ�����
                        p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_H);
						p_msi->target_id = BROADCAST_ID;
						return DEST_MESH;
					}
					break;
				case ETHTYPE_IP:
					p_ip_hdr = (ip_hdr_t *)((uint8_t *)p_eth_hdr+sizeof(eth_hdr_t));
					if (IPH_V(p_ip_hdr) != 4) return 0;
					IP4_ADDR(&ipaddr, 
							p_device_info->local_ip_addr[0],
							p_device_info->local_ip_addr[1],
							p_device_info->local_ip_addr[2],
							p_device_info->local_ip_addr[3]);
					IP4_ADDR(&netmask, 
				             p_device_info->local_netmask_addr[0], 
				             p_device_info->local_netmask_addr[1], 
				             p_device_info->local_netmask_addr[2], 
				             p_device_info->local_netmask_addr[3]);
					//IP�㲥��
					if ((p_ip_hdr->dest.addr & ~netmask.addr) == (IPADDR_BROADCAST & ~netmask.addr))
					{
						p_msi->target_id = BROADCAST_ID;
                        //���ȼ��ϵ�
                        p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_L);
						return DEST_IP|DEST_MESH;
					}
					else if (p_ip_hdr->dest.addr == ipaddr.addr)
					{
                        //bug��
						p_msi->target_id = BROADCAST_ID;
						return DEST_MESH;
					}
					else
					{
						return 0;
					}
					break;
				default: return 0;
			}
		}
		else
		{
			//����ǹ㲥����Ҫ���䣬��ѯ��ַ��õ�Ŀ��ڵ�ID
			addr_table_query(p_eth_hdr->dest.addr, &p_msi->target_id);
			if ((p_msi->target_id > 0) && (p_msi->target_id <= NODE_MAX_NUM) && (p_msi->target_id != GET_DEV_ID(p_device_info->id)))
			{
                switch(htons(p_eth_hdr->type))
                {
                    case ETHTYPE_ARP:
                        //�����ȼ�
                        p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_H);
                        break;
                    case ETHTYPE_IP:
                        p_ip_hdr = (ip_hdr_t *)((uint8_t *)p_eth_hdr+sizeof(eth_hdr_t));
                        if (IPH_V(p_ip_hdr) != 4) return 0;
                        if (IPH_PROTO(p_ip_hdr) == IP_PROTO_ICMP || IPH_PROTO(p_ip_hdr) == IP_PROTO_IGMP)
                        {
                            //�����ȼ�
                            p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_H);
                        }
                        else
                        {
                            //�����ȼ�
                            p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_M);
                        }
                        break;
                    default: return 0;
                }
                return DEST_MESH;
			}
            else
			{	
                return 0;
            }
		}
	}
	//���ݰ������ڱ��ڵ��lwipЭ��ջ
	else if (src_type == SRC_IP)
	{
		p_eth_hdr = (eth_hdr_t *)kbuf->offset;
		
		if (p_eth_hdr->dest.addr[0] == 0xFF
			&& p_eth_hdr->dest.addr[1] == 0xFF
			&& p_eth_hdr->dest.addr[2] == 0xFF
			&& p_eth_hdr->dest.addr[3] == 0xFF
			&& p_eth_hdr->dest.addr[4] == 0xFF
			&& p_eth_hdr->dest.addr[5] == 0xFF)
		{
            if (htons(p_eth_hdr->type) == ETHTYPE_ARP)
            {
                //�����ȼ�
                p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_H);
            }
            else
            {
                //���ȼ��ϵ�
                p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_L);
            }
			p_msi->target_id = BROADCAST_ID;
			return DEST_MESH | DEST_ETH;
		}
		else
		{
			//����ǹ㲥����Ҫ���䣬��ѯ��ַ��õ�Ŀ��ڵ�ID��������Լ�����������������������תmesh
			addr_table_query(p_eth_hdr->dest.addr, &p_msi->target_id);
			if ((p_msi->target_id > 0) && (p_msi->target_id <= NODE_MAX_NUM))
			{
				if(p_msi->target_id == GET_DEV_ID(p_device_info->id))
				{
                    return DEST_ETH;
				}
                else					
				{
                    switch(htons(p_eth_hdr->type))
                    {
                        case ETHTYPE_ARP:
                            //�����ȼ�
                            p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_H);
                            break;
                        case ETHTYPE_IP:
                            p_ip_hdr = (ip_hdr_t *)((uint8_t *)p_eth_hdr+sizeof(eth_hdr_t));
                            if (IPH_V(p_ip_hdr) != 4) return 0;
                            if (IPH_PROTO(p_ip_hdr) == IP_PROTO_ICMP || IPH_PROTO(p_ip_hdr) == IP_PROTO_IGMP)
                            {
                                //�����ȼ�
                                p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_H);
                            }
                            else
                            {
                                //�����ȼ�
                                p_msi->type = MAC_FRM_TYPE_ASM(0, 0, 0, QOS_M);
                            }
                            break;
                        default: return 0;
                    }
                    return DEST_MESH;
                }
			}
			else
			{
                return 0;
            }
		}
	}
	//���ݰ�������mesh���߽��գ���Ҫ���Ƕ��ֳ��ڣ����ת����Ӧ����ԭͷ����Ϣ
	else if (src_type == SRC_MESH)
	{
		p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
		p_eth_hdr = (eth_hdr_t *)kbuf->offset;

		p_msi->sender_id = p_mac_frm_head->sender_id;
		p_msi->target_id = p_mac_frm_head->target_id;
		p_msi->src_id = p_mac_frm_head->src_dev_id;
		p_msi->dest_id = p_mac_frm_head->dest_dev_id;
		p_msi->seq_num = p_mac_frm_head->seq_ctrl.seq_num;
		p_msi->type = p_mac_frm_head->frm_ctrl.type;
		p_msi->snr = p_mac_frm_head->phy;

		if (MAC_FRM_TYPE_PROB(p_msi->type) == PROB)
		{
			if ((p_msi->sender_id == p_msi->src_id) && (p_msi->target_id == p_msi->dest_id) && (p_msi->target_id == BROADCAST_ID))
			{
				return DEST_MGMT;
			}
			else
			{
                return 0;
            }
		}

		if (p_eth_hdr->dest.addr[0] == p_device_info->local_eth_mac_addr[0]
			&& p_eth_hdr->dest.addr[1] == p_device_info->local_eth_mac_addr[1]
			&& p_eth_hdr->dest.addr[2] == p_device_info->local_eth_mac_addr[2]
			&& p_eth_hdr->dest.addr[3] == p_device_info->local_eth_mac_addr[3]
			&& p_eth_hdr->dest.addr[4] == p_device_info->local_eth_mac_addr[4]
			&& p_eth_hdr->dest.addr[5] == p_device_info->local_eth_mac_addr[5])

		{
			return DEST_IP;
		}
		else if (p_eth_hdr->dest.addr[0] == 0xFF
			&& p_eth_hdr->dest.addr[1] == 0xFF
			&& p_eth_hdr->dest.addr[2] == 0xFF
			&& p_eth_hdr->dest.addr[3] == 0xFF
			&& p_eth_hdr->dest.addr[4] == 0xFF
			&& p_eth_hdr->dest.addr[5] == 0xFF)
		{
			//����˹㲥�Ѿ����չ��ˣ����ٴ����������޴ε�ת��
			if (broadcast_rcv_table_judge(p_msi->sender_id, p_msi->seq_num))
			{
				return 0;
			}
			else
			{
				broadcast_rcv_table_add(p_msi->sender_id, p_msi->seq_num);
			}
			
			switch(htons(p_eth_hdr->type))
			{
				case ETHTYPE_ARP:
					
					//��mesh�����յ�ARP�㲥���ݣ���¼Դmac��pc����ڵ㣬��ַ������Ӵ�����Ӧ��
					addr_table_add(p_eth_hdr->src.addr, p_msi->sender_id);
					
					p_etharp_hdr = (etharp_hdr_t *)((uint8_t *)p_eth_hdr+sizeof(eth_hdr_t));
                    ipaddr2_0 = p_device_info->local_ip_addr[1]<<8|p_device_info->local_ip_addr[0];
                    ipaddr2_1 = p_device_info->local_ip_addr[3]<<8|p_device_info->local_ip_addr[2];
					
					if (p_etharp_hdr->dipaddr.addrw[0] == ipaddr2_0
						&& p_etharp_hdr->dipaddr.addrw[1] == ipaddr2_1)
					{
						return DEST_IP;
					}
					else
					{
						return DEST_ETH | DEST_MESH;
					}
					break;
				case ETHTYPE_IP:
					p_ip_hdr = (ip_hdr_t *)((uint8_t *)p_eth_hdr+sizeof(eth_hdr_t));
					if (IPH_V(p_ip_hdr) != 4) return 0;
					IP4_ADDR(&ipaddr, 
							p_device_info->local_ip_addr[0],
							p_device_info->local_ip_addr[1],
							p_device_info->local_ip_addr[2],
							p_device_info->local_ip_addr[3]);
					//IP�㲥������mesh���յ��㲥������Ҫ���Ǳ��أ�������mesh���������ת��
					if ((p_ip_hdr->dest.addr & ~netmask.addr) == (IPADDR_BROADCAST & ~netmask.addr))
					{
						return DEST_ETH|DEST_IP|DEST_MESH;
					}
					else if (p_ip_hdr->dest.addr == ipaddr.addr)
					{
						return DEST_IP;
					}
					else
					{
						return DEST_ETH|DEST_MESH;
					}
					break;
				default: return 0;				
			}
		}
		else
		{
			if(p_msi->target_id == GET_DEV_ID(p_device_info->id))
				return DEST_ETH;
			else					
				return DEST_MESH;
		}
	}
	else
	{
		return 0;
	}
}

//tcpipЭ��ջ�յ����ݺ�Ĵ���ת������
static void nwk_eth_tx_handler(void)
{
	NWK_SEND_ETH_RES_T res = ETH_SEND_EMPTY;
	uint8_t repeat_cnt = 0;
	bool_t flush_flag = PLAT_FALSE;
	
	do
	{
		res = nwk_eth_send(flush_flag);
		if (res == ETH_SEND_FAILED)
		{
			//��ʱ1ms
			osel_systick_delay(1);
			repeat_cnt++;
			if (repeat_cnt > 3)
			{
				flush_flag = PLAT_TRUE;
			}
		}
		else
		{
			flush_flag = PLAT_FALSE;
		}
	} while(res != ETH_SEND_EMPTY);
}


//���������յ����ݺ�Ĵ���
static void nwk_eth_rx_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	uint8_t output_type;
	bool_t ret = PLAT_FALSE;
	mac_send_info_t send_info;
    device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	static uint8_t broadcast_frame_seq = 0;
	
	while (1)
	{
		kbuf = nwk_eth_recv_asyn();
		if (kbuf)
		{
			output_type = nwk_pkt_transfer(SRC_ETH, kbuf, &send_info);
			if (output_type & DEST_IP)
			{
				//�����жϴ����ȷ���ύ����tcpipЭ��ջ
				nwk_tcpip_input(kbuf->offset, kbuf->valid_len);
			}

			if (output_type & DEST_MESH)
			{
				send_info.sender_id = GET_DEV_ID(p_device_info->id);
				send_info.src_id = GET_DEV_ID(p_device_info->id);                
				if (send_info.target_id == BROADCAST_ID)
				{
					send_info.seq_num = broadcast_frame_seq++;
					send_info.dest_id = send_info.target_id;
				}
				else
				{
					send_info.seq_num = 0;
					//��ѯ·�ɱ��õ���һ���ڵ�ID
					send_info.dest_id = route_table_query(send_info.target_id, PLAT_NULL, PLAT_NULL);
					if (send_info.dest_id == 0)
					{
						kbuf_free(kbuf);
						return;
					}
				}

				ret = mac_send(kbuf, &send_info);
				if (!ret)
					kbuf_free(kbuf);
			}
			else
			{
				kbuf_free(kbuf);
			}
		}
		else
		{
			return;
		}
	}
}


//mesh���յ����ݺ�Ĵ���
static void nwk_mesh_rx_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	kbuf_t *kbuf_copy = PLAT_NULL;
	uint8_t output_type;
	bool_t ret = PLAT_FALSE;
	mac_send_info_t send_info;
    device_info_t *p_device_info = device_info_get(PLAT_FALSE);
		
	do
	{		
		kbuf = nwk_mesh_recv_get();		
		if (kbuf)
		{
			output_type = nwk_pkt_transfer(SRC_MESH, kbuf, &send_info);
			//����ǹ�����ư������������֮
			if (output_type & DEST_MGMT)
			{
				probe_frame_parse((probe_data_t *)kbuf->offset, send_info.src_id, send_info.snr);
				continue;
			}
						
			if (output_type & DEST_IP)
			{
				//�����жϴ�����ύ������tcpipЭ��ջ
				nwk_tcpip_input(kbuf->offset, kbuf->valid_len);
			}

			//ͬʱ��Ҫ������·���ض�Ϊ�㲥��
			if ((output_type & DEST_MESH) && (output_type & DEST_ETH) && (send_info.target_id == BROADCAST_ID))
			{
				kbuf_copy = kbuf_alloc(KBUF_BIG_TYPE);
				if (kbuf_copy)
				{
					kbuf_copy->offset = kbuf_copy->base + sizeof(mac_frm_head_t);
					kbuf_copy->valid_len = kbuf->valid_len;
					mem_cpy(kbuf_copy->offset, kbuf->offset, kbuf->valid_len);
			
					//�첽���͸�nwk��eth
					nwk_eth_send_asyn(kbuf_copy);
				}
					
				send_info.src_id = GET_DEV_ID(p_device_info->id);
				send_info.dest_id = send_info.target_id;
				
				ret = mac_send(kbuf, &send_info);
				if (!ret)
					kbuf_free(kbuf);
			
				return;
			}

			if (output_type & DEST_MESH)
			{	
				send_info.src_id = GET_DEV_ID(p_device_info->id);
				//��ѯ·�ɱ��õ���һ���ڵ�ID
				send_info.dest_id = route_table_query(send_info.target_id, PLAT_NULL, PLAT_NULL);
				if (send_info.dest_id == 0)
				{
					kbuf_free(kbuf);
					return;
				}
				
				ret = mac_send(kbuf, &send_info);
				if (!ret)
					kbuf_free(kbuf);
			}
			else if (output_type & DEST_ETH)
			{
				//�첽���͸�nwk��eth
				nwk_eth_send_asyn(kbuf);
			}
			else
			{
				kbuf_free(kbuf);
			}
		}
	}while(kbuf != PLAT_NULL);
}


static void nwk_mesh_timer_handler(void)
{	
	probe_timeout_handle();

	probe_frame_fill();
}


void nwk_handler(uint16_t event_type)
{
	uint16_t object;
	
    if (event_type & NWK_EVENT_ETH_TX)
	{
		object = NWK_EVENT_ETH_TX;
		osel_event_clear(nwk_event_h, &object);
		nwk_eth_tx_handler();
	}
	else if (event_type & NWK_EVENT_ETH_RX)
	{
		object = NWK_EVENT_ETH_RX;
		osel_event_clear(nwk_event_h, &object);
		nwk_eth_rx_handler();
	}
	else if (event_type & NWK_EVENT_MESH_RX)
	{
		object = NWK_EVENT_MESH_RX;
		osel_event_clear(nwk_event_h, &object);
		nwk_mesh_rx_handler();
	}
	else if (event_type & NWK_EVENT_MESH_TIMER)
	{
		object = NWK_EVENT_MESH_TIMER;
		osel_event_clear(nwk_event_h, &object);
		nwk_mesh_timer_handler();
	}
}


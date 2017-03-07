#include <platform.h>
#include <kbuf.h>
#include <nwk.h>
#include <nwk_eth.h>
#include <nwk_mesh.h>
#include <route.h>
#include <mac.h>
#include <device.h>


void nwk_print(void)
{
	neighbor_table_print();
	route_table_print();	
}


//return 1:send to mesh
//return 2:send to local_ip
//return 4:send to eth
uint8_t nwk_pkt_transfer(uint8_t src_type, kbuf_t *kbuf, packet_info_t *pakcet_info)
{
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	eth_hdr_t *p_eth_hdr = PLAT_NULL;
	etharp_hdr_t *p_etharp_hdr = PLAT_NULL;
	ip_hdr_t *p_ip_hdr = PLAT_NULL;
	ip_addr_t ipaddr, netmask;
    uint16_t ipaddr2_0, ipaddr2_1;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);

	//数据包来自内网的设备，如PC
	if (src_type == SRC_ETH)
	{
		p_eth_hdr = (eth_hdr_t *)kbuf->offset;
		
		//从内网收到数据，则源mac的pc必定连接本节点，所以地址表中添加此条对应项
		addr_table_add(p_eth_hdr->src.addr, GET_DEV_ID(p_device_info->id));
			
		//先过滤，如果远端mac需要过滤
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
                         //提高ARP的优先级属性
                        pakcet_info->frm_ctrl.qos_level = QOS_H;
						pakcet_info->target_id = BROADCAST_ID;
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
					//IP广播包
					if ((p_ip_hdr->dest.addr & ~netmask.addr) == (IPADDR_BROADCAST & ~netmask.addr))
					{
						pakcet_info->target_id = BROADCAST_ID;
                        //优先级较低
                        pakcet_info->frm_ctrl.qos_level = QOS_L;
						return DEST_IP|DEST_MESH;
					}
					else if (p_ip_hdr->dest.addr == ipaddr.addr)
					{
						return DEST_IP;
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
			//如果非广播包需要传输，查询地址表得到目标节点ID
			addr_table_query(p_eth_hdr->dest.addr, &pakcet_info->target_id);
			if ((pakcet_info->target_id > 0) && (pakcet_info->target_id <= NODE_MAX_NUM) && (pakcet_info->target_id != GET_DEV_ID(p_device_info->id)))
			{
                switch(htons(p_eth_hdr->type))
                {
                    case ETHTYPE_ARP:
                        //高优先级
                        pakcet_info->frm_ctrl.qos_level = QOS_H;
                        break;
                    case ETHTYPE_IP:
                        p_ip_hdr = (ip_hdr_t *)((uint8_t *)p_eth_hdr+sizeof(eth_hdr_t));
                        if (IPH_V(p_ip_hdr) != 4) return 0;
                        if (IPH_PROTO(p_ip_hdr) == IP_PROTO_ICMP || IPH_PROTO(p_ip_hdr) == IP_PROTO_IGMP)
                        {
                            //高优先级
                            pakcet_info->frm_ctrl.qos_level = QOS_H;
                        }
                        else
                        {
                            //中优先级
                            pakcet_info->frm_ctrl.qos_level = QOS_M;
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
	//数据包来自于本节点的lwip协议栈
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
                //高优先级
                pakcet_info->frm_ctrl.qos_level = QOS_H;
            }
            else
            {
                //优先级较低
               pakcet_info->frm_ctrl.qos_level = QOS_L;
            }
			pakcet_info->target_id = BROADCAST_ID;
			return DEST_MESH | DEST_ETH;
		}
		else
		{
			//如果非广播包需要传输，查询地址表得到目标节点ID，如果是自己则表明传输给内网，否则则转mesh
			addr_table_query(p_eth_hdr->dest.addr, &pakcet_info->target_id);
			if ((pakcet_info->target_id > 0) && (pakcet_info->target_id <= NODE_MAX_NUM))
			{
				if(pakcet_info->target_id == GET_DEV_ID(p_device_info->id))
				{
                    return DEST_ETH;
				}
                else					
				{
                    switch(htons(p_eth_hdr->type))
                    {
                        case ETHTYPE_ARP:
                            //高优先级
                            pakcet_info->frm_ctrl.qos_level = QOS_H;
                            break;
                        case ETHTYPE_IP:
                            p_ip_hdr = (ip_hdr_t *)((uint8_t *)p_eth_hdr+sizeof(eth_hdr_t));
                            if (IPH_V(p_ip_hdr) != 4) return 0;
                            if (IPH_PROTO(p_ip_hdr) == IP_PROTO_ICMP || IPH_PROTO(p_ip_hdr) == IP_PROTO_IGMP)
                            {
                                //高优先级
                                pakcet_info->frm_ctrl.qos_level = QOS_H;
                            }
                            else
                            {
                                //中优先级
                                pakcet_info->frm_ctrl.qos_level = QOS_M;
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
	//数据包来自于mesh无线接收，需要考虑多种出口，如果转发还应保留原头部信息
	else if (src_type == SRC_MESH)
	{
		p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
		p_eth_hdr = (eth_hdr_t *)kbuf->offset;

		pakcet_info->sender_id = p_mac_frm_head->sender_id;
		pakcet_info->target_id = p_mac_frm_head->target_id;
		pakcet_info->src_id = p_mac_frm_head->src_dev_id;
		pakcet_info->dest_id = p_mac_frm_head->dest_dev_id;
		pakcet_info->seq_num = p_mac_frm_head->seq_ctrl.seq_num;
		pakcet_info->frm_ctrl = p_mac_frm_head->frm_ctrl;
		pakcet_info->snr = p_mac_frm_head->phy;

		if (pakcet_info->frm_ctrl.probe_flag == PROBE)
		{
			if ((pakcet_info->sender_id == pakcet_info->src_id) && (pakcet_info->target_id == pakcet_info->dest_id) && (pakcet_info->target_id == BROADCAST_ID))
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
			//如果此广播已经接收过了，则不再处理，避免无限次的转发
			if (broadcast_rcv_table_judge(pakcet_info->sender_id, pakcet_info->seq_num))
			{
				return 0;
			}
			else
			{
				broadcast_rcv_table_add(pakcet_info->sender_id, pakcet_info->seq_num);
			}
			
			switch(htons(p_eth_hdr->type))
			{
				case ETHTYPE_ARP:
					
					//从mesh外网收到ARP广播数据，记录源mac的pc与其节点，地址表中添加此条对应项
					addr_table_add(p_eth_hdr->src.addr, pakcet_info->sender_id);
					
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
					//IP广播，对于mesh端收到广播包，需要考虑本地，内网，mesh三个方向的转发
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
			if (pakcet_info->target_id == GET_DEV_ID(p_device_info->id))
			{
				switch(htons(p_eth_hdr->type))
				{
					case ETHTYPE_ARP:
						
						//从mesh外网收到ARP广播数据，记录源mac的pc与其节点，地址表中添加此条对应项
						addr_table_add(p_eth_hdr->src.addr, pakcet_info->sender_id);
						break;
					default: break;				
				}
				
				return DEST_ETH;
			}
			else
			{
				return DEST_MESH;
			}
		}
	}
	else
	{
		return 0;
	}
}


//tcpip协议栈收到数据后的处理，转置网卡
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
			//延时1ms
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
	} while (res != ETH_SEND_EMPTY);
}


//本地网卡收到数据后的处理
static void nwk_eth_rx_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	uint8_t output_type;
	bool_t ret = PLAT_FALSE;
	packet_info_t packet_info;
    device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	static uint8_t broadcast_frame_seq = 0;
	
	while (1)
	{
		kbuf = nwk_eth_recv_asyn();
		if (kbuf)
		{
            //如果为SNIFFEER直接转到本机ip协议上
            if (GET_MODE_ID(p_device_info->id) == MODE_SINFFER)
            {
                //经过判断处理后，确定提交本地tcpip协议栈
				nwk_tcpip_input(kbuf->offset, kbuf->valid_len);
                kbuf_free(kbuf);
                return;
            }
            
			output_type = nwk_pkt_transfer(SRC_ETH, kbuf, &packet_info);
			if (output_type & DEST_IP)
			{
				//经过判断处理后，确定提交本地tcpip协议栈
				nwk_tcpip_input(kbuf->offset, kbuf->valid_len);
			}

			if (output_type & DEST_MESH)
			{
				packet_info.sender_id = GET_DEV_ID(p_device_info->id);
				packet_info.src_id = GET_DEV_ID(p_device_info->id);                
				if (packet_info.target_id == BROADCAST_ID)
				{
					packet_info.seq_num = broadcast_frame_seq++;
					packet_info.dest_id = packet_info.target_id;
				}
				else
				{
					packet_info.seq_num = 0;
					//查询路由表，得到下一跳节点ID
					packet_info.dest_id = route_table_query(packet_info.target_id, PLAT_NULL, PLAT_NULL);
					if (packet_info.dest_id == 0)
					{
						kbuf_free(kbuf);
						return;
					}
				}

				ret = mac_send(kbuf, &packet_info);
				if (!ret)
				{
					kbuf_free(kbuf);
				}
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


//mesh端收到数据后的处理
static void nwk_mesh_rx_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	kbuf_t *kbuf_copy = PLAT_NULL;
	uint8_t output_type;
	bool_t ret = PLAT_FALSE;
	packet_info_t packet_info;
	probe_data_t *p_probe_data = PLAT_NULL;
	uint16_t chksum = 0;
    device_info_t *p_device_info = device_info_get(PLAT_FALSE);
		
	do
	{		
		kbuf = nwk_mesh_recv_get();		
		if (kbuf)
		{
			output_type = nwk_pkt_transfer(SRC_MESH, kbuf, &packet_info);
			//如果是管理控制包，则解析处理之
			if (output_type & DEST_MGMT)
			{
				p_probe_data = (probe_data_t *)kbuf->offset;
				chksum = p_probe_data->chksum;
				p_probe_data->chksum = 0;
				if (chksum != check16_sum((uint8_t *)p_probe_data, sizeof(probe_data_t)))
				{
					kbuf_free(kbuf);
				}
				else
				{
					probe_frame_parse(p_probe_data, packet_info.src_id, packet_info.snr);
					kbuf_free(kbuf);
				}
				
				continue;
			}
						
			if (output_type & DEST_IP)
			{
				//经过判断处理后，提交到本地tcpip协议栈
				nwk_tcpip_input(kbuf->offset, kbuf->valid_len);
			}

			//同时需要发给两路，必定为广播包
			if ((output_type & DEST_MESH) && (output_type & DEST_ETH) && (packet_info.target_id == BROADCAST_ID))
			{
				kbuf_copy = kbuf_alloc(KBUF_BIG_TYPE);
				if (kbuf_copy)
				{
					kbuf_copy->offset = kbuf_copy->base + sizeof(mac_frm_head_t);
					kbuf_copy->valid_len = kbuf->valid_len;
					mem_cpy(kbuf_copy->offset, kbuf->offset, kbuf->valid_len);
			
					//异步发送给nwk的eth
					nwk_eth_send_asyn(kbuf_copy);
				}
					
				packet_info.src_id = GET_DEV_ID(p_device_info->id);
				packet_info.dest_id = packet_info.target_id;
				
				ret = mac_send(kbuf, &packet_info);
				if (!ret)
				{
					kbuf_free(kbuf);
				}
			
				return;
			}

			if (output_type & DEST_MESH)
			{	
				packet_info.src_id = GET_DEV_ID(p_device_info->id);
				//查询路由表，得到下一跳节点ID
				packet_info.dest_id = route_table_query(packet_info.target_id, PLAT_NULL, PLAT_NULL);
				if (packet_info.dest_id == 0)
				{
					kbuf_free(kbuf);
					return;
				}
				
				ret = mac_send(kbuf, &packet_info);
				if (!ret)
				{
					kbuf_free(kbuf);
				}
			}
			else if (output_type & DEST_ETH)
			{
				//异步发送给nwk的eth
				nwk_eth_send_asyn(kbuf);
			}
			else
			{
				kbuf_free(kbuf);
			}
		}
	}while (kbuf != PLAT_NULL);
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


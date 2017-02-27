#include <platform.h>
#include <kbuf.h>
#include <nwk.h>
#include <nwk_eth.h>
#include <nwk_mesh.h>
#include <mac.h>
#include <device.h>


#if 0
//return 1:send to mesh
//return 2:send to local_ip
//return 4:send to eth
uint8_t nwk_pkt_transfer(uint8_t src_type, kbuf_t *kbuf)
{
	eth_hdr_t *p_eth_hdr = PLAT_NULL;
	etharp_hdr_t *p_etharp_hdr = PLAT_NULL;
	ip_hdr_t *p_ip_hdr = PLAT_NULL;
	ip_addr_t ipaddr, netmask;
    uint16_t ipaddr2_0, ipaddr2_1;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);

	if (src_type == SRC_ETH)
	{
		p_eth_hdr = (eth_hdr_t *)kbuf->offset;
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
						return DEST_IP|DEST_MESH;
					}
					else if (p_ip_hdr->dest.addr == ipaddr.addr)
					{
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
			return DEST_MESH;
		}
	}
	else if (src_type == SRC_IP)
	{
		return DEST_MESH|DEST_ETH;
	}
	else if (src_type == SRC_MESH)
	{
		p_eth_hdr = (eth_hdr_t *)kbuf->offset;
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
						return DEST_ETH;
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
					//IP�㲥��
					if ((p_ip_hdr->dest.addr & ~netmask.addr) == (IPADDR_BROADCAST & ~netmask.addr))
					{
						return DEST_ETH|DEST_IP;
					}
					else if (p_ip_hdr->dest.addr == ipaddr.addr)
					{
						return DEST_IP;
					}
					else
					{
						return DEST_ETH;
					}
					break;
				default: return 0;				
			}
		}
		else
		{
			return DEST_ETH;
		}
	}
	else
	{
		return 0;
	}
}
#else
//return 1:send to mesh
//return 2:send to local_ip
//return 4:send to eth
uint8_t nwk_pkt_transfer(uint8_t src_type, kbuf_t *kbuf, uint8_t *p_dst_id)
{
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	nwk_frm_head_t *p_nwk_frm_head = PLAT_NULL;
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

		//从内网收到数据，则源MAC的设备必定连接本节点，所以地址表中添加此条对应项
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
						*p_dst_id = BROADCAST_ID;
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
						*p_dst_id = BROADCAST_ID;
						return DEST_IP|DEST_MESH;
					}
					else if (p_ip_hdr->dest.addr == ipaddr.addr)
					{
						*p_dst_id = BROADCAST_ID;
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
			addr_table_query(p_eth_hdr->dest.addr, p_dst_id);
			if ((*p_dst_id > 0) && (*p_dst_id <= NODE_MAX_NUM) && (*p_dst_id != GET_DEV_ID(p_device_info->id)))
				return DEST_MESH;
			else
				return 0;
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
			*p_dst_id = BROADCAST_ID;
			return DEST_MESH | DEST_ETH;
		}
		else
		{
			addr_table_query(p_eth_hdr->dest.addr, p_dst_id);
			if ((*p_dst_id > 0) && (*p_dst_id <= NODE_MAX_NUM))
			{
				if(*p_dst_id == GET_DEV_ID(p_device_info->id))
					return DEST_ETH;
				else					
					return DEST_MESH;
			}
			else
				return 0;
		}
	}
	//数据包来自于mesh无线接收
	else if (src_type == SRC_MESH)
	{
		p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
		p_nwk_frm_head = (nwk_frm_head_t *)(kbuf->base + sizeof(mac_frm_head_t));
		p_eth_hdr = (eth_hdr_t *)kbuf->offset;
		
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
					
					//从mesh外网收到ARP广播数据，记录源mac设备与其节点，地址表中添加此条对应项
					addr_table_add(p_eth_hdr->src.addr, p_nwk_frm_head->src_id);
					
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
						*p_dst_id = BROADCAST_ID;
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
					//IP�㲥��
					if ((p_ip_hdr->dest.addr & ~netmask.addr) == (IPADDR_BROADCAST & ~netmask.addr))
					{
						*p_dst_id = BROADCAST_ID;
						return DEST_ETH|DEST_IP|DEST_MESH;
					}
					else if (p_ip_hdr->dest.addr == ipaddr.addr)
					{
						return DEST_IP;
					}
					else
					{
						*p_dst_id = BROADCAST_ID;
						return DEST_ETH|DEST_MESH;
					}
					break;
				default: return 0;				
			}
		}
		else
		{
			addr_table_query(p_eth_hdr->dest.addr, p_dst_id);
			if ((*p_dst_id > 0) && (*p_dst_id <= NODE_MAX_NUM))
			{
				if(*p_dst_id == GET_DEV_ID(p_device_info->id))
					return DEST_ETH;
				else					
					return DEST_MESH;
			}
			else
				return 0;
		}
	}
	else
	{
		return 0;
	}
}

#endif

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
	} while(res != ETH_SEND_EMPTY);
}


//本地网卡收到数据后的处理
static void nwk_eth_rx_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	uint8_t output_type;
	uint8_t nwk_dst_id = 0;
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	
	while (1)
	{
		kbuf = nwk_eth_recv_asyn();
		if (kbuf)
		{
			output_type = nwk_pkt_transfer(SRC_ETH, kbuf, &nwk_dst_id);
			if (output_type & DEST_IP)
			{
				//经过判断处理后，提交到本地tcpip协议栈
				nwk_tcpip_input(kbuf->offset, kbuf->valid_len);
				//这里没有删除kbuf是因为下面mesh要
			}

			if (output_type & DEST_MESH)
			{
				p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
				//填充长度
				p_mac_frm_head->frm_len = kbuf->valid_len;
				//填充目的地址
				p_mac_frm_head->dest_dev_id = BROADCAST_ID;
				//填充类型
				p_mac_frm_head->frm_ctrl.type = MAC_FRM_DATA_TYPE;
				//发送给mac层
				mac_send(kbuf);
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
	uint8_t output_type;
	uint8_t nwk_dst_id = 0;
		
	do
	{		
		kbuf = nwk_mesh_recv_get();		
		if (kbuf)
		{
			output_type = nwk_pkt_transfer(SRC_MESH, kbuf, &nwk_dst_id);
			
			if (output_type & DEST_IP)
			{
				DBG_PRINTF("P");
				//经过判断处理后，提交到本地tcpip协议栈
				nwk_tcpip_input(kbuf->offset, kbuf->valid_len);
				//这里没有删除kbuf是因为下面eth要
			}
			
			if (output_type & DEST_ETH)
			{
				DBG_PRINTF("E");
				//异步发送给nwk的eth
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
	ctrl_timeout_handle();

	ctrl_frame_send();
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


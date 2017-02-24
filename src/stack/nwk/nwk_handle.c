#include <platform.h>
#include <kbuf.h>
#include <nwk.h>
#include <mac.h>
#include <device.h>

#define DEST_MESH		(1u<<0)
#define DEST_IP			(1u<<1)
#define DEST_ETH		(1u<<2)

#define SRC_MESH		(1u<<0)
#define SRC_IP			(1u<<1)
#define SRC_ETH			(1u<<2)

static uint8_t nwk_pkt_transfer(uint8_t src_type, kbuf_t *kbuf);

void nwk_eth_send_cb(void *arg)
{
	kbuf_t *kbuf = (kbuf_t *)arg;
	
	uint16_t object = NWK_EVENT_ETH_TX;
	
	kbuf_free(kbuf);
	
	osel_event_set(nwk_event_h, &object);
}


void nwk_eth_recv_cb(void *arg)
{
	uint16_t object = NWK_EVENT_ETH_RX;
	
	osel_event_set(nwk_event_h, &object);
}


static void nwk_eth_send_asyn(kbuf_t* kbuf)
{
	uint16_t object = NWK_EVENT_ETH_TX;	

	OSEL_DECL_CRITICAL();

	if (kbuf)
	{
		OSEL_ENTER_CRITICAL();
		list_behind_put(&kbuf->list, &nwk_eth_tx_list);
		OSEL_EXIT_CRITICAL();
		
		osel_event_set(nwk_event_h, &object);
	}
}


static NWK_SEND_ETH_RES_T nwk_eth_send(bool_t flush_flag)
{
	static kbuf_t *skbuf = PLAT_NULL;
	NWK_SEND_ETH_RES_T res;

	OSEL_DECL_CRITICAL();

	if (flush_flag && skbuf)
	{
		kbuf_free(skbuf);
		skbuf = PLAT_NULL;
	}

	if (skbuf == PLAT_NULL)
	{
		OSEL_ENTER_CRITICAL();
		skbuf = (kbuf_t *)list_front_get(&nwk_eth_tx_list);
		OSEL_EXIT_CRITICAL();
	}

	if (skbuf)
	{
		if (hal_eth_send(skbuf))
		{
			res = ETH_SEND_SUCCES;
			skbuf = PLAT_NULL;
		}
		else
		{
			res = ETH_SEND_FAILED;
		}
	}
	else
	{
		res = ETH_SEND_EMPTY;
	}

	return res;
}


static kbuf_t *nwk_eth_recv_asyn(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	
	OSEL_DECL_CRITICAL();

	OSEL_ENTER_CRITICAL();
	kbuf = (kbuf_t *)list_front_get(&nwk_eth_rx_list);
	OSEL_EXIT_CRITICAL();

	return kbuf;
}


//tcpip协议栈的出口
err_t nwk_tcpip_output(nwk_tcpip_t *nwk_tcpip, pbuf_t *p)
{
    pbuf_t *q;
    uint16_t pckt_length = 0u;
    uint32_t kbuf_chain_end = 0u;    
	kbuf_t *kbuf = PLAT_NULL;
	kbuf_t *kbuf_copy = PLAT_NULL;
	uint8_t output_type;
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;

	if (nwk_tcpip == PLAT_NULL || p == PLAT_NULL)
	{
		return ERR_BUF;
	}
	
	kbuf = kbuf_alloc(KBUF_BIG_TYPE);
	if (kbuf == PLAT_NULL)
	{
		DBG_TRACE("kbuf_alloc failed \r\n");
		return ERR_BUF;
	}

	kbuf->offset = kbuf->base + sizeof(mac_frm_head_t);

    q = p;
	
    do {
        mem_cpy(kbuf->offset + pckt_length, q->payload, q->len);
        pckt_length += q->len;
        if (q->len == q->tot_len)
        {
            kbuf_chain_end = 1u;
			kbuf->valid_len = pckt_length;
        }
        else
        {
            q = q->next;
        }
    } while (0u == kbuf_chain_end);
	//这边应该对数据包去
	output_type = nwk_pkt_transfer(SRC_IP, kbuf);
	kbuf_copy = kbuf_alloc(KBUF_BIG_TYPE);
	if (kbuf_copy)
	{
		kbuf_copy->offset = kbuf_copy->base + sizeof(mac_frm_head_t);
		kbuf_copy->valid_len = kbuf->valid_len;
		mem_cpy(kbuf_copy->offset, kbuf->offset, kbuf->valid_len);
	}
	
	if (output_type & DEST_MESH)
	{
		p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
		//填充长度
		p_mac_frm_head->frm_len = kbuf->valid_len;
		//填充目的地址
		p_mac_frm_head->dest_dev_id = BROADCAST_ID;
		//帧类型
		p_mac_frm_head->frm_ctrl.type = MAC_FRM_DATA_TYPE;
		mac_send(kbuf);
	}
	
	if (output_type & DEST_ETH)
	{
		//异步发送给NWK的eth
		nwk_eth_send_asyn(kbuf_copy);
	}
	else
	{
		kbuf_free(kbuf_copy);
	}
    return ERR_OK;
}


//tcpip协议栈的入口
static bool_t nwk_tcpip_input(nwk_tcpip_t *nwk_tcpip, uint8_t *buf, uint32_t size)
{
    eth_hdr_t *ethhdr;
    pbuf_t *p, *q;
	uint16_t length = 0;
	uint16_t len = size;

	if (nwk_tcpip == PLAT_NULL || buf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if (p != PLAT_NULL)
	{
		for (q = p; q != PLAT_NULL; q = q->next)
		{
			mem_cpy((uint8_t *)q->payload, buf + length, q->len);
			length += q->len;
		}
	}
	else
	{
		DBG_TRACE("pbuf_alloc failed \r\n");
		return PLAT_FALSE;
	}

    ethhdr = p->payload;

    switch (htons(ethhdr->type))
	{
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
        if (nwk_tcpip->input(p, nwk_tcpip) != ERR_OK)
        {
           pbuf_free(p);
           p = PLAT_NULL;
		   return PLAT_FALSE;
        }
		else
		{			
			return PLAT_TRUE;
		}
    default:
		//DBG_TRACE("no ethhdr type\r\n");
        pbuf_free(p);
        p = PLAT_NULL;
        return PLAT_FALSE;
    }	
}


//tcpip协议栈底层或是无线端收到数据后的处理，转至无线网络协议栈或是本地网卡
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
	}while(res != ETH_SEND_EMPTY);
}


//本地网卡收到数据后的处理，转至无线网络协议栈或是本地tcpip协议栈
static void nwk_eth_rx_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	uint8_t output_type;
	mac_frm_head_t *p_mac_frm_head = PLAT_NULL;
	
	while (1)
	{
		kbuf = nwk_eth_recv_asyn();
		if (kbuf)
		{
			output_type = nwk_pkt_transfer(SRC_ETH, kbuf);
			if (output_type & DEST_IP)
			{
				//经过判断处理后，确定提交本地tcpip协议栈
				nwk_tcpip_input(&nwk_tcpip, kbuf->offset, kbuf->valid_len);
				//这里没有删除kbuf是下面mesh要使用
			}

			if (output_type & DEST_MESH)
			{
				p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
				//填充长度
				p_mac_frm_head->frm_len = kbuf->valid_len;
				//填充目的地址
				p_mac_frm_head->dest_dev_id = BROADCAST_ID;
				//帧类型
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

static void nwk_mesh_rx_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;	
	uint8_t output_type;
	
	OSEL_DECL_CRITICAL();
	
	do
	{		
		OSEL_ENTER_CRITICAL();
		kbuf = (kbuf_t *)list_front_get(&nwk_mesh_rx_list);
		OSEL_EXIT_CRITICAL();
		
		if (kbuf)
		{
			output_type = nwk_pkt_transfer(SRC_MESH, kbuf);
			
			if (output_type & DEST_IP)
			{
				//DBG_PRINTF("P");
				//经过判断处理后，确定提交本地tcpip协议栈
				nwk_tcpip_input(&nwk_tcpip, kbuf->offset, kbuf->valid_len);
				//这里没有删除kbuf是下面eth要使用
			}
			
			if (output_type & DEST_ETH)
			{
				//DBG_PRINTF("E");
				//异步发送给NWK的eth
				nwk_eth_send_asyn(kbuf);
			}
			else
			{
				kbuf_free(kbuf);
			}
		}
	}while(kbuf != PLAT_NULL);
}


//return 1:send to mesh
//return 2:send to local_ip
//return 4:send to eth
static uint8_t nwk_pkt_transfer(uint8_t src_type, kbuf_t *kbuf)
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
		//先过滤，如果远端mac需要过滤的
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
					//IP广播包
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
					//IP广播包
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
}


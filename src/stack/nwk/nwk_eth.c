#include <device.h>
#include <nwk.h>
#include <mac.h>
#include <nwk_eth.h>
#include <route.h>


//以太网交互的接口队列
static list_t nwk_eth_tx_list;
static list_t nwk_eth_rx_list;
static bool_t nwk_eth_flag = PLAT_FALSE;

//TCPIP交互的结构
static nwk_tcpip_t nwk_tcpip;

static void nwk_eth_send_cb(void *arg)
{
	kbuf_t *kbuf = (kbuf_t *)arg;
	
	uint16_t object = NWK_EVENT_ETH_TX;
	
	kbuf_free(kbuf);
	
	osel_event_set(nwk_event_h, &object);
}


static void nwk_eth_recv_cb(void *arg)
{
	uint16_t object = NWK_EVENT_ETH_RX;
	
	osel_event_set(nwk_event_h, &object);
}


void nwk_eth_send_asyn(kbuf_t* kbuf)
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


NWK_SEND_ETH_RES_T nwk_eth_send(bool_t flush_flag)
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
		if (nwk_eth_flag == PLAT_FALSE)
		{
			kbuf_free(skbuf);
			res = ETH_SEND_SUCCES;
			skbuf = PLAT_NULL;
            return res;
		}
		
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


kbuf_t *nwk_eth_recv_asyn(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	
	OSEL_DECL_CRITICAL();

	OSEL_ENTER_CRITICAL();
	kbuf = (kbuf_t *)list_front_get(&nwk_eth_rx_list);
	OSEL_EXIT_CRITICAL();

	return kbuf;
}


//tcpip协议栈的出口
err_t nwk_tcpip_output(nwk_tcpip_t *p_nwk_tcpip, pbuf_t *p)
{
    pbuf_t *q;
    uint16_t pckt_length = 0u;
    uint32_t kbuf_chain_end = 0u;    
	kbuf_t *kbuf = PLAT_NULL;
	kbuf_t *kbuf_copy = PLAT_NULL;
	uint8_t output_type;
	bool_t ret = PLAT_FALSE;
	packet_info_t send_info;
    device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	static uint8_t broadcast_frame_seq = 0;

	if (p_nwk_tcpip == PLAT_NULL || p == PLAT_NULL)
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
	
	//如果为SNIFFEER直接转到本机以太网口上
	if (GET_MODE_ID(p_device_info->id) == MODE_SINFFER)
	{
		nwk_eth_send_asyn(kbuf);
		return ERR_OK;
	}
    
    mem_clr(&send_info, sizeof(packet_info_t));
    
	output_type = nwk_pkt_transfer(SRC_IP, kbuf, &send_info);
	//同时需要发给两路，且必定为广播包
	if ((output_type & DEST_MESH) && (output_type & DEST_ETH) && (send_info.target_id == BROADCAST_ID))
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
		
        send_info.sender_id = GET_DEV_ID(p_device_info->id);
		send_info.src_id = GET_DEV_ID(p_device_info->id);
        send_info.seq_num = broadcast_frame_seq++;        
        send_info.dest_id = send_info.target_id;
		
		ret = mac_send(kbuf, &send_info);
		if (!ret)
			kbuf_free(kbuf);

		return ERR_OK;
	}
	
	if (output_type & DEST_MESH)
	{
        send_info.sender_id = GET_DEV_ID(p_device_info->id);
		send_info.src_id = GET_DEV_ID(p_device_info->id);
        send_info.seq_num = 0;
        send_info.frm_ctrl.qos_level = QOS_M;
		//查询路由表，得到下一跳节点ID
        send_info.dest_id = route_table_query(send_info.target_id, PLAT_NULL, PLAT_NULL);
		if (send_info.dest_id == 0)
		{
			kbuf_free(kbuf);
			return ERR_VAL;
		}
		
		ret = mac_send(kbuf, &send_info);
		if (!ret)
			kbuf_free(kbuf);
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
	
    return ERR_OK;
}

//tcpip协议栈的入口
bool_t nwk_tcpip_input(uint8_t *buf, uint32_t size)
{
    eth_hdr_t *ethhdr;
    pbuf_t *p, *q;
	uint16_t length = 0;
	uint16_t len = size;

	nwk_tcpip_t *p_nwk_tcpip = &nwk_tcpip;

	if (p_nwk_tcpip == PLAT_NULL || buf == PLAT_NULL)
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
        if (p_nwk_tcpip->input(p, p_nwk_tcpip) != ERR_OK)
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


static void nwk_tcpip_init(void)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	ip_addr_t ipaddr, netmask, gateway;
	
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
    
	IP4_ADDR(&gateway, 
             p_device_info->local_gateway_addr[0], 
             p_device_info->local_gateway_addr[1], 
             p_device_info->local_gateway_addr[2], 
             p_device_info->local_gateway_addr[3]);
	
	netif_add(&nwk_tcpip, &ipaddr, &netmask, &gateway, PLAT_NULL, PLAT_NULL, tcpip_input);	
	
    nwk_tcpip.hwaddr[0] = p_device_info->local_eth_mac_addr[0];
    nwk_tcpip.hwaddr[1] = p_device_info->local_eth_mac_addr[1];
    nwk_tcpip.hwaddr[2] = p_device_info->local_eth_mac_addr[2];
    nwk_tcpip.hwaddr[3] = p_device_info->local_eth_mac_addr[3];
    nwk_tcpip.hwaddr[4] = p_device_info->local_eth_mac_addr[4];
    nwk_tcpip.hwaddr[5] = p_device_info->local_eth_mac_addr[5];

    nwk_tcpip.state = 0;
    nwk_tcpip.name[0] = 'E';
    nwk_tcpip.name[1] = '0';

    nwk_tcpip.num = 1;
    nwk_tcpip.hwaddr_len = ETHARP_HWADDR_LEN; 
	nwk_tcpip.mtu = 1500;
    nwk_tcpip.flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	nwk_tcpip.output = etharp_output;
    nwk_tcpip.linkoutput = nwk_tcpip_output;	

	netif_set_up(&nwk_tcpip);

	netif_set_default(&nwk_tcpip);

	tcpip_init(PLAT_NULL, PLAT_NULL);	
}


static bool_t nwk_eth_hw_init(void)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	hal_eth_mcb_t mcb;
	OSEL_DECL_CRITICAL();

    mcb.mac[0] = p_device_info->local_eth_mac_addr[0];
    mcb.mac[1] = p_device_info->local_eth_mac_addr[1];
    mcb.mac[2] = p_device_info->local_eth_mac_addr[2];
    mcb.mac[3] = p_device_info->local_eth_mac_addr[3];
    mcb.mac[4] = p_device_info->local_eth_mac_addr[4];
    mcb.mac[5] = p_device_info->local_eth_mac_addr[5];

	mcb.pkt_offset = sizeof(mac_frm_head_t);	
	mcb.tx_func = nwk_eth_send_cb;
	mcb.rx_func = nwk_eth_recv_cb;
	mcb.rx_list = &nwk_eth_rx_list;

	OSEL_ENTER_CRITICAL();
	list_init(&nwk_eth_tx_list);
	list_init(&nwk_eth_rx_list);
	OSEL_EXIT_CRITICAL();
	
	return hal_eth_init(&mcb);
}


void nwk_idle_hook(void)
{
	static uint32_t detect_cnt = 0;
	
	if (nwk_eth_flag == PLAT_FALSE)
	{
		nwk_eth_flag = nwk_eth_hw_init();
	}
	else
	{
		detect_cnt++;
		if (detect_cnt==0xFFFFF) 
		{
			detect_cnt = 0;
			nwk_eth_flag = hal_eth_link_state();
			if (nwk_eth_flag == PLAT_FALSE)
			{
				DBG_TRACE("Eth disconnect\r\n");
				nwk_eth_deinit();
			}
		}
	}
}


void nwk_eth_deinit(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	
	OSEL_DECL_CRITICAL();

	OSEL_ENTER_CRITICAL();
	do
	{
		kbuf = (kbuf_t *)list_front_get(&nwk_eth_rx_list);
		if (kbuf) kbuf_free(kbuf);
	} while(kbuf);

	do
	{
		kbuf = (kbuf_t *)list_front_get(&nwk_eth_tx_list);
		if (kbuf) kbuf_free(kbuf);
	} while(kbuf);

	OSEL_EXIT_CRITICAL();
		
	hal_eth_deinit();
}


void nwk_eth_init(void)
{
	mem_set(&nwk_tcpip, 0, sizeof(nwk_tcpip_t));
	
	nwk_tcpip_init();
}


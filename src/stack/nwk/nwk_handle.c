#include <platform.h>
#include <kbuf.h>
#include <nwk.h>


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


//tcpipЭ��ջ�ĳ���
err_t nwk_tcpip_output(nwk_tcpip_t *nwk_tcpip, pbuf_t *p)
{
    pbuf_t *q;
    uint16_t pckt_length = 0u;
    uint32_t kbuf_chain_end = 0u;    
	kbuf_t *kbuf = PLAT_NULL;

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

	kbuf->offset = kbuf->base + 8;

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
	//�첽���͸�NWK��eth
	nwk_eth_send_asyn(kbuf);

    return ERR_OK;
}


//tcpipЭ��ջ�����
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


//tcpipЭ��ջ�ײ�������߶��յ����ݺ�Ĵ���ת����������Э��ջ���Ǳ�������
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
	}while(res != ETH_SEND_EMPTY);
}


//���������յ����ݺ�Ĵ���ת����������Э��ջ���Ǳ���tcpipЭ��ջ
static void nwk_eth_rx_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	
	while (1)
	{
		kbuf = nwk_eth_recv_asyn();
		if (kbuf)
		{
			//�����жϴ����ȷ���ύ����tcpipЭ��ջ
			nwk_tcpip_input(&nwk_tcpip, kbuf->offset, kbuf->valid_len);
			kbuf_free(kbuf);			
		}
		else
		{
			return;
		}
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
}


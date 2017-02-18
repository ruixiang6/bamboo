#include <platform.h>
#include <device.h>
#include <nwk.h>
#include <mac.h>

#define NWK_TASK_STK_SIZE			256
#define NWK_TASK_PRIO				OSEL_TASK_PRIO(2)

OSEL_DECLARE_TASK(NWK_TASK, param);

osel_task_t *nwk_task_h;
osel_event_t *nwk_event_h;
//��̫�������Ľӿڶ���
list_t nwk_eth_tx_list;
list_t nwk_eth_rx_list;
//MESH�����Ľӿڶ���
list_t nwk_mesh_rx_list;
//TCPIP�����Ľṹ
nwk_tcpip_t nwk_tcpip;


OSEL_DECLARE_TASK(NWK_TASK, param)
{
	(void)param;
	osel_event_res_t res;
	
	DBG_TRACE("NWK_TASK!\r\n");

	osel_systick_delay(100);
	
	while (1)
	{		
		res = osel_event_wait(nwk_event_h, OSEL_WAIT_FOREVER);
		if (res == OSEL_EVENT_NONE)
		{
			nwk_handler(OSEL_EVENT_GET(nwk_event_h, uint16_t));
		}
	}
}


static bool_t nwk_eth_init(void)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	hal_eth_mcb_t mcb;	

    mcb.mac[0] = p_device_info->local_eth_mac_addr[0];
    mcb.mac[1] = p_device_info->local_eth_mac_addr[1];
    mcb.mac[2] = p_device_info->local_eth_mac_addr[2];
    mcb.mac[3] = p_device_info->local_eth_mac_addr[3];
    mcb.mac[4] = p_device_info->local_eth_mac_addr[4];
    mcb.mac[5] = p_device_info->local_eth_mac_addr[5];
	//��дƫ����
	mcb.pkt_offset = sizeof(mac_frm_head_t);	
	mcb.tx_func = nwk_eth_send_cb;
	mcb.rx_func = nwk_eth_recv_cb;
	mcb.rx_list = &nwk_eth_rx_list;

	list_init(&nwk_eth_tx_list);
	list_init(&nwk_eth_rx_list);
	
	return hal_eth_init(&mcb);
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


void nwk_init(void)
{
	nwk_task_h = osel_task_create(NWK_TASK, 
    								NULL, 
    								NWK_TASK_STK_SIZE, 
    								NWK_TASK_PRIO);
	DBG_ASSERT(nwk_task_h != PLAT_NULL);
	nwk_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(nwk_event_h != PLAT_NULL);	

	mem_set(&nwk_tcpip, 0, sizeof(nwk_tcpip_t));
	
	nwk_tcpip_init();

	list_init(&nwk_mesh_rx_list);

	DBG_TRACE("nwk_init ok\r\n");
}


void nwk_deinit(void)
{
	kbuf_t *kbuf;
	
	hal_eth_deinit();
	osel_task_delete(nwk_task_h);
	osel_event_delete(nwk_event_h);

	do
	{
		kbuf = (kbuf_t *)list_front_get(&nwk_eth_rx_list);
		if (kbuf) kbuf_free(kbuf);
	}while(kbuf);

	do
	{
		kbuf = (kbuf_t *)list_front_get(&nwk_eth_tx_list);
		if (kbuf) kbuf_free(kbuf);
	}while(kbuf);

	do
	{
		kbuf = (kbuf_t *)list_front_get(&nwk_mesh_rx_list);
		if (kbuf) kbuf_free(kbuf);
	}while(kbuf);
}

void nwk_idle_hook(void)
{
	static bool_t nwk_eth_flag = PLAT_FALSE;

	if (nwk_eth_flag == PLAT_FALSE)
	{
		nwk_eth_flag = nwk_eth_init();
	}
}

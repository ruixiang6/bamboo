#include <platform.h>
#include <mss_gpio.h>
#include <mss_ethernet_mac.h>
#include <mss_ethernet_mac_regs.h>
#include <mss_ethernet_mac_types.h>
#include <mss_ethernet_mac_user_config.h>
#include <mss_hpdma.h>
/*
 * PHY addresses of most common SmartFusion2 boards.
 */
#define KSZ8051MNL_PHY_ADDR                 0x01

/* MAC configuration record */
static MAC_cfg_t eth_config;
static fppv_t eth_tx_cb = PLAT_NULL;
static fppv_t eth_rx_cb = PLAT_NULL;
static uint16_t eth_pkt_offset = 0;
static list_t *eth_rx_list = PLAT_NULL;

static void mac_tx_callback(void * caller_info);
static void mac_rx_callback(uint8_t * p_rx_packet, uint32_t pckt_length, void * caller_info);

bool_t hal_eth_init(hal_eth_mcb_t *eth_mcb)
{    
	kbuf_t *kbuf;
	bool_t res;

	if (eth_mcb == PLAT_NULL)
	{
		return PLAT_FALSE;
	}
	
	hal_gpio_output(GPIO_ETH, 1);
	delay_ms(1000);

	MSS_MAC_cfg_struct_def_init(&eth_config);

	eth_config.interface = MII;
    eth_config.phy_addr = KSZ8051MNL_PHY_ADDR; //M88E1340_PHY_ADDR;
    eth_config.speed_duplex_select = MSS_MAC_ANEG_ALL_SPEEDS;
    eth_config.mac_addr[0] = eth_mcb->mac[0];
    eth_config.mac_addr[1] = eth_mcb->mac[1];
    eth_config.mac_addr[2] = eth_mcb->mac[2];
    eth_config.mac_addr[3] = eth_mcb->mac[3];
    eth_config.mac_addr[4] = eth_mcb->mac[4];
    eth_config.mac_addr[5] = eth_mcb->mac[5];
	//
	eth_tx_cb = eth_mcb->tx_func;
	eth_rx_cb = eth_mcb->rx_func;
	eth_rx_list = eth_mcb->rx_list;
   
    res = MSS_MAC_init(&eth_config);
	if (res == PLAT_FALSE)
	{
		hal_gpio_output(GPIO_ETH, 0);
		return PLAT_FALSE;
	}
   
    MSS_MAC_set_tx_callback(mac_tx_callback);
    MSS_MAC_set_rx_callback(mac_rx_callback);

	eth_pkt_offset = eth_mcb->pkt_offset;
	
	//double buf1
	kbuf = kbuf_alloc(KBUF_BIG_TYPE);
	DBG_ASSERT(kbuf != PLAT_NULL);
	kbuf->offset = kbuf->base + eth_pkt_offset;
	MSS_MAC_receive_pkt(kbuf->offset, kbuf);
	//double buf2
	kbuf = kbuf_alloc(KBUF_BIG_TYPE);
	DBG_ASSERT(kbuf != PLAT_NULL);
	kbuf->offset = kbuf->base + eth_pkt_offset;
	MSS_MAC_receive_pkt(kbuf->offset, kbuf);
    
    return res;
}

bool_t hal_eth_deinit()
{
	MSS_MAC_cfg_struct_def_init(&eth_config);
	MSS_MAC_init(&eth_config);

	hal_gpio_output(GPIO_ETH, 0);

	delay_ms(1000);
	
	eth_tx_cb = PLAT_NULL;
	eth_rx_cb = PLAT_NULL;
    eth_pkt_offset = 0;
	eth_rx_list = PLAT_NULL;
	
    return PLAT_TRUE;
}

bool_t hal_eth_link_state()
{
	MAC_speed_t speed;
	uint8_t fullduplex;

	return MSS_MAC_get_link_status(&speed, &fullduplex);
}

uint16_t hal_eth_send(kbuf_t *kbuf)
{
	if (kbuf == PLAT_NULL) return 0;
    
    if (kbuf->valid_len == 0) return 0;
	
	return MSS_MAC_send_pkt((uint8_t const *)kbuf->offset, kbuf->valid_len, kbuf);
}

/**=============================================================================
 *
 */
static void mac_tx_callback(void *caller_info)
{
    /*
     * caller_info points to g_mac_tx_buffer_used. Signal that content of
     * g_mac_tx_buffer has been sent by the MAC by resetting
     * g_mac_tx_buffer_used.
     */
    if (eth_tx_cb)
    {
    	(eth_tx_cb)((kbuf_t *)caller_info);
    }
	else
	{
		kbuf_free((kbuf_t *)caller_info);
	}
}

/**=============================================================================
    Bottom-half of receive packet handler
*/
void mac_rx_callback(uint8_t *p_rx_packet, uint32_t pckt_length, void *caller_info)
{
	kbuf_t *kbuf = PLAT_NULL;	

	kbuf = kbuf_alloc(KBUF_BIG_TYPE);
	if (kbuf == PLAT_NULL) 
	{
		DBG_TRACE("kbuf full\r\n");
		if (!MSS_MAC_receive_pkt(p_rx_packet, caller_info))
		{
			kbuf_free((kbuf_t*)caller_info);
		}
		return;
	}
	else
	{
		kbuf->offset = kbuf->base+eth_pkt_offset;
		if (!MSS_MAC_receive_pkt(kbuf->offset, kbuf))
		{
			kbuf_free(kbuf);			
		}
	}

	kbuf = (kbuf_t *)caller_info;
	if (kbuf == PLAT_NULL || kbuf->offset != p_rx_packet)
	{
		DBG_TRACE("buf adder error\r\n");
		return;
	}

	if (pckt_length>1514 || pckt_length<60) 
	{
		kbuf_free(kbuf);
		return;
	}
	
   	kbuf->valid_len = pckt_length;

	if (eth_rx_cb)
    {
    	if (eth_rx_list)
    	{
    		list_behind_put(&kbuf->list, eth_rx_list);
    	}
    	(eth_rx_cb)((void *)kbuf);
    }
	else
	{
		kbuf_free(kbuf);
	}	
}
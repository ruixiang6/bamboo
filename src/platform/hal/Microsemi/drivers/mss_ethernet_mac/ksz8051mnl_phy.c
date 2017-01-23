/*******************************************************************************
 * (c) Copyright 2011-2013 Microsemi SoC Products Group.  All rights reserved.
 *
 * Micrel KSZ8051MNL PHY interface driver implementation.
 *
 * SVN $Revision: 5964 $
 * SVN $Date: 2013-11-11 22:09:29 +0530 (Mon, 11 Nov 2013) $
 */
#include "mss_ethernet_phy.h"
#include "mss_ethernet_mac_types.h"
#include "mss_ethernet_mac.h"
#include "../../CMSIS/mss_assert.h"
#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************/
/* Preprocessor Macros                                                    */
/**************************************************************************/

#define BMSR_AUTO_NEGOTIATION_COMPLETE  0x0020u

/***************************************************************************//**
 * Address of the PHY on the MII management interface.
 */
static uint8_t g_phy_addr = 0u;

/**************************************************************************//**
 * 
 */
#define ANEG_REQUESTED          0x80000000u
#define FORCED_CFG_REQUESTED    0x40000000u


/**************************************************************************//**
 * 
 */
void eth_phy_init(MAC_cfg_t * cfg)
{
    g_phy_addr = cfg->phy_addr;
}

/**************************************************************************//**
 * 
 */
void eth_phy_set_link_speed(uint32_t speed_duplex_select)
{
    uint32_t forced_config;
    uint32_t requested_speed;
    uint16_t phy_reg;
    
    forced_config = speed_duplex_select & FORCED_CFG_REQUESTED;
    requested_speed = speed_duplex_select & ~(ANEG_REQUESTED | FORCED_CFG_REQUESTED);
    
    if(forced_config != 0u)
    {
        /* Forced link speed configuration requested. */
        
        /* Set MII_BMCR to requested value. */
        phy_reg = MSS_MAC_read_phy_reg(g_phy_addr, MII_BMCR);
        phy_reg &= ~(BMCR_SPEED100 | BMCR_SPEED1000 | BMCR_ANENABLE);
        phy_reg |= (uint16_t)requested_speed;
        MSS_MAC_write_phy_reg(g_phy_addr, MII_BMCR, phy_reg);
        
        /* Reset PHY for cnahges to take effect. */
        phy_reg |= BMCR_RESET;
        MSS_MAC_write_phy_reg(g_phy_addr, MII_BMCR, phy_reg);
    }
    else
    {
    	/* Set auto-negotiation advertisement. */
    
    	/* Set 10Mbps and 100Mbps advertisement. */
    	phy_reg = MSS_MAC_read_phy_reg(g_phy_addr, MII_ADVERTISE);
    	phy_reg &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                 ADVERTISE_100HALF | ADVERTISE_100FULL);
        phy_reg |= (uint16_t)requested_speed;
        MSS_MAC_write_phy_reg(g_phy_addr, MII_ADVERTISE, phy_reg);
    }
}

/**************************************************************************//**
 * 
 */
void eth_phy_autonegotiate(void)
{
    uint16_t phy_reg;
    uint16_t autoneg_complete;
    volatile uint16_t temp;
    uint32_t delay_cnt = 0;
    
    /* Enable auto-negotiation. */
    phy_reg = 0x9300;
    MSS_MAC_write_phy_reg(g_phy_addr, MII_BMCR, phy_reg);
    
    /* Wait for copper auto-negotiation to complete. */
    do {
        phy_reg = MSS_MAC_read_phy_reg(g_phy_addr, MII_BMSR);
        autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
		delay_cnt++;
		delay_ms(100);
		DBG_PRINTF(".");
		if (delay_cnt>30)
		{
			DBG_PRINTF("No detect ETH connectted\r\n");
			break;
		}
    } while(!autoneg_complete);
}

/**************************************************************************//**
 * 
 */
uint8_t eth_phy_get_link_status
(
    MAC_speed_t * speed,
    uint8_t *     fullduplex
)
{
    uint16_t phy_reg;
    uint16_t link_up;
    uint8_t link_status;

    phy_reg = MSS_MAC_read_phy_reg(g_phy_addr, MII_BMSR);
    link_up = phy_reg & BMSR_LSTATUS;
    
    if(link_up != 0u)
    {
        uint16_t op_mode;
        
        /* Link is up. */
        link_status = 1u;
        
        phy_reg = MSS_MAC_read_phy_reg(g_phy_addr, 0x1E);
        op_mode = phy_reg & 0x0007u;
        switch(op_mode)
        {
            case 1:
                *fullduplex = MSS_MAC_FULLDUPLEX_DISABLE;
                *speed = MAC10MBPS;
            break;
            
            case 2:
                *fullduplex = MSS_MAC_FULLDUPLEX_DISABLE;
                *speed = MAC100MBPS;
            break;
            
            case 5:
                *fullduplex = MSS_MAC_FULLDUPLEX_ENABLE;
                *speed = MAC10MBPS;
            break;
            
            case 6:
                *fullduplex = MSS_MAC_FULLDUPLEX_ENABLE;
                *speed = MAC100MBPS;
            break;
            
            default:
                link_status = 0u;
            break;
        }
    }
    else
    {
        /* Link is down. */
        link_status = 0u;
    }
    
    return link_status;
}

#ifdef __cplusplus
}
#endif

/******************************** END OF FILE ******************************/







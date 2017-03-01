#ifndef __HAL_ETH_H
#define __HAL_ETH_H

#include <kbuf.h>

#pragma pack(1)

typedef struct
{
	uint8_t mac[6];
	uint16_t pkt_offset;
	fppv_t tx_func;
	fppv_t rx_func;
	list_t *rx_list;
}hal_eth_mcb_t;

#pragma pack()

bool_t hal_eth_init(hal_eth_mcb_t *eth_mcb);

bool_t hal_eth_deinit(void);

bool_t hal_eth_link_state(void);

uint16_t hal_eth_send(kbuf_t *kbuf);

#endif

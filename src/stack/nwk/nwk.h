#ifndef __NWK_H
#define __NWK_H

#include <platform.h>
#include <kbuf.h>
#include "lwip/tcpip.h"
#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "netif/etharp.h"

#define NWK_EVENT_ETH_RX		(1u<<0)
#define NWK_EVENT_ETH_TX		(1u<<1)

#define	NODE_MAX_NUM				32


typedef struct netif nwk_tcpip_t;
typedef struct pbuf pbuf_t;
typedef struct eth_hdr eth_hdr_t;
typedef struct ip_hdr ip_hdr_t;

#pragma pack(1)
typedef enum
{
	ETH_SEND_SUCCES,
	ETH_SEND_FAILED,
	ETH_SEND_EMPTY
}NWK_SEND_ETH_RES_T;
#pragma pack()

extern osel_task_t *nwk_task_h;
extern osel_event_t *nwk_event_h;
extern list_t nwk_eth_tx_list;
extern list_t nwk_eth_rx_list;
extern nwk_tcpip_t nwk_tcpip;

void nwk_eth_send_cb(void *arg);
void nwk_eth_recv_cb(void *arg);
err_t nwk_tcpip_output(nwk_tcpip_t *nwk_tcpip, pbuf_t *p);
void nwk_init(void);
void nwk_deinit(void);
void nwk_idle_hook(void);
void nwk_handler(uint16_t event_type);

#endif

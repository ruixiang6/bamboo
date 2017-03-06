#ifndef __NWK_ETH_H
#define __NWK_ETH_H

#include <nwk.h>
#include "lwip/tcpip.h"
#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/ip.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"


typedef struct netif nwk_tcpip_t;
typedef struct pbuf pbuf_t;
typedef struct eth_hdr eth_hdr_t;
typedef struct etharp_hdr etharp_hdr_t;
typedef struct ip_hdr ip_hdr_t;

#pragma pack(1)

typedef enum {
	ETH_SEND_SUCCES,
	ETH_SEND_FAILED,
	ETH_SEND_EMPTY
} NWK_SEND_ETH_RES_T;
	
#pragma pack()


extern void nwk_eth_send_asyn(kbuf_t* kbuf);
extern NWK_SEND_ETH_RES_T nwk_eth_send(bool_t flush_flag);
extern kbuf_t *nwk_eth_recv_asyn(void);
extern bool_t nwk_tcpip_input(uint8_t *buf, uint32_t size);
extern void nwk_idle_hook(void);
extern void nwk_eth_deinit(void);
extern void nwk_eth_init(void);

#endif


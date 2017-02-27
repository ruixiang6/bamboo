#ifndef __NWK_H
#define __NWK_H

#include <platform.h>
#include <kbuf.h>


#define NWK_EVENT_ETH_RX		(1u<<0)
#define NWK_EVENT_ETH_TX		(1u<<1)
#define NWK_EVENT_MESH_RX		(1u<<2)
#define NWK_EVENT_MESH_TIMER	(1u<<3)


#define DEST_MESH		(1u<<0)
#define DEST_IP			(1u<<1)
#define DEST_ETH		(1u<<2)

#define SRC_MESH		(1u<<0)
#define SRC_IP			(1u<<1)
#define SRC_ETH			(1u<<2)

#define NODE_MAX_NUM		32
#define NWK_CTRL_TIMEOUT	200000


#pragma pack(1)

typedef struct _nwk_frm_head_t {
	uint8_t src_id;
	uint8_t dst_id;
	uint8_t seq;
	uint8_t alive:4,
			dump:4;
} nwk_frm_head_t;

#pragma pack()




extern osel_task_t *nwk_task_h;
extern osel_event_t *nwk_event_h;


extern uint8_t nwk_pkt_transfer(uint8_t src_type, kbuf_t *kbuf, uint8_t *p_dst_id);
extern void nwk_handler(uint16_t event_type);
extern void nwk_deinit(void);
extern void nwk_init(void);

#endif

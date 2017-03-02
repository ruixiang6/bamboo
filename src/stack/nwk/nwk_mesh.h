#ifndef __NWK_MESH_H
#define __NWK_MESH_H

#include <nwk.h>
#include <addr.h>
#include <neighbor.h>
#include <route.h>
#include <affiliate.h>

#pragma pack(1)

typedef struct _probe_data_t {
	neighbor_field_t nf;
	route_field_t rf;
	//affiliate_field_t af;
	uint16_t chksum;
} probe_data_t;

#pragma pack()


extern void probe_timeout_handle(void);
extern void probe_frame_parse(probe_data_t *p_probe_data, uint8_t src_id, uint8_t snr);
extern void probe_frame_fill(void);
extern kbuf_t *probe_frame_fetch(void);
extern kbuf_t *nwk_mesh_recv_get(void);
extern void nwk_mesh_recv_put(kbuf_t *kbuf);
extern void nwk_mesh_deinit(void);
extern void nwk_mesh_init(void);

#endif


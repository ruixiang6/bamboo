#ifndef __NWK_MESH_H
#define __NWK_MESH_H

#include <nwk.h>
#include <neighbor.h>
#include <route.h>

#pragma pack(1)

typedef struct _ctrl_data_t {
	neighbor_field_t nf;
	route_field_t rf;
	//affiliate_field_t af;
} ctrl_data_t;

#pragma pack()


extern kbuf_t *nwk_mesh_recv_get(void);
extern void nwk_mesh_recv_put(kbuf_t *kbuf);
extern void nwk_mesh_deinit(void);
extern void nwk_mesh_init(void);

#endif


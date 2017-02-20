#ifndef __CTRL_H
#define __CTRL_H

#include <nwk.h>
#include <neighbor.h>
#include <route.h>

#pragma pack(1)

typedef struct _ctrl_frame_t {
	neighbor_field_t nf;
	route_field_t rf;
	//affiliate_field_t af;
} ctrl_frame_t;

#pragma pack()




#endif


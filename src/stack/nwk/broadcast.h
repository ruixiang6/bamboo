#ifndef __BROADCAST_H
#define __BROADCAST_H

#include <nwk.h>


#pragma pack(1)

typedef struct _broadcast_rcv_table_item_t {
	uint8_t frame_seq;
	uint8_t status;
	uint8_t timeout;
	uint8_t dump;
} broadcast_rcv_table_item_t;

typedef struct _broadcast_rcv_table_t {
	broadcast_rcv_table_item_t item[NODE_MAX_NUM];
} broadcast_rcv_table_t;

#pragma pack()


#endif


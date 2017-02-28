#ifndef __ADDR_H
#define __ADDR_H

#include <nwk.h>


#define ADDR_TABLE_MODE_QUEUE		1

#define ADDR_LENGTH					6
#define ADDR_TABLE_MAX_NUM			200


#pragma pack(1)

#if ADDR_TABLE_MODE_QUEUE
typedef struct _addr_queue_item_t {
	list_t list;
	uint8_t addr[ADDR_LENGTH];
	uint8_t id;
	uint8_t timeout;
} addr_queue_item_t;
#else
typedef struct _addr_table_item_t {
	uint8_t addr[ADDR_LENGTH];
	uint8_t id;
	uint8_t timeout;
} addr_table_item_t;

typedef struct _addr_table_t {
	addr_table_item_t item[ADDR_TABLE_MAX_NUM];
} addr_table_t;
#endif

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


extern void addr_table_add(uint8_t *paddr, uint8_t id);
extern void addr_table_query(uint8_t *paddr, uint8_t *p_id);
extern void addr_table_init(void);
extern void broadcast_rcv_table_add(uint8_t src_id, uint8_t frame_seq);
extern bool_t broadcast_rcv_table_judge(uint8_t src_id, uint8_t frame_seq);
extern void broadcast_rcv_table_init(void);


#endif


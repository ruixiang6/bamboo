#ifndef __ROUTE_H
#define __ROUTE_H

#include <nwk.h>


#define ROUTE_HOP_MAX			16
#define ROUTE_HOP_INVALID		(ROUTE_HOP_MAX + 1)


#pragma pack(1)

typedef struct _route_table_item_t {
	uint8_t dst_id;
	uint8_t next_id;
	uint8_t hop;
	uint8_t timeout;
	uint32_t seq;
} route_table_item_t;

typedef struct _route_table_t {
	route_table_item_t item[NODE_MAX_NUM];
	route_table_item_t item_standby[NODE_MAX_NUM];
} route_table_t;

typedef struct _route_field_item_t {
	uint8_t next_id;
	uint8_t hop;
	uint32_t seq;
} route_field_item_t;

typedef struct _route_field_t {
	route_field_item_t item[NODE_MAX_NUM];
} route_field_t;

#pragma pack()


extern void route_field_handle(route_field_t *p_rf, uint8_t id, uint8_t my_id);
extern void route_field_fill(route_field_t *p_rf, uint8_t my_id);
extern void route_table_del(uint8_t next_id);
extern void route_table_add(uint8_t dst_id, uint8_t next_id, uint8_t hop, uint8_t seq, uint8_t flag);
extern uint8_t route_table_query(uint8_t dst_id, uint8_t *p_hop, uint8_t *p_seq);
extern void route_table_init(void);
extern void route_table_to_app(uint8_t *p);
extern void route_table_print(void);

#endif


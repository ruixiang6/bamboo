#ifndef __NEIGHBOR_H
#define __NEIGHBOR_H

#include <platform.h>
#include <nwk.h>

#define NEIGHBOR_TIMEOUT				3
#define NEIGHBOR_SNR_LEVEL				70


#pragma pack(1)

typedef struct _neighbor_table_item_t {
	uint8_t snr;
	uint8_t snr_tx;
	uint8_t timeout;
} neighbor_table_item_t;

typedef struct _neighbor_table_t {
	neighbor_table_item_t item[NODE_MAX_NUM];
} neighbor_table_t;

typedef struct _neighbor_field_item_t {
	uint8_t snr;
} neighbor_field_item_t;

typedef struct _neighbor_field_t {
	neighbor_field_item_t item[NODE_MAX_NUM];
} neighbor_field_t;

#pragma pack()


extern uint8_t neighbor_field_get_snrtx(neighbor_field_t *p_nf, uint8_t id);
extern void neighbor_field_fill(neighbor_field_t *p_nf);
extern bool_t neighbor_table_flush(uint8_t id);
extern bool_t neighbor_table_add(uint8_t id, uint8_t snr, uint8_t snr_tx);
extern void neighbor_table_init(void);


#endif


#ifndef __NEIGHBOR_H
#define __NEIGHBOR_H

#include <platform.h>
#include <kbuf.h>
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

#pragma pack()


extern void neighbor_field_build(void *pdata);
extern bool_t neighbor_table_flush(uint8_t id);
extern bool_t neighbor_table_add(uint8_t id, uint8_t snr, uint8_t snr_tx);
extern void neighbor_table_init(void);


#endif


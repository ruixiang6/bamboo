#ifndef __AFFILIATE_H
#define __AFFILIATE_H

#include <platform.h>
#include <kbuf.h>
#include <nwk.h>

#define AFFILIATE_DATA_LEN				10

#pragma pack(1)

typedef struct _affiliate_table_item_t {
	uint8_t data[AFFILIATE_DATA_LEN];
} affiliate_table_item_t;

typedef struct _affiliate_table_t {
	affiliate_table_item_t item[NODE_MAX_NUM];
} affiliate_table_t;

#pragma pack()


extern void affiliate_field_build(void *pdata);
extern void affiliate_table_add(uint8_t id, uint8_t *pdata);
extern void affiliate_table_init(void);


#endif


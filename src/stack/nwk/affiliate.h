#ifndef __AFFILIATE_H
#define __AFFILIATE_H

#include <platform.h>
#include <nwk.h>


#define AFFILIATE_DATA_LEN				10

#pragma pack(1)

typedef struct _affiliate_table_item_t {
	uint8_t sdata[AFFILIATE_DATA_LEN];
} affiliate_table_item_t;

typedef struct _affiliate_table_t {
	affiliate_table_item_t item[NODE_MAX_NUM];
} affiliate_table_t;

typedef struct _affiliate_field_item_t {
	uint8_t sdata[AFFILIATE_DATA_LEN];
} affiliate_field_item_t;

typedef struct _affiliate_field_t {
	affiliate_field_item_t item[NODE_MAX_NUM];
} affiliate_field_t;

#pragma pack()


extern uint8_t* affiliate_field_get_pdata(affiliate_field_t *p_af, uint8_t id);
extern void affiliate_field_fill(affiliate_field_t *p_af);
extern void affiliate_table_add(uint8_t id, uint8_t *pdata);
extern void affiliate_table_init(void);


#endif


#include <nwk.h>
#include <affiliate.h>



static affiliate_table_t affiliate_table;


void affiliate_field_build(void *pdata)
{
	uint8_t i = 0;
	uint8_t *p = (uint8_t *)pdata;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		mem_cpy(p + sizeof(affiliate_table.item[i].data) * i, affiliate_table.item[i].data, sizeof(affiliate_table.item[i].data));
	}	
}


void affiliate_table_add(uint8_t id, uint8_t *pdata)
{	
	if ((id > 0 ) && (id <= NODE_MAX_NUM))
	{
		mem_cpy(affiliate_table.item[id-1].data, pdata, sizeof(affiliate_table.item[id-1].data));
	}

	return;
}


void affiliate_table_init(void)
{
	uint8_t i = 0;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		mem_set(affiliate_table.item[i].data, 0, sizeof(affiliate_table.item[i].data));
	}
}


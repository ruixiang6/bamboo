#include <nwk.h>
#include <affiliate.h>



static affiliate_table_t affiliate_table;



uint8_t* affiliate_field_get_pdata(affiliate_field_t *p_af, uint8_t id)
{
	uint8_t *p_data = PLAT_NULL;
	
	if ((id > 0 ) && (id <= NODE_MAX_NUM))
	{
		p_data = p_af->item[id-1].sdata;
	}

	return p_data;
}


void affiliate_field_fill(affiliate_field_t *p_af)
{
	uint8_t i = 0;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		mem_cpy(p_af->item[i].sdata, affiliate_table.item[i].sdata, sizeof(affiliate_table.item[i].sdata));
	}	
}


void affiliate_table_add(uint8_t id, uint8_t *pdata)
{	
	if ((id > 0 ) && (id <= NODE_MAX_NUM))
	{
		mem_cpy(affiliate_table.item[id-1].sdata, pdata, sizeof(affiliate_table.item[id-1].sdata));
	}

	return;
}


void affiliate_table_init(void)
{
	uint8_t i = 0;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		mem_set(affiliate_table.item[i].sdata, 0, sizeof(affiliate_table.item[i].sdata));
	}
}


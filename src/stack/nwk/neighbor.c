#include <nwk.h>
#include <neighbor.h>



static neighbor_table_t neighbor_table;


uint8_t neighbor_field_get_snrtx(neighbor_field_t *p_nf, uint8_t my_id)
{
	uint8_t snrtx = 0;
	
	if ((my_id > 0 ) && (my_id <= NODE_MAX_NUM))
	{
		snrtx = p_nf->item[my_id-1].snr;
	}

	return snrtx;
}


void neighbor_field_fill(neighbor_field_t *p_nf)
{
	uint8_t i = 0;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		p_nf->item[i].snr = neighbor_table.item[i].snr;
	}	
}


bool_t neighbor_table_flush(uint8_t id)
{
	if ((id > 0 ) && (id <= NODE_MAX_NUM))
	{
		if (neighbor_table.item[id-1].timeout > 0)
		{
			neighbor_table.item[id-1].timeout--;
			if (neighbor_table.item[id-1].timeout == 0)
			{
				neighbor_table.item[id-1].snr = 0;	
				neighbor_table.item[id-1].snr_tx = 0;
				
				return PLAT_TRUE;
			}
		}
	}

	return PLAT_FALSE;
}


bool_t neighbor_table_add(uint8_t id, uint8_t snr, uint8_t snr_tx)
{	
	if ((id > 0 ) && (id <= NODE_MAX_NUM))
	{
		neighbor_table.item[id-1].snr = snr;	
		neighbor_table.item[id-1].snr_tx = snr_tx;

		if ((snr > NEIGHBOR_SNR_LEVEL) && (snr_tx > NEIGHBOR_SNR_LEVEL))
		{
			neighbor_table.item[id-1].timeout = NEIGHBOR_TIMEOUT;
			
			return PLAT_TRUE;
		}
	}

	return PLAT_FALSE;
}


void neighbor_table_init(void)
{
	uint8_t i = 0;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		neighbor_table.item[i].snr = 0;
		neighbor_table.item[i].snr_tx = 0;
		neighbor_table.item[i].timeout = 0;
	}
}


void neighbor_table_print(void)
{
	uint8_t i = 0;

	DBG_PRINTF("nb:\r\n");
	for (i = 0; i < 5; i++) 
	{
		DBG_PRINTF("  %x:%d-%d-%d\r\n", 
			i+1, neighbor_table.item[i].snr, neighbor_table.item[i].snr_tx, neighbor_table.item[i].timeout);
	}
}


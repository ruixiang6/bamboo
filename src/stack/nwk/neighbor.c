#include <nwk.h>
#include <neighbor.h>



static neighbor_table_t neighbor_table;


void neighbor_field_build(void *pdata)
{
	uint8_t i = 0;
	uint8_t *p = (uint8_t *)pdata;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		p[i] = neighbor_table.item[i].snr;
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


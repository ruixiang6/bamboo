#include <nwk.h>
#include <addr.h>


#if ADDR_TABLE_MODE_QUEUE

static list_t addr_queue_list;
static pool_t *addr_queue_pool;


static void addr_queue_init(void)
{
	list_init(&addr_queue_list);

	addr_queue_pool = (pool_t *)pool_create(ADDR_TABLE_MAX_NUM, sizeof(addr_queue_item_t));
	DBG_ASSERT(addr_queue_pool != PLAT_NULL);
}


static addr_queue_item_t *addr_queue_alloc(void)
{
	addr_queue_item_t *p_aqi = PLAT_NULL;
	
	p_aqi = pool_alloc(addr_queue_pool);

	return p_aqi;
}


static bool_t addr_queue_free(addr_queue_item_t *p_aqi)
{
	if (p_aqi == PLAT_NULL) 
		return PLAT_FALSE;

	pool_free(addr_queue_pool, p_aqi);

	return PLAT_TRUE;
}


void addr_table_del(uint8_t *paddr)
{
	addr_queue_item_t *p_aqi = PLAT_NULL;

	list_for_each_entry(p_aqi, &addr_queue_list, addr_queue_item_t, list) 
	{
		if ((p_aqi->addr[0] == paddr[0]) && (p_aqi->addr[1] == paddr[1]) && (p_aqi->addr[2] == paddr[2])
			&& (p_aqi->addr[3] == paddr[3]) && (p_aqi->addr[4] == paddr[4]) && (p_aqi->addr[5] == paddr[5]))
		{
			p_aqi->addr[0] = 0;
			p_aqi->addr[1] = 0;
			p_aqi->addr[2] = 0;
			p_aqi->addr[3] = 0;
			p_aqi->addr[4] = 0;
			p_aqi->addr[5] = 0;
		
			p_aqi->id = 0;
		
			list_del(&p_aqi->list);
			addr_queue_free(p_aqi);
		
			return;
		}
	}
}


void addr_table_add(uint8_t *paddr, uint8_t id)
{	
	addr_queue_item_t *p_aqi = PLAT_NULL;

	list_for_each_entry(p_aqi, &addr_queue_list, addr_queue_item_t, list) 
	{
		if (p_aqi)
		{
			if ((p_aqi->addr[0] == paddr[0]) && (p_aqi->addr[1] == paddr[1]) && (p_aqi->addr[2] == paddr[2])
				&& (p_aqi->addr[3] == paddr[3]) && (p_aqi->addr[4] == paddr[4]) && (p_aqi->addr[5] == paddr[5]))
			{
				p_aqi->id = id;

				return;
			}
		}
	}

	p_aqi = addr_queue_alloc();
	if (p_aqi) 
	{
		p_aqi->addr[0] == paddr[0];
		p_aqi->addr[1] == paddr[1];
		p_aqi->addr[2] == paddr[2];
		p_aqi->addr[3] == paddr[3];
		p_aqi->addr[4] == paddr[4];
		p_aqi->addr[5] == paddr[5];
		
		p_aqi->id = id;
		
		list_behind_put(&p_aqi->list, &addr_queue_list);
	}
}


void addr_table_query(uint8_t *paddr, uint8_t *p_id)
{
	addr_queue_item_t *p_aqi = PLAT_NULL;
	
	list_for_each_entry(p_aqi, &addr_queue_list, addr_queue_item_t, list) 
	{
		if (p_aqi)
		{
			if ((p_aqi->addr[0] == paddr[0]) && (p_aqi->addr[1] == paddr[1]) && (p_aqi->addr[2] == paddr[2])
				&& (p_aqi->addr[3] == paddr[3]) && (p_aqi->addr[4] == paddr[4]) && (p_aqi->addr[5] == paddr[5]))
			{
				*p_id = p_aqi->id;

				return;
			}
		}
	}

	*p_id = 0;
}
#else

static addr_table_t addr_table;


void addr_table_add(uint8_t *paddr, uint8_t id)
{	
	uint8_t i = 0;

	for (i = 0; i < ADDR_TABLE_MAX_NUM; i++)
	{
		if ((addr_table.item[i].addr[0] == paddr[0]) && (addr_table.item[i].addr[1] == paddr[1]) 
			&& (addr_table.item[i].addr[2] == paddr[2]) && (addr_table.item[i].addr[3] == paddr[3]) 
			&& (addr_table.item[i].addr[4] == paddr[4]) && (addr_table.item[i].addr[5] == paddr[5]))
		{
			addr_table.item[i].id = id;

			return;
		}
		else if ((addr_table.item[i].addr[0] == 0) && (addr_table.item[i].addr[1] == 0) 
			&& (addr_table.item[i].addr[2] == 0) && (addr_table.item[i].addr[3] == 0) 
			&& (addr_table.item[i].addr[4] ==0) && (addr_table.item[i].addr[5] == 0))
		{
			addr_table.item[i].addr[0] == paddr[0];
			addr_table.item[i].addr[1] == paddr[1];
			addr_table.item[i].addr[2] == paddr[2];
			addr_table.item[i].addr[3] == paddr[3];
			addr_table.item[i].addr[4] == paddr[4];
			addr_table.item[i].addr[5] == paddr[5];	

			addr_table.item[i].id = id;

			return;
		}
	}
	
	DBG_PRINTF("addr_table_add error\r\n");
}


void addr_table_query(uint8_t *paddr, uint8_t *p_id)
{
	uint8_t i = 0;

	for (i = 0; i < ADDR_TABLE_MAX_NUM; i++)
	{
		if ((addr_table.item[i].addr[0] == paddr[0]) && (addr_table.item[i].addr[1] == paddr[1]) 
			&& (addr_table.item[i].addr[2] == paddr[2]) && (addr_table.item[i].addr[3] == paddr[3]) 
			&& (addr_table.item[i].addr[4] == paddr[4]) && (addr_table.item[i].addr[5] == paddr[5]))
		{
			*p_id = addr_table.item[i].id;

			return;
		}
	}

	DBG_PRINTF("addr_table_query error\r\n");
}

#endif


#include <nwk.h>
#include <route.h>



static route_table_t route_table;


void route_field_handle(route_field_t *p_rf, uint8_t id, uint8_t my_id)
{
	uint8_t i = 0;

	for (i = 0; i < NODE_MAX_NUM; i++)
	{
		if ((i + 1) == my_id)
		{
			if (p_rf->item[i].seq > route_table.item[i].seq)
			{
				route_table.item[i].seq = ((p_rf->item[i].seq >> 1) << 1) + 2;
				route_table.item[i].next_id = my_id;
				route_table.item[i].hop = 0;
			}
			continue;
		}
		
		if (route_table.item[i].next_id == id)
		{
			route_table.item[i].seq = p_rf->item[i].seq;
			if (p_rf->item[i].hop < ROUTE_HOP_MAX)
			{
				route_table.item[i].hop = p_rf->item[i].hop + 1;
			}
			else
			{
				route_table.item[i].hop = ROUTE_HOP_INVALID;
			}
		}
		else
		{
			if ((p_rf->item[i].seq > route_table.item[i].seq) && (p_rf->item[i].hop < ROUTE_HOP_MAX))
			{
				if ((route_table.item[i].hop > ROUTE_HOP_MAX) || ((p_rf->item[i].hop + 1) < route_table.item[i].hop))
				{
					route_table.item[i].seq = p_rf->item[i].seq;
					route_table.item[i].hop = p_rf->item[i].hop + 1;
					route_table.item[i].next_id = id;
				}
			}
		}
	}
}


void route_field_fill(route_field_t *p_rf, uint8_t my_id)
{
	uint8_t i = 0;

	route_table.item[my_id-1].next_id = my_id;
	route_table.item[my_id-1].hop = 0;	
	route_table.item[my_id-1].seq = ((route_table.item[my_id-1].seq >> 1) << 1) + 2;

	for (i = 0; i < NODE_MAX_NUM; i++)
	{
		p_rf->item[i].next_id = route_table.item[i].next_id;
		p_rf->item[i].hop = route_table.item[i].hop;
		p_rf->item[i].seq = route_table.item[i].seq;
	}	
}


void route_table_del(uint8_t next_id)
{
	uint8_t i = 0;
	
	if ((next_id > 0) && (next_id <= NODE_MAX_NUM))
	{
		for (i = 0; i < NODE_MAX_NUM; i++) 
		{
			if (route_table.item[i].next_id == next_id)
			{
				route_table.item[i].hop = ROUTE_HOP_INVALID;
				route_table.item[i].seq = ((route_table.item[i].seq >> 1) << 1) + 1;
			}
		}
	}
	else
	{
		DBG_PRINTF("route_table_del error\r\n");
	}
}


/*
void route_table_add(uint8_t dst_id, uint8_t next_id, uint8_t hop, uint8_t seq, uint8_t flag)
{	
	if ((dst_id > 0) && (dst_id <= NODE_MAX_NUM) && (next_id > 0) && (next_id <= NODE_MAX_NUM) 
		&& ((hop < ROUTE_HOP_MAX) || (hop == ROUTE_HOP_INVALID)) && (dst_id != local_id) && (next_id != local_id))
	{
		if (flag == 0)
		{
			route_table.item[dst_id-1].next_id = next_id;
			route_table.item[dst_id-1].hop = hop;	
			route_table.item[dst_id-1].seq = seq;
		}
		else
		{
			route_table.item_standby[dst_id-1].next_id = next_id;
			route_table.item_standby[dst_id-1].hop = hop;	
			route_table.item_standby[dst_id-1].seq = seq;			
		}
	}
	else
	{
		DBG_PRINTF("route_table_add error\r\n");
	}
}
*/


uint8_t route_table_query(uint8_t dst_id, uint8_t *p_hop, uint8_t *p_seq)
{
	uint8_t next_id = 0;
	
	if (p_hop)
		*p_hop = 0;
	if (p_seq)
		*p_seq = 0;
	
	if ((dst_id > 0) && (dst_id <= NODE_MAX_NUM))
	{
		if (route_table.item[dst_id-1].hop < ROUTE_HOP_INVALID)
		{
			next_id = route_table.item[dst_id-1].next_id;
			if (p_hop)
				*p_hop = route_table.item[dst_id-1].hop;
			if (p_seq)
				*p_seq = route_table.item[dst_id-1].seq;
		}
		/*
		else if (route_table.item_standby[dst_id-1].hop < ROUTE_HOP_INVALID)
		{
			next_id = route_table.item_standby[dst_id-1].next_id;
			if (p_hop)
				*p_hop = route_table.item_standby[dst_id-1].hop;
			if (p_seq)
				*p_seq = route_table.item_standby[dst_id-1].seq;
		}
		*/
	}
	else
	{
		DBG_PRINTF("route_table_query error\r\n");
	}

	return next_id;
}


void route_table_init(void)
{
	uint8_t i = 0;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		route_table.item[i].dst_id = i + 1;
		route_table.item[i].next_id = 0;
		route_table.item[i].hop = ROUTE_HOP_INVALID;	
		route_table.item[i].timeout = 0;
		route_table.item[i].seq = 0;
		
		route_table.item_standby[i].dst_id = i + 1;
		route_table.item_standby[i].next_id = 0;
		route_table.item_standby[i].hop = ROUTE_HOP_INVALID;	
		route_table.item_standby[i].timeout = 0;
		route_table.item_standby[i].seq = 0;
	}
}


void route_table_print(void)
{
	uint8_t i = 0;

	DBG_PRINTF("rb:\r\n");
	for (i = 0; i < 5; i++) 
	{
		DBG_PRINTF("  %x:%x-%d-%d\r\n", 
			i+1, route_table.item[i].next_id, route_table.item[i].hop, route_table.item[i].seq);
	}
}


#include <nwk.h>
#include <broadcast.h>



static broadcast_rcv_table_t broadcast_rcv_table;


void broadcast_rcv_table_add(uint8_t src_id, uint8_t frame_seq)
{
	broadcast_rcv_table.item[src_id-1].frame_seq = frame_seq;
}


void broadcast_rcv_table_del(uint8_t src_id)
{
	broadcast_rcv_table.item[src_id-1].frame_seq = 0;
}


bool_t broadcast_rcv_table_judge(uint8_t src_id, uint8_t frame_seq)
{
	if ((broadcast_rcv_table.item[src_id-1].frame_seq == frame_seq) && (broadcast_rcv_table.item[src_id-1].frame_seq != 0))
		return PLAT_TRUE;
	else
		return PLAT_FALSE;
}



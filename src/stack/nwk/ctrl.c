#include <device.h>
#include <nwk.h>
#include <ctrl.h>

kbuf_t *kbuf_ctrl_frame = PLAT_NULL;



void ctrl_timeout_handle(void)
{
	uint8_t i = 0;

	for (i = 0; i < NODE_MAX_NUM; i++) 
	{
		if (neighbor_table_flush(i + 1))
		{
			route_table_del(i + 1);
		}
	}		
}


void ctrl_frame_parse(ctrl_frame_t *p_ctrl_frame, uint8_t src_id, uint8_t snr)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	uint8_t my_id = GET_DEV_ID(p_device_info->id);

	if ((my_id == 0) || (my_id > NODE_MAX_NUM))
		return;
	
	if (neighbor_table_add(src_id, snr, neighbor_field_get_snrtx(&p_ctrl_frame->nf, my_id)))
	{
		route_field_handle(&p_ctrl_frame->rf, src_id, my_id);
	}
}


void ctrl_frame_send(void)
{
	ctrl_frame_t *p_ctrl_frame = PLAT_NULL;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	uint8_t my_id = GET_DEV_ID(p_device_info->id);

	if ((my_id == 0) || (my_id > NODE_MAX_NUM))
		return;

	p_ctrl_frame = (ctrl_frame_t *)(kbuf_ctrl_frame->base + 0);

	neighbor_field_fill(&p_ctrl_frame->nf);
	route_field_fill(&p_ctrl_frame->rf, my_id);
	//affiliate_field_fill(&p_ctrl_frame->af);

	//to mac
}


void ctrl_init(void)
{
	kbuf_ctrl_frame = kbuf_alloc(KBUF_BIG_TYPE);
	if (kbuf_ctrl_frame == PLAT_NULL) 
	{
		DBG_ERROR("kbuf_alloc error\r\n");
		return;
	}

	mem_set(kbuf_ctrl_frame->base, 0, KBUF_BIG_SIZE);
}

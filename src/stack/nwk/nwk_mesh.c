#include <device.h>
#include <nwk.h>
#include <nwk_mesh.h>


//MESH层接口队列
static list_t nwk_mesh_rx_list;

static kbuf_t *ctrl_frame = PLAT_NULL;
static uint16_t ctrl_timer_id = 0;


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


void ctrl_frame_parse(ctrl_data_t *p_ctrl_data, uint8_t src_id, uint8_t snr)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	uint8_t my_id = GET_DEV_ID(p_device_info->id);

	if ((my_id == 0) || (my_id > NODE_MAX_NUM))
		return;
	
	if (neighbor_table_add(src_id, snr, neighbor_field_get_snrtx(&p_ctrl_data->nf, my_id)))
	{
		route_field_handle(&p_ctrl_data->rf, src_id, my_id);
	}
}


void ctrl_frame_send(void)
{
	ctrl_data_t *p_ctrl_data = PLAT_NULL;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	uint8_t my_id = GET_DEV_ID(p_device_info->id);

	if ((my_id == 0) || (my_id > NODE_MAX_NUM))
		return;

	p_ctrl_data = (ctrl_data_t *)(ctrl_frame->base + 0);

	neighbor_field_fill(&p_ctrl_data->nf);
	route_field_fill(&p_ctrl_data->rf, my_id);
	//affiliate_field_fill(&p_ctrl_data->af);

	//to mac
}


void ctrl_frame_init(void)
{
	ctrl_frame = kbuf_alloc(KBUF_BIG_TYPE);
	if (ctrl_frame == PLAT_NULL) 
	{
		DBG_ERROR("kbuf_alloc error\r\n");
		return;
	}

	mem_set(ctrl_frame->base, 0, KBUF_BIG_SIZE);
}


static void ctrl_timer_cb(void)
{
	uint16_t object = NWK_EVENT_MESH_TIMER;
	
	osel_event_set(nwk_event_h, &object);

	ctrl_timer_id = hal_timer_free(ctrl_timer_id);
	ctrl_timer_id = hal_timer_alloc(NWK_CTRL_TIMEOUT, ctrl_timer_cb);
}


kbuf_t *nwk_mesh_recv_get(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	OSEL_DECL_CRITICAL();

	OSEL_ENTER_CRITICAL();
	kbuf = (kbuf_t *)list_front_get(&nwk_mesh_rx_list);
	OSEL_EXIT_CRITICAL();

	return kbuf;
}


void nwk_mesh_recv_put(kbuf_t *kbuf)
{
	uint16_t object = NWK_EVENT_MESH_RX;
	OSEL_DECL_CRITICAL();
	
	OSEL_ENTER_CRITICAL();
	list_behind_put(&kbuf->list, &nwk_mesh_rx_list);
	OSEL_EXIT_CRITICAL();

	osel_event_set(nwk_event_h, &object);
}



void nwk_mesh_deinit(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	
	do
	{
		kbuf = (kbuf_t *)list_front_get(&nwk_mesh_rx_list);
		if (kbuf) kbuf_free(kbuf);
	} while(kbuf);	
}


void nwk_mesh_init(void)
{
	list_init(&nwk_mesh_rx_list);
	
	//ctrl_frame_init();
	
	ctrl_timer_id = hal_timer_free(ctrl_timer_id);
	ctrl_timer_id = hal_timer_alloc(NWK_CTRL_TIMEOUT, ctrl_timer_cb);
}


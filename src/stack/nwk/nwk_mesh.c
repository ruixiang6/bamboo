#include <device.h>
#include <nwk.h>
#include <nwk_mesh.h>

//MESH层接口队列
static list_t nwk_mesh_rx_list;

static kbuf_t *probe_frame = PLAT_NULL;
static uint16_t probe_timer_id = 0;


void probe_timeout_handle(void)
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


void probe_frame_parse(probe_data_t *p_probe_data, uint8_t src_id, uint8_t snr)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	uint8_t my_id = GET_DEV_ID(p_device_info->id);

	if ((my_id == 0) || (my_id > NODE_MAX_NUM))
		return;
	
	if (neighbor_table_add(src_id, snr, neighbor_field_get_snrtx(&p_probe_data->nf, my_id)))
	{
		route_field_handle(&p_probe_data->rf, src_id, my_id);
	}
}


void probe_frame_fill(void)
{
	OSEL_DECL_CRITICAL();
	probe_data_t *p_probe_data = PLAT_NULL;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	uint8_t my_id = GET_DEV_ID(p_device_info->id);

	if ((my_id == 0) || (my_id > NODE_MAX_NUM))
		return;

	OSEL_ENTER_CRITICAL();	
	
	p_probe_data = (probe_data_t *)probe_frame->offset;
	
	neighbor_field_fill(&p_probe_data->nf);
	route_field_fill(&p_probe_data->rf, my_id);
	//affiliate_field_fill(&p_probe_data->af);

	p_probe_data->chksum = 0;
	p_probe_data->chksum = check16_sum(p_probe_data, sizeof(probe_data_t));

	OSEL_EXIT_CRITICAL();
}


kbuf_t *probe_frame_fetch(void)
{	
	return probe_frame;
}


static void probe_frame_init(void)
{
	probe_frame = kbuf_alloc(KBUF_BIG_TYPE);
	if (probe_frame == PLAT_NULL) 
	{
		DBG_ERROR("kbuf_alloc error\r\n");
		return;
	}

	mem_set(probe_frame->base, 0, KBUF_BIG_SIZE);

	probe_frame->offset = probe_frame->base + sizeof(mac_frm_head_t);
	probe_frame->valid_len = sizeof(probe_data_t);
}


static void probe_timer_cb(void)
{
	uint16_t object = NWK_EVENT_MESH_TIMER;
	
	osel_event_set(nwk_event_h, &object);

	probe_timer_id = hal_timer_free(probe_timer_id);
	probe_timer_id = hal_timer_alloc(NWK_PROBE_TIMEOUT, probe_timer_cb);
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

	probe_timer_id = hal_timer_free(probe_timer_id);
}


void nwk_mesh_init(void)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	
	list_init(&nwk_mesh_rx_list);

	addr_table_init();
	addr_table_add(p_device_info->local_eth_mac_addr, p_device_info->local_ip_addr, GET_DEV_ID(p_device_info->id));
	broadcast_rcv_table_init();
	neighbor_table_init();
	route_table_init();
	affiliate_table_init();

	gateway_table_init();
		
	probe_frame_init();
	
	probe_timer_id = hal_timer_free(probe_timer_id);
	probe_timer_id = hal_timer_alloc(NWK_PROBE_TIMEOUT*10, probe_timer_cb);
}


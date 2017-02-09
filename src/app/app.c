#include <platform.h>
#include <kbuf.h>
#include <device.h>
#include "Control_IO.h"
#include <app.h>

#define APP_TASK_STK_SIZE			256
#define APP_TASK_PRIO				OSEL_TASK_PRIO(4)

OSEL_DECLARE_TASK(APP_TASK, param);
NMEA_RMC_GGA_MSG *p_nmea_gps_msg = PLAT_NULL;
uint8_t *app_gps_buf = PLAT_NULL;
osel_task_t *app_task_h;
osel_event_t *app_event_h;
app_audio_t app_audio;

void gui_hook_func(void);//per2ms

void app_init(void)
{	
	device_info_t *p_device_info = device_info_get(PLAT_TRUE);	
    /* 创建 APP 任务 */
	app_task_h = osel_task_create(APP_TASK, 
    								NULL, 
    								APP_TASK_STK_SIZE, 
    								APP_TASK_PRIO);
    DBG_ASSERT(app_task_h != PLAT_NULL);
    
    app_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(app_event_h != PLAT_NULL);

	//ControlIO_Power(BD_POWER, PLAT_TRUE);
	p_nmea_gps_msg = heap_alloc(sizeof(NMEA_RMC_GGA_MSG), PLAT_TRUE);
	DBG_ASSERT(p_nmea_gps_msg != PLAT_NULL);
	app_gps_buf = heap_alloc(APP_GPS_BUF_SIZE, PLAT_TRUE);
	DBG_ASSERT(app_gps_buf != PLAT_NULL);
	hal_uart_rx_irq_enable(UART_BD, gps_recv_callback);		
	
	//ControlIO_Power(AUDIO_POWER, PLAT_TRUE);
	mem_set(&app_audio, 0, sizeof(app_audio_t));
	list_init(&app_audio.kbuf_rx_list);
	list_init(&app_audio.kbuf_tx_list);
	hal_audio_init(PLAT_NULL, PLAT_NULL);
	
	//初始化串口回调
	hal_uart_rx_irq_enable(UART_DEBUG, uart_recv_callback);

	//srand(p_device_info->id);
}

OSEL_DECLARE_TASK(APP_TASK, param)
{
	(void)param;
	bool_t res;	
	
	DBG_TRACE("APP_TASK!\r\n");	

	while (1)
	{
		res = osel_event_wait(app_event_h, 5);

		if (res == OSEL_EVENT_NONE)
		{
			app_handler(OSEL_EVENT_GET(app_event_h, uint16_t));
		}
		else if (res == OSEL_EVENT_TIMEOUT)
		{
			app_timeout_handler();    
		}
	}
}

void gps_recv_callback(void)
{
	uint16_t object = APP_EVENT_GPS;

	osel_event_set(app_event_h, &object);
}

void uart_recv_callback(void)
{
	uint16_t object = APP_EVENT_UART;

	osel_event_set(app_event_h, &object);
}

void audio_output_callback(void)
{
	uint8_t audio_recv_buf[AUDIO_BUF_SIZE];
	
	uint16_t object = APP_EVENT_AUDIO;
	
	if (app_audio.mcb.m_flag.ambe_out_flag == PLAT_FALSE)
	{
	   app_audio.mcb.m_flag.ambe_out_flag = PLAT_TRUE;

	   osel_event_set(app_event_h, &object);
	}
	else
	{	
		hal_audio_read(audio_recv_buf);
	}
}

void audio_input_callback(void)
{
	uint16_t object = APP_EVENT_AUDIO;
	
	if (app_audio.mcb.m_flag.ambe_in_flag == PLAT_FALSE)
	{
	   app_audio.mcb.m_flag.ambe_in_flag = PLAT_TRUE;	   
	   osel_event_set(app_event_h, &object);	  
	}
	else
	{
		hal_audio_write((uint8_t *)slience_enforce_2400bps_voice);
	}
}

void app_idle_hook(void)
{
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);	

	if (hal_audio_is_read())
    {
        audio_output_callback();
    }

    if (hal_audio_is_write())
    {
        audio_input_callback();
    }	
}
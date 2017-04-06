#include <platform.h>
#include <kbuf.h>
#include <app.h>
#include <device.h>
#include <test.h>
#include <nwk.h>
#include <nwk_eth.h>
#include <nwk_mesh.h>
#include <mac.h>

#define APP_RECV_TIMEOUT	0xFF

const uint8_t gps_config_fix_plus_cmd[] = "$PMTK285,4,100*38\r\n";

static bool_t app_msgt_login = PLAT_FALSE;

static void app_gps_handler(void);
static void app_uart_handler(void);	
static void app_audio_handler(void);

static void app_audio_out_proc(void);
static void app_audio_in_proc(void);
static void app_gps_proc(char_t *p_data, uint16_t size, uint8_t type);

static void app_msgt_proc(uint16_t timeout_cnt_ms);
static void app_test_nwk_proc(uint16_t timeout_cnt_ms);
static void app_test_mac_proc(uint16_t timeout_cnt_ms);
static void app_test_mac_handler(void);


static void app_sniffer_handler(void);
static void app_sniffer_send(kbuf_t *kbuf);

static void app_msgt_handler(void);


void app_handler(uint16_t event_type)
{
	uint16_t object;
	
	if (event_type & APP_EVENT_GPS) 
	{
		object = APP_EVENT_GPS;
		osel_event_clear(app_event_h, &object);
		app_gps_handler();
	} 
	else if (event_type & APP_EVENT_UART)
	{
		object = APP_EVENT_UART;
		osel_event_clear(app_event_h, &object);
		app_uart_handler();		
	}
	else if (event_type & APP_EVENT_AUDIO)
	{
		object = APP_EVENT_AUDIO;
		osel_event_clear(app_event_h, &object);
		app_audio_handler();		
	}
	else if (event_type & APP_EVENT_MSGT)
	{
		object = APP_EVENT_MSGT;
		osel_event_clear(app_event_h, &object);
		app_msgt_handler();
	}
	else if (event_type & APP_EVENT_SNIFFER)
	{
		object = APP_EVENT_SNIFFER;
		osel_event_clear(app_event_h, &object);
		app_sniffer_handler();
	}
	else if (event_type & APP_EVENT_TEST_MAC)
	{
		object = APP_EVENT_TEST_MAC;
		osel_event_clear(app_event_h, &object);
		app_test_mac_handler();
	}
	else if (event_type & APP_EVENT_SHUTDOWN)
	{
		object = APP_EVENT_SHUTDOWN;
		osel_event_clear(app_event_h, &object);		
		DBG_TRACE("Power Off Pressed\r\n");
		paintShutDownDlg();
		osel_systick_delay(1000);
		ControlIO_PowerOff();
	}
}

void app_timeout_handler(void)
{	
	static uint16_t count = 0;
	count++;

	app_test_mac_proc(count);

	app_test_nwk_proc(count);

	app_msgt_proc(count);
}

static void app_gps_handler(void)
{	
	static uint16_t actual_size = 0;
	char_t *p_start = PLAT_NULL;
	uint8_t offset = 0;
    uint8_t loop;	
	
	actual_size += hal_uart_read(UART_BD, &app_gps_buf[actual_size], APP_GPS_BUF_SIZE-actual_size);
	
    if (actual_size < APP_GPS_BUF_SIZE)
	{
		return;
	}
    
	p_start =  strstr((char_t*)app_gps_buf, "GGA");

	if (p_start != PLAT_NULL)
	{
		offset = actual_size - ((uint32_t)p_start - (uint32_t)app_gps_buf);			
		if (offset>10)
		{				
			for (loop=0; loop<offset; loop++)
			{				
				if (p_start[loop] == '\r' && p_start[loop+1] == '\n')
				{
					break;
				}
			}
			
			if (loop != offset)
			{
                app_gps_proc(p_start+strlen("GGA"), loop-strlen("GGA"), APP_GPS_GGA);
				//DBG_TRACE("GGA\r\n");
			}
		}				
	}

	p_start =  strstr((char_t*)app_gps_buf, "RMC");

	if (p_start != PLAT_NULL)
	{
		offset = actual_size - ((uint32_t)p_start - (uint32_t)app_gps_buf);			
		if (offset>10)
		{				
			for (loop=0; loop<offset; loop++)
			{				
				if (p_start[loop] == '\r' && p_start[loop+1] == '\n')
				{
					break;
				}
			}
			
			if (loop != offset)
			{
                app_gps_proc(p_start+strlen("RMC"), loop-strlen("RMC"), APP_GPS_RMC);
				//DBG_TRACE("RMC\r\n");				
			}
		}				
	}

	mem_set(app_gps_buf, 0, APP_GPS_BUF_SIZE);
    actual_size = 0;
}

static void app_uart_handler(void)
{
	uint8_t temp_buf[18];	
	uint32_t size;
	char_t *p_start = PLAT_NULL;	
	char_t *p_stop = PLAT_NULL;
	
	mem_set(temp_buf, 0, 18);

	size = hal_uart_read(UART_DEBUG, temp_buf, 16);	

	if (size == 0) return;
    hal_uart_send_string(UART_DEBUG, temp_buf, size);
    
	p_start =  strstr((char_t*)temp_buf, "test");

	if (p_start)
	{
		//Enter Test
		osel_event_set(test_event_h, PLAT_NULL);
	}
	else
	{
		p_start =  strstr((char_t*)temp_buf, "send");
		if (p_start)
		{
			app_test_mac.send_frm_interval = 1;
			
			p_stop = strstr(p_start, ";");
			if (p_stop)
			{
				p_start = p_start + strlen("send=");
				*p_stop = '\0';
				sscanf(p_start, "%d", &app_test_mac.send_frm_interval);
			}

			app_test_mac.send_frm_seq = 0;
			app_test_mac.state = PLAT_TRUE;
			return;
		}

		p_start =  strstr((char_t*)temp_buf, "over");
		if (p_start)
		{			
			app_test_mac.state = PLAT_FALSE;
			DBG_PRINTF("Send=%d\r\n", app_test_mac.send_frm_seq);
			return;
		}

		p_start =  strstr((char_t*)temp_buf, "printon");
		if (p_start)
		{
			app_test_nwk.debug_flag = PLAT_TRUE;
			return;
		}

		p_start =  strstr((char_t*)temp_buf, "printoff");
		if (p_start)
		{			
			app_test_nwk.debug_flag = PLAT_FALSE;
			return;
		}

	}
}

static void app_audio_handler(void)
{
	if (app_audio.mcb.m_flag.ambe_out_flag == PLAT_TRUE)
   	{
        //DBG_PRINTF("OUT\r\n");
		app_audio_out_proc();
	  	app_audio.mcb.m_flag.ambe_out_flag = PLAT_FALSE;            
	}       
	
	if (app_audio.mcb.m_flag.ambe_in_flag == PLAT_TRUE)
    {
        //DBG_PRINTF("IN\r\n");
		app_audio_in_proc();
	  	app_audio.mcb.m_flag.ambe_in_flag = PLAT_FALSE;            		
	}
}

static void app_audio_out_proc(void)
{
	uint8_t audio_recv_buf[AUDIO_BUF_SIZE];
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	
	hal_audio_read(audio_recv_buf);
	
    if (audio_recv_buf[0] != 0x13 || audio_recv_buf[1] != 0xEC)
    {
        DBG_PRINTF("ReadHeadErr=[%x][%x]", audio_recv_buf[0], audio_recv_buf[1]);
        return;
    }
	
	if (app_audio.mcb.m_ptt_state == AMBE_PTT_DOWN)
	{
		if (app_audio.rx_kbuf == PLAT_NULL)
		{
			app_audio.rx_kbuf = kbuf_alloc(KBUF_SMALL_TYPE);

			if (app_audio.rx_kbuf)
			{
				mem_cpy(app_audio.rx_kbuf->base, &audio_recv_buf[10], 6);
				app_audio.rx_kbuf->valid_len = 6;							
			}
			else
			{
				DBG_TRACE("no kbuf enough\r\n");
			}
		}
		else
		{	
			//因为为2400bps的压缩率，故每20ms产生6字节有效数据
			mem_cpy(app_audio.rx_kbuf->base+app_audio.rx_kbuf->valid_len, &audio_recv_buf[10], 6);			
			app_audio.rx_kbuf->valid_len += 6;
			//短包最大长度117
			if (app_audio.rx_kbuf->valid_len>=117)
			{
				//Send to NWK
				//mac_send(app_audio.rx_pbuf);
				//Send to myself
				list_behind_put(&app_audio.rx_kbuf->list, &app_audio.kbuf_tx_list);
				//////////////////////
                app_audio.rx_kbuf = PLAT_NULL;
			}
		}
	}
	else
	{
		//当抬起按键，剩余数据也必须要发送出去不用筹齐117个字节
		if (app_audio.rx_kbuf)
		{
			//Send to NWK
			//mac_send(app_audio.rx_pbuf);
			//Send to myself
			list_behind_put(&app_audio.rx_kbuf->list, &app_audio.kbuf_tx_list);
			//////////////////////
            app_audio.rx_kbuf = PLAT_NULL;
		}
	}
}

static void app_audio_in_proc(void)
{
	uint8_t audio_send_buf[AUDIO_BUF_SIZE];	
	static uint8_t loss_audio_cnt = 0;
	static uint16_t send_audio_len = 0;
	uint8_t rate = hal_audio_get_rate();

	if (app_audio.mcb.m_ptt_state == AMBE_PTT_DOWN)
	{
		hal_audio_write((uint8_t *)&slience_enforce_voice[rate][0]);
		//DBG_PRINTF("0");
        return;
	}
	
	mem_set(audio_send_buf, 0, AUDIO_BUF_SIZE);
	
	if (app_audio.tx_kbuf == PLAT_NULL)
	{
		app_audio.tx_kbuf = (kbuf_t *)list_front_get(&app_audio.kbuf_tx_list);

		if (app_audio.tx_kbuf)
		{
			loss_audio_cnt = 0;
			send_audio_len = app_audio.tx_kbuf->valid_len;
		}
	}

	if (app_audio.tx_kbuf)
	{
		audio_send_buf[0] = 0x13;
		audio_send_buf[1] = 0xec;			
		
		if (send_audio_len>=6)
		{
			mem_cpy(&audio_send_buf[10], app_audio.tx_kbuf->base, 6);
			send_audio_len -= 6;
		}
		else
		{
			mem_cpy(&audio_send_buf[10], app_audio.tx_kbuf->base, send_audio_len);
			send_audio_len = 0;
			kbuf_free(app_audio.tx_kbuf);
			app_audio.tx_kbuf = PLAT_NULL;
		}
		
		hal_audio_write(audio_send_buf);
	}
	else
	{
		loss_audio_cnt++;
		if (loss_audio_cnt<3)
		{
			audio_send_buf[0] = 0x13;
			audio_send_buf[1] = 0xec;
			audio_send_buf[2] = 0x00;
			audio_send_buf[3] = 0x80;//control_0 repeat frame
			hal_audio_write(audio_send_buf);
			//DBG_PRINTF("R");
		}
		else
		{
			hal_audio_write((uint8_t *)&slience_enforce_voice[rate][0]);
			loss_audio_cnt = 10;
            //DBG_PRINTF(".");
		}
	}	
}

static void app_gps_proc(char_t *p_data, uint16_t size, uint8_t type)
{
	static uint8_t update_rtc_flag = 0;
	dev_time_t dev_time;
	bool_t flag_day, flag_year;
	hal_rtc_block_t rtc_block;
	uint16_t year;
	uint8_t counter = 0;
	uint8_t off_set = 0;	
	char_t *p_start = PLAT_NULL;
	char_t *p_stop = PLAT_NULL;
	
	if (p_nmea_gps_msg == PLAT_NULL) return;

	hal_uart_printf(UART_BD, (uint8_t *)gps_config_fix_plus_cmd);

	if (type == APP_GPS_RMC)
	{
		p_start = p_data;
		counter = 0;
		p_start = strstr(p_data, ",");

		do
		{
			if (p_start) 
			{
				counter++;
				p_stop = strstr(p_start+1, ",");

				if (p_stop)
				{
					off_set = (uint32_t)p_stop - (uint32_t)p_start - 1;				
					switch(counter)
		            {
		                case 1://time
							if (off_set<NMEA_TIME_SIZE)mem_cpy(p_nmea_gps_msg->time, p_start+1, off_set);							
							break;
		                case 2://flag
							if (off_set<NMEA_FLAG_SIZE)mem_cpy(p_nmea_gps_msg->flag, p_start+1,off_set);							
							break;
		                case 3:	//lat						
							if (off_set<NMEA_LAT1_SIZE) mem_cpy(p_nmea_gps_msg->latitude, p_start+1,off_set);
							break;
		                case 4:
							if (off_set<NMEA_LAT2_SIZE)mem_cpy(p_nmea_gps_msg->latitude2, p_start+1,off_set);
							break;
		                case 5://long
							if (off_set<NMEA_LON1_SIZE)mem_cpy(p_nmea_gps_msg->longitude, p_start+1,off_set);
							break;
		                case 6:
							if (off_set<NMEA_LON2_SIZE)mem_cpy(p_nmea_gps_msg->longitude2, p_start+1,off_set);
							break;	                        
		                case 9://date
							if (off_set<NMEA_DATE_SIZE)mem_cpy(p_nmea_gps_msg->date, p_start+1,off_set);
							break;
			            default:break;
		            }
					p_start = p_stop;
				}				
			}			
		}while(p_start && p_stop);
	}

	if (type == APP_GPS_GGA && p_nmea_gps_msg->flag[0] == 'A')
	{
		p_start = p_data;
		counter = 0;
		p_start = strstr(p_data, ",");

		do
		{
			if (p_start) 
			{
				counter++;
				p_stop = strstr(p_start+1, ",");

				if (p_stop)
				{
					off_set = (uint32_t)p_stop - (uint32_t)p_start - 1;				
					switch(counter)
                    {	                        
                        case 7://sat						
                            if (off_set<NMEA_SAT_SIZE) mem_cpy(p_nmea_gps_msg->satellites, p_start+1,off_set);
                            break;
                        case 9://alt
                            if (off_set<NMEA_ALT_SIZE) mem_cpy(p_nmea_gps_msg->altitude, p_start+1,off_set);
                            break;	                        
                        default:break;
                    }		           
					p_start = p_stop;
				}				
			}			
		}while(p_start && p_stop);
	}

	if (p_nmea_gps_msg->flag[0] == 'A' && update_rtc_flag == 0)
	{
		update_rtc_flag++;
		mem_set(&dev_time, 0, sizeof(dev_time_t));
		mem_set(&rtc_block, 0, sizeof(hal_rtc_block_t));
		/*时间判断*/
		dev_time.hour = (p_nmea_gps_msg->time[0]-'0')*10 + p_nmea_gps_msg->time[1]-'0' + TIME_CHINA_ZONE;
		if (dev_time.hour >= 24)
		{
			dev_time.hour -= 24;
			flag_day = 1;
		}
		else
		{
			flag_day = 0;
		}
		dev_time.minute = (p_nmea_gps_msg->time[2]-'0')*10 + p_nmea_gps_msg->time[3]-'0';
		dev_time.second = (p_nmea_gps_msg->time[4]-'0')*10 + p_nmea_gps_msg->time[5]-'0';		
		//////////////////////////////////////////////////////////////////////
		dev_time.day = (p_nmea_gps_msg->date[0]-'0')*10 + p_nmea_gps_msg->date[1]-'0';
		dev_time.month = (p_nmea_gps_msg->date[2]-'0')*10 + p_nmea_gps_msg->date[3]-'0';
		dev_time.year = (p_nmea_gps_msg->date[4]-'0')*10 + p_nmea_gps_msg->date[5]-'0';
		year = 2000 + dev_time.year;		
	    //判断润年
		if (year % 4 == 0 || year % 400 == 0)
		{
			if (year % 100 != 0)
			{
				flag_year = 1;
			}
			else
			{
				flag_year = 0;
			}
		}
		else
		{
			flag_year = 0;
		}
		//递进判断
		if (flag_day)
		{
			dev_time.day += 1;
			flag_day = 0;
			//判断1 3 5 7 8 10 12月份31天
			if(dev_time.month == 1 ||
				dev_time.month == 3 ||
				dev_time.month == 5 ||
				dev_time.month == 7 ||
				dev_time.month == 8 ||
				dev_time.month == 10 ||
				dev_time.month == 12)
			{
				if (dev_time.day > 31)
				{
					dev_time.day -= 31; 
					dev_time.month++;
					if (dev_time.month > 12)
					{
						dev_time.month = 1;
						dev_time.year++;
					}
				}
			}
			else if (dev_time.month == 2)
			{
				/*闰年*/
				if (flag_year)
				{
					if (dev_time.day > 29)
					{
						dev_time.day -= 29; 
						dev_time.month++;
					}
				}
				else
				{
					if (dev_time.day > 28)
					{
						dev_time.day -= 28;
						dev_time.month++;
					}
				}
			}
			else if (dev_time.month == 4 ||
						dev_time.month == 6 ||
						dev_time.month == 9 ||
						dev_time.month == 11)
			{
				if (dev_time.day > 30)
				{
					dev_time.day -= 30; 
					dev_time.month++;					
				}
			}
		}
	    
	    if (dev_time.year == 0 
	        || dev_time.month == 0
	        || dev_time.day == 0 
	        || dev_time.hour == 0 
	        || dev_time.minute == 0 
	        || dev_time.second == 0     
	        || dev_time.month>12            
	        || dev_time.day>31
	        || dev_time.hour>24
	        || dev_time.minute>59
	        || dev_time.second>59)
	    {
	        DBG_TRACE("Gps time invalid\r\n");
	        return;
	    }

		rtc_block.year = dev_time.year;
		rtc_block.day = dev_time.day;
		rtc_block.month = dev_time.month;
		rtc_block.hour = dev_time.hour;
		rtc_block.minute = dev_time.minute;
		rtc_block.second = dev_time.second;
		rtc_block.week = 1;
		rtc_block.weekday = 1;

		hal_rtc_set(&rtc_block);
	}
	else if (p_nmea_gps_msg->flag[0] == 'A')
	{
		update_rtc_flag++;
	}
}


static void app_msgt_proc(uint16_t timeout_cnt_ms)
{
	uint16_t object = APP_EVENT_MSGT;

	//每隔1S，触发网管软件交互事件
	if (timeout_cnt_ms % 1000 == 0)
	{
		osel_event_set(app_event_h, &object);
	}
}


static void app_test_nwk_proc(uint16_t timeout_cnt_ms)
{
	if (app_test_nwk.debug_flag != PLAT_TRUE)
	{
		return;
	}
	else
	{
		if (timeout_cnt_ms % 5000 == 0)
		{
			nwk_print();
		}
	}
}

static void app_test_mac_proc(uint16_t timeout_cnt_ms)
{
	kbuf_t *kbuf;	
    packet_info_t send_info;
    device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	static uint16_t interval = 0;

	if (app_test_mac.state != PLAT_TRUE)
	{
		return;
	}
    
    if (timeout_cnt_ms>=interval)
    {
        if (app_test_mac.send_frm_interval >= timeout_cnt_ms-interval)
        {
            return;
        }
        else
        {
            interval = timeout_cnt_ms;
        }
    }
    else
    {
         if (app_test_mac.send_frm_interval >= timeout_cnt_ms+0xFFFF-interval)
         {
             return;
         }
         else
         {
             interval = timeout_cnt_ms;
         }
    }	

	kbuf = kbuf_alloc(KBUF_BIG_TYPE);
	
	if (kbuf)
	{
		kbuf->valid_len = 1514;
        mem_set(&send_info, 0, sizeof(packet_info_t));
		send_info.src_id = GET_DEV_ID(p_device_info->id);
        send_info.dest_id = BROADCAST_ID;
        send_info.sender_id = GET_DEV_ID(p_device_info->id);
        send_info.target_id = BROADCAST_ID;
        send_info.seq_num = app_test_mac.send_frm_seq++;
        send_info.frm_ctrl.reserve = PLAT_TRUE;
        send_info.frm_ctrl.qos_level = 0;		
		//发送给mac层
		if (!mac_send(kbuf, &send_info))
		{
			kbuf_free(kbuf);
            return;
		}
        //DBG_PRINTF("tx=%d\r\n", send_info.seq_num);
			
	}
	else
	{
		DBG_TRACE("kbuf is free\r\n");
	}
}

static void app_test_mac_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	mac_frm_head_t *p_mac_frm_head;
	static uint8_t prev_seq = 0;
	uint16_t diff;
	
	OSEL_DECL_CRITICAL();

	do
	{
		OSEL_ENTER_CRITICAL();
		kbuf = (kbuf_t *)list_front_get(&app_test_mac.kbuf_rx_list);
		OSEL_EXIT_CRITICAL();

		if (kbuf)
		{
			p_mac_frm_head = (mac_frm_head_t *)kbuf->base;
			
			if (p_mac_frm_head->sender_id != 0x12)
			{
				kbuf_free(kbuf);
				return;
			}
			
			DBG_PRINTF("Q=%d\r\n", p_mac_frm_head->seq_ctrl.seq_num);
			if (prev_seq <= p_mac_frm_head->seq_ctrl.seq_num)
			{
				diff = p_mac_frm_head->seq_ctrl.seq_num - prev_seq;
			}
			else
			{
				diff = p_mac_frm_head->seq_ctrl.seq_num + 256 - prev_seq;
			}
			
			if (diff>1)
			{
				DBG_PRINTF("*******=%d\r\n", diff-1);
			}			

			prev_seq = p_mac_frm_head->seq_ctrl.seq_num;
			kbuf_free(kbuf);
		}
	}while(kbuf);
}

static void app_sniffer_handler(void)
{
	kbuf_t *kbuf = PLAT_NULL;
	mac_frm_head_t *p_mac_frm_head;
	mac_frm_head_t *p_mac_frm_head_copy;
	kbuf_t *kbuf_copy = PLAT_NULL;
	app_sniffer_frm_head_t *p_sniffer_frm_head;
    uint16_t offset_len;
	OSEL_DECL_CRITICAL();

	do
	{
		OSEL_ENTER_CRITICAL();
		kbuf = (kbuf_t *)list_front_get(&app_sniffer.kbuf_rx_list);
		OSEL_EXIT_CRITICAL();	

		if (kbuf)
		{
			p_sniffer_frm_head = (app_sniffer_frm_head_t *)kbuf->base;
			p_sniffer_frm_head->type = 0;//Sniffer frame type
			kbuf->offset = kbuf->base+sizeof(app_sniffer_frm_head_t);
			p_mac_frm_head = (mac_frm_head_t *)kbuf->offset;

			//如果收到PROB的信息，则不用判断地址，可能是一个拼帧，都需要上传给设备
			if (p_mac_frm_head->frm_ctrl.probe_flag == PROBE)
			{
				if (p_mac_frm_head->frm_ctrl.qos_level != NONE)
				{
					kbuf_copy = kbuf_alloc(KBUF_BIG_NUM);
					if (kbuf_copy)
					{
						p_sniffer_frm_head = (app_sniffer_frm_head_t *)kbuf_copy->base;
						kbuf_copy->offset = kbuf_copy->base+sizeof(app_sniffer_frm_head_t);
						p_mac_frm_head_copy = (mac_frm_head_t *)kbuf_copy->offset;
						//copy头信息
						mem_cpy(p_mac_frm_head_copy, p_mac_frm_head, sizeof(mac_frm_head_t));
						//减去probe_data_t,得到原有数据的长度
						offset_len = p_mac_frm_head->frm_len - sizeof(probe_data_t);
						//copy这ctrl_data_t部分数据
						kbuf_copy->offset = kbuf_copy->offset+sizeof(mac_frm_head_t);
						kbuf->offset = kbuf->offset+sizeof(mac_frm_head_t);
						//copy probe部分
						mem_cpy(kbuf_copy->offset, kbuf->offset+offset_len, sizeof(probe_data_t));
						//将帧类型变为纯PROB的数据包
                        p_mac_frm_head_copy->frm_ctrl.qos_level = NONE;
						p_mac_frm_head_copy->frm_ctrl.probe_flag = PROBE;
						p_mac_frm_head_copy->dest_dev_id = BROADCAST_ID;
						p_mac_frm_head_copy->target_id = BROADCAST_ID;
						p_mac_frm_head_copy->frm_len = sizeof(probe_data_t);
						kbuf_copy->valid_len = sizeof(mac_frm_head_t)+p_mac_frm_head_copy->frm_len;
						//发送数据
						p_sniffer_frm_head->head = APP_SNIFF_HEAD;
						p_sniffer_frm_head->length = kbuf_copy->valid_len;
                        p_sniffer_frm_head->type = 0;
						kbuf_copy->valid_len = sizeof(app_sniffer_frm_head_t)+kbuf_copy->valid_len;
						app_sniffer_send(kbuf_copy);
						kbuf_free(kbuf_copy);
					}
				}
				else
				{					
					//probe_data_t数据的长度
					p_mac_frm_head->frm_len = sizeof(probe_data_t);
					kbuf->valid_len = sizeof(mac_frm_head_t) + p_mac_frm_head->frm_len;
					//发送数据
					p_sniffer_frm_head->head = APP_SNIFF_HEAD;
					p_sniffer_frm_head->length = kbuf->valid_len;
					kbuf->valid_len = sizeof(app_sniffer_frm_head_t)+kbuf->valid_len;
					app_sniffer_send(kbuf);
					kbuf_free(kbuf);
					continue;
				}
			}
			
			if (p_mac_frm_head->frm_ctrl.qos_level >= QOS_L
				&& p_mac_frm_head->frm_ctrl.qos_level <= QOS_H)
			{
	            if (p_mac_frm_head->frm_ctrl.probe_flag == PROBE)
				{
					//将帧类型变为纯QOS的数据包	
					p_mac_frm_head->frm_ctrl.probe_flag = NONE;
					p_mac_frm_head->frm_len -= sizeof(probe_data_t);
				}
				
				if (p_mac_frm_head->frm_len)
				{
					kbuf->valid_len = sizeof(mac_frm_head_t)+p_mac_frm_head->frm_len;
					//发送数据
					p_sniffer_frm_head->head = APP_SNIFF_HEAD;
					p_sniffer_frm_head->length = kbuf->valid_len;
                    p_sniffer_frm_head->type = 0;
					kbuf->valid_len = sizeof(app_sniffer_frm_head_t)+kbuf->valid_len;
					app_sniffer_send(kbuf);
                    kbuf_free(kbuf);
				}
				else
				{
					kbuf_free(kbuf);
				}
			}
			else
			{
				kbuf_free(kbuf);
			}			
		}
	}while(kbuf);
}

static void app_sniffer_send(kbuf_t *kbuf)
{	
	int32_t len;	
	
	if (app_sniffer.socket_id>=0)
	{
		len = sendto(app_sniffer.socket_id, kbuf->base, kbuf->valid_len, 0, (struct sockaddr *)&app_sniffer.s_addr, sizeof(app_sniffer.s_addr));
	}
}


static bool_t app_msgt_config_pkt_parse(void)
{
	
	app_msgt_frm_head_t *p_msgt_frm_head = (app_msgt_frm_head_t *)app_msgt.rx_buf->base;
	uint8_t *p = app_msgt.rx_buf->offset;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);

	app_msgt.rx_buf->valid_len = 0;

	if (p_msgt_frm_head->type == APP_MSGT_TYPE_CONFIG)
	{
		switch (p_msgt_frm_head->stype) {
			case APP_MSGT_STYPE_CONFIG_LOGIN:
			{
				//此处应比较密码，待添加
				app_msgt_login = PLAT_TRUE;
				p_msgt_frm_head->length = 1 + sizeof(app_msgt_frm_head_t);
				p_msgt_frm_head->stype = APP_MSGT_STYPE_CONFIG_LOGIN_ACK;
				p[0] = 0;
				app_msgt.rx_buf->valid_len = p_msgt_frm_head->length;
				break;
			}
			case APP_MSGT_STYPE_CONFIG_ID:
			{
				p_msgt_frm_head->length = 4 + sizeof(app_msgt_frm_head_t);
				p_msgt_frm_head->stype = APP_MSGT_STYPE_CONFIG_ID_ACK;
				app_msgt.rx_buf->valid_len = p_msgt_frm_head->length;
				if (app_msgt_login == PLAT_TRUE)
				{
					//此处应判断ID合法性，待添加
					mem_cpy(p_device_info->id, p, 4);
					p[0] = 0;
				}
				else
				{
					p[0] = 1;
				}
				break;
			}
			case APP_MSGT_STYPE_CONFIG_IP:
			{
				p_msgt_frm_head->length = 12 + sizeof(app_msgt_frm_head_t);
				p_msgt_frm_head->stype = APP_MSGT_STYPE_CONFIG_IP_ACK;
				app_msgt.rx_buf->valid_len = p_msgt_frm_head->length;
				if (app_msgt_login == PLAT_TRUE)
				{
					//此处应判断IP合法性，待添加
					mem_cpy(p_device_info->local_ip_addr, p, 4);
					mem_cpy(p_device_info->local_netmask_addr, p+4, 4);
					mem_cpy(p_device_info->local_netmask_addr, p+8, 4);
					p[0] = 0;
				}
				else
				{
					p[0] = 1;
				}
				break;
			}
			case APP_MSGT_STYPE_CONFIG_POWER:
			{
				p_msgt_frm_head->length = 1 + sizeof(app_msgt_frm_head_t);
				p_msgt_frm_head->stype = APP_MSGT_STYPE_CONFIG_POWER_ACK;
				app_msgt.rx_buf->valid_len = p_msgt_frm_head->length;
				if (app_msgt_login == PLAT_TRUE)
				{
					//待添加
					p[0] = 0;
				}
				else
				{
					p[0] = 1;
				}
				break;
			}
			case APP_MSGT_STYPE_CONFIG_MOUNT:
			{
				p_msgt_frm_head->stype = APP_MSGT_STYPE_CONFIG_POWER_ACK;
				if (app_msgt_login == PLAT_TRUE)
				{
					addr_table_get_mount(GET_DEV_ID(p_device_info->id), p);

					p_msgt_frm_head->length = p[0] + sizeof(app_msgt_frm_head_t);
					p_msgt_frm_head->stype = APP_MSGT_STYPE_CONFIG_POWER_ACK;
					app_msgt.rx_buf->valid_len = p_msgt_frm_head->length;
				}
				else
				{
					p[0] = 0;
				}
				break;

			}
			case APP_MSGT_STYPE_CONFIG_FP:
			{
				p_msgt_frm_head->length = 1 + sizeof(app_msgt_frm_head_t);
				p_msgt_frm_head->stype = APP_MSGT_STYPE_CONFIG_FP_ACK;
				app_msgt.rx_buf->valid_len = p_msgt_frm_head->length;
				if (app_msgt_login == PLAT_TRUE)
				{
					//待添加
					p[0] = 0;
				}
				else
				{
					p[0] = 1;
				}
				break;
			}
			default:
			{
				return PLAT_FALSE;
			}
		}

		return PLAT_TRUE;
	}

	return PLAT_FALSE;
}


static void app_msgt_config_pkt_recv(void)
{
	int32_t len;
	bool_t ret = PLAT_FALSE;
	struct sockaddr_in addr;
	int32_t addr_len = sizeof(struct sockaddr_in);
	
	if (app_msgt.socket_id >= 0)
	{
		len = recvfrom(app_msgt.socket_id, app_msgt.rx_buf->base, KBUF_BIG_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
		if (len > 0)
		{
			ret = app_msgt_config_pkt_parse();
			if (ret)
			{
				sendto(app_msgt.socket_id, app_msgt.rx_buf->base, app_msgt.rx_buf->valid_len, 0, (struct sockaddr *)&addr, sizeof(addr));
			}
		}
	}	
}


static void app_msgt_status_pkt_fill(void)
{	
	app_msgt_frm_head_t *p_msgt_frm_head = (app_msgt_frm_head_t *)app_msgt.tx_buf->base;
	uint8_t *p = app_msgt.tx_buf->offset;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	
	p_msgt_frm_head->head = APP_MSGT_HEAD;
	p_msgt_frm_head->length = 150 + sizeof(app_msgt_frm_head_t);
	p_msgt_frm_head->type = APP_MSGT_TYPE_STATUS;
	p_msgt_frm_head->stype = APP_MSGT_STYPE_STATUS_PUSH;

	//id
	mem_cpy(p, p_device_info->id, 4);
	//ip,netmask,gateway
	mem_cpy(p+4, p_device_info->local_ip_addr, 4);
	mem_cpy(p+8, p_device_info->local_netmask_addr, 4);
	mem_cpy(p+12, p_device_info->local_gateway_addr, 4);
	//macaddr
	mem_cpy(p+16, p_device_info->local_eth_mac_addr, 6);
	//功率和电量待填
	//pos
	mem_cpy(p+24, (uint8_t *)&p_device_info->pos, 10);
	//route
	route_table_to_app(p+34);

	app_msgt.tx_buf->valid_len = p_msgt_frm_head->length;
}


static void app_msgt_status_pkt_send(void)
{
	int32_t len;

	app_msgt_status_pkt_fill();
	
	if (app_msgt.socket_id >= 0)
	{
		len = sendto(app_msgt.socket_id, app_msgt.tx_buf->base, app_msgt.tx_buf->valid_len, 0, (struct sockaddr *)&app_msgt.s_addr_bc, sizeof(app_msgt.s_addr_bc));
	}
}


static void app_msgt_handler(void)
{
	static uint8_t count = 0;

	//每隔5S，发送状态广播包
	count++;
	if (count == 5)
	{
		count = 0;
		//app_msgt_status_pkt_send();
	}

	//每隔1S，非阻塞接收网管软件的消息
	//app_msgt_config_pkt_recv();
}

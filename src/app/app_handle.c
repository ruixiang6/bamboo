#include <platform.h>
#include <kbuf.h>
#include <app.h>
#include <device.h>
#include <test.h>

#define APP_RECV_TIMEOUT	0xFF

const uint8_t gps_config_fix_plus_cmd[] = "$PMTK285,4,100*38\r\n";

static void app_gps_handler(void);
static void app_uart_handler(void);	
static void app_audio_handler(void);

static void app_audio_out_proc(void);
static void app_audio_in_proc(void);
static void app_gps_proc(char_t *p_data, uint16_t size, uint8_t type);

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
}

void app_timeout_handler(void)
{	
	static uint16_t count = 0;

	count++;
	if (count == 2000)
	{
		count = 0;

	}
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

	if (app_audio.mcb.m_ptt_state == AMBE_PTT_DOWN)
	{
		hal_audio_write((uint8_t *)slience_enforce_2400bps_voice);            
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
			hal_audio_write((uint8_t *)slience_enforce_2400bps_voice);            
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

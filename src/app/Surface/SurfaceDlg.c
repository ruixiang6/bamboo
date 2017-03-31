#include <platform.h>
#include <GUI.h>
#include <app.h>
#include <device.h>
#include <mac.h>

#define TITLE_BAT_X				120
#define TITLE_WIFI_X			30
#define TITLE_AUDIO_X			60
#define TITLE_USB_X				88
#define TITLE_GPS_X				3

#define MAIN_LINE_X_L			5
#define MAIN_LINE_X_TITLE		20
#define MAIN_LINE_X_M			35
#define MAIN_LINE_X_R			80
#define MAIN_LINE1_Y			8
#define MAIN_LINE2_Y			21
#define MAIN_LINE3_Y			34
#define MAIN_LINE4_Y			46

static uint8_t line1_length = 0;
static uint8_t line2_length = 0;
static uint8_t line3_length = 0;

void paintEmptyDlg(void);
static uint8_t addNullStr(char_t *str, uint8_t act_len);

void paintBootDlg(void)
{
	device_info_t *p_device_info = device_info_get(PLAT_TRUE);	

	GUI_SetFont(&GUI_FontHZ_SimSun_12);
	GUI_DispStringAt("自组网传输设备", MAIN_LINE_X_TITLE, MAIN_LINE2_Y);
	GUI_DispStringAt(p_device_info->version, MAIN_LINE_X_M, MAIN_LINE3_Y);
	osel_systick_delay(3000);
	GUI_DispStringAt("              ", MAIN_LINE_X_TITLE, MAIN_LINE2_Y);
}

void paintTitleDlg(bool_t update, uint8_t type)
{
	device_info_t *p_device_info = device_info_get(update);

	if (update)
	{
		GUI_ClearRect(0, 0, GUI_XMAX, MAIN_LINE1_Y-1);
	}

	if (type == TITLE_TYPE_ALL || type == TITLE_TYPE_BAT)
	{
		if (p_device_info->dev_state&0x10)
		{
			//充电中状态
			GUI_DrawVLine(TITLE_BAT_X-4, 1, 4);
			GUI_DrawVLine(TITLE_BAT_X-3, 2, 4);
			GUI_DrawVLine(TITLE_BAT_X-2, 3, 4);
			GUI_DrawVLine(TITLE_BAT_X-1, 4, 4);
		}
		else if (p_device_info->dev_state&0x20)
		{
			//清除
			GUI_ClearRect(TITLE_BAT_X-3, 0, TITLE_BAT_X-1, 4);
			//充满状态
			GUI_DrawVLine(TITLE_BAT_X-4, 1, 4);
			GUI_DrawVLine(TITLE_BAT_X-3, 1, 4);
			GUI_DrawVLine(TITLE_BAT_X-2, 1, 4);
		}
		else
		{
			//清除
			GUI_ClearRect(TITLE_BAT_X-4, 0, TITLE_BAT_X-1, 4);
		}
		
		//0-9表示电池电量
		if ((p_device_info->dev_state&0x0F) >= 8)
		{
			GUI_DrawBitmap(&bmbatFimg, TITLE_BAT_X, 1);
		}
		else if ((p_device_info->dev_state&0x0F) >= 2)
		{
			GUI_DrawBitmap(&bmbatMimg, TITLE_BAT_X, 1);
		}
		else
		{
			GUI_DrawBitmap(&bmbatEimg, TITLE_BAT_X, 1);
		}		
	}   

	if (type == TITLE_TYPE_ALL || type == TITLE_TYPE_WIFI)
	{
		if (p_device_info->func_bit.wifi)
		{
			GUI_DrawBitmap(&bmwifiimg, TITLE_WIFI_X, 1);
		}
	}

	if (type == TITLE_TYPE_ALL || type == TITLE_TYPE_USB)
	{
		if (p_device_info->dev_state & 0x40)
		{
			GUI_DrawHLine(1, TITLE_USB_X,TITLE_USB_X+8);
			GUI_DrawHLine(3, TITLE_USB_X,TITLE_USB_X+8);
		}
		else
		{
			GUI_ClearRect(TITLE_USB_X, 0, TITLE_USB_X+4, 4);
		}
	}
	
	if (type == TITLE_TYPE_ALL || type == TITLE_TYPE_AUDIO)
	{
		if (p_device_info->func_bit.audio)
		{
			if (p_device_info->dev_state & 0x80)
			{
				GUI_ClearRect(TITLE_AUDIO_X-2, 0, TITLE_AUDIO_X+9, 6);
				GUI_DrawBitmap(&bmheadsetimg, TITLE_AUDIO_X-2, 1);
			}
			else
			{
				GUI_ClearRect(TITLE_AUDIO_X-2, 0, TITLE_AUDIO_X+9, 6);
				GUI_DrawBitmap(&bmspeakimg, TITLE_AUDIO_X, 0);				
			}			
		}
        else
        {
            GUI_ClearRect(TITLE_AUDIO_X-2, 0, TITLE_AUDIO_X+9, 6);
        }
	}
		

	if (p_device_info->func_bit.gps)
	{
		if (p_nmea_gps_msg->flag[0] == 'A')
		{
			GUI_DrawCircle(TITLE_GPS_X, 3, 1);
		}
		else
		{
			GUI_ClearRect(TITLE_GPS_X-1, 1, TITLE_GPS_X+1, 4);
		}
		GUI_DrawCircle(TITLE_GPS_X, 3, 3);
	}

	if (type == TITLE_TYPE_SPEAK_ON)
	{
		GUI_DrawHLine(1, TITLE_AUDIO_X+9, TITLE_AUDIO_X+11);
		GUI_DrawHLine(3, TITLE_AUDIO_X+9, TITLE_AUDIO_X+11);
		GUI_DrawHLine(5, TITLE_AUDIO_X+9, TITLE_AUDIO_X+11);
	}
	else if (type == TITLE_TYPE_SPEAK_OFF)
	{
		GUI_ClearRect(TITLE_AUDIO_X+9, 1, TITLE_AUDIO_X+11, 5);
	}
}

void paintMainDlg(bool_t update)
{
	char_t temp_str[LCD_X_DOT];	
	device_info_t *p_device_info = device_info_get(update);

	if (update)
	{
		//GUI_ClearRect(MAIN_LINE_X_L, MAIN_LINE1_Y, GUI_XMAX, GUI_YMAX);
		paintEmptyDlg();
	}
	
	sprintf(temp_str, "设备号:%04X", p_device_info->id);
	line1_length = addNullStr(temp_str, line1_length);	
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE1_Y);	

	if (p_nmea_gps_msg->flag[0] == 'A' || mac_neighbor_num_get()-1)
	{
		//邻居个数
		sprintf(temp_str, "组网数量[%d]", mac_neighbor_num_get());
	}
	else
	{
		sprintf(temp_str, "组网同步中...");
	}
	line2_length = addNullStr(temp_str, line2_length);
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE2_Y);	

	if (wlan_socket_mcb && wlan_connect_device_num)
	{
		sprintf(temp_str, "本地WiFi连接[%d]成功", wlan_connect_device_num);//"有线连接"
	}
	else if (wlan_connect_device_num)
	{
		sprintf(temp_str, "本地WiFi连接[%d]", wlan_connect_device_num);//"有线连接"
	}
	else
	{
		sprintf(temp_str, "本地连接中...");
	}
	
	line3_length = addNullStr(temp_str, line3_length);	
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE3_Y);
	
	sprintf(temp_str, "左右配置");
	GUI_DispStringAt(temp_str, MAIN_LINE_X_R, MAIN_LINE4_Y);	
}

void paintMainStateDlg(void)
{
	static void *prev_wlan = PLAT_NULL;
	static uint8_t prev_wlan_num = 0;
	
	char_t temp_str[LCD_X_DOT];	
	//device_info_t *p_device_info = device_info_get(update);
	
	if (p_nmea_gps_msg->flag[0] == 'A' || mac_neighbor_num_get()-1)
	{
		//邻居个数
		sprintf(temp_str, "组网数量[%d]", mac_neighbor_num_get());
	}
	else
	{
		sprintf(temp_str, "组网同步中...");
	}
	line2_length = addNullStr(temp_str, line2_length);	
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE2_Y);	

	if (prev_wlan != wlan_socket_mcb)
	{
		if (wlan_socket_mcb)
		{
			sprintf(temp_str, "本地WiFi连接[%d]成功", wlan_connect_device_num);//"有线连接"
		}
		else if (wlan_connect_device_num)
		{
			sprintf(temp_str, "本地WiFi连接[%d]", wlan_connect_device_num);//"有线连接"
		}
		else
		{
			sprintf(temp_str, "本地连接中...");
		}	
		prev_wlan = wlan_socket_mcb;
	}
	else if (prev_wlan_num != wlan_connect_device_num)
	{
		if (wlan_connect_device_num)
		{
			sprintf(temp_str, "本地WiFi连接[%d]", wlan_connect_device_num);//"有线连接"
		}
		else
		{
			sprintf(temp_str, "本地连接中...");
		}		
		prev_wlan_num = wlan_connect_device_num;
	}
	else
	{
		return;
	}
	
	line3_length = addNullStr(temp_str, line3_length);	
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE3_Y);	
}

int8_t paintConfigDlg(bool_t update, int8_t flag)
{
    static int8_t step = 0;
	char_t temp_str[LCD_X_DOT];
    
	if (update)
	{
		//GUI_ClearRect(MAIN_LINE_X_L, MAIN_LINE1_Y, GUI_XMAX, GUI_YMAX);
		paintEmptyDlg();
        
        sprintf(temp_str, " ");
        line1_length = addNullStr(temp_str, line1_length);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE1_Y);

        sprintf(temp_str, " ");
        line2_length = addNullStr(temp_str, line2_length);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE2_Y);

        sprintf(temp_str, " ");
        line3_length = addNullStr(temp_str, line3_length);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE3_Y);
	}

	if (flag == 0) return step;
	
    step += flag;
    
    if (step < 0) step = 0;
    if (step > 2) step = 2;	
    
    if (step == 0)
    {
        GUI_SetTextMode(GUI_TEXTMODE_REV);
		sprintf(temp_str, "WiFi设置");
		line1_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE1_Y);
        GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
		sprintf(temp_str, "定位状态");
		line2_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE2_Y);
        GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
		sprintf(temp_str, "其他设置");
		line3_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE3_Y);
    }
    else if (step == 1)
    {
    	GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
		sprintf(temp_str, "WiFi设置");
		line1_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE1_Y);
        GUI_SetTextMode(GUI_TEXTMODE_REV);
		sprintf(temp_str, "定位状态");
		line2_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE2_Y);
        GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
		sprintf(temp_str, "其他设置");
		line3_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE3_Y);
    }
    else if (step == 2)
    {
		GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
		sprintf(temp_str, "WiFi设置");
		line1_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE1_Y);
        GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
		sprintf(temp_str, "定位状态");
		line2_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE2_Y);
        GUI_SetTextMode(GUI_TEXTMODE_REV);
		sprintf(temp_str, "其他设置");
		line3_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE3_Y);
    }

	line1_length += MAIN_LINE_X_M - MAIN_LINE_X_L;
	line2_length += MAIN_LINE_X_M - MAIN_LINE_X_L;
	line3_length += MAIN_LINE_X_M - MAIN_LINE_X_L;
	
    GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
    //显示基带版本号
    sprintf(temp_str, "v:%x", hal_rf_ds_get_reg(HAL_RF_DS_REG_BB_VER)<<16|hal_rf_of_get_reg(HAL_RF_OF_REG_VERSION));
    
    GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE4_Y);
	GUI_DispStringAt("右键返回", MAIN_LINE_X_R, MAIN_LINE4_Y);

    return step;
}

void paintWiFiDlg(bool_t update)
{
	char_t temp_str[LCD_X_DOT];
	device_info_t *p_device_info = device_info_get(update);

	if (update)
	{
		//GUI_ClearRect(MAIN_LINE_X_L, MAIN_LINE1_Y, GUI_XMAX, GUI_YMAX);
		paintEmptyDlg();
	}

	sprintf(temp_str, "SSID:%s", p_device_info->tmp_wlan_config.ssid);	
	line1_length = addNullStr(temp_str, line1_length);
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE1_Y);

	sprintf(temp_str, "密码:%s", p_device_info->tmp_wlan_config.pwd);
	line2_length = addNullStr(temp_str, line2_length);
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE2_Y);

	if (p_device_info->tmp_wlan_config.channel_no>12)
	{
		sprintf(temp_str, "频段:%03d@5GHz", p_device_info->tmp_wlan_config.channel_no);
	}
	else
	{
		sprintf(temp_str, "频段:%03d@2.4GHz", p_device_info->tmp_wlan_config.channel_no);
	}
	line3_length = addNullStr(temp_str, line3_length);
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE3_Y);

	GUI_DispStringAt("左键随机", MAIN_LINE_X_L, MAIN_LINE4_Y);

	GUI_DispStringAt("右键返回", MAIN_LINE_X_R, MAIN_LINE4_Y);
}

void paintGPSDlg(bool_t update)
{
	char_t temp_str[LCD_X_DOT];
	uint16_t data = 0;
	
	if (update)
	{
		//GUI_ClearRect(MAIN_LINE_X_L, MAIN_LINE1_Y, GUI_XMAX, GUI_YMAX);
		paintEmptyDlg();
	}

	//hal_sensor_get_data(SENSOR_COMPASS, &data);

	if (p_nmea_gps_msg->flag[0] == 'A')
	{
		sprintf(temp_str, "经度:%s-%s", p_nmea_gps_msg->longitude2, p_nmea_gps_msg->longitude);
		line1_length = addNullStr(temp_str, line1_length);
		GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE1_Y);

		sprintf(temp_str, "纬度:%s-%s",p_nmea_gps_msg->latitude2, p_nmea_gps_msg->latitude);
		line2_length = addNullStr(temp_str, line2_length);
		GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE2_Y);
		p_nmea_gps_msg->altitude[6] = '\0';//卡一下显示的字符串长度
		sprintf(temp_str, "高度:%s-方向:%03d", p_nmea_gps_msg->altitude, data);
		line3_length = addNullStr(temp_str, line3_length);
		GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE3_Y);
	}
	else
	{
		sprintf(temp_str, "经度:X-DDDMM.MMMM");
		line1_length = addNullStr(temp_str, line1_length);
		GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE1_Y);

		sprintf(temp_str, "纬度:X-DDMM.MMMM");
		line2_length = addNullStr(temp_str, line2_length);
		GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE2_Y);

		sprintf(temp_str, "高度:0000 方向:%03d", data);
		line3_length = addNullStr(temp_str, line3_length);
		GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE3_Y);
	}	

	GUI_DispStringAt("右键返回", MAIN_LINE_X_R, MAIN_LINE4_Y);
}

int8_t paintSubConfigDlg(bool_t update, int8_t flag)
{
	static int8_t step = 0;	
	char_t temp_str[LCD_X_DOT];
	device_info_t *p_device_info = device_info_get(update);
    
	if (update)
	{
		//GUI_ClearRect(MAIN_LINE_X_L, MAIN_LINE1_Y, GUI_XMAX, GUI_YMAX);
		paintEmptyDlg();
        
        sprintf(temp_str, " ");
        line1_length = addNullStr(temp_str, line1_length);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE1_Y);

        sprintf(temp_str, " ");
        line2_length = addNullStr(temp_str, line2_length);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE2_Y);

        sprintf(temp_str, " ");
        line3_length = addNullStr(temp_str, line3_length);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE3_Y);
	}

	//if (flag == 0) return step;
	
    step += flag;
    
    if (step < 0) step = 0;
    if (step > 1) step = 1;	
    
    if (step == 0)
    {
        GUI_SetTextMode(GUI_TEXTMODE_REV);
		if (p_device_info->tmp_func_bit.audio == 0)
		{
			sprintf(temp_str, "话音:关");
		}
		else if (p_device_info->tmp_func_bit.audio == 1)
		{
			sprintf(temp_str, "话音:低");
		}
		else if (p_device_info->tmp_func_bit.audio == 2)
		{
			sprintf(temp_str, "话音:中");
		}
		else if (p_device_info->tmp_func_bit.audio == 3)
		{
			sprintf(temp_str, "话音:高");
		}
		line1_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE1_Y);
        GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
		if (p_device_info->tmp_func_bit.wifi == 0)
		{
			sprintf(temp_str, "WiFi:关");
		}
		else if (p_device_info->tmp_func_bit.wifi == 1)
		{
			sprintf(temp_str, "WiFi:开");
		}
		
		line2_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE2_Y);      	
    }
    else if (step == 1)
    {
		GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
		if (p_device_info->tmp_func_bit.audio == 0)
		{
			sprintf(temp_str, "话音:关");
		}
		else if (p_device_info->tmp_func_bit.audio == 1)
		{
			sprintf(temp_str, "话音:低");
		}
		else if (p_device_info->tmp_func_bit.audio == 2)
		{
			sprintf(temp_str, "话音:中");
		}
		else if (p_device_info->tmp_func_bit.audio == 3)
		{
			sprintf(temp_str, "话音:高");
		}
		line1_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE1_Y);
        GUI_SetTextMode(GUI_TEXTMODE_REV);
		if (p_device_info->tmp_func_bit.wifi == 0)
		{
			sprintf(temp_str, "WiFi:关");
		}
		else if (p_device_info->tmp_func_bit.wifi == 1)
		{
			sprintf(temp_str, "WiFi:开");
		}
		
		line2_length = strlen(temp_str);
        GUI_DispStringAt(temp_str, MAIN_LINE_X_M, MAIN_LINE2_Y);   
    }

	line1_length += MAIN_LINE_X_M - MAIN_LINE_X_L;
	line2_length += MAIN_LINE_X_M - MAIN_LINE_X_L;
	//line3_length += MAIN_LINE_X_M - MAIN_LINE_X_L;
	
    GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
	GUI_DispStringAt("左键保存", MAIN_LINE_X_L, MAIN_LINE4_Y);
	GUI_DispStringAt("右键返回", MAIN_LINE_X_R, MAIN_LINE4_Y);

    return step;
}

void paintShutDownDlg(void)
{
	GUI_Clear();
	GUI_DispStringAt("关机!", MAIN_LINE_X_M+16, MAIN_LINE2_Y);	
}

void paintResetDlg(void)
{
	GUI_Clear();
	GUI_DispStringAt("重启设备中!", MAIN_LINE_X_M, MAIN_LINE2_Y);
}

void paintEmptyDlg(void)
{
	char_t temp_str[LCD_X_DOT];

	strcpy(temp_str, "                    ");

	//GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE1_Y);
	//GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE2_Y);
	//GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE3_Y);
	GUI_DispStringAt(temp_str, MAIN_LINE_X_L, MAIN_LINE4_Y);
}

//传递一个字符串和需要的长度，如果字符串长度小于len,则在str后部补齐len长度
static uint8_t addNullStr(char_t *str, uint8_t act_len)
{
	uint8_t str_len = strlen(str);
	int8_t str_diff_len = act_len - str_len;
	char_t null_str[LCD_X_DOT];
	
	if (str_diff_len > 0)
	{
		mem_set(null_str, ' ', str_diff_len);

		null_str[str_diff_len] = '\0';
		
		strcat(str, null_str);
	}

	return str_len;
}
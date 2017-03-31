#include <gui_1.h>
#include <os_cfg.h>
#include <GUITouchConf.h>
#include <platform.h>
#include "Control_IO.h"
#include "GUI.h"

#include "app.h"
#include "device.h"
#include "wlan.h"

#define GUI_TASK_STK_SIZE			OSEL_TASK_STACK_SIZE_MAX
#define GUI_TASK_PRIO				OSEL_TASK_PRIO(5)
#define KEY_PRESSED_CNT				5
#define KEY_PRESSED_LONG_CNT		199
#define POWER_OFF_CNT				200
#define KEY_PRESSED_PTT_CNT			300

#define MACHINE_STEP_MAIN			0x00
#define MACHINE_STEP_CFG			0x10
#define MACHINE_STEP_CFG_WIFI		0x11
#define MACHINE_STEP_CFG_GPS		0x12
#define MACHINE_STEP_CFG_MISC		0x13

typedef struct
{
  	int32_t right;
	int32_t left;
	int32_t up;
	int32_t down;
	int32_t mid;
	int32_t ptt;
}key_t;

OSEL_DECLARE_TASK(GUI_TASK, param);

static osel_task_t *gui_task_handle;
static osel_event_t *gui_lock_event_handle;
static fpv_t gui_hook_func = PLAT_NULL;
static key_t key;

uint8_t machine_step = 0;
//test
//extern void test(void);

void GUI_REG_HOOK(fpv_t func)
{
	gui_hook_func = func;
}


void GUI_X_Init(void)
{
	//Empty
}

void LCD_InitPanel()
{	
	//LCD �������ܽ�
	hal_lcd_init();

	key.left = 0;
	key.down = 0;
	key.right = 0;
	key.mid = 0;
	key.up = 0;
}
#if 1

void GUI_Key_X_Exec(void)
{
	//int Pressed = 0;
	//int Key = 0;
	uint8_t key_value;
	static bool_t screen_closed = PLAT_FALSE;
	pbuf_t *pbuf;
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);
	static uint32_t refresh_cnt = 0;
	static bool_t m_pre_ptt_state = 0;
	//�������
	key_value = ControlIO_KeyStatus();

	//�жϰ����������Ĺ�ϵ
	if (machine_step == MACHINE_STEP_MAIN)
	{
		if (key.right>KEY_PRESSED_PTT_CNT 
			|| key.left>KEY_PRESSED_PTT_CNT
			|| key.down>KEY_PRESSED_PTT_CNT
			|| key.up>KEY_PRESSED_PTT_CNT
			|| key.mid >KEY_PRESSED_PTT_CNT
			|| key.ptt >KEY_PRESSED_PTT_CNT)
		{
			app_audio.mcb.m_ptt_state = AMBE_PTT_DOWN;
			//DBG_PRINTF("T");
			if (p_device_info->func_bit.audio)
			{
				if (m_pre_ptt_state != app_audio.mcb.m_ptt_state)
				{
					paintTitleDlg(PLAT_FALSE, TITLE_TYPE_SPEAK_ON);
				}
				
			}
		}
		else
		{
			app_audio.mcb.m_ptt_state = AMBE_PTT_UP;
			
			if (p_device_info->func_bit.audio)
			{
				if (m_pre_ptt_state != app_audio.mcb.m_ptt_state)
				{
					paintTitleDlg(PLAT_FALSE, TITLE_TYPE_SPEAK_OFF);
				}
			}			
		}

		m_pre_ptt_state = app_audio.mcb.m_ptt_state;

		if (refresh_cnt >= 1000)
		{
			//����ˢ��
			refresh_cnt = 0;			
			paintTitleDlg(PLAT_FALSE, TITLE_TYPE_ALL);
			//ˢ״̬
			paintMainStateDlg();
		}
		refresh_cnt++;
	}
	else
	{
		app_audio.mcb.m_ptt_state = AMBE_PTT_UP;
		if (p_device_info->func_bit.audio)
		{
			paintTitleDlg(PLAT_FALSE, TITLE_TYPE_SPEAK_OFF);
		}
		
		if (refresh_cnt >= 1000)
		{
			refresh_cnt = 0;
			if (machine_step == MACHINE_STEP_CFG_GPS)
			{
				paintGPSDlg(PLAT_FALSE);
			}

			paintTitleDlg(PLAT_FALSE, TITLE_TYPE_ALL);
		}
		refresh_cnt++;
	}
	//�ж϶����˵�PTT����
	if (key_value & PTT_IO_KEY)
	{
		if (screen_closed == PLAT_FALSE)
		{
			key.ptt++;
		}
		
		else if (key.ptt == KEY_PRESSED_LONG_CNT)
		{			
			//PTT��������Ч��		
		}
		else if (key.ptt == KEY_PRESSED_CNT)
		{			
			//PTT�����̰�Ч��			
		}
	}
	else
	{	
		if (key.ptt >= KEY_PRESSED_CNT && key.ptt < KEY_PRESSED_LONG_CNT)
		{
			//PTT�����ͷŶ̰�Ч��			
		}
		else if (key.ptt >= KEY_PRESSED_LONG_CNT && key.ptt < POWER_OFF_CNT)
		{
			//PTT�����ͷų���Ч��
		}
		else if (key.ptt >= POWER_OFF_CNT)
		{
			//PTT�����ͷ����Ч��
		}
		key.ptt = 0;
	}
	
	
	//�ж���Ч�İ�������
	if (key_value & RIGTH_IO_KEY)
	{
		if (screen_closed == PLAT_FALSE)
		{
			key.right++;
		}
		else if (key.right == 0)
		{
			//�ɴ���Ļ����
			//hal_lcd_backlight(PLAT_TRUE);			
			//screen_closed = PLAT_FALSE;
		}
		
		if (key.right >= POWER_OFF_CNT)
		{
			if (key.mid)
			{
				//�ػ���Ҫ�м䰴�����Ҽ�һ����
				DBG_TRACE("Power Off Pressed\r\n");
				//MESSAGEBOX_Create("Power Off", NULL, 0);
				paintShutDownDlg();
				osel_systick_delay(1000);
				ControlIO_PowerOff();
			}			
		}
		else if (key.right == KEY_PRESSED_LONG_CNT)
		{			
			//�Ҽ�����Ч��		
		}
		else if (key.right == KEY_PRESSED_CNT)
		{			
			//�Ҽ��̰�Ч��
			if (key.left)//������
			{
				if (machine_step == MACHINE_STEP_MAIN)
				{					
					paintConfigDlg(PLAT_TRUE, -1);
					machine_step = MACHINE_STEP_CFG;
				}
                key.left = 0;
			}
		}
	}
	else
	{	
		if (key.right >= KEY_PRESSED_CNT && key.right < KEY_PRESSED_LONG_CNT)
		{
			//�Ҽ��ͷŶ̰�Ч��
			if (machine_step == MACHINE_STEP_CFG_WIFI 
				|| machine_step == MACHINE_STEP_CFG_GPS
				|| machine_step == MACHINE_STEP_CFG_MISC)
			{
				machine_step = MACHINE_STEP_CFG;
				paintConfigDlg(PLAT_TRUE, -1);
			}
			else if (machine_step != MACHINE_STEP_MAIN)
			{
                if (key.left == 0)
                {
                	paintMainDlg(PLAT_TRUE);
                    machine_step = MACHINE_STEP_MAIN;                    
                }				
			}
		}
		else if (key.right >= KEY_PRESSED_LONG_CNT && key.right < POWER_OFF_CNT)
		{
			//�Ҽ��ͷų���Ч��
		}
		else if (key.right >= POWER_OFF_CNT)
		{
			//�Ҽ��ͷ����Ч��
		}
		key.right = 0;
	}

	if (key_value & UP_IO_KEY)
	{
		if (screen_closed == PLAT_FALSE)
		{
			key.up++;
		}
		else if (key.up == 0)
		{
			//�ɴ���Ļ����
			//hal_lcd_backlight(PLAT_TRUE);			
			//screen_closed = PLAT_FALSE;
		}
		
		if (key.up == KEY_PRESSED_CNT)
		{			
			//�ϼ��̰�Ч��
			if (machine_step == MACHINE_STEP_CFG)
			{
				paintConfigDlg(PLAT_FALSE, -1);
			}
			else if (machine_step == MACHINE_STEP_CFG_MISC)
			{
				paintSubConfigDlg(PLAT_FALSE, -1);
			}
		}	
	}
	else
	{
		if (key.up >= KEY_PRESSED_CNT)
		{			
			//�ϼ��ͷŶ̰�Ч��		
		}
		key.up = 0;
	}

	if (key_value & DOWN_IO_KEY)
	{
		if (screen_closed == PLAT_FALSE)
		{
			key.down++;
		}
		else if (key.down == 0)
		{
			//�ɴ���Ļ����
			//hal_lcd_backlight(PLAT_TRUE);			
			//screen_closed = PLAT_FALSE;
		}
		
		if (key.down == KEY_PRESSED_CNT)
		{
			//�¼��̰�Ч��
			if (machine_step == MACHINE_STEP_CFG)
			{
				paintConfigDlg(PLAT_FALSE, 1);
			}
			else if (machine_step == MACHINE_STEP_CFG_MISC)
			{
				paintSubConfigDlg(PLAT_FALSE, 1);
			}
		}
	}
	else
	{
		if (key.down >= KEY_PRESSED_CNT)
		{			
			//�¼��ͷŶ̰�Ч��
		}
		key.down = 0;
	}

	if (key_value & LEFT_IO_KEY)
	{
		if (screen_closed == PLAT_FALSE)
		{
			key.left++;
		}
		else if (key.left == 0)
		{
			//�ɴ���Ļ����
			//hal_lcd_backlight(PLAT_TRUE);			
			//screen_closed = PLAT_FALSE;
		}
		
		if (key.left == KEY_PRESSED_LONG_CNT)
		{			
			//�������Ч��
		}		
		else if (key.left == KEY_PRESSED_CNT)
		{			
			//����̰�Ч��
		}
	}
	else
	{
		if (key.left >= KEY_PRESSED_CNT && key.left < KEY_PRESSED_LONG_CNT)
		{
			//����ͷŶ̰�Ч��
			if (machine_step == MACHINE_STEP_CFG_WIFI)
			{
				//�������WIFI�������SSID��Ƶ��				
				sprintf(p_device_info->tmp_wlan_config.pwd, "%010x", rand());
				//��֧��26��ͨ��2.4 1-11 5 36~48 149-161 165
				p_device_info->tmp_wlan_config.channel_no = rand()%26+1;
				//����Ƶ��
				if (p_device_info->tmp_wlan_config.channel_no>=12
					&& p_device_info->tmp_wlan_config.channel_no<=18)
				{
					p_device_info->tmp_wlan_config.channel_no -= 12;
					p_device_info->tmp_wlan_config.channel_no = 
						p_device_info->tmp_wlan_config.channel_no*2 + 36; 
				}
				else if (p_device_info->tmp_wlan_config.channel_no>=19
					&& p_device_info->tmp_wlan_config.channel_no<=25
					)
				{
					p_device_info->tmp_wlan_config.channel_no -= 19;
					p_device_info->tmp_wlan_config.channel_no = 
						p_device_info->tmp_wlan_config.channel_no*2 + 149;
				}
				else if (p_device_info->tmp_wlan_config.channel_no==26)
				{
					p_device_info->tmp_wlan_config.channel_no = 165;
				}
				paintWiFiDlg(PLAT_FALSE);
			}
			else if (machine_step == MACHINE_STEP_CFG_MISC)
			{
				//�����������õ�ֵ
				//wifi�б仯���������豸,Ҳ�����˻����ı���
				if (p_device_info->tmp_func_bit.wifi != p_device_info->func_bit.wifi)
				{
                    p_device_info->boot_flag = 1;//������
					device_info_set(p_device_info, PLAT_FALSE);
					paintResetDlg();
					osel_systick_delay(500);
					osel_event_set(test_event_h, PLAT_NULL);				
				}
				//�����б仯
				if (p_device_info->tmp_func_bit.audio != p_device_info->func_bit.audio)
				{
					device_info_set(p_device_info, PLAT_FALSE);
					do
					{
						pbuf = (pbuf_t *)list_front_get(&app_audio.pbuf_rx_list);
						if (pbuf) pbuf_free(pbuf);
					}while(pbuf);

					do
					{
						pbuf = (pbuf_t *)list_front_get(&app_audio.pbuf_tx_list);
						if (pbuf) pbuf_free(pbuf);
					}while(pbuf);
						
					if (p_device_info->func_bit.audio == 0)
					{
						//�ر�����ͨ�����ͷ���������
						ControlIO_Power(AUDIO_POWER, PLAT_FALSE);
						hal_audio_deinit();						
					}
					else
					{                        
						//��ʼ����������������
						ControlIO_Power(AUDIO_POWER, PLAT_TRUE);
						mem_set(&app_audio, 0, sizeof(app_audio_t));
						list_init(&app_audio.pbuf_rx_list);
						list_init(&app_audio.pbuf_tx_list);						
				    	hal_audio_init(PLAT_NULL, PLAT_NULL);
						hal_audio_change_gain(p_device_info->func_bit.audio);
					}
					//ˢ�±���
					paintTitleDlg(PLAT_FALSE, TITLE_TYPE_AUDIO);
				}
				//��������������				
				paintConfigDlg(PLAT_TRUE, 1);
				machine_step = MACHINE_STEP_CFG;
			}
            
		}
		else if (key.left >= KEY_PRESSED_LONG_CNT)
		{
			//����ͷŶ̰�Ч��
		}
		key.left = 0;
	}

	if (key_value & MID_IO_KEY)
	{
		if (screen_closed == PLAT_FALSE)
		{
			key.mid++;
		}
		else if (key.mid == 0)
		{
			//�ɴ���Ļ����
			//hal_lcd_backlight(PLAT_TRUE);			
			//screen_closed = PLAT_FALSE;
		}
		
		if (key.mid == KEY_PRESSED_LONG_CNT)
		{
			//�м�����Ч��
		}		
		else if (key.mid == KEY_PRESSED_CNT)
		{
			//�м��̰�Ч��
		}
	}
	else
	{
		if (key.mid >= KEY_PRESSED_CNT && key.mid < KEY_PRESSED_LONG_CNT)
		{
			//�м��ͷŶ̰�Ч��
			if (machine_step == MACHINE_STEP_CFG)
			{
				if (paintConfigDlg(PLAT_FALSE, 0) == 0)
				{
					paintWiFiDlg(PLAT_TRUE);
					machine_step = MACHINE_STEP_CFG_WIFI;					
				}
				else if (paintConfigDlg(PLAT_FALSE, 0) == 1)
				{					
					paintGPSDlg(PLAT_TRUE);
					machine_step = MACHINE_STEP_CFG_GPS;
				}
				else if (paintConfigDlg(PLAT_FALSE, 0) == 2)
				{					
					paintSubConfigDlg(PLAT_TRUE, -1);
					machine_step = MACHINE_STEP_CFG_MISC;
				}
			}
			else if (machine_step == MACHINE_STEP_CFG_WIFI)
			{				
				//�Ƚϲ����Ƿ��޸�
				if (p_device_info->func_bit.wifi
                    &&  memcmp(&p_device_info->tmp_wlan_config, &p_device_info->wlan_config, sizeof(wlan_config_t)) != 0)
				{
                    p_device_info->boot_flag = 1;//������
                    device_info_set(p_device_info, PLAT_FALSE);
					paintResetDlg();
					osel_systick_delay(500);
					hal_board_reset();
				}
				else
				{
					//��������ѡ�����
					paintConfigDlg(PLAT_TRUE, 1);
				}
				machine_step = MACHINE_STEP_CFG;
			}
			else if (machine_step == MACHINE_STEP_CFG_MISC)
			{
				//��ʾwifi��������ֵ�仯
				if (paintSubConfigDlg(PLAT_FALSE, 0) == 0)
				{
					if (p_device_info->tmp_func_bit.audio == 3)
					{
						p_device_info->tmp_func_bit.audio = 0;
					}
					else
					{
						p_device_info->tmp_func_bit.audio++;
					}					
				}
				else if (paintSubConfigDlg(PLAT_FALSE, 0) == 1)
				{
					
					if (p_device_info->tmp_func_bit.wifi == 1)
					{
						p_device_info->tmp_func_bit.wifi = 0;
					}
					else
					{
						p_device_info->tmp_func_bit.wifi++;
					}
				}
				paintSubConfigDlg(PLAT_FALSE, 0);
			}
		}
		else if (key.mid >= KEY_PRESSED_LONG_CNT)
		{
			//�м��ͷų���Ч��
		}
		key.mid = 0;
	}	
}
#endif
/**** ExecIdle - called if nothing else is left to do ****/
void GUI_X_ExecIdle(void)
{
	osel_systick_delay(2);
}

/**** Timing routines - required for blinking ****/
int  GUI_X_GetTime(void)
{
	return osel_systick_get();
}

void GUI_X_Delay(int Period)
{
	osel_systick_delay(Period);    
}

/**** Multitask routines - required only if multitasking is used (#define GUI_OS 1) ****/
//OSEL_DECL_CRITICAL();

void GUI_X_Unlock(void)
{
	osel_event_set(gui_lock_event_handle, NULL);    
    //OSEL_ENTER_CRITICAL();
}
void GUI_X_Lock(void)
{
	osel_event_wait(gui_lock_event_handle, OSEL_WAIT_FOREVER);
	//OSEL_EXIT_CRITICAL();
}

uint32_t  GUI_X_GetTaskId(void)
{
	return gui_task_handle->prio;
}

void GUI_X_InitOS(void)
{
	/*���� GUI ���� */
    gui_task_handle = osel_task_create(GUI_TASK,
                                         NULL,
                                         GUI_TASK_STK_SIZE,
                                         GUI_TASK_PRIO
                                        );
    DBG_ASSERT(gui_task_handle != NULL);	
    gui_lock_event_handle = osel_event_create(OSEL_EVENT_TYPE_SEM, 1);
	DBG_ASSERT(gui_lock_event_handle != NULL);

}

OSEL_DECLARE_TASK(GUI_TASK, param)
{
    (void)param;

	osel_systick_delay(3000);
	
    while(1)
    {
    	GUI_Key_X_Exec();
    	GUI_Exec();
		if (gui_hook_func != PLAT_NULL)
		{
			(gui_hook_func)();
		}
    	GUI_X_ExecIdle();		
    }
}

/**** Recording (logs/warnings and errors) - required only for higher levels ****/
void GUI_X_Log(const char *s)
{
	DBG_TRACE("[L]%s\r\n", s);
}

void GUI_X_Warn(const char *s)
{
	DBG_TRACE("[W]%s\r\n", s);
}

void GUI_X_ErrorOut(const char *s)
{
	DBG_TRACE("[E]%s\r\n", s);
}
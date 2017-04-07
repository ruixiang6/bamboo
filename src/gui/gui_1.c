#include <gui_1.h>
#include <os_cfg.h>
#include <GUITouchConf.h>
#include <platform.h>
#include "Control_IO.h"
#include "GUI.h"
#include <app.h>
#include <device.h>

static osel_event_t *gui_lock_event_handle;
static key_status_t key;

void GUI_X_Init(void)
{
	//
	mem_clr(&key, sizeof(key_status_t));
}

void LCD_InitPanel()
{	
	//LCD 控制器管脚
	hal_lcd_init();
}

void GUI_Key_X_Exec(void)
{
	uint8_t key_value;
	
	key_value = ControlIO_KeyStatus();
	////////////////////////////////
	if (key_value & PTT_KEY)
	{
		//DBG_TRACE("PTT_KEY Pressed\r\n");
		key.ptt++;
	}
	else
	{
		key.ptt = 0;
	}
	/////////////////////////////////
	if (key_value & RIGHT_DOWN_KEY)
	{
		//DBG_TRACE("RIGHT_DOWN_KEY Pressed\r\n");
		key.right_down++;
        if (key.right_down>1000)
		{
			//关机
			uint16_t object = APP_EVENT_SHUTDOWN;
			osel_event_set(app_event_h, &object);
		}
	}
	else
	{
		key.right_down = 0;
	}
	/////////////////////////////////
	if (key_value & LEFT_DOWN_KEY)
	{
		//DBG_TRACE("LEFT_DOWN_KEY Pressed\r\n");
		key.left_down++;
	}
	else
	{
		key.left_down = 0;
	}
	/////////////////////////////////
	if (key_value & MID_KEY)
	{
		//DBG_TRACE("MID_KEY Pressed\r\n");
		key.mid++;
	}
	else
	{
		key.mid = 0;
	}
	//////////////////////////////////
	if (key_value & LEFT_UP_KEY)
	{
		//DBG_TRACE("LEFT_UP_KEY Pressed\r\n");
		key.left_up++;
	}
	else
	{
		key.left_up = 0;
	}
	//////////////////////////////////
	if (key_value & RIGHT_UP_KEY)
	{
		//DBG_TRACE("RIGHT_UP_KEY Pressed\r\n");
		key.right_up++;		
	}
	else
	{
		key.right_up = 0;
	}
}

/**** ExecIdle - called if nothing else is left to do ****/
void GUI_X_ExecIdle(void)
{
	
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
//static OSEL_DECL_CRITICAL();

void GUI_X_Unlock(void)
{	
    //OSEL_ENTER_CRITICAL();
    osel_event_set(gui_lock_event_handle, NULL);   
}
void GUI_X_Lock(void)
{	
	//OSEL_EXIT_CRITICAL();
	osel_event_set(gui_lock_event_handle, NULL);   
}

uint32_t  GUI_X_GetTaskId(void)
{
	return 0;
}

void GUI_X_InitOS(void)
{	
	gui_lock_event_handle = osel_event_create(OSEL_EVENT_TYPE_SEM, 1);
	DBG_ASSERT(gui_lock_event_handle != NULL);
}

void gui_proc()
{    
    GUI_Key_X_Exec();
    GUI_Exec();		
    GUI_X_ExecIdle();
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

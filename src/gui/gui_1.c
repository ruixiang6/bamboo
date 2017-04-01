#include <gui_1.h>
#include <os_cfg.h>
#include <GUITouchConf.h>
#include <platform.h>
#include "Control_IO.h"
#include "GUI.h"
#include <app.h>
#include <device.h>

static osel_event_t *gui_lock_event_handle;

void GUI_X_Init(void)
{
	//Empty
}

void LCD_InitPanel()
{	
	//LCD 控制器管脚
	hal_lcd_init();
}

void GUI_Key_X_Exec(void)
{

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
#include <platform.h>
#include <GUI.h>
#include <app.h>
#include <device.h>

void paintBootPicDlg(void)
{
	GUI_DrawBitmap(&bmbootimg, 0, 0);
    osel_systick_delay(2000);
}

void paintBootDlg(void)
{	
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);	

	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_SimSun_12);
	GUI_DispStringAt(FONT_ZZWCSSB, MAIN_LINE_X_TITLE, MAIN_LINE2_Y);
	GUI_DispStringAt(p_device_info->version, MAIN_LINE_X_M, MAIN_LINE3_Y);
	osel_systick_delay(2000);
}

void paintShutDownDlg(void)
{
	GUI_Clear();
	GUI_DispStringAt(FONT_GJ, MAIN_LINE_X_M+16, MAIN_LINE2_Y);	
}
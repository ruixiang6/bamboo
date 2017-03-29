#include <platform.h>
#include "Control_IO.h"

#define POWER_ON()				{hal_gpio_output(PWR_LOCK, 1);}
#define POWER_OFF()				{hal_gpio_output(PWR_LOCK, 0);}

#define SHIFT_OUT_CLK_HIGH()	{hal_gpio_output(PWR_SHIFT_CLK, 1);}
#define SHIFT_OUT_CLK_LOW()		{hal_gpio_output(PWR_SHIFT_CLK, 0);}
#define SHIFT_OUT_DIN_HIGH()	{hal_gpio_output(PWR_SHIFT_DAT, 1);}
#define SHIFT_OUT_DIN_LOW()		{hal_gpio_output(PWR_SHIFT_DAT, 0);}
#define SHIFT_OUT_LATCH_HIGH()	{hal_gpio_output(PWR_SHIFT_LOCK, 1);}
#define SHIFT_OUT_LATCH_LOW()	{hal_gpio_output(PWR_SHIFT_LOCK, 0);}
#define SHIFT_OUT_CLR_HIGH()	{hal_gpio_output(PWR_SHIFT_CLR, 1);}
#define SHIFT_OUT_CLR_LOW()		{hal_gpio_output(PWR_SHIFT_CLR, 0);}

device_type_t dev_type = DEV_NULL;

static void ControlIO_Output(uint8_t bitmap);

void ControlIO_Init(void)
{
	dev_type = ControlIO_Get_Version();

	POWER_ON();
	//ControlIO_DecPowerOn();

	SHIFT_OUT_CLR_HIGH();
	ControlIO_Output(0x0);

	switch(dev_type)
	{
		case HANDSET:
			ControlIO_Power(HD_PA1_6002_POWER, PLAT_TRUE);
			ControlIO_Power(HD_GPS_POWER, PLAT_TRUE);
			//ControlIO_Power(HD_PA2_POWER, PLAT_TRUE);
			ControlIO_Power(HD_SPEAKER_POWER, PLAT_TRUE);			
			//ControlIO_Power(HD_AMBE1K_POWER, PLAT_TRUE);
			ControlIO_Power(HD_PHY_WIFI_POWER, PLAT_TRUE);
			ControlIO_Power(HD_OLED_POWER, PLAT_TRUE);
			break;
		default:break;
	}
}

device_type_t ControlIO_Get_Version(void)
{
	uint8_t version = 0;
	
	version |= hal_gpio_input(VERSION_0);
	version |= hal_gpio_input(VERSION_1)<<1;
	version |= hal_gpio_input(VERSION_2)<<2;

	return (device_type_t)version;
}
 
void ControlIO_DecPowerOn(void)
{
	switch(dev_type)
	{
		case HANDSET:
			for (uint8_t i=0; i<150; i++)
			{
				if (hal_gpio_input(HD_KEY_5))
				{
					POWER_OFF();
					DBG_TRACE("Power off\r\n");
					while(1);
				}
				delay_ms(20);
			}
			DBG_TRACE("Power on\r\n");
			break;
		default:break;
	}
}

void ControlIO_PowerOff(void)
{
	DBG_TRACE("Power off\r\n");
	POWER_OFF();
	while(1);
}

uint8_t ControlIO_Power(uint8_t power, bool_t flag)
{
	static uint8_t bitmap = 0;

	if (power == 0) return bitmap;

	for (uint8_t index= 0; index<8; index++)
	{
		if (power & (1u<<index))
		{
			if (flag)
			{
				bitmap |= (1u<<index);
			}
			else
			{
				bitmap &= ~(1u<<index);
			}
		}
	}

	ControlIO_Output(bitmap);

	return bitmap;
}

/*******************************************/
//|07|06|05|04|03|02|01|00|
/*******************************************/
static void ControlIO_Output(uint8_t bitmap)
{
	uint8_t i;

	SHIFT_OUT_LATCH_HIGH();
	SHIFT_OUT_DIN_LOW();
	for (i=0; i<8; i++)
	{

		if ((bitmap>>(7-i))&0x01)
		{
			SHIFT_OUT_DIN_HIGH();
		}
		else
		{
			SHIFT_OUT_DIN_LOW();
		}
		SHIFT_OUT_CLK_LOW();
		SHIFT_OUT_CLK_HIGH();
	}
	SHIFT_OUT_DIN_LOW();
	SHIFT_OUT_LATCH_LOW();
	SHIFT_OUT_LATCH_HIGH();
	SHIFT_OUT_CLK_LOW();
}



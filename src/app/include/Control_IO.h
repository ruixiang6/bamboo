#ifndef _CONTROL_IO_H
#define _CONTROL_IO_H

typedef enum
{
	HD_KEY_0 = 5,			//in
	HD_KEY_1 = 9,			//in
	HD_KEY_2 = 11,			//in
	HD_KEY_3 = 12,			//in
	HD_KEY_4 = 25,			//in
	HD_KEY_5 = 26,			//in
	HD_PWR_LOCK = 1,		//out	
	HD_WLAN_STATE = 7,		//in
	HD_DP_DET = 8,			//in
	HD_RES_0 = 3,			//in
	HD_RES_1 = 4,			//out
	HD_RES_2 = 23,			//nc
	HD_RES_3 = 6,			//nc	
}handset_io_t;

typedef enum
{
	RIGHT_DOWN_KEY = (1u<<0),
	LEFT_DOWN_KEY = (1u<<1),
	MID_KEY = (1u<<2),
	LEFT_UP_KEY = (1u<<3),
	RIGHT_UP_KEY = (1u<<4),
	PTT_KEY = (1u<<5),
}key_t;

typedef struct
{
  	int32_t right_down;
	int32_t left_down;
	int32_t left_up;
	int32_t right_up;
	int32_t mid;
	int32_t ptt;
}key_status_t;


typedef enum
{
	HD_PA1_6002_POWER 	= (1u<<0),
	HD_PA2_POWER 		= (1u<<1),
	HD_SPEAKER_POWER	= (1u<<2),
	HD_GPS_POWER 		= (1u<<3),
	HD_AMBE1K_POWER 	= (1u<<4),
	HD_PHY_WIFI_POWER 	= (1u<<5),
	HD_OLED_POWER 		= (1u<<6),
}handset_power_t;

typedef enum
{
	VERSION_0 = 0,		//in
	VERSION_1 = 10,		//in
	VERSION_2 = 29,		//in
	PWR_LOCK = 1,		//out
	PWR_SHIFT_LOCK = 2,	//out
	PWR_SHIFT_CLR =	14,	//out
	PWR_SHIFT_CLK = 15,	//out
	PWR_SHIFT_DAT = 16,	//out
	OLCD_RST = 27,		//out
	OLCD_DC = 13,		//out
	ETH_RST = 31,		//out
	AMBE_SOCK = 17,		//out
	AMBE_SDO = 18,		//in
	AMBE_SICK = 19,		//out
	AMBE_SDI = 20,		//out
	AMBE_SISTRB = 21,	//out
	AMBE_SOSTRB = 22,	//out
	AMBE_EPR = 30,		//in
	AMBE_DPE = 24,		//in
	AMBE_RST = 28,		//out	
}common_io_t;

typedef enum
{
	DEV_NULL = 0,
	HANDSET	 = 1,
	REPEATER = 2,
	PORTABLE = 3
}device_type_t;

extern device_type_t dev_type;

void ControlIO_Init(void);
device_type_t ControlIO_Get_Version(void);
void ControlIO_PowerOff(void);
void ControlIO_DecPowerOn(void);
uint8_t ControlIO_Power(uint8_t power, bool_t flag);
uint8_t ControlIO_KeyStatus(void);


#endif


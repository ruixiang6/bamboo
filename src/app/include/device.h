#ifndef _DEVICE_H
#define _DEVICE_H

#include <platform.h>
#include <nwk.h>

#define GET_NWK_ID(a)				(a[0])
#define GET_MESH_ID(a)				(a[1])
#define GET_DEV_ID(a)				(a[2]|(a[3]<<8)|(a[4]<<16))
#define SET_NWK_ID(a, b)			{a[0] = b;}
#define SET_MESH_ID(a, b)			{a[1] = b;}
#define SET_DEV_ID(a, b)			{a[2] = b&0xFF; a[3] = (b>>8)&0xFF; a[4] = (b>>16)&0xFF;}

#pragma pack(1)

/**
 * device time
 */
typedef struct
{
	uint32_t year		:6,
	 		 month		:4,
	 		 day		:5,
	 		 hour		:5,
	 		 minute		:6,
	 		 second		:6;
} dev_time_t;

/**
 * device position
 */
typedef struct __s_dev_pos
{
	
	uint16_t long_int;		/**< longitude integer part */
	uint16_t long_dec;		/**< longitude decimal part */
	bool_t we;
	
	uint16_t lat_int;		/**< latitude integer part */
	uint16_t lat_dec;		/**< latitude decimal part */
	bool_t sn;
} dev_pos_t;

typedef struct
{
	uint32_t 	dummy:	24,
				mode:	8;
}dev_func_bit_t;

typedef struct
{
    uint8_t id[5];
	char_t version[10];
	
	union
	{
		dev_func_bit_t func_bit;
		uint32_t func_word;
	};	

	uint8_t eth_local_mac_addr[6];
	//dont save
	dev_pos_t pos;	
	dev_time_t time;	
}device_info_t;

#pragma pack()

void device_info_init(void);
device_info_t *device_info_get(bool_t flag);
bool_t device_info_set(device_info_t *device_info, bool_t force_update);

#endif
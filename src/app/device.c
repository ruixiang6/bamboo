#include <platform.h>
#include "device.h"
#include <string.h>

#define VERSION		"V1.0.2.3"
#define SAVE_SIZE	(sizeof(device_info_t)-sizeof(dev_time_t)-sizeof(dev_pos_t))

#define DEVICE_BASE_ADDR		HAL_FLASH_BASE_ADDR
#define DEVICE_MEM_MAX_SIZE		HAL_FLASH_SIZE

static device_info_t *pg_device_info;

void device_info_init(void)
{
	device_info_t *p_flash_device_info = (device_info_t *)DEVICE_BASE_ADDR;

	DBG_ASSERT(SAVE_SIZE <= DEVICE_MEM_MAX_SIZE);
	
	pg_device_info = heap_alloc(sizeof(device_info_t), PLAT_TRUE);
	DBG_ASSERT(pg_device_info != PLAT_NULL);
	
    //莫名必须赋值，不然flash的数据判断问题
    mem_cpy(pg_device_info->id, p_flash_device_info->id, 5);

	if (GET_NWK_ID(pg_device_info->id) == 0xff || GET_NWK_ID(pg_device_info->id) == 0 || 1)
	{		
		SET_NWK_ID(pg_device_info->id, 0x01);
		SET_MESH_ID(pg_device_info->id, 0x01);
		SET_DEV_ID(pg_device_info->id, 0x010101);
		
		strcpy(pg_device_info->version, VERSION);
		
        pg_device_info->eth_local_mac_addr[0] = 0x4C;
		pg_device_info->eth_local_mac_addr[1] = 0xCC;
		pg_device_info->eth_local_mac_addr[2] = 0x6A;
		pg_device_info->eth_local_mac_addr[3] = 0x12;
		pg_device_info->eth_local_mac_addr[4] = 0x3B;
		pg_device_info->eth_local_mac_addr[5] = 0x9B;

		hal_flash_write(DEVICE_BASE_ADDR, (uint8_t *)pg_device_info, SAVE_SIZE);
	}
	else
	{
		mem_cpy(pg_device_info, p_flash_device_info, SAVE_SIZE);
		strcpy(pg_device_info->version, VERSION);
	}
	DBG_PRINTF("Device ID=0x%x, Mesh ID=0x%x, NWK ID=0x%x\r\n", 
				GET_DEV_ID(pg_device_info->id), 
				GET_MESH_ID(pg_device_info->id),
				GET_NWK_ID(pg_device_info->id));
	
	DBG_PRINTF("Device Software Version=%s\r\n", pg_device_info->version);
	DBG_PRINTF("Use Mode=%d\r\n", pg_device_info->func_bit.mode);
}

device_info_t *device_info_get(bool_t flag)
{	
	return pg_device_info;
}

bool_t device_info_set(device_info_t *device_info, bool_t force_update)
{
	if (pg_device_info == device_info)
	{
		if (force_update == PLAT_TRUE)
		{
			hal_flash_write(HAL_FLASH_BASE_ADDR, (uint8_t *)pg_device_info, SAVE_SIZE);
			return PLAT_TRUE;
		}		
		return PLAT_FALSE;
	}
	else
	{
		return PLAT_FALSE;
	}
}
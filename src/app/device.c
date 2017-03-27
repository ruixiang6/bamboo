#include <platform.h>
#include "device.h"
#include <string.h>

#define VERSION		"V.1.0.2.5"
#define SAVE_SIZE	(sizeof(device_info_t)-sizeof(dev_time_t)-sizeof(dev_pos_t))

#define DEVICE_BASE_ADDR		HAL_FLASH_BASE_ADDR
#define DEVICE_MEM_MAX_SIZE		HAL_FLASH_SIZE

static device_info_t *pg_device_info = PLAT_NULL;

void device_info_init(void)
{
	device_info_t *p_flash_device_info = (device_info_t *)DEVICE_BASE_ADDR;

	DBG_ASSERT(SAVE_SIZE <= DEVICE_MEM_MAX_SIZE);

	if (pg_device_info == PLAT_NULL)
	{
		pg_device_info = heap_alloc(sizeof(device_info_t), PLAT_TRUE);
		DBG_ASSERT(pg_device_info != PLAT_NULL);
	}
	
    //莫名必须赋值，不然flash的数据判断问题
    mem_cpy(pg_device_info->id, p_flash_device_info->id, 4);

	if (GET_DEV_ID(pg_device_info->id) == 0xff || GET_DEV_ID(pg_device_info->id) == 0)
	{
		SET_TYPE_ID(pg_device_info->id, TYPE_214);
		SET_MODE_ID(pg_device_info->id, MODE_NORMAL);
		SET_MESH_ID(pg_device_info->id, 0x12);
		SET_DEV_ID(pg_device_info->id, 0x01);
		
		strcpy(pg_device_info->version, VERSION);
		
		pg_device_info->local_ip_addr[0] = 192;
		pg_device_info->local_ip_addr[1] = 168;
		pg_device_info->local_ip_addr[2] = 12;
		pg_device_info->local_ip_addr[3] = 66;

		pg_device_info->local_gateway_addr[0] = 192;
		pg_device_info->local_gateway_addr[1] = 168;
		pg_device_info->local_gateway_addr[2] = 12;
		pg_device_info->local_gateway_addr[3] = 1;

		pg_device_info->local_netmask_addr[0] = 255;
		pg_device_info->local_netmask_addr[1] = 255;
		pg_device_info->local_netmask_addr[2] = 255;
		pg_device_info->local_netmask_addr[3] = 0;

        pg_device_info->local_eth_mac_addr[0] = 0x4C;
		pg_device_info->local_eth_mac_addr[1] = pg_device_info->local_ip_addr[0];
		pg_device_info->local_eth_mac_addr[2] = pg_device_info->local_ip_addr[1];
		pg_device_info->local_eth_mac_addr[3] = pg_device_info->local_ip_addr[2];
		pg_device_info->local_eth_mac_addr[4] = pg_device_info->local_ip_addr[3];
		pg_device_info->local_eth_mac_addr[5] = 0x9B;

		pg_device_info->remote_eth_mac_addr[0] = 0xFF;
		pg_device_info->remote_eth_mac_addr[1] = 0xFF;
		pg_device_info->remote_eth_mac_addr[2] = 0xFF;
		pg_device_info->remote_eth_mac_addr[3] = 0xFF;
		pg_device_info->remote_eth_mac_addr[4] = 0xFF;
		pg_device_info->remote_eth_mac_addr[5] = 0xFF;

		pg_device_info->remote_ip_addr[0] = 192;
		pg_device_info->remote_ip_addr[1] = 168;
		pg_device_info->remote_ip_addr[2] = 12;
		pg_device_info->remote_ip_addr[3] = 55;

		pg_device_info->remote_port = 60000;

		hal_flash_write(DEVICE_BASE_ADDR, (uint8_t *)pg_device_info, SAVE_SIZE);
	}
	else
	{
		mem_cpy(pg_device_info, p_flash_device_info, SAVE_SIZE);
		strcpy(pg_device_info->version, VERSION);
	}	
	DBG_PRINTF("software version=%s\r\n", pg_device_info->version);

	DBG_PRINTF("type(dec)=%d\r\nmode(dec)=%d\r\ndev_id(hex)=%x\r\nmesh_id(hex)=%x\r\n", 
				GET_TYPE_ID(pg_device_info->id),
				GET_MODE_ID(pg_device_info->id),
				GET_DEV_ID(pg_device_info->id),
				GET_MESH_ID(pg_device_info->id));

	DBG_PRINTF("local_mac(hex)=%x:%x:%x:%x:%x:%x\r\n", 
				pg_device_info->local_eth_mac_addr[0],
				pg_device_info->local_eth_mac_addr[1],
				pg_device_info->local_eth_mac_addr[2],
				pg_device_info->local_eth_mac_addr[3],
				pg_device_info->local_eth_mac_addr[4],
				pg_device_info->local_eth_mac_addr[5]);

	DBG_PRINTF("local_ip(dec)=%d:%d:%d:%d\r\n", 
				pg_device_info->local_ip_addr[0],
				pg_device_info->local_ip_addr[1],
				pg_device_info->local_ip_addr[2],
				pg_device_info->local_ip_addr[3]);

	DBG_PRINTF("gateway(dec)=%d:%d:%d:%d\r\n", 
				pg_device_info->local_gateway_addr[0],
				pg_device_info->local_gateway_addr[1],
				pg_device_info->local_gateway_addr[2],
				pg_device_info->local_gateway_addr[3]);
	
	DBG_PRINTF("netmask(dec)=%d:%d:%d:%d\r\n", 
				pg_device_info->local_netmask_addr[0],
				pg_device_info->local_netmask_addr[1],
				pg_device_info->local_netmask_addr[2],
				pg_device_info->local_netmask_addr[3]);

	DBG_PRINTF("remote_mac(hex)=%x:%x:%x:%x:%x:%x\r\n", 
				pg_device_info->remote_eth_mac_addr[0],
				pg_device_info->remote_eth_mac_addr[1],
				pg_device_info->remote_eth_mac_addr[2],
				pg_device_info->remote_eth_mac_addr[3],
				pg_device_info->remote_eth_mac_addr[4],
				pg_device_info->remote_eth_mac_addr[5]);

	DBG_PRINTF("remote_ip(dec)=%d:%d:%d:%d\r\n", 
				pg_device_info->remote_ip_addr[0],
				pg_device_info->remote_ip_addr[1],
				pg_device_info->remote_ip_addr[2],
				pg_device_info->remote_ip_addr[3]);
	
	DBG_PRINTF("remote_port(dec)=%d\r\n", pg_device_info->remote_port);
	
	srand(GET_DEV_ID(pg_device_info->id));
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
			hal_flash_write(DEVICE_BASE_ADDR, (uint8_t *)pg_device_info, SAVE_SIZE);
			return PLAT_TRUE;
		}		
		return PLAT_FALSE;
	}
	else
	{
		return PLAT_FALSE;
	}
}
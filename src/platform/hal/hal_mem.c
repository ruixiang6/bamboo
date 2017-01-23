#include <platform.h>
#include <m2sxxx.h>
#include <mss_nvm.h>
#include <cortex_nvic.h>
#include <hal_mem.h>

bool_t hal_flash_write(uint32_t flash_ptr, uint8_t *buf, uint16_t len)
{
	nvm_status_t status;
		
	if (flash_ptr >= HAL_FLASH_BASE_ADDR 
		&& flash_ptr<HAL_FLASH_END_ADDR)
	{
		if (flash_ptr+len >= HAL_FLASH_BASE_ADDR
		&& flash_ptr+len<HAL_FLASH_END_ADDR)
		{
			status = NVM_write(flash_ptr, buf, len, NVM_DO_NOT_LOCK_PAGE);
			if (status == NVM_SUCCESS)
			{
				return PLAT_TRUE;
			}			
		}
	}
	
	return PLAT_FALSE;
}

bool_t hal_flash_read(uint32_t flash_ptr, uint8_t *buf, uint16_t len)
{
	if (flash_ptr >= HAL_FLASH_BASE_ADDR 
		&& flash_ptr<HAL_FLASH_END_ADDR)
	{
		if (flash_ptr+len >= HAL_FLASH_BASE_ADDR
		&& flash_ptr+len<HAL_FLASH_END_ADDR)
		{
			mem_cpy((void *)buf, (void *)flash_ptr, len);

			return PLAT_TRUE;
		}
	}
	
	return PLAT_FALSE;
}

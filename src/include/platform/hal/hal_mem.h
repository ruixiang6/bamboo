#ifndef __HAL_MEM_H
#define __HAL_MEM_H

#define HAL_FLASH_BASE_ADDR		0x60078000
#define HAL_FLASH_END_ADDR		0x60080000
#define HAL_FLASH_SIZE			1024

bool_t hal_flash_write(uint32_t flash_ptr, uint8_t *buf, uint16_t len);

bool_t hal_flash_read(uint32_t flash_ptr, uint8_t *buf, uint16_t len);

#endif
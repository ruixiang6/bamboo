 /**
 * provides an abstraction for hardware.
 *
 * @file hal_spi.h
 * @author qhw
 *
 * @addtogroup HAL_SPI HAL spi
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_SPI_H
#define __HAL_SPI_H

typedef enum
{	
	HAL_SPI_IDLE,
	HAL_SPI_BUSY,
	HAL_SPI_ACTIVE
}hal_spi_state_t;

typedef enum
{
	HAL_SPI_RECV_BUF,
	HAL_SPI_SEND_BUF
}hal_spi_buf_type_t;

/**
 * 	
 */
void hal_spi_init();
/**
 * 	
 */
void hal_spi_dma_init(fpv_t func);
/**
 * 	
 */
bool_t hal_spi_slave_recv(void *buf, uint32_t size);
/**
 * 	
 */
bool_t hal_spi_slave_send(void *buf, uint32_t size);
/**
 * 	
 */
void hal_spi_set_state(hal_spi_state_t state);
/**
 * 	
 */
hal_spi_state_t hal_spi_get_state(void);
/**
 * 	
 */
uint32_t hal_spi_get_resource(hal_spi_buf_type_t type);
/**
 * 	
 */
void hal_spi_reset(hal_spi_buf_type_t type);
#endif
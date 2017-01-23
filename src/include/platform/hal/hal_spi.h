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
	SPI_MODE_8BITS = 0,
	SPI_MODE_32BITS
}SPI_XFER_MODE_T;

/**
 * 	
 */
void hal_spi_init(void);
/**
 * 	
 */
bool_t hal_spi_master_xfer(void *tx_buf, uint32_t tx_byte_size,
							void *rx_buf, uint32_t rx_byte_size,
							SPI_XFER_MODE_T mode);
#endif
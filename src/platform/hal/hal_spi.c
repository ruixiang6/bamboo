#include <platform.h>
#include <m2sxxx.h>
#include <mss_pdma.h>
#include <mss_spi.h>
#include <mss_gpio.h>

    
/***************************************************************************//**
 * Mask of transfer protocol and SPO, SPH bits within control register.
 */
#define PROTOCOL_MODE_MASK  ((uint32_t)0x030000C0)

/***************************************************************************//**
 * Mask of theframe count bits within the SPI control register.
 */
#define TXRXDFCOUNT_MASK    ((uint32_t)0x00FFFF00u)
#define TXRXDFCOUNT_SHIFT   ((uint32_t)8u)

/***************************************************************************//**
 * SPI hardware FIFO depth.
 */
#define RX_FIFO_SIZE    4u
#define BIG_FIFO_SIZE   32u

/***************************************************************************//**
 * 
 */
#define RX_IRQ_THRESHOLD    (BIG_FIFO_SIZE / 2u)

/***************************************************************************//**
  Marker used to detect that the configuration has not been selected for a
  specific slave when operating as a master.
 */
#define NOT_CONFIGURED  0xFFFFFFFFu

/***************************************************************************//**
 * CONTROL register bit masks
 */
#define CTRL_ENABLE_MASK    0x00000001u
#define CTRL_MASTER_MASK    0x00000002u

/***************************************************************************//**
  Registers bit masks
 */
/* CONTROL register. */
#define MASTER_MODE_MASK        0x00000002u
#define CTRL_RX_IRQ_EN_MASK     0x00000010u
#define CTRL_TX_IRQ_EN_MASK     0x00000020u
#define CTRL_REG_RESET_MASK     0x80000000u
#define BIGFIFO_MASK            0x20000000u
#define CTRL_CLKMODE_MASK       0x10000000u

/* CONTROL2 register */
#define ENABLE_CMD_IRQ_MASK     0x00000010u
#define ENABLE_SSEND_IRQ_MASK   0x00000020u

/* STATUS register */
#define TX_DONE_MASK            0x00000001u
#define RX_DATA_READY_MASK      0x00000002u
#define RX_OVERFLOW_MASK        0x00000004u
#define RX_FIFO_EMPTY_MASK      0x00000040u
#define TX_FIFO_FULL_MASK       0x00000100u
#define TX_FIFO_EMPTY_MASK      0x00000400u

/* MIS register. */
#define TXDONE_IRQ_MASK         0x00000001u
#define RXDONE_IRQ_MASK         0x00000002u
#define CMD_IRQ_MASK            0x00000010u
#define SSEND_IRQ_MASK          0x00000020u

/* COMMAND register */
#define AUTOFILL_MASK           0x00000001u
#define TX_FIFO_RESET_MASK      0x00000008u
#define RX_FIFO_RESET_MASK      0x00000004u

#define FIFO_DEEP_32BITS	8
#define FIFO_DEEP_8BITS		32

void hal_spi_init()
{
  	SPI_REVB_TypeDef *hw_reg;
    
	hw_reg = ((SPI_REVB_TypeDef *) SPI1_BASE);
    /* reset SPI1 */
    SYSREG->SOFT_RST_CR |= SYSREG_SPI1_SOFTRESET_MASK;    
    /* Take SPI1 out of reset. */
    SYSREG->SOFT_RST_CR &= ~SYSREG_SPI1_SOFTRESET_MASK;
	
    hw_reg->CONTROL &= ~CTRL_REG_RESET_MASK;	
	/* Set the master mode. */
    hw_reg->CONTROL |= CTRL_MASTER_MASK;
   	/* Set the protocol mode0. */
    hw_reg->CONTROL &= ~CTRL_ENABLE_MASK;
    /* Set default frame size to byte size and number of data frames to 32. */
    hw_reg->CONTROL = MASTER_MODE_MASK |
					   BIGFIFO_MASK |
					   CTRL_CLKMODE_MASK |
					   (uint32_t)MSS_SPI_MODE0 |
					   (hw_reg->CONTROL & ~TXRXDFCOUNT_MASK) |
					   ((uint32_t)1 << TXRXDFCOUNT_SHIFT);    
    /* APB1_CLK/clk_div, clk_gen = (clk_div / 2u) - 1u*/
	hw_reg->CLK_GEN |= 0;//APB1/2
    hw_reg->CONTROL |= CTRL_ENABLE_MASK;	
}

bool_t hal_spi_master_xfer(void *tx_buf, 
							uint32_t tx_byte_size,
							void *rx_buf,
							uint32_t rx_byte_size,
							SPI_XFER_MODE_T mode)
{
	uint8_t *byte_tx_buf = (uint8_t *)tx_buf;
	uint8_t *byte_rx_buf = (uint8_t *)rx_buf;
	uint32_t *dword_tx_buf = (uint32_t *)tx_buf;
	uint32_t *dword_rx_buf = (uint32_t *)tx_buf;
	uint32_t tx_dword_size = 0;
	uint32_t xfer_idx = 0;
	uint32_t deep_fifo_idx = 0;
	uint32_t tx_idx = 0;
	uint32_t rx_idx = 0;
	
	SPI_REVB_TypeDef *hw_reg;

	hw_reg = ((SPI_REVB_TypeDef *) SPI1_BASE);

	hw_reg->CONTROL &= ~CTRL_ENABLE_MASK;
	
	if (mode == SPI_MODE_8BITS)
	{
		hw_reg->TXRXDF_SIZE = 8;
	}
	else if (mode == SPI_MODE_32BITS)
	{
		if (tx_byte_size%4)
		{
			hw_reg->TXRXDF_SIZE = 8;
			mode = SPI_MODE_8BITS;
		}
		else
		{
			tx_dword_size = tx_byte_size/4;
			hw_reg->TXRXDF_SIZE = 32;
		}
	}
	else
	{
		return PLAT_FALSE;
	}
	
	hw_reg->CONTROL |= CTRL_ENABLE_MASK;

	/* Flush the Tx and Rx FIFOs. */
	hw_reg->COMMAND |= (TX_FIFO_RESET_MASK | RX_FIFO_RESET_MASK);
	
	/* Flush the receive FIFO. */
    while((hw_reg->STATUS & RX_FIFO_EMPTY_MASK) == 0u)
    {
        hw_reg->RX_DATA;       
    } 
	
	/* Set slave select */
    hw_reg->SLAVE_SELECT |= ((uint32_t)1 << (uint32_t)MSS_SPI_SLAVE_0);

	if (mode == SPI_MODE_32BITS)
	{
		while(xfer_idx < tx_dword_size)
		{
			if ((hw_reg->STATUS & RX_FIFO_EMPTY_MASK) == 0)
			{
				if (dword_rx_buf && (rx_idx < rx_byte_size))
				{
					dword_rx_buf[rx_idx] = hw_reg->RX_DATA;
					rx_idx++;
				}
				else
				{
					hw_reg->RX_DATA;
				}
				xfer_idx++;				
				if (deep_fifo_idx) deep_fifo_idx--;
			}

			if ((hw_reg->STATUS & TX_FIFO_FULL_MASK) == 0
				&& tx_idx < tx_dword_size)
			{
				if (deep_fifo_idx < FIFO_DEEP_32BITS)
				{
					if (dword_tx_buf)
					{
						hw_reg->TX_DATA = dword_tx_buf[tx_idx];
					}
					else
					{
						hw_reg->TX_DATA = 0xA5A5A5A5;
					}
					tx_idx++;
					deep_fifo_idx++;
				}
			}
		}
	}
	else
	{
		while(xfer_idx < tx_byte_size)
		{
			if ((hw_reg->STATUS & RX_FIFO_EMPTY_MASK) == 0)
			{
				if (byte_rx_buf && (rx_idx < rx_byte_size))
				{
					byte_rx_buf[rx_idx] = (uint8_t)hw_reg->RX_DATA;
					rx_idx++;
				}
				else
				{
					(uint8_t)hw_reg->RX_DATA;
				}

				xfer_idx++;
				if (deep_fifo_idx) deep_fifo_idx--;
			}

			if ((hw_reg->STATUS & TX_FIFO_FULL_MASK) == 0
				&& tx_idx < tx_byte_size)
			{
				if (deep_fifo_idx < FIFO_DEEP_8BITS)
				{
					if (byte_tx_buf)
					{
						hw_reg->TX_DATA = byte_tx_buf[tx_idx];
					}
					else
					{
						hw_reg->TX_DATA = 0xA5;
					}
					tx_idx++;
					deep_fifo_idx++;
				}
			}
		}
	}

	hw_reg->SLAVE_SELECT &= ~((uint32_t)1 << (uint32_t)MSS_SPI_SLAVE_0);

	return PLAT_TRUE;
}

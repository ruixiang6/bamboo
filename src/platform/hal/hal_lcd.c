#include <platform.h>
#include <mss_gpio.h>
#include <mss_spi.h>
#include <Control_IO.h>

//本版本支持OLED设备SPD0301    
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

static inline void oled_write(bool_t dc, uint8_t data);
static void hal_lcd_cfg(void);
static uint8_t *p_display_ram = NULL;

static void ssp0_init(void);
static void ssp0_write(bool_t dc, uint8_t data);

#define CMD 	0
#define DATA 	1

#define OLED_DC_L()    		{hal_gpio_output(OLCD_DC, 0);}
#define OLED_DC_H()   		{hal_gpio_output(OLCD_DC, 1);}

#define OLED_RST_L()    	{hal_gpio_output(OLCD_RST, 0);}
#define OLED_RST_H()   		{hal_gpio_output(OLCD_RST, 1);}

void hal_lcd_init(void)
{
	OLED_DC_L();
	
	OLED_RST_L();

	ssp0_init();
	
	hal_lcd_cfg();

	p_display_ram = heap_alloc(1024, 1);
	DBG_ASSERT(p_display_ram != PLAT_NULL);
}

static void hal_lcd_cfg(void)
{
	OLED_RST_L();
    delay_us(100);
    OLED_RST_H();
    delay_us(100);
	
    hal_lcd_display(0);
    //initial settings configration

    oled_write(CMD,0xd5);/* set display clk divide ratio */
    oled_write(CMD,0xf0);/* set oscillator frequency */

    oled_write(CMD,0xa8);/* set mutilplex ratio */
    oled_write(CMD,0x3f);/* 1/64 */

    oled_write(CMD,0xd3);/* set display offset */
    oled_write(CMD,0x40);/* set display start line */

    oled_write(CMD,0x40);/* set normal display */

    oled_write(CMD,0xad);/* set master configration */
    oled_write(CMD,0x8e);/*  */

    oled_write(CMD,0xa1);/* set segment remap */

    oled_write(CMD,0xc8);/* set com scan direction */

    oled_write(CMD,0xda);/* set com pin configuartion */
    oled_write(CMD,0x12);

    oled_write(CMD,0x81);/* set com contrast control */
    oled_write(CMD,0x32);

    oled_write(CMD,0xD9);/* set pre-charge period */
    oled_write(CMD,0xF1);

    oled_write(CMD,0xdb);/* set VCOMH Deselect level */
    oled_write(CMD,0x30);

    oled_write(CMD,0xa4);/* set Entire Display on-off */	
	
	hal_lcd_clear();
    
    hal_lcd_display(1);	
}

void hal_lcd_display(uint8_t state)
{
	if (state)
	{
		oled_write(CMD,0xaf);/* set display on */
	}
	else
	{
		oled_write(CMD,0xae);/* set display off */
	}
}

void hal_lcd_clear(void)
{
	uint8_t m, i, j;
	
	oled_write(CMD, 0x20);	/*set page mode*/
	
	for (m=0; m<8; m++)
	{
		for (i=0; i<8; i++)
		{
			for (j=0; j<16; j++)
			{				
				oled_write(CMD, 0xB0+m);	/*set lower addr*/
				oled_write(CMD, j);			/*set lower addr*/
				oled_write(CMD, 0x10+i);	/*set high addr*/
				oled_write(DATA, 0);
			}
		}
	}
}

void hal_lcd_set_pixel(uint8_t x, uint8_t y, uint8_t pixel)
{
	if (x>=LCD_X_DOT || y>=LCD_Y_DOT)
	{
		return;
	}

	uint8_t col_low, col_high, page_num, row;
	uint8_t pixel_byte;

	col_high = x/16;
	col_low = x%16;
	page_num = y/8;
	row = y%8;

	pixel_byte = *(uint8_t *)(p_display_ram + x + 128*page_num);

	if (pixel) pixel_byte |= (1u<<row);
	else pixel_byte &= ~(1u<<row);

	oled_write(CMD, 0xB0+page_num);		/*set page*/
	oled_write(CMD, col_low);		/*set lower addr*/
	oled_write(CMD, 0x10+col_high);	/*set high addr*/
	oled_write(DATA, pixel_byte);

	*(uint8_t *)(p_display_ram + x + 128*page_num) = pixel_byte;
}

uint8_t hal_lcd_get_pixel(uint8_t x, uint8_t y)
{
	if (x>=LCD_X_DOT || y>=LCD_Y_DOT)
	{
		return 0;
	}

	uint8_t page_num, row;
	uint8_t pixel_byte;

	page_num = y/8;
	row = y%8;

	pixel_byte = *(uint8_t *)(p_display_ram + x + 128*page_num);
	
	if (pixel_byte & (1u<<row))
	{
	  	return 1;
	}
	else
	{
	  	return 0;
	}	
}

void hal_lcd_backlight(bool_t state)
{
	if (state) 
	{
		
	}
	else
	{
		
	}
}

static inline void oled_write(bool_t dc, uint8_t data)
{
#if 0
    uint8_t i;
	
    OLED_CS_L();
	
    if (dc)
    {
        OLED_DC_H();
    }
    else
    {
        OLED_DC_L();
    }

    for (i=8; i!=0; i--)
    {
        OLED_CLK_L();
        if((data >> (i-1)) & 1)
        {
            OLED_MOSI_H();            
        }
        else
        {
            OLED_MOSI_L();            
        }
        //delay_us(1);       // 大于200ns
        OLED_CLK_H();
        //delay_us(1);       // 大于200ns
    }
    OLED_CS_H();
#endif
	ssp0_write(dc, data);
}

static void ssp0_init(void)
{
	SPI_REVB_TypeDef *hw_reg;
    
	hw_reg = ((SPI_REVB_TypeDef *) SPI0_BASE);
    /* reset SPI0 */
    SYSREG->SOFT_RST_CR |= SYSREG_SPI0_SOFTRESET_MASK;    
    /* Take SPI1 out of reset. */
    SYSREG->SOFT_RST_CR &= ~SYSREG_SPI0_SOFTRESET_MASK;
	
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
	hw_reg->TXRXDF_SIZE = 8;
    hw_reg->CONTROL |= CTRL_ENABLE_MASK;
}

static inline void ssp0_write(bool_t dc, uint8_t data)
{
	SPI_REVB_TypeDef *hw_reg;
    uint32_t loop; 

	hw_reg = ((SPI_REVB_TypeDef *) SPI0_BASE);
	
	/* Flush the Tx and Rx FIFOs. */
	hw_reg->COMMAND |= (TX_FIFO_RESET_MASK | RX_FIFO_RESET_MASK);

	if (dc)
	{
		OLED_DC_H();
	}
	else
	{
		OLED_DC_L();
	}
	
	/* Flush the receive FIFO. */
    while((hw_reg->STATUS & RX_FIFO_EMPTY_MASK) == 0u)
    {
        hw_reg->RX_DATA;       
    }	
	/* Set slave select */
	hw_reg->SLAVE_SELECT |= ((uint32_t)1 << (uint32_t)MSS_SPI_SLAVE_0);
	loop = 0x0f;//最小延时
    while(loop--);
    while (hw_reg->STATUS & TX_FIFO_FULL_MASK);
	hw_reg->TX_DATA = data;
    loop = 0x0f;
    while(loop--);
	hw_reg->SLAVE_SELECT &= ~((uint32_t)1 << (uint32_t)MSS_SPI_SLAVE_0);
}

#if 0
void hal_lcd_test(void)
{
    OLED_RST_L();
    delay_us(100);
    OLED_RST_H();
    delay_us(100);
    oled_write(CMD,0xae);/* set display off */

    //initial settings configration

    oled_write(CMD,0xd5);/* set display clk divide ratio */
    oled_write(CMD,0xf0);/* set oscillator frequency */

    oled_write(CMD,0xa8);/* set mutilplex ratio */
    oled_write(CMD,0x3f);/* 1/64 */

    oled_write(CMD,0xd3);/* set display offset */
    oled_write(CMD,0x40);/* set display start line */

    oled_write(CMD,0x40);/* set normal display */

    oled_write(CMD,0xad);/* set master configration */
    oled_write(CMD,0x8e);/*  */

    oled_write(CMD,0xa1);/* set segment remap */

    oled_write(CMD,0xc8);/* set com scan direction */

    oled_write(CMD,0xda);/* set com pin configuartion */
    oled_write(CMD,0x12);

    oled_write(CMD,0x81);/* set com contrast control */
    oled_write(CMD,0x32);

    oled_write(CMD,0xD9);/* set pre-charge period */
    oled_write(CMD,0xF1);

    oled_write(CMD,0xdb);/* set VCOMH Deselect level */
    oled_write(CMD,0x30);

    oled_write(CMD,0xa4);/* set Entire Display on-off */

    // clear screen
    oled_write(CMD,0xaf);/* set display on */
	//oled_write(CMD,0xae);/* set display off */
	/*************************************************************/
	oled_write(CMD, 0x20);	/*set page mode*/
	
	static uint8_t m;
	static uint8_t i;
	static uint8_t j;
	static uint8_t data = 0;
	
	while(1)
	{
	  	
		for (m=0; m<8; m++)
		{
			for (i=0; i<8; i++)
			{
				for (j=0; j<16; j++)
				{				
					oled_write(CMD, 0xB0+m);	/*set lower addr*/
					oled_write(CMD, j);			/*set lower addr*/
					oled_write(CMD, 0x10+i);	/*set high addr*/
					oled_write(DATA, data);
				}
			}
		}
		
		if (data) data = 0;
		else data = 0xFF;
		
		delay_ms(1000);
	}
	
    delay_ms(200);    //delay > 100ms

	hal_lcd_set_pixel(0, 0, 1);
	hal_lcd_set_pixel(0, 1, 1);
	hal_lcd_set_pixel(0, 2, 1);
	hal_lcd_set_pixel(0, 3, 1);
	hal_lcd_set_pixel(0, 4, 1);
	hal_lcd_set_pixel(0, 5, 1);
	
	hal_lcd_set_pixel(3, 0, 1);
	hal_lcd_set_pixel(3, 1, 1);
	hal_lcd_set_pixel(3, 2, 1);
	hal_lcd_set_pixel(3, 3, 1);
	hal_lcd_set_pixel(3, 4, 1);
	hal_lcd_set_pixel(3, 5, 1);
	
	hal_lcd_set_pixel(127, 63, 1);
	
	hal_lcd_set_pixel(0, 0, 0);
	
	hal_lcd_set_pixel(127, 63, 0);
	
	for (i=0; i<64; i++)
	{
	  	for(j=0; j<128; j++)
		{
		  	hal_lcd_set_pixel(j, i, 1);
		}
	}
	
	static uint8_t pixel;
	
	hal_lcd_set_pixel(0, 0, 1);
	hal_lcd_set_pixel(0, 1, 1);
	hal_lcd_set_pixel(127, 33, 1);
	hal_lcd_set_pixel(0, 3, 1);
	
	pixel = hal_lcd_get_pixel(0, 0);
	
	pixel = hal_lcd_get_pixel(0, 1);
	
	hal_lcd_set_pixel(0, 0, 0);
	
	pixel = hal_lcd_get_pixel(0, 1);
	
	pixel = hal_lcd_get_pixel(127, 33);
	
	hal_lcd_set_pixel(127, 33, 0);
	
	pixel = hal_lcd_get_pixel(127, 33);
}
#endif

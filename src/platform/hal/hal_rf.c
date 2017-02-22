#include <platform.h>
#include <m2sxxx.h>
#include <cortex_nvic.h>
#include <mss_pdma.h>
#include <mss_gpio.h>
#include <hal_rf.h>
#include <hal_gpio.h>

#define HAL_RF_PARAM_SAVE_ADDR	(HAL_FLASH_BASE_ADDR+HAL_FLASH_SIZE)
#define HAL_RF_PARAM_MAX_SIZE	HAL_FLASH_SIZE

static hal_rf_param_t rf_param;

#define LMS_6002_PWR_EN		{\
							hal_gpio_output(GPIO_RF_MODULE, 1);\
							}

#define LMS_6002_PWR_DIS	{\
							hal_gpio_output(GPIO_RF_MODULE, 0);\
							}

#define PA_PWR_EN			{\
							hal_gpio_output(GPIO_RF_PA, 1);\
							}
#define PA_PWR_DIS			{\
							hal_gpio_output(GPIO_RF_PA, 0);\
							}

#define FAILED	PLAT_FALSE
#define SUCCESS	PLAT_TRUE

#define PLLOUT_ALWAYS_ENABLE	0x40

static void RESET_LMS6002(void)
{
	*(uint32_t *)HAL_RF_MISC_SPI_REG_CONF &= ~0x000000001;
	delay_us(100);
	*(uint32_t *)HAL_RF_MISC_SPI_REG_CONF |= 0x000000001;
}

static void WRITE_LMS6002(uint8_t addr, uint8_t value)		
{
	uint32_t temp = 0;	

	temp = (0x80|addr)<<8|(value);
	*(uint32_t *)HAL_RF_MISC_SPI_REG_TX_DATA = temp;
    delay_us(10);
	while ((*(uint32_t *)HAL_RF_MISC_SPI_REG_CONF&0x04) != 0);
    delay_us(10);
}

static uint8_t READ_LMS6002(uint8_t addr)
{
	uint32_t temp = 0;	

	temp = (addr)<<8|0x00;
	*(uint32_t *)HAL_RF_MISC_SPI_REG_TX_DATA = temp;
    delay_us(10);
	while ((*(uint32_t *)HAL_RF_MISC_SPI_REG_CONF&0x04) != 0); 
    delay_us(10);
	temp = *(uint32_t *)HAL_RF_MISC_SPI_REG_RX_DATA;

	return (uint8_t)temp;
}
static void LMS6002D_Toplayer(void);
static uint8_t LMS6002D_RxChainConf(void);
static uint8_t LMS6002D_TxChainConf(void);
static uint8_t LMS6002D_DCCalibration( uint8_t addr, uint8_t dc_addr );
static uint8_t LMS6002D_DCCalOfTuningModule(void);
static void LMS6002D_BandwidthOfTuningModule(void);
static uint8_t LMS6002D_DCCalOfTXLPF(void);
static uint8_t LMS6002D_DCCalOfRXLPF(void);
static uint8_t LMS6002D_DCCalOfRXVGA2(void);
static void LMS6002D_EnterLoopback(void);
static void LMS6002D_ExitLoopback(void);
static uint8_t LMS6002D_TxCalLoLeakage(void);

static void config_lms6002_init();
static void config_lms6002_recv();
static void config_lms6002_send();
static void config_lms6002_idle();

static void rf_ofdm_send_dma_isr(void);
static void rf_ofdm_recv_dma_isr(void);

static struct
{
	hal_rf_ofdm_t *hw;
	uint8_t *rx_dma_buf;
	uint32_t rx_dma_size;
	uint8_t *tx_dma_buf;
	uint32_t tx_dma_size;
	fpv_t rx_dma_handler;
	fpv_t tx_dma_handler;
	fpv_t tx_req_handler;
	fpv_t rx_req_handler;
} ofdm_handler = 
{
	.hw = HAL_RF_OFDM,
	.rx_dma_buf = PLAT_NULL,
	.tx_dma_buf = PLAT_NULL,
	.rx_dma_handler = PLAT_NULL,
	.tx_dma_handler = PLAT_NULL,
	.tx_req_handler = PLAT_NULL,
	.rx_req_handler = PLAT_NULL
};

static struct
{
	hal_rf_misc_t *hw;
	fpv_t tmr_handler[8];
} misc_handler = 
{
	.hw = HAL_RF_MISC
};

bool_t hal_rf_misc_calib_freq(hal_rf_freq_t *freq_cal)
{
	uint8_t freq_sel, nint;
	uint32_t x, nfrac;
	fp64_t f_nint, f_nfrac;
#if 1
	if (freq_cal->lo>=232.5 && freq_cal->lo<285.625)
	{
		freq_sel = (0x17<<2) | 1;
	}
	else if (freq_cal->lo>=285.625 && freq_cal->lo<336.875)
	{
		freq_sel = (0x2F<<2) | 1;
	}
	else if (freq_cal->lo>=336.875 && freq_cal->lo<405)
	{
		freq_sel = (0x37<<2) | 1;
	}
	else if (freq_cal->lo>=405 && freq_cal->lo<571.25)
	{
		freq_sel = (0x3F<<2) | 1;
	}
	else if (freq_cal->lo>=571.25 && freq_cal->lo<673.75)
	{
		freq_sel = (0x2E<<2) | 1;
	}
	else if (freq_cal->lo>=673.75 && freq_cal->lo<810)
	{
		freq_sel = (0x36<<2) | 1;
	}
	else if (freq_cal->lo>=810 && freq_cal->lo<930)
	{
		freq_sel = (0x3E<<2) | 1;
	}
	else if (freq_cal->lo>=930 && freq_cal->lo<1142.5)
	{
		freq_sel = (0x25<<2) | 1;
	}
	else if (freq_cal->lo>=1142.5 && freq_cal->lo<1347.5)
	{
		freq_sel = (0x2D<<2) | 1;
	}
	else if (freq_cal->lo>=1347.5 && freq_cal->lo<1620)
	{
		freq_sel = (0x35<<2) | 1;
	}
	else if (freq_cal->lo>=1620 && freq_cal->lo<1860)
	{
		freq_sel = (0x3D<<2) | 1;
	}
	else if (freq_cal->lo>=1860 && freq_cal->lo<2285)
	{
		freq_sel = (0x24<<2) | 1;
	}
	else if (freq_cal->lo>=2285 && freq_cal->lo<2695)
	{
		freq_sel = (0x2C<<2) | 1;
	}
	else if (freq_cal->lo>=2695 && freq_cal->lo<3240)
	{
		freq_sel = (0x34<<2) | 1;
	}
	else if (freq_cal->lo>=3240 && freq_cal->lo<3720)
	{
		freq_sel = (0x3C<<2) | 1;
	}
	else
	{
		return PLAT_FALSE;
	}
#endif    
    x = (freq_sel>>2)&0x7;

	x = pow(2, x-3);
    
    f_nint = freq_cal->lo/(fp32_t)freq_cal->ref;

	f_nint = x*f_nint;

	nint = (uint8_t)f_nint;
    
    f_nfrac = f_nint - nint;
    
    x = pow(2, 23);

	f_nfrac = x*f_nfrac;

	nfrac = (uint32_t)f_nfrac;    

	freq_cal->pll = freq_sel;
	freq_cal->set1 = (nint&0xFE)>>1;
	freq_cal->set4 = nfrac&0xFF;
	freq_cal->set3 = (nfrac>>8)&0xFF;
	freq_cal->set2 = (nfrac>>16)&0xFF | (nint&0x01)<<7;
	
	return PLAT_TRUE;
}

void hal_rf_init(void)
{
 	uint32_t loop = 0;
    hal_rf_param_t *p_rf_param = PLAT_NULL;

	hal_rf_param_init();
	
	PA_PWR_DIS;
	
	LMS_6002_PWR_DIS;
	delay_ms(500);
	LMS_6002_PWR_EN;	
	delay_ms(200);
	
	/* reset fpga */
	SYSREG->SOFT_RST_CR &= ~SYSREG_FPGA_SOFTRESET_MASK;
	/* reset fic0 */
	SYSREG->SOFT_RST_CR &= ~SYSREG_FIC32_0_SOFTRESET_MASK;	
	
	delay_ms(5);
	
	while(1)
	{
		//等待apb,ofdm,dsss时钟稳定
		if (HAL_RF_MISC->pll_lock == 0x07) break;
		DBG_TRACE("Pll clock waiting = %d\r\n", loop);
		loop++;
		if (loop>200)
		{
		  	hal_board_reset();
		}
	};
	//获得基带控制6002的spi控制权	
	HAL_RF_MISC->spi_ctrl |= (1u<<1);
    config_lms6002_init();
	config_lms6002_idle();	
	//释放基带控制6002的spi控制权	
	HAL_RF_MISC->spi_ctrl &= ~(1u<<1);	
		
	//Enable dma channel 2 send ofdm frame
	PDMA_configure
	(
		PDMA_CHANNEL_2,
		PDMA_TO_FIC_0_DMAREADY_0,
		PDMA_HIGH_PRIORITY | PDMA_WORD_TRANSFER | PDMA_INC_DEST_FOUR_BYTES | PDMA_INC_SRC_FOUR_BYTES,
		PDMA_DEFAULT_WRITE_ADJ
	);
	PDMA_set_irq_handler(PDMA_CHANNEL_2, rf_ofdm_send_dma_isr);		
	PDMA_enable_irq(PDMA_CHANNEL_2);

	//Enable dma channel 3 recv ofdm frame
	PDMA_configure
	(
		PDMA_CHANNEL_3,
		PDMA_FROM_FIC_0_DMAREADY_1,
		PDMA_HIGH_PRIORITY | PDMA_WORD_TRANSFER | PDMA_INC_DEST_FOUR_BYTES | PDMA_INC_SRC_FOUR_BYTES,
		PDMA_DEFAULT_WRITE_ADJ
	);
	PDMA_set_irq_handler(PDMA_CHANNEL_3, rf_ofdm_recv_dma_isr);		
	PDMA_enable_irq(PDMA_CHANNEL_3);
    
    p_rf_param = hal_rf_param_get();
    //rf功放值
    hal_rf_misc_set_rf_tx_pow(p_rf_param->pa_power[p_rf_param->use_level]);	
	//lms功放值
	hal_rf_misc_set_lms_tx_pow(p_rf_param->ofdm_lms_power[p_rf_param->use_level]<<16|0);
    //tx_pow值
    hal_rf_of_set_reg(HAL_RF_OF_SCL_TX_POW, p_rf_param->ofdm_scl_power[p_rf_param->use_level]);

	//OFDM中断使用FIC1
	NVIC_ClearPendingIRQ(FabricIrq1_IRQn);
	/*注册FIC1中断 */
	NVIC_EnableIRQ(FabricIrq1_IRQn);
	//MISC中断使用FIC2
	NVIC_ClearPendingIRQ(FabricIrq2_IRQn);
	/*注册FIC2中断 */
    NVIC_EnableIRQ(FabricIrq2_IRQn);	
	//给PA上电
	PA_PWR_EN;
	//配置完RF给灯上电
	hal_gpio_output(GPIO_LED, 1);
}

void hal_rf_param_init(void)
{
	hal_rf_param_t *p_flash_rf_param = (hal_rf_param_t *)HAL_RF_PARAM_SAVE_ADDR;
	uint8_t index;

	DBG_ASSERT(sizeof(hal_rf_param_t)<=HAL_RF_PARAM_MAX_SIZE);
	
	if (p_flash_rf_param->freq_cal.pll == 0xFF)
	{
		rf_param.freq_cal.lo = 1445.0;
		rf_param.freq_cal.ref = 40;
	    hal_rf_misc_calib_freq(&rf_param.freq_cal);
		for (index=0; index<HAL_RF_PARA_NUM; index++)
		{			
			rf_param.pa_power[index] = 0xcfe;			
			rf_param.ofdm_scl_power[index] = 0x509;
			rf_param.ofdm_rssi_offset[index] = 0;
			rf_param.ofdm_lms_power[index] = 0x121e;
		}
		hal_flash_write(HAL_RF_PARAM_SAVE_ADDR, (uint8_t *)&rf_param, sizeof(hal_rf_param_t));
	}
	else
	{
		mem_cpy(&rf_param, p_flash_rf_param, sizeof(hal_rf_param_t));
		
		if (rf_param.freq_cal.lo<=232.5 
				|| rf_param.freq_cal.lo>=3720)
		{
			rf_param.freq_cal.lo = 1445.0;
			rf_param.freq_cal.ref = 40;
	    	hal_rf_misc_calib_freq(&rf_param.freq_cal);
			hal_flash_write(HAL_RF_PARAM_SAVE_ADDR, (uint8_t *)&rf_param, sizeof(hal_rf_param_t));
		}
	}
	//判断频率范围
	if (rf_param.freq_cal.lo>=1300 && rf_param.freq_cal.lo<=1330)
	{
		rf_param.use_level = 0;
	}
	else if (rf_param.freq_cal.lo>=1430 && rf_param.freq_cal.lo<=1460)
	{
		rf_param.use_level = 1;
	}
	else
	{
		rf_param.use_level = 1;
	}    
	
    DBG_TRACE("Freq LO = %f\r\n", rf_param.freq_cal.lo);
	DBG_TRACE("Freq PLL = 0x%X\r\n", rf_param.freq_cal.pll);
	DBG_TRACE("Freq Set1 = 0x%X\r\n", rf_param.freq_cal.set1);
	DBG_TRACE("Freq Set2 = 0x%X\r\n", rf_param.freq_cal.set2);
	DBG_TRACE("Freq Set3 = 0x%X\r\n", rf_param.freq_cal.set3);
	DBG_TRACE("Freq Set4 = 0x%X\r\n", rf_param.freq_cal.set4);			
	DBG_TRACE("PA Power = 0x%X\r\n", rf_param.pa_power[rf_param.use_level]);
	DBG_TRACE("SCL Power = 0x%X\r\n", rf_param.ofdm_scl_power[rf_param.use_level]);
	DBG_TRACE("LMS Power = 0x%X\r\n", rf_param.ofdm_lms_power[rf_param.use_level]);
	DBG_TRACE("RSSI Offset= 0x%X\r\n", rf_param.ofdm_rssi_offset[rf_param.use_level]);
}

hal_rf_param_t *hal_rf_param_get(void)
{
	//判断频率范围
	if (rf_param.freq_cal.lo>=1300 && rf_param.freq_cal.lo<=1330)
	{
		rf_param.use_level = 0;
	}
	else if (rf_param.freq_cal.lo>=1430 && rf_param.freq_cal.lo<=1460)
	{
		rf_param.use_level = 1;
	}
	else
	{
		rf_param.use_level = 1;
	}
	
	return &rf_param;
}

bool_t hal_rf_param_set(hal_rf_param_t *p_rf_param)
{
	if (p_rf_param != &rf_param) return PLAT_FALSE;

	hal_flash_write(HAL_RF_PARAM_SAVE_ADDR, (uint8_t *)p_rf_param, sizeof(hal_rf_param_t));

	return PLAT_TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void hal_rf_of_set_reg(uint32_t addr, uint32_t value)
{
	//检查地址的合法性和可写性
	switch (addr)
	{
		case HAL_RF_OF_SCL_FCTR:
		case HAL_RF_OF_COARSE_THRED:
		case HAL_RF_OF_FINE_THRED:
		case HAL_RF_OF_COARSE_SCALE:
		case HAL_RF_OF_FINE_SCALE:
		case HAL_RF_OF_FREQ1_EST:
		case HAL_RF_OF_FREQ2_EST:
		case HAL_RF_OF_FREQ3_EST:
		case HAL_RF_OF_I_DC_OFFSET:
		case HAL_RF_OF_Q_DC_OFFSET:
		case HAL_RF_OF_PAPR_THRED:
		case HAL_RF_OF_REF_POW:
		case HAL_RF_OF_CUR_POW:
		case HAL_RF_OF_AGC_VAL:
		case HAL_RF_OF_INTER_CANCEL:
		case HAL_RF_OF_INTER_THRED:
		case HAL_RF_OF_SIG_POW:
		case HAL_RF_OF_NOI_POW:
		case HAL_RF_OF_CCA_LEN:
		case HAL_RF_OF_LOSE_GATE:
		case HAL_RF_OF_SCL_TX_POW:
			*(uint32_t *)addr = value;
			//保存值
			break;
		default:break;
	}
}

uint32_t hal_rf_of_get_reg(uint32_t addr)
{
	//检查地址的合法性和可写性
	switch (addr)
	{
		case HAL_RF_OF_SCL_FCTR:
		case HAL_RF_OF_COARSE_THRED:
		case HAL_RF_OF_FINE_THRED:
		case HAL_RF_OF_COARSE_SCALE:
		case HAL_RF_OF_FINE_SCALE:
		case HAL_RF_OF_FREQ1_EST:
		case HAL_RF_OF_FREQ2_EST:
		case HAL_RF_OF_FREQ3_EST:
		case HAL_RF_OF_I_DC_OFFSET:
		case HAL_RF_OF_Q_DC_OFFSET:
		case HAL_RF_OF_PAPR_THRED:	
		case HAL_RF_OF_CUR_POW:
		case HAL_RF_OF_REF_POW:
		case HAL_RF_OF_AGC_VAL:
		case HAL_RF_OF_INTER_CANCEL:
		case HAL_RF_OF_INTER_THRED:
		case HAL_RF_OF_SIG_POW:
		case HAL_RF_OF_NOI_POW:
		case HAL_RF_OF_CCA_LEN:
		case HAL_RF_OF_LOSE_GATE:
		case HAL_RF_OF_SCL_TX_POW:
			return *(uint32_t *)addr;
		default:return 0;
	}
}

void hal_rf_of_reset(void)
{
	HAL_RF_OFDM->phy_conf &= 0XFFFFFFFFFE;
	delay_us(10);
	HAL_RF_OFDM->phy_conf |= 0x00000001;
	delay_us(10);
}

void hal_rf_of_int_enable(uint32_t int_type)
{
	HAL_RF_OFDM->int_mask |= int_type;
}

void hal_rf_of_int_disable(uint32_t int_type)
{
	HAL_RF_OFDM->int_mask &= ~int_type;
}

void hal_rf_of_int_clear(uint32_t int_type)
{
	HAL_RF_OFDM->int_clr_status |= int_type;
}

void hal_rf_of_int_reg_handler(uint32_t int_type, fpv_t handler)
{
	if (int_type == HAL_RF_OF_DMA_RX_FIN_INT)
	{
		ofdm_handler.rx_dma_handler = handler;
	}
	else if (int_type == HAL_RF_OF_DMA_TX_FIN_INT)
	{
		ofdm_handler.tx_dma_handler = handler;
	}
	else if (int_type == HAL_RF_OF_TX_FIN_INT)
	{
		ofdm_handler.tx_req_handler = handler;
	}
	else if (int_type == HAL_RF_OF_RX_FIN_INT)
	{
		ofdm_handler.rx_req_handler = handler;
	}
}

bool_t hal_rf_of_set_dma_ram(uint8_t type, void *buf, uint32_t size)
{
	uint32_t aligned_len = 0;
	
	if (buf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}
	
	if (type == HAL_RF_OF_SEND_M)
	{
		ofdm_handler.tx_dma_buf = buf;
		ofdm_handler.tx_dma_size = size;

		aligned_len = ((size) + 3) & ~3;
		aligned_len = aligned_len>>2;
		
		PDMA_start(PDMA_CHANNEL_2, (uint32_t)buf, HAL_RF_OF_REG_TX_RAM, aligned_len);
		return PLAT_TRUE;	
	}
	else if (type == HAL_RF_OF_RECV_M)
	{
		ofdm_handler.rx_dma_buf = buf;
		ofdm_handler.rx_dma_size = size;

		aligned_len = size & ~3;
		aligned_len = aligned_len>>2;
		
		PDMA_start(PDMA_CHANNEL_3, HAL_RF_OF_REG_RX_RAM, (uint32_t)buf, aligned_len);
		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}

uint8_t *hal_rf_of_get_dma_ram(uint8_t type, uint32_t *size)
{
	if (size == PLAT_NULL)
	{
		return PLAT_NULL;
	}

	if (type == HAL_RF_OF_SEND_M)
	{
		*size = ofdm_handler.tx_dma_size;
		return ofdm_handler.tx_dma_buf;
	}
	else if (type == HAL_RF_OF_RECV_M)
	{
		*size = ofdm_handler.rx_dma_size;
		return ofdm_handler.rx_dma_buf;
	}
	else
	{
		return PLAT_NULL;
	}
}

bool_t hal_rf_of_write_ram(void *buf, uint32_t size)
{
	uint32_t aligned_len = 0;
	
	if (buf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	aligned_len = ((size) + 3) & ~3;

	aligned_len = aligned_len>>2;

	for (uint32_t i=0; i<aligned_len; i++)
	{
		*((uint32_t *)(HAL_RF_OF_REG_TX_RAM)+i) = ((uint32_t *)buf)[i];
	}
	return PLAT_TRUE;
}

bool_t hal_rf_of_read_ram(void *buf, uint32_t size)
{
	uint32_t aligned_len = 0;
	
	if (buf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	aligned_len = size & ~3;

	aligned_len = aligned_len>>2;

	for (uint32_t i=0; i<aligned_len; i++)
	{
		((uint32_t *)buf)[i] = *((uint32_t *)(HAL_RF_OF_REG_RX_RAM)+i);
	}
	return PLAT_TRUE;
}

bool_t hal_rf_of_set_state(uint8_t state)
{
	switch(state)
	{
		case HAL_RF_OF_IDLE_M:
            config_lms6002_idle();
			HAL_RF_OFDM->trc_cmd = HAL_RF_OF_IDLE_M;          
			break;
		case HAL_RF_OF_SEND_M:
			config_lms6002_send();
			HAL_RF_OFDM->trc_cmd = HAL_RF_OF_SEND_M;  
			break;
		case HAL_RF_OF_RECV_M:
			config_lms6002_recv();
			HAL_RF_OFDM->trc_cmd = HAL_RF_OF_RECV_M;  
			break;
		case HAL_RF_OF_CCA_M:
			config_lms6002_send();
			HAL_RF_OFDM->trc_cmd = HAL_RF_OF_CCA_M;
			break;
		default: 
			return PLAT_FALSE;
	}
	return PLAT_TRUE;
}

uint8_t hal_rf_of_get_state(void)
{
	uint32_t state = HAL_RF_OFDM->trc_cmd&0x0000000f;
	
	if (state == HAL_RF_OF_IDLE_S)
	{
		return HAL_RF_OF_IDLE_M;
	}
	else if (state >= HAL_RF_OF_RECV_RDY_S && state <= HAL_RF_OF_RECV_CODE_S)
	{
		return HAL_RF_OF_RECV_M;
	}
	else if (state >= HAL_RF_OF_SEND_RDY_S && state <= HAL_RF_OF_SEND_DATA_S)
	{
		return HAL_RF_OF_SEND_M;
	}
	else if (state == HAL_RF_OF_CCA_S)
	{
		return HAL_RF_OF_CCA_M;
	}
	else
	{
		return 0xFF;
	}
}

void hal_rf_misc_set_pa_ctrl(uint8_t state)
{
	//HAL_RF_MISC->pa_ctrl = state;
	misc_handler.hw->pa_ctrl = state;
}

void hal_rf_misc_set_rf_tx_pow(uint16_t pow)
{
	//HAL_RF_MISC->rf_tx_pow = pow;
	misc_handler.hw->rf_tx_pow = pow;
}

void hal_rf_misc_set_lms_tx_pow(uint32_t pow)
{
	misc_handler.hw->lms_tx_pow = pow;
}

void hal_rf_misc_set_rf_switch(uint16_t sw)
{
	misc_handler.hw->rf_switch = sw;
}

void hal_rf_misc_set_lms_bw_cfg(uint32_t bw)
{
	misc_handler.hw->lms_bw_cfg = bw;
}

void hal_rf_misc_int_enable(uint32_t int_type)
{
	misc_handler.hw->int_mask |= int_type;
}

void hal_rf_misc_int_disable(uint32_t int_type)
{
	misc_handler.hw->int_mask &= ~int_type;
}

void hal_rf_misc_int_clear(uint32_t int_type)
{
	misc_handler.hw->int_clr |= int_type;
}

void hal_rf_misc_int_reg_handler(uint32_t int_type, fpv_t handler)
{
	uint8_t index;

	for (index=0; index<8; index++)
	{
		if (int_type>>(index) & 1)
		{
			break;
		}
	}

	if (index == 8) return;

	misc_handler.tmr_handler[index] = handler;
}


uint8_t hal_rf_misc_get_pa_ctrl(void)
{
	return (uint8_t)(misc_handler.hw->pa_ctrl);
}

void hal_rf_misc_set_timer(uint8_t index, uint32_t value)
{
	misc_handler.hw->tmr_var[index] = value;
}

uint32_t hal_rf_misc_get_timer(uint8_t index)
{
	return misc_handler.hw->tmr_var[index];
}

const int8_t OFDM_CAL_ARRAY_1445[] =
{-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-10,-11,-12,-13,-14,-15,-16,
-17,-18,-19,-20,-21,-22,-23,-24,-25,-27,-28,-29,-30,-31,-32,-33,-34,-35,-36,-37,
-38,-39,-40,-41,-42,-43,-44,-45,-46,-47,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,
-57,-58,-59,-60,-62,-63,-64,-65,-66,-67,-68,-69,-70,-71,-72,-73,-74,-75,-76,-77,
-78,-78,-79,-80,-81,-82,-83,-84,-85,-86,-87,-88,-89,-90,-91,-92,-93,-95,-96,-97,
-98,-99,-100,-101,-102,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,
-103,-103,-103,-103,-103,-103,-103};//1445

const int8_t OFDM_CAL_ARRAY_1315[] =
{-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-15,-17,-18,-19,
-20,-21,-22,-23,-24,-25,-26,-27,-28,-29,-30,-31,-32,-33,-34,-35,-36,-37,-38,-38,
-39,-40,-41,-42,-44,-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-57,-58,-59,
-60,-61,-62,-63,-64,-65,-66,-67,-69,-71,-72,-73,-74,-75,-76,-77,-78,-78,-79,-80,
-81,-82,-83,-84,-85,-86,-87,-88,-89,-90,-91,-92,-93,-94,-95,-96,-97,-98,-99,
-100,-101,-102,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,
-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103,-103};//1315

int8_t hal_rf_ofdm_cal_agc(uint8_t value, int8_t offset)
{
	int8_t cal_value;
		
	if (rf_param.freq_cal.lo>=1300 && rf_param.freq_cal.lo<=1330)
	{
		cal_value = OFDM_CAL_ARRAY_1315[value];
	}
	else if (rf_param.freq_cal.lo>=1430 && rf_param.freq_cal.lo<=1460)
	{
		cal_value = OFDM_CAL_ARRAY_1445[value];
	}
	else
	{
		cal_value = OFDM_CAL_ARRAY_1445[value];
	}

	return cal_value+offset;
}

int8_t hal_rf_ofdm_cal_pow(uint8_t value, int8_t offset)
{
	return value+offset;
}


fp32_t hal_rf_ofdm_cal_dbfs(uint32_t value)
{
	fp32_t cal_value;

	cal_value = 10*log10((fp32_t)((fp32_t)(value/1024)/1024));
	
	return cal_value;
}

fp32_t hal_rf_ofdm_cal_sn(uint16_t s_pow, uint16_t n_pow)
{
	fp32_t lqi_dB = 0; 
	fp32_t snr=0;

	snr=2.0*(fp32_t)s_pow/((fp32_t)n_pow)-1.0;

	if(snr>0) lqi_dB = 10*log10(snr);
    else lqi_dB = 0;

	return lqi_dB;
}


static void rf_ofdm_send_dma_isr(void)
{
  	HAL_RF_OFDM->int_clr_status = HAL_RF_OF_DMA_TX_FIN_INT;
	PDMA_clear_irq(PDMA_CHANNEL_2);	

	if (ofdm_handler.tx_dma_handler)
	{
		(*(ofdm_handler.tx_dma_handler))();
	}	
}

static void rf_ofdm_recv_dma_isr(void)
{
  	HAL_RF_OFDM->int_clr_status = HAL_RF_OF_DMA_RX_FIN_INT;
	PDMA_clear_irq(PDMA_CHANNEL_3);	

	if (ofdm_handler.rx_dma_handler)
	{
		(*(ofdm_handler.rx_dma_handler))();
	}	
}

void FabricIrq1_IRQHandler(void)
{	
	if (HAL_RF_OFDM->int_mask_status & HAL_RF_OF_RX_FIN_INT)
	{
		HAL_RF_OFDM->int_clr_status = HAL_RF_OF_RX_FIN_INT;
		if (ofdm_handler.rx_req_handler)
		{
			(*(ofdm_handler.rx_req_handler))();
		}
		HAL_RF_OFDM->int_clr_status = HAL_RF_OF_RX_FIN_INT;
	}

	if (HAL_RF_OFDM->int_mask_status & HAL_RF_OF_COARSE_SYNC_INT)
	{
		HAL_RF_OFDM->int_clr_status = HAL_RF_OF_COARSE_SYNC_INT;		
	}

	if (HAL_RF_OFDM->int_mask_status & HAL_RF_OF_FINE_SYNC_INT)
	{
		HAL_RF_OFDM->int_clr_status = HAL_RF_OF_FINE_SYNC_INT;		
	}

	if (HAL_RF_OFDM->int_mask_status & HAL_RF_OF_TX_FIN_INT)
	{
		HAL_RF_OFDM->int_clr_status = HAL_RF_OF_TX_FIN_INT;
		if (ofdm_handler.tx_req_handler)
		{
			(*(ofdm_handler.tx_req_handler))();
		}
		HAL_RF_OFDM->int_clr_status = HAL_RF_OF_TX_FIN_INT;
	}
}

void FabricIrq2_IRQHandler(void)
{
	uint32_t int_status = HAL_RF_MISC->int_mask_status;
	uint8_t index;
	
	for(index=0; index<8; index++)
	{
		if (int_status>>index & 1u)
		{			
			if (misc_handler.tmr_handler[index])
			{
				(*(misc_handler.tmr_handler[index]))();
			}

			HAL_RF_MISC->int_clr |= 1u<<index;
		}
	}
}


static void config_lms6002_init()
{
	RESET_LMS6002();	
    
    
    static uint8_t regvaal;
    
    regvaal = READ_LMS6002( 0x04 );
  
	if (READ_LMS6002(0x04) != 0x22)
	{
		DBG_TRACE("Read ID Failed\r\n");
	}
	
	/* 1.Top layer! */
	DBG_TRACE("Top layer!\r\n");
	LMS6002D_Toplayer();
	/* 2.TX chain configure! */
	DBG_TRACE("TX chain configure!\r\n");
	if (LMS6002D_TxChainConf() == FAILED)
	{		
		DBG_TRACE("Configure Failed!\r\n");
	}    
	/* 3.RX chain configure! */
	DBG_TRACE("RX chain configure!\r\n");
    if (LMS6002D_RxChainConf() == FAILED)
    {    	
    	DBG_TRACE("Configure Failed!\r\n");
    }
    /* 4.a DC offset cancellation of the tuning module */
	DBG_TRACE("DC offset cancellation of the tuning module\r\n");
	if( LMS6002D_DCCalOfTuningModule() == FAILED )
    {    	
        DBG_TRACE("Configure Failed!\r\n");
    } 

    /* 4.b Execute LPF bandwidth tuning procedure */
	DBG_TRACE("Execute LPF bandwidth tuning procedure\r\n");
    LMS6002D_BandwidthOfTuningModule();

          

    /* 6 RXLPF DC offset cancellation of I/Q filter */
	
    DBG_TRACE("RXLPF DC offset cancellation of I/Q filter\r\n");
    
    if( LMS6002D_DCCalOfRXLPF() == FAILED )
    {
    	//ControlIO_PowerOff();
        DBG_TRACE("Configure Failed!\r\n");
    }     
    
     /* 5 TXLPF DC offset cancellation of I/Q filter */
	DBG_TRACE("TXLPF DC offset cancellation of I/Q filter\r\n");
    if( LMS6002D_DCCalOfTXLPF() == FAILED )
    {
		DBG_TRACE("Configure Failed!\r\n");
    }
    /* 7 RXVGA2 DC offset cancellation of I/Q filter */
	
    DBG_TRACE("RXVGA2 DC offset cancellation of I/Q filter\r\n");
    
    if( LMS6002D_DCCalOfRXVGA2(  ) == FAILED )
    {
    	DBG_TRACE("Configure Failed!\r\n");
    } 
    
    /* 8 TX LO leakage cancellation */
    //DBG_TRACE("TX LO leakage cancellation\r\n");
    //LMS6002D_TxCalLoLeakage();    
}

static void config_lms6002_recv()
{

}

static void config_lms6002_send()
{

}

static void config_lms6002_idle()
{
	if ((*(uint32_t *)HAL_RF_MISC_SPI_REG_CONF & (1u<<1)))
	{
		WRITE_LMS6002(0x05, 0x22);   //IDLE态
	}
}

static void LMS6002D_Toplayer(void)
{		
	WRITE_LMS6002(0x05, 0x3E);//顶层，接收，发射各模块全开，以及配置为四线串口
    delay_ms( 50 );
    READ_LMS6002(0x45);
    
	WRITE_LMS6002(0x09, 0xC5|PLLOUT_ALWAYS_ENABLE);//PLL ALWAYS OUTPUT!!!!!RXOUTSW开关闭合，PLLLCLKOUT使能，Rx&Tx DSM SPI clock enabled，其它校准时钟使能都不打开
	WRITE_LMS6002(0x34, 0x06);//发射端滤波器模块打开，并且带宽设置为7MHZ
	WRITE_LMS6002(0x54, 0x06);//接收端滤波器模块打开，并且带宽设置为7MHZ
	WRITE_LMS6002(0x57, 0x14);//ADC/DAC模块关闭，DAC内部输出负载电阻设为200欧姆，DAC参考电流电阻设置为外置
	WRITE_LMS6002(0x47, 0x40);//LO buffer的偏置电流设置为4*5/6=3.3mA
	WRITE_LMS6002(0x59, 0x29);//RX LPF配置:参考增益1.75V，共模电压960mV，参考Buffer1.0X，ADC输入Buffer使能
	WRITE_LMS6002(0x64, 0x1E);//RXVGA2共模电压设置为900mV,且RXVGA2模块使能打开
	WRITE_LMS6002(0x79, 0x37);//控制片上LNA负载电阻
    
}

static uint8_t LMS6002D_TxChainConf(void)
{
	uint8_t cmin;
    uint8_t cmax;
    uint8_t error;
	uint32_t value;

	//WRITE_LMS6002(0x17, 0xE3 );//TX PLL配置：VCO regulator旁路且关掉，电阻短路，且输出上拉偏置电流设置为30uA
 	WRITE_LMS6002(0x15, rf_param.freq_cal.pll); //发射PLL配置
    WRITE_LMS6002(0x10, rf_param.freq_cal.set1);//发射PLL配置频率中除数的整数部分，NINT[8:1]
    WRITE_LMS6002(0x11, rf_param.freq_cal.set2);//发射PLL配置频率中除数的整数部分NINT[0],以及除数的小数部分NFRAC[22:16]
    WRITE_LMS6002(0x12, rf_param.freq_cal.set3);//发射PLL配置频率中除数的小数部分NFRAC[15:8]
    WRITE_LMS6002(0x13, rf_param.freq_cal.set4);//发射PLL配置频率中除数的小数部分NFRAC[7:0]

    WRITE_LMS6002(0x14, 0x8C );//PLL使能打开，且因为计算出数据的小数部分NFRAC为0，所以Delta sigma设置为旁路
	cmin = 63;
    cmax = 0;
    error = 1;    
	for(uint8_t loop=0;loop<64;loop++ )
    {
        WRITE_LMS6002(0x19, 0x80+loop);//默认VOVCOREG配置为1.9V，0x19地址中低6位是电容的配置，0~63
        delay_us(1036);
        value=READ_LMS6002(0x1A);
         if(value  == 0x03 )   //比较后的状态。有个问题，为什么0x1B中的bit3 VCO的比较使能没有打开，应该要打开了才可以读比较的结果？？
        {
            error = 0;
            if( loop < cmin )
                cmin = loop;
            if( loop > cmax )
                cmax = loop;
        }
    }    
    if( error==1 )
    {
        return FAILED;
    }
    else
    {
        value = (cmin+cmax)/2+1;
    }
    WRITE_LMS6002(0x19, 0x80+value);//将校准得出的电容值重新写入

	return SUCCESS;
}

static uint8_t LMS6002D_RxChainConf(void)
{
	uint8_t cmin;
    uint8_t cmax;
    uint8_t error;
	uint32_t value;
	
	WRITE_LMS6002( 0x27, 0xE3 );   //RX PLL配置：VCO regulator旁路且关掉，电阻短路，且输出上拉偏置电流设置为30uA

	WRITE_LMS6002(0x25, rf_param.freq_cal.pll);   //接收PLL配置范围内
    WRITE_LMS6002(0x20, rf_param.freq_cal.set1);//接收PLL配置频率中除数的整数部分，NINT[8:1]
    WRITE_LMS6002(0x21, rf_param.freq_cal.set2);//接收PLL配置频率中除数的整数部分NINT[0],以及除数的小数部分NFRAC[22:16]
    WRITE_LMS6002(0x22, rf_param.freq_cal.set3);//接收PLL配置频率中除数的小数部分NFRAC[15:8]
    WRITE_LMS6002(0x23, rf_param.freq_cal.set4);//接收PLL配置频率中除数的小数部分NFRAC[7:0]
    WRITE_LMS6002( 0x24, 0x8C );   //PLL使能打开，且因为计算出数据的小数部分NFRAC为0，所以Delta sigma设置为旁路
	cmin = 63;
    cmax = 0;
    error = 1;    
    for( uint8_t loop=0;loop<64;loop++ )
    {
        WRITE_LMS6002( 0x29, 0x80+loop );//默认VOVCOREG配置为1.9V，0x29地址中低6位是电容的配置，0~63
      delay_us(36);
        value = READ_LMS6002( 0x2A ); 
        if( value == 0x03 )            //比较后的状态。有个问题，为什么0x2B中的bit3 VCO的比较使能没有打开，应该要打开了才可以读比较的结果？？
        {
            error = 0;
            if( loop < cmin )
                cmin = loop;
            if( loop > cmax )
                cmax = loop;
        }
    }    
    if( error==1 )
    {
        return FAILED;
    }
    else
    {
        value = (cmin+cmax)/2+1;
    }
    WRITE_LMS6002( 0x29, 0x80+value );//将校准得出的电容值重新写入

	return SUCCESS;
}


/*******************************************************************************
*函数名称 LMS6002D_DCCalibration( UInt8 addr, UInt8 dc_addr )
*创建时间 ：2010.06.27
*创建人   ：Sympathique
*函数功能 ：LMS6002D General DC Calibration
*输入参数 ：addr     Register Address
            dc_addr  Calibration Module
*输出参数 ：无
*返回信息 ：Calibration results SUCCESS/FAILED
*修改日期 ：
*修改人 　：
*修改内容 ：
*******************************************************************************/
static uint8_t LMS6002D_DCCalibration( uint8_t addr, uint8_t dc_addr )
{
    
    uint8_t try_cnt;
    uint8_t reg_val;    
    
    for( try_cnt=0;try_cnt<50;try_cnt++ )
    {  
        WRITE_LMS6002( addr, 0x08|dc_addr );//关闭DC Calibration
        delay_ms(1);
        WRITE_LMS6002( addr, 0x28|dc_addr );//启动DC Calibration
        delay_ms(1);
        WRITE_LMS6002( addr, 0x08|dc_addr );//关闭DC Calibration
        delay_ms(1);
        reg_val = READ_LMS6002( addr-2 );   //读地址0x01
        if( reg_val&0x02 )                  //判断是否校准完成
        {
            continue;
        }
        if( (reg_val&0x1C)==0x00 || (reg_val&0x1C)==0x1C )   //DC_LOCK[2:0]的值为“000”或“111”时，没有锁定，其它值时表明锁定
        {
            continue;
        }
       else
       {
            return SUCCESS;
       }   
    }

    return FAILED;

}
/*******************************************************************************
*函数名称 LMS6002D_DCCalofTuningModule(  )
*创建时间 ：2010.06.27
*创建人   ：Sympathique
*函数功能 ：LMS6002D DC offset cancellation of the tuning module
*输入参数 ：无
*输出参数 ：无
*返回信息 ：Calibration results SUCCESS/FAILED
*修改日期 ：
*修改人 　：
*修改内容 ：
*******************************************************************************/

static uint8_t LMS6002D_DCCalOfTuningModule(void)
{
    
    uint8_t dccal;
    uint8_t regval;
    
    regval = READ_LMS6002( 0x09 );
    WRITE_LMS6002( 0x09, regval|0x20|PLLOUT_ALWAYS_ENABLE );    //将LPF SPI DCCAL clock使能置1
    
    if( LMS6002D_DCCalibration( 0x03, 0 ) == FAILED)
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE );     //如果校准失败，则将Tx LPF SPI DCCAL clock使能置0，即默认值写入
        return FAILED;
    }
    delay_us( 32 );
    dccal = READ_LMS6002( 0x00 );
    WRITE_LMS6002( 0x35, dccal );//将DC calibration得到的值更新到TX LPF中的DCO_DACCAL的值
    WRITE_LMS6002( 0x55, dccal );//将DC calibration得到的值更新到RX LPF中的DCO_DACCAL的值
    
    WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE );         // 校准OK之后，仍然要将LPF SPI DCCAL clock使能置0，即默认值写入
    
    return SUCCESS;
    
}

/*******************************************************************************
*函数名称 LMS6002D_BandwidthOfTuningModule(  )
*创建时间 ：2010.06.27
*创建人   ：Sympathique
*函数功能 ：LMS6002D Execute LPF bandwidth tuning procedure
*输入参数 ：无
*输出参数 ：无
*返回信息 ：无
*修改日期 ：
*修改人 　：
*修改内容 ：
*******************************************************************************/

static void LMS6002D_BandwidthOfTuningModule(void)
{
    
    uint8_t rccal;

    WRITE_LMS6002( 0x07, 0x05 ); //设置滤波器带宽为4.375MHZ
    WRITE_LMS6002( 0x07, 0x85 ); //将EN_CAL_LPF置1
    WRITE_LMS6002( 0x06, 0x0D ); //RST_CAL_LPFCAL复位打开，复位时间要超过100ns
    WRITE_LMS6002( 0x06, 0x0C ); //RST_CAL_LPFCAL复位关掉
    
    rccal = READ_LMS6002( 0x01 );
    rccal = (rccal >> 1) & 0x70;  //??????
    WRITE_LMS6002( 0x36, 0x80|rccal ); //0x36地址中(对应TXLPF)的PD_DCOCMP_LPF、PD_DCODAC_LPF、PD_DCOREF_LPF、PD_FIL_LPF全部enabled
    WRITE_LMS6002( 0x56, 0x80|rccal ); //0x56地址中(对应RXLPF)的PD_DCOCMP_LPF、PD_DCODAC_LPF、PD_DCOREF_LPF、PD_FIL_LPF全部enabled
    
    WRITE_LMS6002( 0x07, 0x05 ); //最后再将EN_CAL_LPF置0
      
}

/*******************************************************************************
*函数名称 LMS6002D_DCCalOfTXLPF(  )
*创建时间 ：2010.06.27
*创建人   ：Sympathique
*函数功能 ：LMS6002D TXLPF DC offset cancellation of I/Q filter
*输入参数 ：无
*输出参数 ：无
*返回信息 ：Calibration results SUCCESS/FAILED
*修改日期 ：
*修改人 　：
*修改内容 ：
*******************************************************************************/

static uint8_t LMS6002D_DCCalOfTXLPF(void)
{
    
    uint8_t regval;
    
    regval = READ_LMS6002( 0x09 );
    WRITE_LMS6002( 0x09, regval|0x02|PLLOUT_ALWAYS_ENABLE );  //将TX LPF SPI DCCAL clock enabled

    delay_ms( 10 );
    
    if( LMS6002D_DCCalibration( 0x33, 0 )==FAILED)           //TX LPF dc offset cancellation for I filter
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE );  //如果校准失败，将TX LPF SPI DCCAL clock disenabled
        return FAILED;    
    }
    delay_ms( 10 );
    if( LMS6002D_DCCalibration( 0x33, 1 )==FAILED )          //TX LPF dc offset cancellation for Q filter，
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE );  //如果校准失败，将TX LPF SPI DCCAL clock disenabled
        return FAILED;  
    }
    delay_ms( 10 );
    
    WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE );      //如果校准成功，仍然将TX LPF SPI DCCAL clock disenabled

	return SUCCESS;    
}

/*******************************************************************************
*函数名称 LMS6002D_DCCalOfRXLPF(  )
*创建时间 ：2010.06.27
*创建人   ：Sympathique
*函数功能 ：LMS6002D TXLPF DC offset cancellation of I/Q filter
*输入参数 ：无
*输出参数 ：无
*返回信息 ：Calibration results SUCCESS/FAILED
*修改日期 ：
*修改人 　：
*修改内容 ：
*******************************************************************************/

static uint8_t LMS6002D_DCCalOfRXLPF(void)
{
  
    uint8_t regval;
    
    delay_ms( 1 );
    regval = READ_LMS6002( 0x09 );
    delay_ms( 1 );
    WRITE_LMS6002( 0x09, regval|0x08|PLLOUT_ALWAYS_ENABLE ); //将RX LPF SPI DCCAL clock enabled
    
    delay_ms( 1 );
    
    if( LMS6002D_DCCalibration( 0x53, 0 )==FAILED )         //RX LPF dc offset cancellation for I filter
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE ); //如果校准失败，将RX LPF SPI DCCAL clock disenabled
        return FAILED;    
    }
    delay_ms( 1 );
    if( LMS6002D_DCCalibration( 0x53, 1 )==FAILED )         //RX LPF dc offset cancellation for Q filter
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE ); //如果校准失败，将RX LPF SPI DCCAL clock disenabled
        return FAILED;  
    }
    delay_ms( 1 );
    
    WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE );     //如果校准成功，仍然将TX LPF SPI DCCAL clock disenabled
    return SUCCESS;
    
}

/*******************************************************************************
*函数名称 LMS6002D_DCCalOfRXVGA2(  )
*创建时间 ：2010.06.27
*创建人   ：Sympathique
*函数功能 ：LMS6002D RXVGA2 DC offset cancellation of I/Q filter
*输入参数 ：无
*输出参数 ：无
*返回信息 ：Calibration results SUCCESS/FAILED
*修改日期 ：
*修改人 　：
*修改内容 ：
*******************************************************************************/

static uint8_t LMS6002D_DCCalOfRXVGA2(void)
{
    
    uint8_t regval;
    
    delay_ms( 1 );
    regval = READ_LMS6002( 0x09 );
    WRITE_LMS6002( 0x09, regval|0x10|PLLOUT_ALWAYS_ENABLE );//将RX VGA2 DCCAL clock enabled
    
    delay_ms( 1 );
    if( LMS6002D_DCCalibration( 0x63, 0 )==FAILED )         //RX VGA2 DC offset cancellation for DC reference module
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE ); //如果校准失败，则将RX VGA2 DCCAL clock disenabled
        return FAILED;   
    }
    delay_ms( 1 );
    if( LMS6002D_DCCalibration( 0x63, 1 )==FAILED )         //RX VGA2 DC offset cancellation for VGA2A I channel
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE ); //如果校准失败，则将RX VGA2 DCCAL clock disenabled
        return FAILED;    
    }
    delay_ms( 1 );
    if( LMS6002D_DCCalibration( 0x63, 2 )==FAILED )         //RX VGA2 DC offset cancellation for VGA2A Q channel
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE ); //如果校准失败，则将RX VGA2 DCCAL clock disenabled
        return FAILED;  
    }
    delay_ms( 1 );
    if( LMS6002D_DCCalibration( 0x63, 3 )==FAILED )         //RX VGA2 DC offset cancellation for VGA2B I channel
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE ); //如果校准失败，则将RX VGA2 DCCAL clock disenabled
        return FAILED;  
    }
    delay_ms( 1 );
    if( LMS6002D_DCCalibration( 0x63, 4 )==FAILED )         //RX VGA2 DC offset cancellation for VGA2B Q channel
    {  
        WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE ); //如果校准失败，则将RX VGA2 DCCAL clock disenabled
        return FAILED;   
    }
    delay_ms( 1 );
    WRITE_LMS6002( 0x09, regval|PLLOUT_ALWAYS_ENABLE );     //如果校准成功，仍然将RX VGA2 DCCAL clock disenabled
    return SUCCESS;
    
}


/*******************************************************************************
*函数名称 LMS6002D_TxCalLoLeakage
*创建时间 ：2014.04.01
*创建人   ：herrychien
*函数功能 ：LMS6002D calibrate tx LO leakage cancellation
*输入参数 ：无
*输出参数 ：无
*返回信息 ：Calibration results SUCCESS/FAILED
*修改日期 ：
*修改人 　：
*修改内容 ：
*******************************************************************************/
static uint8_t LMS6002D_TxCalLoLeakage(void)
{
#if 0    
    uint8_t regval, loop;	
	uint32_t best_erf_val, erf_val;
	uint8_t course_i, course_q;
	uint8_t medium_i, medium_q;
	uint8_t fine_i, fine_q;
	
    //enter loopback
    LMS6002D_EnterLoopback();
    
    delay_ms(10);
	//enable leakage test
	HAL_RF_OFDM->loopback = 0x6;//lo leakage calcation enabled
	
	//cal course i
	DBG_TRACE("Step1\r\n");
	//fixed DAC_Q value=128
	DBG_TRACE("Course Fixed DAC_Q=128\r\n");
	WRITE_LMS6002(0x43, 128);	
	
	best_erf_val = 0XFFFFFF;
	regval = 255;//init
	course_i = regval;
	
	for (loop=0; loop<17; loop++)
	{
		//variable DAC_I
		WRITE_LMS6002(0x42, regval);
        delay_ms(10);
        
        erf_val = 0;
        for(uint32_t i=0;i<32;i++){
		HAL_RF_OFDM->loleak_trig = 1;
		delay_ms(10);
		while (HAL_RF_OFDM->loleak_trig != 0);
		erf_val += HAL_RF_OFDM->loleak_mef;
		//DBG_TRACE("Course i=0x%x, ERF=%d\r\n", regval, erf_val);
    }

		if (erf_val<best_erf_val)
		{
			best_erf_val = erf_val;
			course_i = regval;
		}
		regval = regval-15;
	}

	//cal course q
	DBG_TRACE("Step2\r\n");
	//fixed DAC_I value = Best course_i
	DBG_TRACE("Course Fixed DAC_I=0x%x\r\n", course_i);
	WRITE_LMS6002(0x42, course_i);	
	erf_val = 0XFFFFFF;
	best_erf_val = 0XFFFFFF;
	regval = 255;//init
	course_q = regval;
	
	for (loop=0; loop<17; loop++)
	{
		////variable DAC_Q
		WRITE_LMS6002(0x43, regval);
        delay_ms(10);
		HAL_RF_OFDM->loleak_trig = 1;
		delay_ms(10);
		while (HAL_RF_OFDM->loleak_trig != 0);
		erf_val = HAL_RF_OFDM->loleak_mef;
		//DBG_TRACE("Course q=0x%x,ERF=%d\r\n", regval, erf_val);
		
		if (erf_val<best_erf_val)
		{
			best_erf_val = erf_val;
			course_q = regval;
		}
		regval = regval-15;
	}
	///////////////////////////////////////////////////////
	//cal medium i
	DBG_TRACE("Step3\r\n");
	//fixed DAC_Q value=course_q
	DBG_TRACE("Medium Fixed DAC_Q=0x%x\r\n", course_q);
	WRITE_LMS6002(0x43, course_q);
	erf_val = 0XFFFFFF;
	best_erf_val = 0XFFFFFF;	
	regval = course_i+15;//init
	medium_i = regval;
	
	for (loop=0; loop<30; loop++)
	{
		//variable DAC_I
		WRITE_LMS6002(0x42, regval);
        delay_ms(10);
		HAL_RF_OFDM->loleak_trig = 1;
		delay_ms(10);
		while (HAL_RF_OFDM->loleak_trig != 0);
		erf_val = HAL_RF_OFDM->loleak_mef;
		//DBG_TRACE("Medium i=0x%x, ERF=%d\r\n", regval, erf_val);
		
		if (erf_val<best_erf_val)
		{
			best_erf_val = erf_val;
			medium_i = regval;
		}
		regval = regval-1;
	}

	//cal medium q
	DBG_TRACE("Step4\r\n");
	//fixed DAC_I value=medium_i
	DBG_TRACE("Medium Fixed DAC_I=0x%x\r\n", medium_i);
	WRITE_LMS6002(0x42, medium_i);
	erf_val = 0XFFFFFF;
	best_erf_val = 0XFFFFFF;
	regval = course_q+15;//init
	medium_q = regval;
	
	for (loop=0; loop<30; loop++)
	{
		//DAC_Q
		WRITE_LMS6002(0x43, regval);
        delay_ms(10);
		HAL_RF_OFDM->loleak_trig = 1;
		delay_ms(10);
		while (HAL_RF_OFDM->loleak_trig != 0);
		erf_val = HAL_RF_OFDM->loleak_mef;
		//DBG_TRACE("Medium q=0x%x, ERF=%d\r\n", regval, erf_val);
			
		if (erf_val<best_erf_val)
		{
			best_erf_val = erf_val;
			medium_q = regval;
		}
		regval = regval-1;
	}
	//////////////////////////////////////////////
	//cal fine i
	DBG_TRACE("Step5\r\n");
	//fixed DAC_Q value=medium_q
	DBG_TRACE("Fine Fixed DAC_Q=0x%x\r\n", medium_q);
	WRITE_LMS6002(0x43, medium_q);
	erf_val = 0XFFFFFF;
	best_erf_val = 0XFFFFFF;	
	regval = medium_i+2;//init
	fine_i = regval;
	
	for (loop=0; loop<4; loop++)
	{
		//variable DAC_I
		WRITE_LMS6002(0x42, regval);
        delay_ms(10);
		HAL_RF_OFDM->loleak_trig = 1;
		delay_ms(10);
		while (HAL_RF_OFDM->loleak_trig != 0);
		erf_val = HAL_RF_OFDM->loleak_mef;
		//DBG_TRACE("Fine i=0x%x, ERF=%d\r\n", regval, erf_val);
		
		if (erf_val<best_erf_val)
		{
			best_erf_val = erf_val;
			fine_i = regval;
		}
		regval = regval-1;
	}

	//cal fine q
	DBG_TRACE("Step6\r\n");
	//fixed DAC_I value=fine_i
	DBG_TRACE("Fine Fixed DAC_I=0x%x\r\n", fine_i);
	WRITE_LMS6002(0x42, fine_i);
	erf_val = 0XFFFFFF;
	best_erf_val = 0XFFFFFF;
	regval = medium_q+2;//init
	fine_q = regval;
	
	for (loop=0; loop<4; loop++)
	{
		//DAC_Q
		WRITE_LMS6002(0x43, regval);
        delay_ms(10);
		HAL_RF_OFDM->loleak_trig = 1;
		delay_ms(10);
		while (HAL_RF_OFDM->loleak_trig != 0);
		erf_val = HAL_RF_OFDM->loleak_mef;
		//DBG_TRACE("Fine q=0x%x, ERF=%d\r\n", regval, erf_val);

		if (erf_val<best_erf_val)
		{
			best_erf_val = erf_val;
			fine_q = regval;
		}
		regval = regval-1;
	}

	
    //enter best i&q
    
	DBG_TRACE("Best Fine i=0x%x, q=0x%x\r\n", fine_i, fine_q);

    
    WRITE_LMS6002(0x42, 0x72);
    WRITE_LMS6002(0x43, 0xae);
    
    delay_ms(10);
    HAL_RF_OFDM->loleak_trig = 1;
	delay_ms(10);
    while (HAL_RF_OFDM->loleak_trig != 0);
    erf_val = HAL_RF_OFDM->loleak_mef;
	DBG_TRACE("FINE i=0x%x, FINE q=0x%x, ERF=%d\r\n", 0x72, 0xae, erf_val);
    
    
    WRITE_LMS6002(0x42, fine_i);
    WRITE_LMS6002(0x43, fine_q);
    
    delay_ms(10);
    HAL_RF_OFDM->loleak_trig = 1;
	delay_ms(10);
    while (HAL_RF_OFDM->loleak_trig != 0);
    erf_val = HAL_RF_OFDM->loleak_mef;
	DBG_TRACE("FINE i=0x%x, FINE q=0x%x, ERF=%d\r\n", fine_i, fine_q, erf_val);
    
    //exit loopback
	LMS6002D_ExitLoopback();
    HAL_RF_OFDM->loopback = 0x2;

#endif	
    return SUCCESS;
    
    
}

static void LMS6002D_EnterLoopback(void)
{
    //下面是通过BB环回来的数据，经过ENV/PKDETECT开关，LBEN_OPIN开关
    /*
    WRITE_LMS6002(0x05, 0x3E);   //顶层，发射，接收以及四线串口配置
    WRITE_LMS6002(0x08, 0x10);   //环回模式：TXMIX输出连接到LNA1
    //WRITE_LMS6002(0x64, 0x1C);   //RXVGA2模块关闭
    WRITE_LMS6002(0x09, 0x85|PLLOUT_ALWAYS_ENABLE);//RXOUTSW开关关闭，PLLCLKOUT enabled,RX DSM SPI clock enabled,TX DSM SPI clock enabled
	WRITE_LMS6002(0x0B, 0x08);   //RF loop back switch powered down
    //WRITE_LMS6002(0x34, 0x22);
    WRITE_LMS6002(0x34, 0x00);   //TX LPF powered down
    //WRITE_LMS6002(0x54, 0x22);
    WRITE_LMS6002(0x54, 0x00);   //RX LPF powered down
    WRITE_LMS6002(0x40, 0x02);   //TXRF modules enabled
    WRITE_LMS6002(0x41, 0x1F);   //TXVGA1增益配置为-4dB
    WRITE_LMS6002(0x44, 0x00);   //PA1 ON,PA2 OFF,RF loop back powerd up,envelop/peak detector
    //WRITE_LMS6002(0x45, 0x00); //TXVGA2增益配置为0dB
    WRITE_LMS6002(0x45, 0x01);   //AUXPA envelop detector output
    WRITE_LMS6002(0x46, 0xFF);   //LOOPBBEN[1:0]switch closed  0x0C
    WRITE_LMS6002(0x47, 0x61);   //
    WRITE_LMS6002(0x4C, 0x07);   //设置AUXPA的增益
    WRITE_LMS6002(0x4D, 0x80);   //设置AUXPA的增益
    WRITE_LMS6002(0x57, 0x94);   //ADC/DAC模块enabled
    WRITE_LMS6002(0x5A, 0x30);   //Rx Fsync Polarity为0
	WRITE_LMS6002(0x65, 0x0A);
    //WRITE_LMS6002(0x65, 0x09);   //有数据显示
    WRITE_LMS6002(0x75, 0xC0);
    WRITE_LMS6002(0x76, 0x78);
    //WRITE_LMS6002(0x76, 0x78);   //有数据显示
   	WRITE_LMS6002(0x77, 0x00);
   	*/
    
    //下面是将PA环回来的数据   测试出LO泄漏为-50dbc，不需要执行发射命令

    WRITE_LMS6002(0x05, 0x3E);   //顶层，发射，接收以及四线串口配置
    WRITE_LMS6002(0x75, 0xC0);   //LNA 关闭
    WRITE_LMS6002(0x08, 0x01);   //环回模式：TXMIX输出连接到LNA1
    WRITE_LMS6002(0x64, 0x1E);   //RXVGA2模块打开
    WRITE_LMS6002(0x09, 0x85|PLLOUT_ALWAYS_ENABLE);//RXOUTSW开关关闭，PLLCLKOUT enabled,RX DSM SPI clock enabled,TX DSM SPI clock enabled
	WRITE_LMS6002(0x0B, 0x09);   //RF loop back switch powered down
    WRITE_LMS6002(0x34, 0x22);
    //WRITE_LMS6002(0x34, 0x00);   //TX LPF powered down
    WRITE_LMS6002(0x54, 0x22);
    //WRITE_LMS6002(0x54, 0x00);   //RX LPF powered down
    WRITE_LMS6002(0x40, 0x02);   //TXRF modules enabled
    WRITE_LMS6002(0x41, 0x1F);   //TXVGA1增益配置为-14dB
    WRITE_LMS6002(0x44, 0x09);   //PA1 OFF,PA2 OFF,RF loop back powerd up,envelop/peak detector
    WRITE_LMS6002(0x45, 0x00);   //AUXPA envelop detector output
    WRITE_LMS6002(0x46, 0x00);   //LOOPBBEN[1:0]switch closed  0x0C
    WRITE_LMS6002(0x47, 0x61);   //
    WRITE_LMS6002(0x4C, 0x07);   //
    WRITE_LMS6002(0x4D, 0x80);   //
    WRITE_LMS6002(0x57, 0x94);   //ADC/DAC模块enabled
    WRITE_LMS6002(0x5A, 0x30);   //Rx Fsync Polarity为0
	WRITE_LMS6002(0x65, 0x0A);
    WRITE_LMS6002(0x76, 0x78);
   	WRITE_LMS6002(0x77, 0x00);    
}

static void LMS6002D_ExitLoopback(void)
{
	WRITE_LMS6002(0x08, 0x00);  
    WRITE_LMS6002(0x75, 0xD0);   //LNA 关闭
	WRITE_LMS6002(0x0B, 0x08);
    WRITE_LMS6002(0x44, 0x0B);    
}

 

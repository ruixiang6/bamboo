#include <mac.h>
#include <phy.h>

static phy_tmr_t phy_tmr_array[MAX_PHY_TMR_NUM];

void phy_ofdm_init(fpv_t send_func, fpv_t recv_func)
{
	//OFDM进入IDLE态
	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);	
	//使能ofdm接收和发送中断
	hal_rf_of_int_reg_handler(HAL_RF_OF_TX_FIN_INT, send_func);
	hal_rf_of_int_reg_handler(HAL_RF_OF_RX_FIN_INT, recv_func);		
	hal_rf_of_reset();
	//清除中断
	hal_rf_of_int_clear(HAL_RF_OF_TX_FIN_INT);
	hal_rf_of_int_clear(HAL_RF_OF_RX_FIN_INT);
	
	//开启接收中断
	hal_rf_of_int_enable(HAL_RF_OF_TX_FIN_INT);	
	//开启发送中断
	hal_rf_of_int_enable(HAL_RF_OF_RX_FIN_INT);
	//进入接收状态
	hal_rf_of_set_state(HAL_RF_OF_RECV_M);
}

void phy_tmr_init(void)
{
	uint8_t index;
	
	for(index=0; index<MAX_PHY_TMR_NUM; index++)
	{
		phy_tmr_array[index].tmr_int = 1u<<index;
		hal_rf_misc_int_reg_handler(phy_tmr_array[index].tmr_int, PLAT_NULL);
		hal_rf_misc_int_clear(phy_tmr_array[index].tmr_int);
		hal_rf_misc_int_disable(phy_tmr_array[index].tmr_int);		
		phy_tmr_array[index].count = 0;
		phy_tmr_array[index].used = PLAT_FALSE;
	}
}

uint8_t phy_tmr_alloc(fpv_t func)
{
	uint8_t index;
	
	for(index=0; index<MAX_PHY_TMR_NUM; index++)
	{
		if (phy_tmr_array[index].used == PLAT_FALSE)
		{
			phy_tmr_array[index].count = 0;
			phy_tmr_array[index].used = PLAT_TRUE;
			hal_rf_misc_int_reg_handler(phy_tmr_array[index].tmr_int, func);
			hal_rf_misc_int_clear(phy_tmr_array[index].tmr_int);						
			return index+1;
		}
	}

	return 0;
}

bool_t phy_tmr_free(uint8_t id)
{
	uint8_t index;

	if (id>MAX_PHY_TMR_NUM || id==0) return PLAT_FALSE;

	index = id-1;
	
	if (phy_tmr_array[index].used)
	{
		hal_rf_misc_set_timer(index, 0);
		hal_rf_misc_int_clear(phy_tmr_array[index].tmr_int);
		hal_rf_misc_int_disable(phy_tmr_array[index].tmr_int);
		hal_rf_misc_int_reg_handler(phy_tmr_array[index].tmr_int, PLAT_NULL);
		phy_tmr_array[index].count = 0;
		phy_tmr_array[index].used = PLAT_FALSE;
		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}

bool_t phy_tmr_start(uint8_t id, uint32_t delay_us)
{
	OSEL_DECL_CRITICAL();
	uint8_t index;

	if (id>MAX_PHY_TMR_NUM || id==0) return PLAT_FALSE;

	index = id-1;

	OSEL_ENTER_CRITICAL();
	if (phy_tmr_array[index].used && delay_us)
	{
		phy_tmr_array[index].count = delay_us;
		hal_rf_misc_int_enable(phy_tmr_array[index].tmr_int);
		hal_rf_misc_set_timer(index, phy_tmr_array[index].count);
		OSEL_EXIT_CRITICAL();
		return PLAT_TRUE;
	}
	else
	{
		OSEL_EXIT_CRITICAL();
		return PLAT_FALSE;
	}
	
}

bool_t phy_tmr_stop(uint8_t id)
{
	OSEL_DECL_CRITICAL();
	uint8_t index;

	if (id>MAX_PHY_TMR_NUM || id==0) return PLAT_FALSE;

	index = id-1;

	OSEL_ENTER_CRITICAL();
	if (phy_tmr_array[index].used)
	{
		hal_rf_misc_set_timer(index, 0);
		hal_rf_misc_int_clear(phy_tmr_array[index].tmr_int);
		hal_rf_misc_int_disable(phy_tmr_array[index].tmr_int);		
		phy_tmr_array[index].count = 0;
		OSEL_EXIT_CRITICAL();
		return PLAT_TRUE;
	}
	else
	{
		OSEL_EXIT_CRITICAL();
		return PLAT_FALSE;
	}	
}

bool_t phy_tmr_add(uint8_t id, uint32_t delay_us)
{
	OSEL_DECL_CRITICAL();
	uint32_t cur_us;
	uint8_t index;

	if (id>MAX_PHY_TMR_NUM || id==0) return PLAT_FALSE;

	index = id-1;

	OSEL_ENTER_CRITICAL();
	if (phy_tmr_array[index].used && delay_us)
	{
		cur_us = hal_rf_misc_get_timer(index);
		phy_tmr_array[index].count = delay_us+cur_us;
		hal_rf_misc_int_clear(phy_tmr_array[index].tmr_int);
		hal_rf_misc_int_enable(phy_tmr_array[index].tmr_int);
		hal_rf_misc_set_timer(index, delay_us+cur_us);
		OSEL_EXIT_CRITICAL();
		return PLAT_TRUE;
	}
	else
	{
		OSEL_EXIT_CRITICAL();
		return PLAT_FALSE;
	}
}

bool_t phy_tmr_repeat(uint8_t id)
{
	OSEL_DECL_CRITICAL();
	uint8_t index;
   
	if (id>MAX_PHY_TMR_NUM || id==0) return PLAT_FALSE;

	index = id-1;

	OSEL_ENTER_CRITICAL();
	if (phy_tmr_array[index].used && phy_tmr_array[index].count)
	{
		hal_rf_misc_int_clear(phy_tmr_array[index].tmr_int);
		hal_rf_misc_int_enable(phy_tmr_array[index].tmr_int);
		hal_rf_misc_set_timer(index, phy_tmr_array[index].count);        
		OSEL_EXIT_CRITICAL();
       	return PLAT_TRUE;
	}
	else
	{
		OSEL_EXIT_CRITICAL();
		return PLAT_FALSE;
	}
}

bool_t phy_ofdm_write(uint8_t *buf, uint32_t size)
{
	if (buf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	hal_rf_of_write_ram(buf, size);

	return PLAT_TRUE;
}

bool_t phy_ofdm_read(uint8_t *buf, uint32_t size)
{
	if (buf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	hal_rf_of_read_ram(buf, size);
	
	return PLAT_TRUE;
}

void phy_ofdm_recv(void)
{
	hal_rf_of_set_state(HAL_RF_OF_RECV_M);
}

void phy_ofdm_send(void)
{
	hal_rf_of_set_state(HAL_RF_OF_SEND_M);	
}

void phy_ofdm_idle(void)
{
	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);		
}

int8_t phy_ofdm_cca(void)
{
	uint32_t cur_pow = 0;
	uint32_t agc_value = 0;
	int8_t cca = 0;
    uint8_t mode = 0;
	hal_rf_param_t *p_rf_param = hal_rf_param_get();

	mode = HAL_RF_OFDM->trc_cmd;
	
    if (mode <= HAL_RF_OF_RECV_TRAIN_SHORT_S)
    {
    	cur_pow = hal_rf_of_get_reg(HAL_RF_OF_CUR_POW)>>8;
		//DBG_PRINTF("P-");
		cca = hal_rf_ofdm_cal_pow(cur_pow, -72);
	}
	else
	{
		if (mode == HAL_RF_OF_CCA_S)
		{
			agc_value = hal_rf_of_get_reg(HAL_RF_OF_AGC_VAL)>>8;
		}
		else
		{
			agc_value = hal_rf_of_get_reg(HAL_RF_OF_AGC_VAL) & 0xFF;
		}
		//DBG_PRINTF("A-");
		cca = hal_rf_ofdm_cal_agc(agc_value, 28);		
	}
	//DBG_PRINTF("CCA=%d\r\n", cca);

	return cca;
}

uint16_t phy_ofdm_snr(void)
{
	uint16_t s_pow = hal_rf_of_get_reg(HAL_RF_OF_SIG_POW);
	uint16_t n_pow = hal_rf_of_get_reg(HAL_RF_OF_NOI_POW);
	uint16_t snr = 0;

	//将信噪比传给帧头
	snr = (uint16_t)(hal_rf_ofdm_cal_sn(s_pow, n_pow)*10);
	if (snr>255) snr = 255;
    
    return snr;
}

void phy_init(void)
{
	/* 射频初始化 */
	hal_rf_init();
}

void phy_deinit(void)
{
	//OFDM进入IDLE态
	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);

	//关闭接收中断
	hal_rf_of_int_disable(HAL_RF_OF_TX_FIN_INT);	
	//关闭发送中断
	hal_rf_of_int_disable(HAL_RF_OF_RX_FIN_INT);
}



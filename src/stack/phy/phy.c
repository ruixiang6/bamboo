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

	if (hal_fpga_tim_exist())
	{
		for(index=0; index<MAX_PHY_TMR_NUM; index++)
		{
			phy_tmr_array[index].tmr_int = index;
			hal_fpga_tim_disable(index);
			hal_fpga_tim_int_unreg(index);
			hal_fpga_tim_int_clear(index);			
			hal_fpga_tim_int_disable(index);		
			phy_tmr_array[index].count = 0;
			phy_tmr_array[index].used = PLAT_FALSE;
		}
	}
	else
	{
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
	
}

uint8_t phy_tmr_alloc(fpv_t func)
{
	uint8_t index;

	if (hal_fpga_tim_exist())
	{
		for(index=0; index<MAX_PHY_TMR_NUM; index++)
		{
			if (phy_tmr_array[index].used == PLAT_FALSE)
			{
				phy_tmr_array[index].count = 0;
				phy_tmr_array[index].used = PLAT_TRUE;
				hal_fpga_tim_int_reg(index, func);
				hal_fpga_tim_enable(index);
				hal_fpga_tim_int_clear(index);
				hal_fpga_tim_int_enable(index);
				return index+1;
			}
		}
	}
	else
	{
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
		if (hal_fpga_tim_exist())
		{
			hal_fpga_tim_disable(index);
			hal_fpga_tim_int_clear(index);
			hal_fpga_tim_int_disable(index);
			hal_fpga_tim_int_unreg(index);
		}
		else
		{
			hal_rf_misc_set_timer(index, 0);
			hal_rf_misc_int_clear(phy_tmr_array[index].tmr_int);
			hal_rf_misc_int_disable(phy_tmr_array[index].tmr_int);
			hal_rf_misc_int_reg_handler(phy_tmr_array[index].tmr_int, PLAT_NULL);
		}
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

	if (id>MAX_PHY_TMR_NUM || id==0) 
	{
		return PLAT_FALSE;
	}

	index = id-1;

	OSEL_ENTER_CRITICAL();
	if (phy_tmr_array[index].used && delay_us)
	{		
		if (hal_fpga_tim_exist())
		{
			phy_tmr_array[index].count = 12.5*delay_us;
			hal_fpga_tim_int_enable(index);
			hal_fpga_tim_set_value(index, phy_tmr_array[index].count);
		}
		else
		{
			phy_tmr_array[index].count = delay_us;
			hal_rf_misc_int_enable(phy_tmr_array[index].tmr_int);
			hal_rf_misc_set_timer(index, phy_tmr_array[index].count);
		}
		OSEL_EXIT_CRITICAL();		
		return PLAT_TRUE;
	}
	else
	{
		OSEL_EXIT_CRITICAL();
		DBG_PRINTF("L");
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
		if (hal_fpga_tim_exist())
		{			
			hal_fpga_tim_int_disable(index);
			hal_fpga_tim_int_clear(index);
			hal_fpga_tim_set_value(index, 0);
		}
		else
		{
			hal_rf_misc_set_timer(index, 0);
			hal_rf_misc_int_clear(phy_tmr_array[index].tmr_int);
			hal_rf_misc_int_disable(phy_tmr_array[index].tmr_int);
		}
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
	uint32_t cur_cnt;
	uint8_t index;

	if (id>MAX_PHY_TMR_NUM || id==0) return PLAT_FALSE;

	index = id-1;

	OSEL_ENTER_CRITICAL();
	if (phy_tmr_array[index].used && delay_us)
	{
		if (hal_fpga_tim_exist())
		{
			cur_cnt = hal_fpga_tim_get_value(index);
			phy_tmr_array[index].count = cur_cnt+(25*delay_us);	
			hal_fpga_tim_int_enable(index);
			hal_fpga_tim_set_value(index, phy_tmr_array[index].count);
		}
		else
		{
			cur_cnt = hal_rf_misc_get_timer(index);
			phy_tmr_array[index].count = delay_us+cur_cnt;			
			hal_rf_misc_int_enable(phy_tmr_array[index].tmr_int);
			hal_rf_misc_set_timer(index, phy_tmr_array[index].count);
		}
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
		if (hal_fpga_tim_exist())
		{
			hal_fpga_tim_int_enable(index);
			hal_fpga_tim_set_value(index, phy_tmr_array[index].count);
		}
		else
		{			
			hal_rf_misc_int_enable(phy_tmr_array[index].tmr_int);
			hal_rf_misc_set_timer(index, phy_tmr_array[index].count);
		}
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

bool_t phy_ofdm_cca(void)
{
	bool_t flag;
	
	hal_rf_ofdm_cal_rssi(PLAT_TRUE, &flag);
	
	return flag;
}

uint16_t phy_ofdm_snr(void)
{
	uint16_t snr;
	
	//将信噪比传给帧头
	snr = (uint16_t)(hal_rf_ofdm_cal_sn()*10);
	if (snr>255) snr = 255;
    
    return snr;
}

void phy_init(void)
{
	/* 射频初始化 */
	hal_rf_init();

	if (phy_version()>=0x5022)
	{
		//版本高于0x5020时，可以使用此定时来替代基带定时器
		hal_fpga_tim_init();
		DBG_PRINTF("Fpga Timer\r\n");
	}
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

uint16_t phy_version(void)
{
	return hal_rf_of_get_reg(HAL_RF_OF_REG_VERSION);
}


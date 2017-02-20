#include <mac.h>
#include <phy.h>

static kbuf_t *phy_send_frm = PLAT_NULL;

static void phy_ofdm_recv_cb(void)
{
	uint32_t crc32 = 0;
	uint16_t object = MAC_EVENT_OF_RX;
	mac_frm_head_t *p_mac_head = PLAT_NULL;
	uint16_t s_pow = hal_rf_of_get_reg(HAL_RF_OF_SIG_POW);
	uint16_t n_pow = hal_rf_of_get_reg(HAL_RF_OF_NOI_POW);
	uint16_t snr = 0;
	
	kbuf_t *kbuf = kbuf_alloc(KBUF_BIG_TYPE);

	if (kbuf == PLAT_NULL)
	{
		return;
	}
	
	hal_rf_of_read_ram(kbuf->base, sizeof(mac_frm_head_t));

	p_mac_head = (mac_frm_head_t *)kbuf->base;

	crc32 = p_mac_head->crc32;
	p_mac_head->crc32 = 0;

	if (crc32 != crc32_tab((uint8_t *)p_mac_head, 0, sizeof(mac_frm_head_t)-sizeof(uint32_t)))
	{
		kbuf_free(kbuf);
		return;
	}
   
	if (p_mac_head->phy+1<=MAX_PHY_OFDM_FRM_MULTI)//max = 1888Bytes
	{
		kbuf->valid_len = (p_mac_head->phy+1)*HAL_RF_OF_REG_MAX_RAM_SIZE;
		//将信噪比传给帧头
		snr = (uint16_t)(hal_rf_ofdm_cal_sn(s_pow, n_pow)*10);
		if (snr>255) snr = 255;
		p_mac_head->phy = (uint8_t)snr;
		hal_rf_of_read_ram(p_mac_head, kbuf->valid_len);
		list_behind_put(&kbuf->list, &mac_ofdm_recv_list);
		osel_event_set(mac_event_h, &object);
	}
	else
	{
		kbuf_free(kbuf);
	}
}


static void phy_ofdm_send_cb(void)
{	
	uint16_t object = MAC_EVENT_OF_TX;

	if (phy_send_frm)
	{
		kbuf_free(phy_send_frm);
		phy_send_frm = PLAT_NULL;
	}
    //DBG_PRINTF("send\r\n");
	//进入接收状态
	hal_rf_of_set_state(HAL_RF_OF_RECV_M);	
	osel_event_set(mac_event_h, &object);
}

static void phy_csma_nav_cb(void)
{
	uint16_t object = MAC_EVENT_CSMA;
	osel_event_set(mac_event_h, &object);
}

static void phy_ofdm_init(void)
{
	//OFDM进入IDLE态
	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);	
	//使能ofdm接收和发送中断
	hal_rf_of_int_reg_handler(HAL_RF_OF_TX_FIN_INT, phy_ofdm_send_cb);
	hal_rf_of_int_reg_handler(HAL_RF_OF_RX_FIN_INT, phy_ofdm_recv_cb);		
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

static void phy_tmr_init(void)
{
	hal_rf_misc_int_reg_handler(HAL_RF_MISC_TMR0_INT, phy_csma_nav_cb);
	hal_rf_misc_int_reg_handler(HAL_RF_MISC_TMR1_INT, PLAT_NULL);
	hal_rf_misc_int_reg_handler(HAL_RF_MISC_TMR2_INT, PLAT_NULL);

	hal_rf_misc_int_clear(HAL_RF_MISC_TMR0_INT);
	hal_rf_misc_int_clear(HAL_RF_MISC_TMR1_INT);
	hal_rf_misc_int_clear(HAL_RF_MISC_TMR2_INT);
	
	hal_rf_misc_int_enable(HAL_RF_MISC_TMR0_INT);
	hal_rf_misc_int_enable(HAL_RF_MISC_TMR1_INT);
	hal_rf_misc_int_enable(HAL_RF_MISC_TMR2_INT);
}

void phy_tmr_start(uint32_t delay_us)
{
	hal_rf_misc_set_timer(0, delay_us);
}

void phy_tmr_stop(void)
{
	hal_rf_misc_set_timer(0, 0);
}

void phy_tmr_add(uint32_t delay_us)
{
	uint32_t cur_us;
	
	cur_us = hal_rf_misc_get_timer(0);

	hal_rf_misc_set_timer(0, delay_us+cur_us);
}

bool_t phy_ofdm_send(kbuf_t *kbuf)
{
	mac_frm_head_t *p_mac_head = PLAT_NULL;
	int8_t cca;

	if (kbuf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	if (phy_send_frm && phy_send_frm != kbuf)
	{
		return PLAT_FALSE;
	}

	if (phy_send_frm != kbuf)
	{
		p_mac_head = (mac_frm_head_t *)kbuf->base;
		//放入tx_ram
		hal_rf_of_write_ram(p_mac_head, (p_mac_head->phy+1)*HAL_RF_OF_REG_MAX_RAM_SIZE);
		phy_send_frm = kbuf;
	}
	
	cca = phy_ofdm_cca();
	if (cca > MAC_CCA_THREDHOLD)
	{
		return PLAT_FALSE;
	}
	else
	{
		hal_rf_of_set_state(HAL_RF_OF_IDLE_M);
		delay_us(150);
		hal_rf_of_set_state(HAL_RF_OF_SEND_M);
		return PLAT_TRUE;
	}
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


void phy_init(void)
{
	/* 射频初始化 */
	hal_rf_init();	

	phy_ofdm_init();

    phy_tmr_init();	
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



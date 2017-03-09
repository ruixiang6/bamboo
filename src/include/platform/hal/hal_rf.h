#ifndef __HAL_RF_H
#define __HAL_RF_H

#define   __RO     volatile const       /*!< Defines 'read only' permissions                 */
#define   __WO     volatile             /*!< Defines 'write only' permissions                */
#define   __RW     volatile             /*!< Defines 'read / write' permissions              */

#define HAL_RF_BUFFER_ALIGN				4

#define HAL_RF_OF_BASE_ADDR 			0x31000000
#define HAL_RF_MISC_BASE_ADDR 			0x32000000
#define HAL_RF_PARAM_BASE_ADDR			HAL_FLASH_BASE_ADDR

////////////////////LMS6002 SPI Control/////////////////////
#define HAL_RF_MISC_SPI_REG_TX_DATA			(HAL_RF_MISC_BASE_ADDR+0x0)
#define HAL_RF_MISC_SPI_REG_RX_DATA			(HAL_RF_MISC_BASE_ADDR+0x4)
#define HAL_RF_MISC_SPI_REG_CONF			(HAL_RF_MISC_BASE_ADDR+0x8)
////////////////////Power Amplifier/////////////////////////
#define HAL_RF_MISC_PA_CTRL					(HAL_RF_MISC_BASE_ADDR+0x10)
////////////////////PLL/////////////////////////////////////
#define HAL_RF_MISC_PLL_CTRL				(HAL_RF_MISC_BASE_ADDR+0x14)
////////////////////PLL/////////////////////////////////////
#define HAL_RF_MISC_LMS_TX_POW_CTRL			(HAL_RF_MISC_BASE_ADDR+0x18)
#define HAL_RF_MISC_RF_TX_POW_CTRL			(HAL_RF_MISC_BASE_ADDR+0x1C)
#define HAL_RF_MISC_RF_SWITCH_CTRL			(HAL_RF_MISC_BASE_ADDR+0x20)
#define HAL_RF_MISC_LMS_BW_CFG_CTRL			(HAL_RF_MISC_BASE_ADDR+0x24)
//////////////////////ERF CAL///////////////////////////
#define HAL_RF_LO_LEAKAGE_EN				(HAL_RF_MISC_BASE_ADDR+0x54)
#define HAL_RF_ERF_EN						(HAL_RF_MISC_BASE_ADDR+0x58)
#define HAL_RF_ERF_VALUE					(HAL_RF_MISC_BASE_ADDR+0x5c)
/////////////////////AGC TEST///////////////////////////
#define HAL_RF_MISC_AGC_CFG					(HAL_RF_MISC_BASE_ADDR+0x60)
#define HAL_RF_MISC_AGC_GAIN 				(HAL_RF_MISC_BASE_ADDR+0x64)
#define HAL_RF_MISC_AGC_CTRL 				(HAL_RF_MISC_BASE_ADDR+0x68)
#define HAL_RF_MISC_AGC_CFG2				(HAL_RF_MISC_BASE_ADDR+0x6C)
/////////////////////Timer///////////////////////////
#define HAL_RF_MISC_TMR0_VAR				(HAL_RF_MISC_BASE_ADDR+0x70)
#define HAL_RF_MISC_TMR1_VAR				(HAL_RF_MISC_BASE_ADDR+0x74)
#define HAL_RF_MISC_TMR2_VAR				(HAL_RF_MISC_BASE_ADDR+0x78)
#define HAL_RF_MISC_TMR3_VAR				(HAL_RF_MISC_BASE_ADDR+0x7c)
#define HAL_RF_MISC_TMR4_VAR				(HAL_RF_MISC_BASE_ADDR+0x80)
#define HAL_RF_MISC_TMR5_VAR				(HAL_RF_MISC_BASE_ADDR+0x84)
#define HAL_RF_MISC_TMR6_VAR				(HAL_RF_MISC_BASE_ADDR+0x88)
#define HAL_RF_MISC_TMR7_VAR				(HAL_RF_MISC_BASE_ADDR+0x8c)
/////////////////////IQ///////////////////////////
#define HAL_RF_MISC_RX_I_BUF				(HAL_RF_MISC_BASE_ADDR+0x4000)
#define HAL_RF_MISC_RX_Q_BUF				(HAL_RF_MISC_BASE_ADDR+0x8000)
////////////////////MISC INT///////////////////////////////
#define HAL_RF_MISC_TMR7_INT				(1u<<7)
#define HAL_RF_MISC_TMR6_INT				(1u<<6)
#define HAL_RF_MISC_TMR5_INT				(1u<<5)
#define HAL_RF_MISC_TMR4_INT				(1u<<4)
#define HAL_RF_MISC_TMR3_INT				(1u<<3)
#define HAL_RF_MISC_TMR2_INT				(1u<<2)
#define HAL_RF_MISC_TMR1_INT				(1u<<1)
#define HAL_RF_MISC_TMR0_INT				(1u<<0)
/////////////////////////OFDM/////////////////////////////////////////
#define HAL_RF_OF_REG_ID				(HAL_RF_OF_BASE_ADDR+0x00)
#define HAL_RF_OF_REG_VERSION			(HAL_RF_OF_BASE_ADDR+0x04)
#define HAL_RF_OF_REG_INT_MASK			(HAL_RF_OF_BASE_ADDR+0x08)
#define HAL_RF_OF_REG_INT_RAW_STATUS	(HAL_RF_OF_BASE_ADDR+0x0c)
#define HAL_RF_OF_REG_INT_MASK_STATUS	(HAL_RF_OF_BASE_ADDR+0x10)
#define HAL_RF_OF_REG_INT_CLR_STATUS	(HAL_RF_OF_BASE_ADDR+0x14)
#define HAL_RF_OF_REG_RXCONF			(HAL_RF_OF_BASE_ADDR+0x18)
#define HAL_RF_OF_REG_PHY_CONF			(HAL_RF_OF_BASE_ADDR+0x1c)
#define HAL_RF_OF_REG_TX_DELAY			(HAL_RF_OF_BASE_ADDR+0x20)
#define HAL_RF_OF_REG_RX_DELAY			(HAL_RF_OF_BASE_ADDR+0x24)
#define HAL_RF_OF_REG_PATH_DELAY		(HAL_RF_OF_BASE_ADDR+0x28)

#define HAL_RF_OF_REG_TRX_CMD			(HAL_RF_OF_BASE_ADDR+0x40)

#define HAL_RF_OF_SCL_FCTR				(HAL_RF_OF_BASE_ADDR+0x80)
#define HAL_RF_OF_COARSE_THRED 			(HAL_RF_OF_BASE_ADDR+0x84)
#define HAL_RF_OF_FINE_THRED			(HAL_RF_OF_BASE_ADDR+0x88)
#define HAL_RF_OF_COARSE_SCALE			(HAL_RF_OF_BASE_ADDR+0x8c)
#define HAL_RF_OF_FINE_SCALE			(HAL_RF_OF_BASE_ADDR+0x90)
#define HAL_RF_OF_FREQ1_EST				(HAL_RF_OF_BASE_ADDR+0x94)
#define HAL_RF_OF_FREQ2_EST				(HAL_RF_OF_BASE_ADDR+0x98)
#define HAL_RF_OF_FREQ3_EST				(HAL_RF_OF_BASE_ADDR+0x9c)
#define HAL_RF_OF_I_DC_OFFSET			(HAL_RF_OF_BASE_ADDR+0xa0)
#define HAL_RF_OF_Q_DC_OFFSET			(HAL_RF_OF_BASE_ADDR+0xa4)
#define HAL_RF_OF_PAPR_THRED			(HAL_RF_OF_BASE_ADDR+0xa8)
#define HAL_RF_OF_REF_POW   			(HAL_RF_OF_BASE_ADDR+0xac)
#define HAL_RF_OF_CUR_POW   			(HAL_RF_OF_BASE_ADDR+0xb0)
#define HAL_RF_OF_AGC_VAL	  			(HAL_RF_OF_BASE_ADDR+0xb4)
#define HAL_RF_OF_INTER_CANCEL 			(HAL_RF_OF_BASE_ADDR+0xb8)
#define HAL_RF_OF_INTER_THRED  			(HAL_RF_OF_BASE_ADDR+0xbc)

#define HAL_RF_OF_SIG_POW   			(HAL_RF_OF_BASE_ADDR+0xc0)
#define HAL_RF_OF_NOI_POW	   			(HAL_RF_OF_BASE_ADDR+0xc4)
#define HAL_RF_OF_CCA_LEN	   			(HAL_RF_OF_BASE_ADDR+0xc8)
#define HAL_RF_OF_LOSE_GATE	   			(HAL_RF_OF_BASE_ADDR+0xcc)
#define HAL_RF_OF_SCL_TX_POW   			(HAL_RF_OF_BASE_ADDR+0xd0)

#define QPSK                            0x01
#define QAM16                           0x02
#define QAM64                           0x03
#define HAL_RF_OF_REG_MOD               QPSK           

#define HAL_RF_OF_REG_MAX_RAM_SIZE		(472*HAL_RF_OF_REG_MOD)
#define HAL_RF_OF_REG_MAX_RAM_WORD		(118*HAL_RF_OF_REG_MOD)

#define HAL_RF_OF_REG_TX_RAM			(HAL_RF_OF_BASE_ADDR+0x4000)
#define HAL_RF_OF_REG_RX_RAM			(HAL_RF_OF_BASE_ADDR+0x8000)
//////////////////////OFDM MODE////////////////////////////
#define HAL_RF_OF_IDLE_M				0
#define HAL_RF_OF_SEND_M				1
#define HAL_RF_OF_RECV_M				2
#define HAL_RF_OF_CCA_M					3
////////////////////OFDM STATE/////////////////////////////
#define HAL_RF_OF_IDLE_S					0
#define HAL_RF_OF_SEND_RDY_S				1
#define HAL_RF_OF_SEND_TRAIN_S				2
#define HAL_RF_OF_SEND_PROC_S				3
#define HAL_RF_OF_SEND_DATA_S				4
#define HAL_RF_OF_RECV_RDY_S				8
#define HAL_RF_OF_RECV_TRAIN_SHORT_S		9
#define HAL_RF_OF_RECV_TRAIN_LONG_S			10
#define HAL_RF_OF_RECV_DATA_S				11
#define HAL_RF_OF_RECV_CODE_S				12
#define HAL_RF_OF_CCA_S						13
////////////////////OFDM INT///////////////////////////////
#define HAL_RF_OF_DMA_RX_FIN_INT		(1u<<7)
#define HAL_RF_OF_DMA_TX_FIN_INT		(1u<<6)
#define HAL_RF_OF_RX_FIN_INT			(1u<<4)
#define HAL_RF_OF_FINE_SYNC_INT			(1u<<3)
#define HAL_RF_OF_COARSE_SYNC_INT		(1u<<2)
#define HAL_RF_OF_TX_FIN_INT			(1u<<1)
#define HAL_RF_OF_TX_START_INT			(1u<<0)
///////////////////////////////////////////////////////////

typedef struct
{
	__RO uint32_t id;
	__RO uint32_t version;
	__RW uint32_t int_mask;
	__RO uint32_t int_raw_status;
	__RO uint32_t int_mask_status;
	__WO uint32_t int_clr_status;
	__RW uint32_t rxconf;
	__RW uint32_t phy_conf;
	__RW uint32_t tx_delay;
	__RW uint32_t rx_delay;
    __RW uint32_t path_delay;
    uint32_t  RESERVED0[5];
	__RW uint32_t trc_cmd;
	uint32_t  RESERVED1[15];
	__RW uint32_t scl_fctr;
	__RW uint32_t coarse_thresh;
	__RW uint32_t fine_thresh;
	__RW uint32_t coarse_scl;	
	__RW uint32_t fine_scl;
	__RO uint32_t freq1_est;
	__RO uint32_t freq2_est;
	__RO uint32_t freq3_est;
	__RO uint32_t dc_i_offset;
	__RO uint32_t dc_q_offset;
	__RW uint32_t papr_thresh;
    __RW uint32_t ref_pow;
    __RO uint32_t cur_pow;
    __RW uint32_t agc_val;
	__RW uint32_t interf_cancel;
	__RW uint32_t interf_thresh;
	__RW uint32_t signal_pow;//0x00c0
	__RW uint32_t noise_pow;
	__RW uint32_t cca_len;
	__RW uint32_t lose_gate;
	__RW uint32_t scl_tx_pow;
}hal_rf_ofdm_t;

typedef struct
{
	__RW uint32_t spi_wdata;
	__RO uint32_t spi_rdata;
	__RW uint32_t spi_ctrl;
	uint32_t RESERVED0;
	__RW uint32_t pa_ctrl;
	__RO uint32_t pll_lock;
    __RW uint32_t lms_tx_pow;
	__RW uint32_t rf_tx_pow;
	__RW uint32_t rf_switch;
	__RW uint32_t lms_bw_cfg;
	uint32_t  RESERVED1[6];
	__RW uint32_t int_mask;
	__RO uint32_t int_raw_status;
	__RO uint32_t int_mask_status;
	__WO uint32_t int_clr;
	uint32_t  RESERVED2;
	__RW uint32_t lo_leakage_en;
	__RW uint32_t erf_calc_en;
	__RW uint32_t erf_result;
	__RW uint32_t agc_cfg;
	__RW uint32_t agc_gain;
	__RW uint32_t agc_ctrl;
	__RW uint32_t agc_cfg2;//0x006c
	__RW uint32_t tmr_var[8];
}hal_rf_misc_t;

#pragma pack(1)

#define HAL_RF_PARA_NUM			10

typedef struct
{
	uint8_t pll;
	uint8_t set1;
	uint8_t set2;
	uint8_t set3;
	uint8_t set4;
	fp64_t lo;
	uint8_t ref;
}hal_rf_freq_t;

typedef struct
{
	hal_rf_freq_t freq_cal;
	uint8_t use_level;
	uint32_t pa_power[HAL_RF_PARA_NUM];//0@1300 1@1400
	uint32_t ofdm_lms_power[HAL_RF_PARA_NUM];//0@1300 1@1400
	uint32_t ofdm_scl_power[HAL_RF_PARA_NUM];//0@1300 1@1400
	int32_t	ofdm_rssi_offset[HAL_RF_PARA_NUM];//0@1300 1@1400
    fp32_t ofdm_rssi_thred;//
}hal_rf_param_t;

#pragma pack()

#define HAL_RF_OFDM     ((hal_rf_ofdm_t *) HAL_RF_OF_BASE_ADDR)
#define HAL_RF_MISC 	((hal_rf_misc_t *) HAL_RF_MISC_BASE_ADDR)


//初始化RF
void hal_rf_init(void);
///////////////////////////////////////////////////////////////////
//OFDM系统API
//配置OFDM寄存器值
void hal_rf_of_set_reg(uint32_t addr, uint32_t value);
//获得OFDM寄存器值
uint32_t hal_rf_of_get_reg(uint32_t addr);
//复位OFDM的逻辑
void hal_rf_of_reset(void);
//使能OFDM中断
void hal_rf_of_int_enable(uint32_t int_type);
//关闭OFDM中断
void hal_rf_of_int_disable(uint32_t int_type);
//清除OFDM中断
void hal_rf_of_int_clear(uint32_t int_type);
//注册中断函数
void hal_rf_of_int_reg_handler(uint32_t int_type, fpv_t handler);
//设置DMA接收或者发送通道
bool_t hal_rf_of_set_dma_ram(uint8_t type, void *buf, uint32_t size);
//获得DMA接收或者发送通道的地址
uint8_t *hal_rf_of_get_dma_ram(uint8_t type, uint32_t *size);
//初始化AGC
bool_t hal_rf_of_agc_init(uint8_t val);
//发送数据至tx_ram
bool_t hal_rf_of_write_ram(void *buf, uint32_t size);
//接收数据从rx_ram
bool_t hal_rf_of_read_ram(void *buf, uint32_t size);
//设置RF状态
bool_t hal_rf_of_set_state(uint8_t state);
//查询RF状态
uint8_t hal_rf_of_get_state(void);
//获得OFDM状态
fp32_t hal_rf_ofdm_cal_agc(bool_t realtime);
//获得OFDM状态
uint32_t hal_rf_ofdm_cal_pow(bool_t realtime);
//获得OFDM状态
fp32_t hal_rf_ofdm_cal_rssi(bool_t realtime, bool_t *flag);
//获得OFDM状态
fp32_t hal_rf_ofdm_cal_sn(void);

///////////////////////////////////////////////////////////////////
//使能时隙中断
void hal_rf_misc_int_enable(uint32_t int_type);
//关闭时隙中断
void hal_rf_misc_int_disable(uint32_t int_type);
//清除时隙中断
void hal_rf_misc_int_clear(uint32_t int_type);
//注册中断函数
void hal_rf_misc_int_reg_handler(uint32_t int_type, fpv_t handler);
//设置定时器
void hal_rf_misc_set_timer(uint8_t index, uint32_t value);
//读取定时器
uint32_t hal_rf_misc_get_timer(uint8_t index);
//设置PA状态
void hal_rf_misc_set_pa_ctrl(uint8_t state);
//获得PA状态
uint8_t hal_rf_misc_get_pa_ctrl(void);
//设置rf发射功率
void hal_rf_misc_set_rf_tx_pow(uint16_t pow);
//设置lms发射功率
void hal_rf_misc_set_lms_tx_pow(uint32_t pow);
//设置滤波器切换
void hal_rf_misc_set_rf_switch(uint16_t sw);
//设置滤波器带宽
void hal_rf_misc_set_lms_bw_cfg(uint32_t bw);

bool_t hal_rf_misc_calib_freq(hal_rf_freq_t *freq);
//////////////////////////////////////////////////////////////////
void hal_rf_param_init(void);

hal_rf_param_t *hal_rf_param_get(void);

bool_t hal_rf_param_set(hal_rf_param_t *rf_param);

uint8_t hal_rf_param_level(fp64_t rf_lo);

#endif

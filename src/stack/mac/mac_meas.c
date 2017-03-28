#include <mac.h>
#include <nwk.h>
#include <phy.h>
#include <platform.h>

static fp32_t cca_value = 0;
static uint16_t meas_cnt = 0;

void mac_meas_cca_thred(void)
{
	uint32_t state;

	if (meas_cnt == 0)
	{
		cca_value = phy_ofdm_get_thred();
		if (cca_value == 0)
		{
			return;
		}
		//DBG_PRINTF("cca=%0.2f\r\n", cca_value);
	}
	//读取当前状态
	state = HAL_RF_OFDM->trc_cmd&0x0000000f;

	if (state == HAL_RF_OF_RECV_RDY_S
        || state == HAL_RF_OF_RECV_TRAIN_SHORT_S
        || state == HAL_RF_OF_CCA_S)
	{
		//权重
		cca_value = cca_value*0.7 + phy_ofdm_measure_thred()*0.3;
		meas_cnt++;		

		if (meas_cnt == 0xFFFF)
		{
			meas_cnt = 1;
			phy_ofdm_set_thred(cca_value+5);
            //DBG_PRINTF("cca=%0.2f\r\n", cca_value);
		}
	}    
}

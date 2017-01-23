#include <platform.h>
#include <mss_gpio.h>

#define SOCK_INIT		{MSS_GPIO_config(MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE);}
#define SOCK_0			{MSS_GPIO_set_output(MSS_GPIO_0, 0);}
#define SOCK_1			{MSS_GPIO_set_output(MSS_GPIO_0, 1);}

#define SDO_INIT		{MSS_GPIO_config(MSS_GPIO_1, MSS_GPIO_INPUT_MODE);}
#define SDO_STATE		(MSS_GPIO_get_inputs()&(1u<<MSS_GPIO_1))

#define SICK_INIT		{MSS_GPIO_config(MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE);}
#define SICK_0			{MSS_GPIO_set_output(MSS_GPIO_2, 0);}
#define SICK_1			{MSS_GPIO_set_output(MSS_GPIO_2, 1);}

#define SDI_INIT		{MSS_GPIO_config(MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE);}
#define SDI_0			{MSS_GPIO_set_output(MSS_GPIO_3, 0);}
#define SDI_1			{MSS_GPIO_set_output(MSS_GPIO_3, 1);}

#define SISTRB_INIT		{MSS_GPIO_config(MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE);}
#define SISTRB_0		{MSS_GPIO_set_output(MSS_GPIO_4, 0);}
#define SISTRB_1		{MSS_GPIO_set_output(MSS_GPIO_4, 1);}

#define SOSTRB_INIT		{MSS_GPIO_config(MSS_GPIO_5, MSS_GPIO_OUTPUT_MODE);}
#define SOSTRB_0		{MSS_GPIO_set_output(MSS_GPIO_5, 0);}
#define SOSTRB_1		{MSS_GPIO_set_output(MSS_GPIO_5, 1);}

#define AMBE_RST_INIT	{MSS_GPIO_config(MSS_GPIO_8, MSS_GPIO_OUTPUT_MODE);}
#define AMBE_RST_0		{MSS_GPIO_set_output(MSS_GPIO_8, 0);}
#define AMBE_RST_1		{MSS_GPIO_set_output(MSS_GPIO_8, 1);}

#define PTT_KEY_INIT	{MSS_GPIO_config(MSS_GPIO_10, MSS_GPIO_INPUT_MODE);}//19
#define PTT_KEY_STATE	(MSS_GPIO_get_inputs()&(1u<<MSS_GPIO_10))//19

#define DP_DET_INIT		{MSS_GPIO_config(MSS_GPIO_20, MSS_GPIO_INPUT_MODE);}
#define DP_DET_STATE	(MSS_GPIO_get_inputs()&(1u<<MSS_GPIO_20))

#define EPR_PIN_INIT	{\
						MSS_GPIO_config(MSS_GPIO_6, MSS_GPIO_INPUT_MODE|MSS_GPIO_IRQ_EDGE_POSITIVE);\
						MSS_GPIO_clear_irq(MSS_GPIO_6);\
						MSS_GPIO_enable_irq(MSS_GPIO_6);\
						}
#define EPR_PIN_STATE   (MSS_GPIO_get_inputs()&(1u<<MSS_GPIO_6))

#define DPE_PIN_INIT	{\
						MSS_GPIO_config(MSS_GPIO_7, MSS_GPIO_INPUT_MODE|MSS_GPIO_IRQ_EDGE_POSITIVE);\
						MSS_GPIO_clear_irq(MSS_GPIO_7);\
						MSS_GPIO_enable_irq(MSS_GPIO_7);\
						}
#define DPE_PIN_STATE   (MSS_GPIO_get_inputs()&(1u<<MSS_GPIO_7))

uint8_t audio_num_per_frm = 0;

fpv_t audio_epr_proc_cb = PLAT_NULL;
fpv_t audio_dpe_proc_cb = PLAT_NULL;

void hal_audio_init(fpv_t epr_cb, fpv_t dpe_cb)
{
	SOCK_INIT;
	SDO_INIT;
	SICK_INIT;
    SICK_0;
	SDI_INIT;
	SISTRB_INIT;
    SISTRB_1;
	SOSTRB_INIT;
    SOSTRB_1;
	AMBE_RST_INIT;    
    DP_DET_INIT;
    PTT_KEY_INIT;
	//写入信号和读取信号	
	//滑动，回声，激活打开	
	//采用被动串行方式	
	//采用默认采用2400bps速率0000

	if (epr_cb)
	{
		audio_epr_proc_cb = epr_cb;
		//数据准备好，可读取
		EPR_PIN_INIT;
	}

	if (dpe_cb)
	{
		audio_dpe_proc_cb = dpe_cb;
		//数据准备好，可写入
		DPE_PIN_INIT;
	}
    
    hal_audio_reset();	
}

void hal_audio_reset(void)
{
	//重启
	AMBE_RST_1;
	delay_ms(100);
	AMBE_RST_0;
	delay_ms(10);
	AMBE_RST_1;
	delay_ms(100);
}

void hal_audio_read(uint8_t *buffer)
{
	uint8_t i, j, temp;
	uint8_t *ptr = buffer;
	
	for(i=0; i<AUDIO_FRAME_SIZE/2; i++)
	{
		SOSTRB_0;
		SOCK_0;
		temp = 0;
		for (j=0; j<8; j++)
		{
			SOCK_1;
			if (SDO_STATE)	temp |= 1u<<(7-j);
			SOCK_0;
		}
		*ptr++ = temp;
		SOSTRB_1;
		temp = 0;
		for (j=0; j<8; j++)
		{
			SOCK_1;
			if (SDO_STATE)	temp |= 1u<<(7-j);
			SOCK_0;            
		}
		*ptr++ = temp;
	}
}

void hal_audio_write(uint8_t *buffer)
{
	uint8_t i, j, temp;
	uint8_t *ptr = buffer;

	for(i=0; i<AUDIO_FRAME_SIZE/2; i++)
	{
		SISTRB_0;
		SICK_0;
        temp = 0;
		SICK_1;
		temp = *ptr++;
		for (j=0; j<8; j++)
		{
			SICK_0;
			if (temp&(1u<<(7-j)))
			{
				SDI_1;
			}
			else
			{
				SDI_0;
			}
			SICK_1;
		}
		SISTRB_1;
		temp = *ptr++;
		for (j=0; j<8; j++)
		{
			SICK_0;
			if (temp&(1u<<(7-j)))
			{
				SDI_1;
			}
			else
			{
				SDI_0;
			}
			SICK_1;
		}
	}
}

bool_t hal_audio_is_write(void)
{
    return DPE_PIN_STATE;
}

bool_t hal_audio_is_read(void)
{
    return EPR_PIN_STATE;
}

void hal_audio_change_rate(uint8_t type)
{
	hal_audio_reset();
}

void hal_audio_change_gain(uint8_t level)
{
	uint8_t buffer[AUDIO_FRAME_SIZE];
	
	mem_clr(buffer, AUDIO_FRAME_SIZE);
	buffer[0] = 0x13;
	buffer[1] = 0xec;
	buffer[2] = 0x02;	//ID
	buffer[3] = 0x00;	//Control_0
	if (level == 3)
	{
		buffer[4] = 0xE5;
		buffer[5] = 0xFF;	//Control_1
	}
	else if (level == 2)
	{
		buffer[4] = 0xE5;
		buffer[5] = 0xC0;	//Control_1
	}
	else
	{
		buffer[4] = 0xE5;
		buffer[5] = 0x80;	//Control_1
	}		
	buffer[6] = 0x01;
	buffer[7] = 0xF4;	//Control_2
	hal_audio_write(buffer);

	hal_audio_write(buffer);
	hal_audio_write(buffer);
	hal_audio_write(buffer);
	hal_audio_write(buffer);
}


bool_t hal_audio_is_ptt(void)
{
	//低电平表示按下
	if (PTT_KEY_STATE == 0) return PLAT_TRUE;
	else return PLAT_FALSE;
}

void hal_audio_deinit(void)
{
	AMBE_RST_0;
	audio_epr_proc_cb = PLAT_NULL;
	audio_dpe_proc_cb = PLAT_NULL;
}

void GPIO6_IRQHandler(void)
{
	//可读中断
	MSS_GPIO_clear_irq(MSS_GPIO_6);	
   	if (audio_epr_proc_cb)
   	{
   		(audio_epr_proc_cb)();
   	}
}

void GPIO7_IRQHandler(void)
{
	//可写中断
	MSS_GPIO_clear_irq(MSS_GPIO_7);	
   	if (audio_dpe_proc_cb)
   	{
   		(audio_dpe_proc_cb)();
   	}
}

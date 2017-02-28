
#include <platform.h>
#include <test.h>
#include <mss_gpio.h>
#include <Control_IO.h>
#include <device.h>

const uint8_t slience_enforce_2400bps_voice[] = 
{
	//0000
	0x13,0xec,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBC,0xA8,0xC2,0x42,0x72,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


const uint8_t TEST_INTERFACE[] = 
"\r\n\
<***********************************************>\r\n\
Test Software Version 4.4(Burst RF) (by qhw)\r\n\
1.RF DSSS Test\r\n\
2.RF OFDM Test\r\n\
3.Misc Test\r\n\
4.Configuration\r\n\
5.Reset\r\n\
Please input NUM to select function!\r\n\
<***********************************************>\r\n";

const uint8_t RF_DSSS_INTERFACE[] = 
"\r\nNo support!\r\n";


const uint8_t RF_OFDM_INTERFACE[] = 
"\r\nRF OFDM Test\r\n";

const char_t RF_OFDM_CMD[TEST_OFDM_CMD_MAX][TSET_OFDM_SCRIPT_LEN] =
{
	"tx",
	"rx",
	"cca",
	"exit",
	"?"
};

const char_t RF_OFDM_PARA[TEST_OFDM_PARA_MAX][TSET_OFDM_SCRIPT_LEN] =
{
	"-m=",
	"-p=",
	"-l=",
	"-n=",
	"-t=",
	"-src=",
	"-dst=",
	"-v=",
	"-ac=",
	"-dma="
};

const uint8_t MISC_TEST_INTERFACE[] = 
"\r\n\
Misc Test \r\n\
1.Enter BD Test\r\n\
2.Enter LPDDR WR Test\r\n\
3.Enter AMBE Test\r\n\
4.Enter OLED Test\r\n\
5.Enter ETH Test\r\n\
6.Enter Power Test\r\n\
0.Back\r\n\
Please input NUM to select function!\r\n\
<***********************************************>\r\n";

const uint8_t CONFIGURATION_INTERFACE[] = 
"\r\n\
Configuration \r\n\
1.DSSS Register\r\n\
2.OFDM Register\r\n\
3.MISC Regsiter\r\n\
4.Read RAM\r\n\
5.Set RF Parameter\r\n\
6.Set Device Parameter\r\n\
0.Back\r\n\
Please input NUM to select function!\r\n\
<***********************************************>\r\n";

#define REFRESH_TIME_MS		100

osel_event_t *test_event_h;

static uint8_t *uart_buf;
static test_ofdm_frame_t *p_of_frm, *p_of_frm1, *p_of_frm2;
static uint16_t test_rf_timeout_id = 0;
static void test_rf_timeout_cb(void);

volatile test_cb_t test_cb;

static void config_set_register(void);
static void config_set_rf_param(void);
static void config_set_param(void);

static void test_ofdm_dma_send_cb(void);
static void test_ofdm_dma_recv_cb(void);
static void test_ofdm_send_cb(void);
static void test_ofdm_recv_cb(void);

static void test_uart_rx_cb(void);

static void test_dsss_hander(void);
static void test_ofdm_send(test_rf_ofdm_t *p_test_ofdm);
static void test_ofdm_recv(test_rf_ofdm_t *p_test_ofdm);
static void test_ofdm_cca(test_rf_ofdm_t *p_test_ofdm);
static void test_ofdm_handler(void);
static void test_misc_handler(void);
static void test_config_handler(void);
static test_rf_ofdm_t *test_ofdm_script(void);

void test_init(void)
{	
	/*创建 TEST EVENT任务 */	
	test_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(test_event_h != PLAT_NULL);
}

void test_handler(void)
{
	osel_event_wait(test_event_h, OSEL_WAIT_FOREVER);
	
	p_of_frm1 = (test_ofdm_frame_t *)heap_alloc(sizeof(test_ofdm_frame_t), PLAT_TRUE);
	DBG_ASSERT(p_of_frm1 != PLAT_NULL);
	p_of_frm2 = (test_ofdm_frame_t *)heap_alloc(sizeof(test_ofdm_frame_t), PLAT_TRUE);
	DBG_ASSERT(p_of_frm2 != PLAT_NULL);
	uart_buf = heap_alloc(TEST_UART_MAX_LEN, PLAT_TRUE);
	DBG_ASSERT(uart_buf != PLAT_NULL);	
	//初始化mcb
	mem_clr((void *)&test_cb, sizeof(test_cb_t));
			
	//使能串口
	hal_uart_rx_irq_enable(UART_DEBUG, test_uart_rx_cb);		
	//使能dma_ofdm接收和发送中断
	hal_rf_of_int_reg_handler(HAL_RF_OF_DMA_TX_FIN_INT, test_ofdm_dma_send_cb);
	hal_rf_of_int_reg_handler(HAL_RF_OF_DMA_RX_FIN_INT, test_ofdm_dma_recv_cb);
	//使能ofdm接收和发送中断
	hal_rf_of_int_reg_handler(HAL_RF_OF_TX_FIN_INT, test_ofdm_send_cb);
	hal_rf_of_int_reg_handler(HAL_RF_OF_RX_FIN_INT, test_ofdm_recv_cb);	
	////////////////////////////////////////////////
	test_cb.machine_state = STEP_LEVEL0;			

	while(1)
	{
		switch(test_cb.machine_state&0xF0)
		{
			case STEP_LEVEL0://Total Application Interface!
				DBG_PRINTF(TEST_INTERFACE);			
	        	test_cb.machine_state = STEP_LEVEL0;
				test_cb.uart_recv_date = PLAT_FALSE;
				test_cb.uart_state = UART_MENU_STATE;
				while(!test_cb.uart_recv_date);
	            break;
			case STEP_LEVEL1://RF DSSS TEST Interface!					
				DBG_PRINTF(RF_DSSS_INTERFACE);
				test_cb.machine_state = STEP_LEVEL0;
				break;
			case STEP_LEVEL2://RF OFDM TEST Interface!
				if (test_cb.rf_inited == PLAT_FALSE)
				{
					hal_rf_init();
					test_cb.rf_inited = PLAT_TRUE;
				}
				DBG_PRINTF(RF_OFDM_INTERFACE);			
	        	test_cb.machine_state = STEP_LEVEL2;					
				//OFDM测试
				test_ofdm_handler();
				break;
			case STEP_LEVEL3://Misc Test Interface!
				DBG_PRINTF(MISC_TEST_INTERFACE);
	        	test_cb.machine_state = STEP_LEVEL3;
				test_cb.uart_state = UART_MENU_STATE;
				test_cb.uart_recv_date = PLAT_FALSE;					
				while(!test_cb.uart_recv_date);
				//其他测试
				test_misc_handler();
				break;
			case STEP_LEVEL4://Test Config Parameter!
				if (test_cb.rf_inited == PLAT_FALSE)
				{
					hal_rf_init();
					test_cb.rf_inited = PLAT_TRUE;
				}
				DBG_PRINTF(CONFIGURATION_INTERFACE);	
				test_cb.machine_state = STEP_LEVEL4;
				test_cb.uart_state = UART_MENU_STATE;
				test_cb.uart_recv_date = PLAT_FALSE;					
				while(!test_cb.uart_recv_date);
				test_config_handler();
				break;
			case STEP_LEVEL5://Back to Reset!					
				hal_board_reset();
				break;				
			default:
				break;
		}
	}
}

static void test_dsss_hander(void)
{
	
}

static test_rf_ofdm_t *test_ofdm_script(void)
{
	static test_rf_ofdm_t test_ofdm;
	char_t *start_str = PLAT_NULL;
	char_t *stop_str = PLAT_NULL;
	uint32_t value;	
	uint8_t i;

	test_cb.uart_state = UART_CMD_STATE;
	test_cb.uart_recv_date = PLAT_FALSE;
	while(!test_cb.uart_recv_date);
	test_cb.uart_state = UART_MENU_STATE;
    
    mem_clr(&test_ofdm, sizeof(test_rf_ofdm_t));

	for (i=0; i<TEST_OFDM_CMD_MAX; i++)
	{
		start_str = (char_t *)uart_buf;
		start_str = strstr(start_str, RF_OFDM_CMD[i]);
		if (start_str)
		{
			test_ofdm.cmd = i;
			break;
		}		
	}

	if (start_str == PLAT_NULL) return PLAT_NULL;	
	
	for (i=0; i<TEST_OFDM_PARA_MAX; i++)
	{
		start_str = (char_t *)uart_buf;
		start_str = strstr(start_str, RF_OFDM_PARA[i]);
        if (start_str) stop_str = strstr(start_str, ";");
		if (start_str && stop_str)
		{
			start_str = start_str + strlen(RF_OFDM_PARA[i]);
			*stop_str = '\0';
			sscanf(start_str, "%d", &value);
            *stop_str = ' ';
			switch(i)
			{
				case TEST_OFDM_PARA_MODE:
					if (value<=2) test_ofdm.mode = value;
					else test_ofdm.mode = 0;
					break;
				case TEST_OFDM_PARA_DISPLAY:
					test_ofdm.frm_disp = value;
					break;
				case TEST_OFDM_PARA_LEN:
					if (value<=TEST_OFDM_FRAME_MAX_LEN && value>0) 
					{
						if (value%HAL_RF_OF_REG_MAX_RAM_SIZE)
						{
							value = value/HAL_RF_OF_REG_MAX_RAM_SIZE+1;//472字节的倍数
							value = value*HAL_RF_OF_REG_MAX_RAM_SIZE;
						}						
						test_ofdm.frm_len = value;
					}
					else 
					{
						test_ofdm.frm_len = 1888;
					}
					break;
				case TEST_OFDM_PARA_NUM:
					test_ofdm.frm_num = value;
					break;
				case TEST_OFDM_PARA_INTEV:
					test_ofdm.frm_interv_ms = value;
					break;
				case TEST_OFDM_PARA_SRC:
					test_ofdm.src_addr = value;
					break;
				case TEST_OFDM_PARA_DST:
					test_ofdm.dst_addr = value;
					break;
				case TEST_OFDM_PARA_VALUE:
					test_ofdm.value = value;
					break;
				case TEST_OFDM_PARA_CSMA:
					if (value) test_ofdm.csma_flag = PLAT_TRUE;
					else test_ofdm.csma_flag = PLAT_FALSE;
					break;
				case TEST_OFDM_PARA_DMA:
					if (value) test_ofdm.dma_flag = PLAT_TRUE;
					else test_ofdm.dma_flag = PLAT_FALSE;
					break;
			}
		}
		else
		{
			switch(i)
			{
				case TEST_OFDM_PARA_MODE:
					test_ofdm.mode = 0;
					break;
				case TEST_OFDM_PARA_DISPLAY:
					test_ofdm.frm_disp = 36;
					break;
				case TEST_OFDM_PARA_LEN:
					test_ofdm.frm_len = 1888;
					break;
				case TEST_OFDM_PARA_NUM:
					test_ofdm.frm_num = 1500;
					break;
				case TEST_OFDM_PARA_INTEV:
					test_ofdm.frm_interv_ms = 5;
					break;
				case TEST_OFDM_PARA_SRC:
					test_ofdm.src_addr = 0xff;
					break;
				case TEST_OFDM_PARA_DST:
					test_ofdm.dst_addr = 0xff;
					break;
				case TEST_OFDM_PARA_VALUE:
					test_ofdm.value = 0xa5;
					break;
				case TEST_OFDM_PARA_CSMA:
					test_ofdm.csma_flag = 0;
					break;
				case TEST_OFDM_PARA_DMA:
					test_ofdm.dma_flag = 0;
					break;
			}
		}
	}
	return &test_ofdm;
}

static void test_ofdm_send(test_rf_ofdm_t *p_test_ofdm)
{
	uint16_t i=0;	
	hal_rf_param_t *p_rf_param = hal_rf_param_get();	
	uint8_t clean_cca_cnt = 0;

	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);

	switch(p_test_ofdm->mode)
	{
		case TEST_OFDM_SEND_NORMAL:
			if (p_test_ofdm->dma_flag)
			{
				hal_rf_of_int_enable(HAL_RF_OF_DMA_TX_FIN_INT);
				hal_rf_of_int_clear(HAL_RF_OF_DMA_TX_FIN_INT);
			}
			else
			{
				hal_rf_of_int_enable(HAL_RF_OF_TX_FIN_INT);
				hal_rf_of_int_clear(HAL_RF_OF_TX_FIN_INT);
			}
			
			//初始化此帧
			p_of_frm1->head.seq_num = 0;
			p_of_frm1->head.seq_total_num = p_test_ofdm->frm_num;
			p_of_frm1->head.dst_addr = p_test_ofdm->dst_addr;
			p_of_frm1->head.src_addr = p_test_ofdm->src_addr;

			for(i=0; i<p_test_ofdm->frm_len-TEST_FRAME_HEAD_SIZE; i++)
			{
				p_of_frm1->payload[i] = p_test_ofdm->value;
			}
			
			p_of_frm1->head.frm_len = (p_test_ofdm->frm_len-1)/HAL_RF_OF_REG_MAX_RAM_SIZE;

			test_cb.rf_state = HAL_RF_OF_SEND_M;

			for (i=0; i<p_test_ofdm->frm_num && test_cb.rf_state==HAL_RF_OF_SEND_M; i++)
			{
				test_cb.rf_send_data = PLAT_TRUE;
				p_of_frm1->head.seq_num++;
				*(uint32_t *)&p_of_frm1->payload[p_test_ofdm->frm_len-TEST_FRAME_HEAD_SIZE-4]
						= crc32_tab((uint8_t *)p_of_frm1, 0, p_test_ofdm->frm_len-4);
				if (p_test_ofdm->dma_flag)
				{
					//使用dma方式
					hal_rf_of_set_dma_ram(HAL_RF_OF_SEND_M, p_of_frm1, p_test_ofdm->frm_len);
				}
				else
				{
					//放入tx_ram
					hal_rf_of_write_ram(p_of_frm1, p_test_ofdm->frm_len);
					//csma模式
					if (p_test_ofdm->csma_flag)
					{
						hal_rf_of_set_state(HAL_RF_OF_CCA_M);
						delay_us(150);
						clean_cca_cnt = 0;
                        bool_t flag;
						do
						{
							hal_rf_ofdm_cal_rssi(PLAT_TRUE, &flag);
							if (flag == PLAT_TRUE)
							{
								clean_cca_cnt++;
								if (clean_cca_cnt>10)
								{
									hal_rf_of_set_state(HAL_RF_OF_IDLE_M);
									break;
								}
							}
							else
							{
								DBG_PRINTF(".");
								clean_cca_cnt = 0;
							}
						}while(test_cb.rf_state==HAL_RF_OF_SEND_M);
					}
					hal_rf_of_set_state(HAL_RF_OF_SEND_M);
				}
				//等待传输完毕
				while(hal_rf_of_get_state() != HAL_RF_OF_IDLE_M 
					&& test_cb.rf_send_data == PLAT_TRUE
					&& test_cb.rf_state==HAL_RF_OF_SEND_M);

				DBG_PRINTF("Send:%d\r\n", p_of_frm1->head.seq_num);

				delay_ms(p_test_ofdm->frm_interv_ms);
			}
			break;
		case TEST_OFDM_SEND_CW:
			DBG_PRINTF("RF Carrier Wave Test\r\n");
			HAL_RF_MISC->rf_switch |= (1u<<2);//单载波使能
			test_cb.rf_state = HAL_RF_OF_SEND_M;
			while(test_cb.rf_state == HAL_RF_OF_SEND_M);        	
			HAL_RF_MISC->rf_switch &= ~(1u<<2);//单载波去能
			break;
		case TEST_OFDM_SEND_FULL:
			DBG_PRINTF("RF Send Full Test\r\n");
			//强制dma不打开
			p_test_ofdm->dma_flag = 0;
			hal_rf_of_int_enable(HAL_RF_OF_TX_FIN_INT);
			hal_rf_of_int_clear(HAL_RF_OF_TX_FIN_INT);
			test_cb.rf_state = HAL_RF_OF_SEND_M;
			test_cb.rf_send_data = PLAT_TRUE;
			hal_rf_of_set_state(HAL_RF_OF_SEND_M);
FULL_SEND:
			//等待传输完毕			
			while(hal_rf_of_get_state() != HAL_RF_OF_IDLE_M 
					&& test_cb.rf_send_data == PLAT_TRUE
					&& test_cb.rf_state==HAL_RF_OF_SEND_M);
			if (test_cb.rf_state==HAL_RF_OF_SEND_M)
			{
				test_cb.rf_send_data = PLAT_TRUE;
				hal_rf_of_set_state(HAL_RF_OF_SEND_M);
				goto FULL_SEND;
			}
			
			break;
		default: return;
	}	
		
	//关闭发送
	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);
	if (p_test_ofdm->dma_flag)
	{
		//关闭dma请求发送中断
		hal_rf_of_int_disable(HAL_RF_OF_DMA_TX_FIN_INT);
		//清除中断
		hal_rf_of_int_clear(HAL_RF_OF_DMA_TX_FIN_INT);
	}
	else
	{
		hal_rf_of_int_disable(HAL_RF_OF_TX_FIN_INT);
		//清除中断
		hal_rf_of_int_clear(HAL_RF_OF_TX_FIN_INT);
	}			
}

static void test_ofdm_recv(test_rf_ofdm_t *p_test_ofdm)
{
	volatile uint32_t total_frame_count = 0;
	volatile uint32_t loss_frame_count = 0;
	volatile uint32_t err_frame_count = 0;
	volatile uint32_t recv_frame_count = 0;

	volatile uint16_t seq_num = 0;
	volatile int16_t prev_seq_num = 0;

	uint16_t frame_len;
	fp32_t snr;	

	switch (p_test_ofdm->mode)
	{
		case TEST_OFDM_RECV_SIG_DEV:	DBG_PRINTF("Signal Device Test!\r\n");
			break;
		case TEST_OFDM_RECV_P2P: DBG_PRINTF("P2P Test!\r\n");
			break;
		case TEST_OFDM_RECV_DISP: DBG_PRINTF("Display!\r\n");
			break;
		default:return;
	}

	//分配一个定时器作为收不到数据时的读取当前能量
	test_rf_timeout_id = hal_timer_alloc(REFRESH_TIME_MS*1000, test_rf_timeout_cb);
	//创建超时位
	test_cb.rf_time_out = PLAT_TRUE;
	test_cb.rf_state = HAL_RF_OF_RECV_M;
	//内场第一帧
	test_cb.rf_recv_inside_first = PLAT_TRUE;
	if (p_test_ofdm->mode == 0) prev_seq_num = -1;
	else prev_seq_num = 0;
	////////////////////////////////////////////////
	if (p_test_ofdm->dma_flag)
	{
		//清除中断
		hal_rf_of_int_clear(HAL_RF_OF_DMA_RX_FIN_INT);
        //开启dma请求接收中断
		hal_rf_of_int_enable(HAL_RF_OF_DMA_RX_FIN_INT);
		//使用dma方式
		hal_rf_of_set_dma_ram(HAL_RF_OF_RECV_M, p_of_frm1, TEST_OFDM_FRAME_MAX_LEN);
	}
	else
	{
		//清除中断
		hal_rf_of_int_clear(HAL_RF_OF_RX_FIN_INT);
        //开启接收中断
		hal_rf_of_int_enable(HAL_RF_OF_RX_FIN_INT);
		//赋值一块空间
		p_of_frm = p_of_frm1;
        hal_rf_of_set_state(HAL_RF_OF_RECV_M);
	}
	
	while(test_cb.rf_state == HAL_RF_OF_RECV_M)
	{
		test_cb.rf_recv_data = PLAT_FALSE;		
		while(!test_cb.rf_recv_data);
		if (test_cb.rf_state != HAL_RF_OF_RECV_M) goto RECV_OVER;
		
		test_cb.rf_time_out = PLAT_FALSE;

		snr = hal_rf_ofdm_cal_sn();
		hal_rf_ofdm_cal_rssi(PLAT_FALSE, PLAT_NULL);
		
		switch (p_test_ofdm->mode)
		{
			case TEST_OFDM_RECV_SIG_DEV://信号源内场
				if (p_of_frm->head.seq_num>=TEST_OFDM_INSIDE_MAX_FRAME_SEQ)
				{
					goto ERROR_INSIDE_RECV;
				}
            
                if (p_of_frm->head.frm_len<=15)
                {
                    frame_len = (p_of_frm->head.frm_len+1)*HAL_RF_OF_REG_MAX_RAM_SIZE;
                }
                else
                {
                    goto ERROR_INSIDE_RECV;
                }
			
				if(*(uint32_t *)&p_of_frm->payload[frame_len-TEST_FRAME_HEAD_SIZE-4]
							== crc32_tab((uint8_t *)p_of_frm, 0, frame_len-4))
				{
					seq_num = p_of_frm->head.seq_num;
				
					if ((seq_num - prev_seq_num)==1
						|| (seq_num == 0 && prev_seq_num == TEST_OFDM_INSIDE_MAX_FRAME_SEQ-1))
                    {
                        //正确接收
                    }
                    else if (test_cb.rf_recv_inside_first == PLAT_FALSE)
                    {
                        if(seq_num > prev_seq_num)
                        {
                            loss_frame_count += seq_num - 1 - prev_seq_num;
                        }
                        else
                        {
                            loss_frame_count += seq_num + TEST_OFDM_INSIDE_MAX_FRAME_SEQ - 1 - prev_seq_num;
                        }
                    	DBG_PRINTF("LO\r\n");
                    }
					DBG_PRINTF("R%u|Sq%u|SNR%0.1f|\r\n", recv_frame_count, seq_num, snr);
					prev_seq_num = seq_num;
					recv_frame_count++;
					test_cb.rf_recv_inside_first = PLAT_FALSE;
					continue;
				}
ERROR_INSIDE_RECV:
				if (test_cb.rf_recv_inside_first == PLAT_FALSE)
				{
                	DBG_PRINTF("E-SNR%0.1f|\r\n", snr);
					err_frame_count++;
				}
				break;
			case TEST_OFDM_RECV_P2P://设备外场
				if (p_of_frm->head.frm_len<=15)
                {
                    frame_len = (p_of_frm->head.frm_len+1)*HAL_RF_OF_REG_MAX_RAM_SIZE;
                }
                else
                {
                    goto ERROR_OUTSIDE_RECV;
                }
				
				if(*(uint32_t *)&p_of_frm->payload[frame_len-TEST_FRAME_HEAD_SIZE-4]
						== crc32_tab((uint8_t *)p_of_frm, 0, frame_len-4))
				{
					//判断地址位是否符合
					if (p_test_ofdm->src_addr != 0xFF)
					{
						if (p_of_frm->head.dst_addr != p_test_ofdm->src_addr)
						{
							break;
						}
					}
					seq_num = p_of_frm->head.seq_num;
					DBG_PRINTF("Sq%u|L%u|SNR%0.1f|\r\n", seq_num, frame_len, snr);				
					if((seq_num - prev_seq_num)>1)
					{
						loss_frame_count += seq_num - prev_seq_num - 1;
						DBG_PRINTF("LO%u!\r\n", loss_frame_count);							
					}
					prev_seq_num = seq_num;
					if (prev_seq_num == p_of_frm->head.seq_total_num)
					{
						test_cb.rf_state = HAL_RF_OF_IDLE_M;
					}
					continue;
				}				
ERROR_OUTSIDE_RECV:
				err_frame_count++;
				DBG_PRINTF("E%u|SNR%0.1f|\r\n", err_frame_count, snr);
				break;
			case TEST_OFDM_RECV_DISP://接收显示
				hal_uart_send_string(UART_DEBUG, (uint8_t *)p_of_frm, p_test_ofdm->frm_disp);
				break;
		}
	}
	
RECV_OVER:
	test_rf_timeout_id = hal_timer_free(test_rf_timeout_id);
	test_cb.rf_time_out = PLAT_FALSE;
	//关闭接收
	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);
	if (p_test_ofdm->dma_flag)
	{
		//关闭dma请求发送中断
		hal_rf_of_int_disable(HAL_RF_OF_DMA_RX_FIN_INT);
		//清除中断
		hal_rf_of_int_clear(HAL_RF_OF_DMA_RX_FIN_INT);
	}
	else
	{
		hal_rf_of_int_disable(HAL_RF_OF_RX_FIN_INT);
		//清除中断
		hal_rf_of_int_clear(HAL_RF_OF_RX_FIN_INT);
	}
	
	if(loss_frame_count == 0 && err_frame_count != 0)
	{
		loss_frame_count = 0;
		total_frame_count = err_frame_count;
	}
	else
	{
        if (loss_frame_count>=err_frame_count)
        {
          	loss_frame_count = loss_frame_count - err_frame_count;
        }

		if (p_test_ofdm->mode == 0)//信号源
		{
			total_frame_count = loss_frame_count + err_frame_count + recv_frame_count;
		}
		else
		{
			total_frame_count = seq_num;
			recv_frame_count = total_frame_count - loss_frame_count - err_frame_count;
		}						
	}

	if (total_frame_count)
	{
		DBG_PRINTF("Total Frame Count=%u\r\n", total_frame_count);		
		DBG_PRINTF("Received Count=%u\r\n", recv_frame_count);	
		DBG_PRINTF("Error Frame Count=%u\r\n", err_frame_count);	
		DBG_PRINTF("Loss Frame Count=%u\r\n", loss_frame_count);		
		DBG_PRINTF("Loss Frame Rate=%.4f%%\r\n", ((fp32_t)((fp32_t)loss_frame_count/(fp32_t)total_frame_count))*100);				
	}
}

static void test_ofdm_cca(test_rf_ofdm_t *p_test_ofdm)
{
	hal_rf_of_set_state(HAL_RF_OF_CCA_M);
	//分配一个定时器作为
	test_rf_timeout_id = hal_timer_alloc(REFRESH_TIME_MS*1000, test_rf_timeout_cb);	
	test_cb.rf_state = HAL_RF_OF_CCA_M;
	
	while(test_cb.rf_state == HAL_RF_OF_CCA_M);
	
	test_rf_timeout_id = hal_timer_free(test_rf_timeout_id);

	hal_rf_of_set_state(HAL_RF_OF_IDLE_M);	
}


static void test_ofdm_handler(void)
{
	uint8_t i;
	test_rf_ofdm_t *p_test_ofdm = PLAT_NULL;

    do
    {
        p_test_ofdm = test_ofdm_script();
		if (p_test_ofdm == PLAT_NULL) DBG_PRINTF("\r\nInput Invalid CMD!\r\n");	
    }while(p_test_ofdm == PLAT_NULL);

	switch(p_test_ofdm->cmd)
	{
	case TEST_OFDM_CMD_EXIT:
		test_cb.machine_state = STEP_LEVEL0;		
		break;
	case TEST_OFDM_CMD_TX:
		DBG_PRINTF("TX Mode\r\n");		
		test_ofdm_send(p_test_ofdm);
		DBG_PRINTF("Send Over! -m=%d;-l=%d;-n=%d;-t=%d\r\n", 
					p_test_ofdm->mode,
					p_test_ofdm->frm_len,
					p_test_ofdm->frm_num,
					p_test_ofdm->frm_interv_ms);		
		break;
	case TEST_OFDM_CMD_RX:
		DBG_PRINTF("RX Mode\r\n");
		DBG_PRINTF("Receiving frame,press key 'Q' to quit!\r\n");
		test_ofdm_recv(p_test_ofdm); 
		DBG_PRINTF("Recv Over!\r\n");
		break;
	case TEST_OFDM_CMD_CCA:
		DBG_PRINTF("CCA Mode\r\n");
		DBG_PRINTF("Press key 'Q' to quit!\r\n");
		test_ofdm_cca(p_test_ofdm); 
		DBG_PRINTF("CCA Over!\r\n");
		break;
	case TEST_OFDM_CMD_HELP:
	
		DBG_PRINTF("\r\nCMD:");
		for (i=0; i<TEST_OFDM_CMD_MAX; i++)
		{			
			DBG_PRINTF("%s ", RF_OFDM_CMD[i]);
		}
		DBG_PRINTF("\r\nParameter:");
		for (i=0; i<TEST_OFDM_PARA_MAX; i++)
		{			
			DBG_PRINTF("%sxxxxx;", RF_OFDM_PARA[i]);
		}
		DBG_PRINTF("\r\n(m):0(sig&send)1(dev&cw)2(disp&full send)");
		DBG_PRINTF("\r\n(p):disp");
		DBG_PRINTF("\r\n(l):packet len");
		DBG_PRINTF("\r\n(n):packet num");
		DBG_PRINTF("\r\n(t):packet intervel");
		DBG_PRINTF("\r\n(v):packet payload");
		DBG_PRINTF("\r\n(ca):packet csma");
		DBG_PRINTF("\r\n(dma):packet dma\r\n");
	default:break;
	}	
}

void test_rf_timeout_cb(void)
{
    fp32_t rssi = 0;
	hal_rf_param_t *p_rf_param = hal_rf_param_get();
	
	test_rf_timeout_id = hal_timer_free(test_rf_timeout_id);
	test_rf_timeout_id = hal_timer_alloc(REFRESH_TIME_MS*1000, test_rf_timeout_cb);

	if (test_cb.rf_time_out == PLAT_FALSE)
	{
		test_cb.rf_time_out = PLAT_TRUE;
		return;
	}
	else
	{
		test_cb.rf_time_out = PLAT_TRUE;
	}
	
	rssi = hal_rf_ofdm_cal_rssi(PLAT_TRUE, PLAT_NULL);
	DBG_PRINTF("RS%0.1f|\r\n", rssi);
	//创建超时位
	test_cb.rf_time_out = PLAT_TRUE;
}


static void test_config_handler(void)
{
	uint32_t index, addr, value;
	
	switch (test_cb.machine_state)
	{
	case STEP_LEVEL4_1://DSSS Register
		DBG_PRINTF("Read DSSS Register!\r\n");				
		break;
	case STEP_LEVEL4_2://OFDM Register
		DBG_PRINTF("Read OFDM Register!\r\n");
		index = 0;
		for(addr = HAL_RF_OF_BASE_ADDR; addr<=HAL_RF_OF_SCL_TX_POW; addr=addr+4)
		{
			value = *(uint32_t *)addr;
			DBG_PRINTF("%d.[0x%08X]=0x%08X\r\n", index++, addr, value);					
		}
		config_set_register();		
		break;
	case STEP_LEVEL4_3://MISC Register
		DBG_PRINTF("Read MISC Register!\r\n");
		index = 0;
		for(addr = HAL_RF_MISC_BASE_ADDR; addr<=HAL_RF_MISC_AGC_CFG2; addr=addr+4)
		{
			value = *(uint32_t *)addr;
			DBG_PRINTF("%d.[0x%08X]=0x%08X\r\n", index++, addr, value);
		}		
		config_set_register();		
		break;
	case STEP_LEVEL4_4:
		index = 0;
		DBG_PRINTF("11111111");

		for(addr = HAL_RF_MISC_BASE_ADDR+0x4000; addr<=HAL_RF_MISC_BASE_ADDR+0x7fff; addr=addr+4)
		{
			value = *(uint32_t *)addr;
			hal_uart_send_string(UART_DEBUG, (uint8_t *)&value, sizeof(uint32_t));
		}

		DBG_PRINTF("00000000");

		for(addr = HAL_RF_MISC_BASE_ADDR+0x8000; addr<=HAL_RF_MISC_BASE_ADDR+0xbfff; addr=addr+4)
		{
			value = *(uint32_t *)addr;
			hal_uart_send_string(UART_DEBUG, (uint8_t *)&value, sizeof(uint32_t));
		}
		test_cb.uart_state = UART_MENU_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		while(!test_cb.uart_recv_date);
	case STEP_LEVEL4_5:
		hal_uart_printf(UART_DEBUG, "Set RF Param!\r\n");
		config_set_rf_param();
		break;
	case STEP_LEVEL4_6:
		hal_uart_printf(UART_DEBUG, "Set Param!\r\n");
		config_set_param();
		break;
	default:break;
	}//case STEP_LEVEL4  end
}

static void test_lpddr_handler(void)
{
	volatile uint32_t start_addr = 0xa0000000;
	
	DBG_PRINTF("32bits ddr write\r\n");	
	for (uint32_t loop=0; loop<DDR_32BIT_SIZE; loop++)
	{
		*(((uint32_t *)start_addr)+loop) = 0x12345678;
	}	
	DBG_PRINTF("32bits ddr read\r\n");	
	for (uint32_t loop=0; loop<DDR_32BIT_SIZE; loop++)
	{
		if (*(((uint32_t *)start_addr)+loop) != 0x12345678)
		{
		  	DBG_PRINTF("err\r\n");
		}
	}	
	DBG_PRINTF("16bits ddr write\r\n");
	for (uint32_t loop=0; loop<DDR_16BIT_SIZE; loop++)
	{
		*(((uint16_t *)start_addr)+loop) = 0xabcd;
	}
	DBG_PRINTF("16bits ddr read\r\n");
	for (uint32_t loop=0; loop<DDR_16BIT_SIZE; loop++)
	{
		if (*(((uint16_t *)start_addr)+loop) != 0xabcd)
		{
		  	DBG_PRINTF("err\r\n");
		}
	}
	DBG_PRINTF("8bits ddr write\r\n");	
	for (uint32_t loop=0; loop<DDR_8BIT_SIZE; loop++)
	{
		*(((uint8_t *)start_addr)+loop) = 0xAA;
	}
	DBG_PRINTF("8bits ddr read\r\n");
	for (uint32_t loop=0; loop<DDR_16BIT_SIZE; loop++)
	{
		if (*(((uint8_t *)start_addr)+loop) != 0xAA)
		{
		  	DBG_PRINTF("err\r\n");
		}
	}	
}

extern void netio_init(void);//@netio.c

static void test_eth_handler(void)
{
	DBG_PRINTF("ping ip or win32-i386 -t ip\r\n");
	netio_init();
}

static void test_power_handler(void)
{	
	
}

static void bd_print(void)
{
	uint8_t buffer[8], len;

	len = hal_uart_read(UART_BD, buffer, 8);

	hal_uart_send_string(UART_DEBUG, buffer, len);
}

////////////////////Config//////////////////////////////////
static void config_set_register(void)
{
	uint32_t addr, value;
	char_t *str_start = PLAT_NULL;
	char_t *str_stop = PLAT_NULL;
	
READ_SCRIPT:
	DBG_PRINTF("Input Register\r\n");
	DBG_PRINTF("Example:[0x31000000]=0x1234;\r\n");

	mem_set(uart_buf, 0, sizeof(uart_buf));
	test_cb.uart_recv_date = PLAT_FALSE;
	while(!test_cb.uart_recv_date);
	str_start = (char_t *)uart_buf;
	str_stop = (char_t *)uart_buf;

	str_start = strstr((char_t *)str_start, "exit");
	if (str_start) return;
    else str_start = (char_t *)uart_buf;
    
	while(1)
	{			
		str_start = strstr((char_t *)str_start, "[0x3");
		str_stop = strstr((char_t *)str_start, "]=");
		if (str_start && str_stop)
		{
			addr = 0;

			str_start = str_start + strlen("[0x");
			*str_stop = '\0';			
			sscanf(str_start, "%x", &addr);
            *str_stop = ' ';
			str_start = str_stop;
		}
		else
		{
			goto READ_SCRIPT;
		}

		str_start = strstr((char_t *)str_start, "=0x");
		str_stop = strstr((char_t *)str_start, ";");

		if (str_start && str_stop)
		{
			value = 0;
			str_start = str_start + strlen("=0x");
			*str_stop = '\0';
			sscanf(str_start, "%x", &value);
            *str_stop = ' ';
			str_start = str_stop;
		}
		else
		{
			goto READ_SCRIPT;
		}

		*(uint32_t *)addr = value;
		DBG_PRINTF("\r\nSet OK-[0x%x]=0x%x!", addr, value);
	}
}

static void config_set_rf_param(void)
{
	char_t *str = PLAT_NULL;
	fp64_t freq_lo = 0;
    fp32_t thred = 0;
	uint32_t value = 0;
	int32_t rssi_offset = 0;
	bool_t update_flag = PLAT_FALSE;
	uint8_t level;
	hal_rf_param_t *p_rf_param = hal_rf_param_get();

	hal_rf_param_init();
	
	DBG_PRINTF("Input Parameter(rf_lo,rf_ofdm_lms_power,rf_pa_power\r\n");
	DBG_PRINTF("rf_ofdm_scl_power,rf_ofdm_rssi_offset,rf_ofdm_rssi_thred,rf_default)\r\n");
	DBG_PRINTF("Example:rf_lo=XXXX.XXXXXX(Unit:MHz)\r\n");	
	test_cb.uart_recv_date = PLAT_FALSE;
	while(!test_cb.uart_recv_date);
	/////////设置RF本振///////////////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "rf_lo=");
	if (str)
	{
		DBG_PRINTF("\r\n");
		str = str + strlen("rf_lo=");
		sscanf(str, "%lf", &freq_lo);
		level = hal_rf_param_level(freq_lo);
		if (level != 0xFF)
	    {
	        p_rf_param->freq_cal.lo = freq_lo;
			hal_rf_misc_calib_freq(&p_rf_param->freq_cal);
			update_flag = PLAT_TRUE;
			
	        DBG_PRINTF("Set Freq OK!=[%lf]\r\n", freq_lo);			
			test_cb.rf_inited = PLAT_FALSE;
	    }
	    else
	    {
	        DBG_PRINTF("Set Freq Range Invalid\r\n");
	    }
	}
	
	p_rf_param = hal_rf_param_get();
	
	////////设置pa_power//////////////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "rf_pa_power=");
	if (str)
	{
		str = str + strlen("rf_pa_power=");
        DBG_PRINTF("\r\n");
		sscanf(str, "%x", &value);
		p_rf_param->pa_power[p_rf_param->use_level] = value;
		//rf功放值
    	hal_rf_misc_set_rf_tx_pow(p_rf_param->pa_power[p_rf_param->use_level]);		
        update_flag = PLAT_TRUE;
		DBG_PRINTF("Set RF PA Power OK!=[0x%x]\r\n", p_rf_param->pa_power[p_rf_param->use_level]);
	}
	////////设置rf_ofdm_lms_power//////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "rf_ofdm_lms_power=");
	if (str)
	{
		str = str + strlen("rf_ofdm_lms_power=");
        DBG_PRINTF("\r\n");
		sscanf(str, "%x", &value);
		p_rf_param->ofdm_lms_power[p_rf_param->use_level] = value;		
		//lms功放值
		hal_rf_misc_set_lms_tx_pow(p_rf_param->ofdm_lms_power[p_rf_param->use_level]<<16|0);
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set OFDM LMS Power OK!=[0x%x]\r\n", p_rf_param->ofdm_lms_power[p_rf_param->use_level]);
	}
	////////设置rf_ofdm_rssi_offset////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "rf_ofdm_rssi_offset=");
	if (str)
	{
		str = str + strlen("rf_ofdm_rssi_offset=");
		DBG_PRINTF("\r\n");
		sscanf(str, "%d", &rssi_offset);
		p_rf_param->ofdm_rssi_offset[p_rf_param->use_level] = rssi_offset;
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set OFDM RSSI Offset OK!=[%d]\r\n",p_rf_param->ofdm_rssi_offset[p_rf_param->use_level]);
	}
	////////设置rf_ofdm_scl_pow////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "rf_ofdm_scl_power=");
	if (str)
	{
		str = str + strlen("rf_ofdm_scl_power=");
		DBG_PRINTF("\r\n");
		sscanf(str, "%x", &value);
		p_rf_param->ofdm_scl_power[p_rf_param->use_level] = value;
		hal_rf_of_set_reg(HAL_RF_OF_SCL_TX_POW, p_rf_param->ofdm_scl_power[p_rf_param->use_level]);
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set OFDM SCL Power OK!=[0x%x]\r\n", p_rf_param->ofdm_scl_power[p_rf_param->use_level]);
	}
	////////设置rf_ofdm_rssi_thred////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "rf_ofdm_rssi_thred=");
	if (str)
	{
		str = str + strlen("rf_ofdm_rssi_thred=");
		DBG_PRINTF("\r\n");
		sscanf(str, "%f", &thred);
		p_rf_param->ofdm_rssi_thred = thred;		
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set OFDM RSSI THRED = [%0.1f]\r\n", p_rf_param->ofdm_rssi_thred);
	}
	////////设置rf_default////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "rf_default");
	if (str)
	{
		p_rf_param->freq_cal.pll = 0xff;
		hal_rf_param_set(p_rf_param);
		hal_rf_param_init();
		DBG_PRINTF("Set RF Param Default OK!\r\n");
	}
	//////////////////////////////////////////
	if (update_flag == PLAT_TRUE)
	{		
		hal_rf_param_set(p_rf_param);
		DBG_PRINTF("Save parameter!\r\n");
	}
}

static void config_set_param()
{
	char_t *str = PLAT_NULL;
	char_t *stop_str = PLAT_NULL;
	uint32_t value = 0;
	bool_t update_flag = PLAT_FALSE;	
	device_info_t *p_device_info = device_info_get(PLAT_FALSE);

	device_info_init();
	
	DBG_PRINTF("Input Parameter(type,mode,mesh_id,dev_id,local_mac,ip,gateway,netmask,remote_mac)\r\n");
	DBG_PRINTF("Example:type=XX(Hex or Dec)\r\n");	
	test_cb.uart_recv_date = PLAT_FALSE;
	while(!test_cb.uart_recv_date);
	/////////设置type///////////////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "type=");
	if (str)
	{
		DBG_PRINTF("\r\n");
		str = str + strlen("type=");
		sscanf(str, "%d", &value);
	    SET_TYPE_ID(p_device_info->id, value);
		update_flag = PLAT_TRUE;
        DBG_PRINTF("Set Type OK!=[%d]\r\n", value);		
	}	
	////////设置mode//////////////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "mode=");
	if (str)
	{
		str = str + strlen("mode=");
        DBG_PRINTF("\r\n");
		sscanf(str, "%d", &value);
		SET_MODE_ID(p_device_info->id, value);
        update_flag = PLAT_TRUE;
		DBG_PRINTF("Set Mode OK!=[%d]\r\n", value);
	}
	////////设置mesh_id//////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "mesh_id=");
	if (str)
	{
		str = str + strlen("mesh_id=");
        DBG_PRINTF("\r\n");
		sscanf(str, "%x", &value);
		SET_MESH_ID(p_device_info->id, value);
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set Mesh ID OK!=[%d]\r\n", value);
	}
	////////设置dev_id//////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "dev_id=");
	if (str)
	{
		str = str + strlen("dev_id=");
        DBG_PRINTF("\r\n");
		sscanf(str, "%x", &value);
		SET_DEV_ID(p_device_info->id, value);
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set Dev ID OK!=[%d]\r\n", value);
	}
	////////设置local_mac//////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "local_mac=");
	if (str)
	{
		str = str + strlen("local_mac=");
		DBG_PRINTF("\r\n");
		
		for (uint8_t index=0; index<6; index++)
		{
			stop_str = strstr((char_t *)str, ":");
			if (stop_str) stop_str[0] = '\0';
			sscanf(str, "%x", &value);
			p_device_info->local_eth_mac_addr[index] = value;
			str = &stop_str[1];
		}				
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set Mac OK!=%x:%x:%x:%x:%x:%x\r\n", 
				p_device_info->local_eth_mac_addr[0],
				p_device_info->local_eth_mac_addr[1],
				p_device_info->local_eth_mac_addr[2],
				p_device_info->local_eth_mac_addr[3],
				p_device_info->local_eth_mac_addr[4],
				p_device_info->local_eth_mac_addr[5]);
	}
	////////设置ip//////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "ip=");
	if (str)
	{
		str = str + strlen("ip=");
		DBG_PRINTF("\r\n");
		
		for (uint8_t index=0; index<4; index++)
		{
			stop_str = strstr((char_t *)str, ":");
			if (stop_str) stop_str[0] = '\0';
			sscanf(str, "%d", &value);
			p_device_info->local_ip_addr[index] = value;
			str = &stop_str[1];
		}				
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set IP OK!=%d:%d:%d:%d\r\n", 
				p_device_info->local_ip_addr[0],
				p_device_info->local_ip_addr[1],
				p_device_info->local_ip_addr[2],
				p_device_info->local_ip_addr[3]);
	}
	////////设置gateway//////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "gateway=");
	if (str)
	{
		str = str + strlen("gateway=");
		DBG_PRINTF("\r\n");
		
		for (uint8_t index=0; index<4; index++)
		{
			stop_str = strstr((char_t *)str, ":");
			if (stop_str) stop_str[0] = '\0';
			sscanf(str, "%d", &value);
			p_device_info->local_gateway_addr[index] = value;
			str = &stop_str[1];
		}				
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set Gateway OK!=%d:%d:%d:%d\r\n", 
				p_device_info->local_gateway_addr[0],
				p_device_info->local_gateway_addr[1],
				p_device_info->local_gateway_addr[2],
				p_device_info->local_gateway_addr[3]);
	}
	////////设置netmask//////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "netmask=");
	if (str)
	{
		str = str + strlen("netmask=");
		DBG_PRINTF("\r\n");
		
		for (uint8_t index=0; index<4; index++)
		{
			stop_str = strstr((char_t *)str, ":");
			if (stop_str) stop_str[0] = '\0';
			sscanf(str, "%d", &value);
			p_device_info->local_netmask_addr[index] = value;
			str = &stop_str[1];
		}				
		update_flag = PLAT_TRUE;
		DBG_PRINTF("Set Netmask OK!=%d:%d:%d:%d\r\n", 
				p_device_info->local_netmask_addr[0],
				p_device_info->local_netmask_addr[1],
				p_device_info->local_netmask_addr[2],
				p_device_info->local_netmask_addr[3]);
	}
	////////设置remote mac//////////////
	str = (char_t *)uart_buf;
	str = strstr((char_t *)str, "remote_mac=");
	if (str)
	{
		str = str + strlen("remote_mac=");
		DBG_PRINTF("\r\n");
		
		for (uint8_t index=0; index<6; index++)
		{
			stop_str = strstr((char_t *)str, ":");
			if (stop_str) stop_str[0] = '\0';
			sscanf(str, "%x", &value);
			p_device_info->remote_eth_mac_addr[index] = value;
			str = &stop_str[1];
		}				
		update_flag = PLAT_TRUE;
		
		DBG_PRINTF("Set Remote MAC OK!=%x:%x:%x:%x:%x:%x\r\n", 
				p_device_info->remote_eth_mac_addr[0],
				p_device_info->remote_eth_mac_addr[1],
				p_device_info->remote_eth_mac_addr[2],
				p_device_info->remote_eth_mac_addr[3],
				p_device_info->remote_eth_mac_addr[4],
				p_device_info->remote_eth_mac_addr[5]);
	}
	//////////////////////////////////////////
	if (update_flag == PLAT_TRUE)
	{		
		device_info_set(p_device_info, PLAT_TRUE);
		DBG_PRINTF("Save parameter!\r\n");
	}
}

static void test_misc_handler(void)
{
	switch(test_cb.machine_state)
	{
	case STEP_LEVEL3_1://BD
		DBG_PRINTF("BD Test\r\n");
		test_cb.uart_state = UART_MENU_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		hal_uart_rx_irq_enable(UART_BD, bd_print);
		while(!test_cb.uart_recv_date);		
		hal_uart_rx_irq_disable(UART_BD);		
		break;
	case STEP_LEVEL3_2://DDR
		DBG_PRINTF("64MB LPDDR Test\r\n");		
		test_cb.uart_state = UART_MENU_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_lpddr_handler();
		DBG_PRINTF("LPDDR Test Over\r\n");
		break;
	case STEP_LEVEL3_3://AMBE
		DBG_PRINTF("AMBE Test\r\n");		
		test_cb.uart_state = UART_MENU_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		//test_ambe_handler();		
		while(!test_cb.uart_recv_date);
		DBG_PRINTF("AMBE Test Over\r\n");		
		break;
	case STEP_LEVEL3_4://LCD
		DBG_PRINTF("OLED Test\r\n");		
		test_cb.uart_state = UART_MENU_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		//test_oled_handler();
		DBG_PRINTF("OLED Test Over\r\n");
		break;		
	case STEP_LEVEL3_5://ETH 
		hal_uart_printf(UART_DEBUG, "ETH Test\r\n");		
		test_cb.uart_state = UART_MENU_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_eth_handler();
		while(!test_cb.uart_recv_date)
		{
			osel_systick_delay(5000);
		}
		DBG_PRINTF("ETH Test Over\r\n");
		break;
	case STEP_LEVEL3_6://Power
		hal_uart_printf(UART_DEBUG, "Power Test\r\n");		
		test_cb.uart_state = UART_MENU_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		//test_power_handler();		
		while(!test_cb.uart_recv_date);
		DBG_PRINTF("Power Test Over\r\n");
		break;	
	}
}

static void test_ofdm_dma_send_cb(void)
{
	test_cb.rf_send_data = PLAT_FALSE;
}

static void test_ofdm_dma_recv_cb(void)
{
	uint32_t size;

	p_of_frm = (test_ofdm_frame_t *)hal_rf_of_get_dma_ram(HAL_RF_OF_RECV_M, &size);

	test_cb.rf_recv_data = PLAT_TRUE;	
    
    if ((uint32_t)p_of_frm == (uint32_t)p_of_frm1)
    {
        hal_rf_of_set_dma_ram(HAL_RF_OF_RECV_M, p_of_frm2, TEST_OFDM_FRAME_MAX_LEN);				
    }
    else
    {
        hal_rf_of_set_dma_ram(HAL_RF_OF_RECV_M, p_of_frm1, TEST_OFDM_FRAME_MAX_LEN);				
    }
}

static void test_ofdm_send_cb(void)
{
	test_cb.rf_send_data = PLAT_FALSE;
}

static void test_ofdm_recv_cb(void)
{
	test_cb.rf_recv_data = PLAT_TRUE;	

	hal_rf_of_read_ram(p_of_frm, 4);
    
    p_of_frm->head.frm_len += 1;
    
	if (p_of_frm->head.frm_len<=16)
	{
		hal_rf_of_read_ram(p_of_frm, p_of_frm->head.frm_len*HAL_RF_OF_REG_MAX_RAM_SIZE);
	}
}


static void test_uart_rx_cb(void)
{	
	static uint16_t i = 0;
	uint8_t temp;

	while(hal_uart_read(UART_DEBUG, &temp, 1))
	{
		if(test_cb.uart_state == UART_MENU_STATE)
		{
			switch(temp)
			{
				case '0':
					test_cb.machine_state = STEP_LEVEL0;
					break;
				case '1':
				  	if(test_cb.machine_state == STEP_LEVEL0)
					{
					  	test_cb.machine_state = STEP_LEVEL1;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_1;
					}
					else if(test_cb.machine_state == STEP_LEVEL4)
					{
						test_cb.machine_state = STEP_LEVEL4_1;
					}
					break;
				case '2':
					if(test_cb.machine_state == STEP_LEVEL0)
					{
						test_cb.machine_state = STEP_LEVEL2;
					}					
					else if (test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_2;
					}
					else if(test_cb.machine_state == STEP_LEVEL4)
					{
						test_cb.machine_state = STEP_LEVEL4_2;
					}
					break;
				case '3':
					if(test_cb.machine_state == STEP_LEVEL0)
					{
						test_cb.machine_state = STEP_LEVEL3;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_3;
					}
					else if(test_cb.machine_state == STEP_LEVEL4)
					{
						test_cb.machine_state = STEP_LEVEL4_3;
					}
					break;
				case '4':
					if(test_cb.machine_state == STEP_LEVEL0)
					{
						test_cb.machine_state = STEP_LEVEL4;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_4;
					}
					else if(test_cb.machine_state == STEP_LEVEL4)
					{
						test_cb.machine_state = STEP_LEVEL4_4;
					}
					break;
				case '5':
					if(test_cb.machine_state == STEP_LEVEL0)
					{
						test_cb.machine_state = STEP_LEVEL5;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_5;
					}
					else if(test_cb.machine_state == STEP_LEVEL4)
					{
						test_cb.machine_state = STEP_LEVEL4_5;
					}
					break;
				case '6':
					if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_6;
					}
					else if(test_cb.machine_state == STEP_LEVEL4)
					{
						test_cb.machine_state = STEP_LEVEL4_6;
					}
					break;
				case 'q':
				case 'Q':					
					test_cb.rf_state = HAL_RF_OF_IDLE_M;
					test_cb.rf_recv_data = PLAT_TRUE;
					test_cb.rf_send_data = PLAT_FALSE;
					break;
				default:
                    test_cb.uart_recv_date = PLAT_TRUE;
					return;
			}
			test_cb.uart_recv_date = PLAT_TRUE;
			//默认转命令行输入方式
			test_cb.uart_state = UART_CMD_STATE;
		}
		else if (test_cb.uart_state == UART_CMD_STATE)
		{
			if (temp == '\b' && i)
			{
				i--;
			}
			else
			{
				uart_buf[i++]=temp;
			}
			
			hal_uart_send_char(UART_DEBUG, temp);

			if(uart_buf[i-1]=='\r'||(i>TEST_UART_MAX_LEN))
			{					
				test_cb.uart_recv_date = PLAT_TRUE;
				uart_buf[i]='\0';
				i=0;
			}
		}
	}
}
